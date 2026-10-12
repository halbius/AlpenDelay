// ============================================================================
// DelayCtrl_Parameters.cpp
// ============================================================================

// Suppress SDK-internal uninitialised-member warning in AAX_CBinaryDisplayDelegate
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
#include <cmath>
#include <cstdio>

#ifdef _DEBUG
#  ifdef _WIN32
#    include <windows.h>
#    define DBG(msg) OutputDebugStringA("[AlpenDelay] " msg "\n")
#  else
#    include <stdio.h>
#    define DBG(msg) fprintf(stderr, "[AlpenDelay] " msg "\n")
#  endif
#else
#  define DBG(msg)
#endif

// ============================================================================
AAX_CEffectParameters* AAX_CALLBACK DelayCtrl_Parameters::Create()
{
    return new DelayCtrl_Parameters();
}

DelayCtrl_Parameters::DelayCtrl_Parameters()
    : AAX_CEffectParameters()
    , mStemFormat(AAX_eStemFormat_Mono)
    , mNumChannels(1)
    , mSampleRate(48000.f)
    , mUnitIsMs(false)
    , mLastUnitIsMs(false)
{}

DelayCtrl_Parameters::~DelayCtrl_Parameters() {}

// ============================================================================
// AddDelayParam
// ============================================================================
void DelayCtrl_Parameters::AddDelayParam(const char* paramID,
                                          const char* name,
                                          bool        registerPacket)
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
    p->SetType(AAX_eParameterType_Continuous);  // rotary knob
    mParameterManager.AddParameter(p.release());

    if (registerPacket)
        mPacketDispatcher.RegisterPacket(
            paramID, eAlgPortID_CoefsIn,
            this, &DelayCtrl_Parameters::UpdateCoefs);
}

// ============================================================================
// EffectInit
// ============================================================================
AAX_Result DelayCtrl_Parameters::EffectInit()
{
    DBG("EffectInit");

    Controller()->GetInputStemFormat(&mStemFormat);
    mNumChannels = DelayCtrl_ChannelCount(mStemFormat);
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

    // Unit Mode toggle (Samples / ms)
    {
        std::unique_ptr<AAX_IParameter> p(new AAX_CParameter<bool>(
            kDelayCtrl_UnitModeID, AAX_CString("Unit"), false,
            AAX_CBinaryTaperDelegate<bool>(),
            AAX_CBinaryDisplayDelegate<bool>("Smpl", "ms"), true));
        p->SetNumberOfSteps(2);
        p->SetType(AAX_eParameterType_Discrete);
        mParameterManager.AddParameter(p.release());
        // No DSP packet – display only
    }

    // Master Delay knob
    AddDelayParam(kDelayCtrl_MasterDelayID, "Master Delay", true);

    // Per-channel delay knobs – only for channels that actually exist.
    // Mono uses only Master (no redundant per-channel knob).
    if (mNumChannels > 1)
    {
        for (int ch = 0; ch < mNumChannels; ++ch)
        {
            char name[32];
            snprintf(name, sizeof(name), "Dly %s", DelayCtrl_ChName(mStemFormat, ch));
            AddDelayParam(DelayCtrl_ChParamID(ch), name, true);
        }
    }

    return AAX_SUCCESS;
}

// ============================================================================
// NotificationReceived
// (AAX_eNotificationEvent_SampleRateChanged does not exist in SDK 2.x;
//  Pro Tools creates a new instance on SR changes, so EffectInit covers it.)
// ============================================================================
AAX_Result DelayCtrl_Parameters::NotificationReceived(
    AAX_CTypeID iType, const void* iData, uint32_t iSize)
{
    return AAX_CEffectParameters::NotificationReceived(iType, iData, iSize);
}

// ============================================================================
// TimerWakeup
// Detects Unit Mode toggle and immediately refreshes all delay displays.
// ============================================================================
AAX_Result DelayCtrl_Parameters::TimerWakeup()
{
    RefreshUnitMode();

    if (mUnitIsMs != mLastUnitIsMs)
    {
        mLastUnitIsMs = mUnitIsMs;
        MarkAllDelaysDirty();   // force immediate display update on all surfaces
    }

    return AAX_CEffectParameters::TimerWakeup();
}

// ============================================================================
// UpdateCoefs
// ============================================================================
AAX_Result DelayCtrl_Parameters::UpdateCoefs(AAX_CPacket& ioPacket)
{
    SDelayCtrl_Coefs& coefs = *ioPacket.GetPtr<SDelayCtrl_Coefs>();
    coefs.numChannels = mNumChannels;

    float masterDelay = 0.f;
    {
        AAX_IParameter* p = mParameterManager.GetParameterByID(kDelayCtrl_MasterDelayID);
        if (p) p->GetValueAsFloat(&masterDelay);
    }

    for (int ch = 0; ch < kDelayCtrl_MaxChannels; ++ch)
    {
        float chDelay = 0.f;
        // For Mono, or channels beyond active count, per-channel offset = 0
        if (mNumChannels > 1 && ch < mNumChannels)
        {
            AAX_IParameter* p = mParameterManager.GetParameterByID(DelayCtrl_ChParamID(ch));
            if (p) p->GetValueAsFloat(&chDelay);
        }

        float total = masterDelay + chDelay;
        if (total < 0.f) total = 0.f;
        if (total > static_cast<float>(kDelayCtrl_MaxDelaySamples))
            total = static_cast<float>(kDelayCtrl_MaxDelaySamples);

        coefs.chDelaySamples[ch] = total;
    }
    return AAX_SUCCESS;
}

// ============================================================================
// Helpers
// ============================================================================
void DelayCtrl_Parameters::RefreshSampleRate()
{
    AAX_CSampleRate sr = 0.0;
    if (AAX_SUCCESS == Controller()->GetSampleRate(&sr) && sr > 0.0)
        mSampleRate = static_cast<float>(sr);
}

void DelayCtrl_Parameters::RefreshUnitMode()
{
    AAX_IParameter* p = mParameterManager.GetParameterByID(kDelayCtrl_UnitModeID);
    if (p) { bool v = false; p->GetValueAsBool(&v); mUnitIsMs = v; }
}

void DelayCtrl_Parameters::MarkAllDelaysDirty()
{
    // Touching each delay parameter forces Pro Tools to re-query
    // ValueToString on every connected surface immediately.
    mPacketDispatcher.SetDirty(kDelayCtrl_MasterDelayID);
    if (mNumChannels > 1)
        for (int ch = 0; ch < mNumChannels; ++ch)
            mPacketDispatcher.SetDirty(DelayCtrl_ChParamID(ch));
}
