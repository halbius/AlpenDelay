// ============================================================================
// DelayCtrl_Parameters.cpp
// ============================================================================

// SDK-interne Warnung (uninitialisiertes Member in AAX_CBinaryDisplayDelegate)
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 26495)
#endif
#include "AAX_CBinaryDisplayDelegate.h"
#ifdef _MSC_VER
#  pragma warning(pop)
#endif

#include "DelayCtrl_Parameters.h"
#include "DelayCtrl_Defs.h"
#include "DelayCtrl_Alg.h"

#include "AAX_CBinaryTaperDelegate.h"
#include "AAX_CLinearTaperDelegate.h"
#include "AAX_Assert.h"
#include "AAX_IController.h"

#include <memory>
#include <string>
#include <cstring>
#include <cstdio>

AAX_CEffectParameters* AAX_CALLBACK DelayCtrl_Parameters::Create()
{
    return new DelayCtrl_Parameters();
}

DelayCtrl_Parameters::DelayCtrl_Parameters()
    : AAX_CEffectParameters()
    , mFormat(nullptr)
    , mNumChannels(1)
    , mSampleRate(48000.f)
    , mUnitIsMs(false)
    , mUnitChanged(false)
{}

DelayCtrl_Parameters::~DelayCtrl_Parameters() {}

// ============================================================================
// Ein Delay-Drehgeber (kontinuierlich, float-Samples intern)
// ============================================================================
void DelayCtrl_Parameters::AddDelayParam(const char* paramID, const char* name,
                                          const char* shortName1, const char* shortName2)
{
    std::unique_ptr<AAX_IParameter> p(
        new AAX_CParameter<float>(
            paramID,
            AAX_CString(name),
            static_cast<float>(kDelayCtrl_DefaultDelay),
            AAX_CLinearTaperDelegate<float>(
                static_cast<float>(kDelayCtrl_MinDelaySamples),
                static_cast<float>(kDelayCtrl_MaxDelaySamples)),
            DelayDisplayDelegate(&mUnitIsMs, &mSampleRate),
            true));
    p->SetNumberOfSteps(kDelayCtrl_NumSteps);
    p->SetType(AAX_eParameterType_Continuous);   // Drehgeber
    if (shortName1) p->AddShortenedName(AAX_CString(shortName1));
    if (shortName2) p->AddShortenedName(AAX_CString(shortName2));
    mParameterManager.AddParameter(p.release());

    mPacketDispatcher.RegisterPacket(
        paramID, eAlgPortID_CoefsIn,
        this, &DelayCtrl_Parameters::UpdateCoefs);
}

// ============================================================================
// EffectInit
// ============================================================================
AAX_Result DelayCtrl_Parameters::EffectInit()
{
    AAX_EStemFormat stem = AAX_eStemFormat_None;
    Controller()->GetInputStemFormat(&stem);
    mFormat      = DelayCtrl_FindFormat(stem);
    mNumChannels = mFormat ? mFormat->numChannels : 1;
    RefreshSampleRate();

    // Master Bypass
    {
        std::unique_ptr<AAX_IParameter> p(new AAX_CParameter<bool>(
            cDefaultMasterBypassID, AAX_CString("Master Bypass"), false,
            AAX_CBinaryTaperDelegate<bool>(),
            AAX_CBinaryDisplayDelegate<bool>("bypass", "on"), true));
        p->SetNumberOfSteps(2);
        p->SetType(AAX_eParameterType_Discrete);
        mParameterManager.AddParameter(p.release());
        mPacketDispatcher.RegisterPacket(cDefaultMasterBypassID, eAlgPortID_BypassIn);
    }

    // Einheit Samples / ms (nur Anzeige, kein DSP-Packet)
    {
        std::unique_ptr<AAX_IParameter> p(new AAX_CParameter<bool>(
            kDelayCtrl_UnitModeID, AAX_CString("Unit"), false,
            AAX_CBinaryTaperDelegate<bool>(),
            AAX_CBinaryDisplayDelegate<bool>("Smpl", "ms"), true));
        p->SetNumberOfSteps(2);
        p->SetType(AAX_eParameterType_Discrete);
        mParameterManager.AddParameter(p.release());
    }

    // Master
    AddDelayParam(kDelayCtrl_MasterDelayID, "Master Delay", "Master", "Mst");

    // Kanal-Delays nur für Mehrkanalformate, benannt nach Kanalrolle
    if (mFormat && mNumChannels > 1)
    {
        for (int ch = 0; ch < mNumChannels; ++ch)
        {
            const int   role  = mFormat->roles[ch];
            const char* label = DelayCtrl_RoleLabel(role);
            const std::string full = std::string("Delay ") + label;
            const std::string mid  = std::string("Dly ")   + label;
            const std::string tiny = std::string("D ")     + label;
            AddDelayParam(DelayCtrl_RoleParamID(role), full.c_str(),
                          mid.c_str(), tiny.c_str());
        }
    }
    return AAX_SUCCESS;
}

// ============================================================================
// Umschalten Samples/ms sofort erkennen
// ============================================================================
AAX_Result DelayCtrl_Parameters::UpdateParameterNormalizedValue(
    AAX_CParamID iParameterID, double iValue, AAX_EUpdateSource iSource)
{
    const AAX_Result result =
        AAX_CEffectParameters::UpdateParameterNormalizedValue(iParameterID, iValue, iSource);

    if (iParameterID && std::strcmp(iParameterID, kDelayCtrl_UnitModeID) == 0)
    {
        mUnitIsMs    = (iValue > 0.5);   // Delegates sehen den neuen Wert sofort
        mUnitChanged = true;             // Neuabfrage der Anzeigen erfolgt im Timer
    }
    return result;
}

AAX_Result DelayCtrl_Parameters::TimerWakeup()
{
    if (mUnitChanged)
    {
        mUnitChanged = false;
        RepostDelayValues();
    }
    return AAX_CEffectParameters::TimerWakeup();
}

// Postet den unveränderten Wert jedes Delay-Parameters erneut an den Host.
// Bewusst OHNE Touch/Release, damit keine Automation geschrieben wird.
void DelayCtrl_Parameters::RepostDelayValues()
{
    auto repost = [this](const char* id)
    {
        AAX_IParameter* p = mParameterManager.GetParameterByID(id);
        if (p) p->SetNormalizedValue(p->GetNormalizedValue());
    };
    repost(kDelayCtrl_MasterDelayID);
    if (mFormat && mNumChannels > 1)
        for (int ch = 0; ch < mNumChannels; ++ch)
            repost(DelayCtrl_RoleParamID(mFormat->roles[ch]));
}

// ============================================================================
// Koeffizienten für den DSP
// ============================================================================
AAX_Result DelayCtrl_Parameters::UpdateCoefs(AAX_CPacket& ioPacket)
{
    SDelayCtrl_Coefs& coefs = *ioPacket.GetPtr<SDelayCtrl_Coefs>();
    coefs.numChannels = mNumChannels;

    float masterDelay = 0.f;
    if (AAX_IParameter* p = mParameterManager.GetParameterByID(kDelayCtrl_MasterDelayID))
        p->GetValueAsFloat(&masterDelay);

    for (int ch = 0; ch < kDelayCtrl_MaxChannels; ++ch)
    {
        float chDelay = 0.f;
        if (mFormat && mNumChannels > 1 && ch < mNumChannels)
        {
            const char* id = DelayCtrl_RoleParamID(mFormat->roles[ch]);
            if (AAX_IParameter* p = mParameterManager.GetParameterByID(id))
                p->GetValueAsFloat(&chDelay);
        }
        float total = masterDelay + chDelay;
        if (total < 0.f) total = 0.f;
        if (total > static_cast<float>(kDelayCtrl_MaxDelaySamples))
            total = static_cast<float>(kDelayCtrl_MaxDelaySamples);
        coefs.chDelaySamples[ch] = total;
    }
    return AAX_SUCCESS;
}

void DelayCtrl_Parameters::RefreshSampleRate()
{
    AAX_CSampleRate sr = 0.0;
    if (AAX_SUCCESS == Controller()->GetSampleRate(&sr) && sr > 0.0)
        mSampleRate = static_cast<float>(sr);
}
