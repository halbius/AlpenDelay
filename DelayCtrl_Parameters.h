#pragma once
#ifndef DelayCtrl_PARAMETERS_H
#define DelayCtrl_PARAMETERS_H

// ============================================================================
// DelayCtrl_Parameters.h
//
// AAX parameter class for DelayCtrl.
//
// Parameters:
//   MasterDelay  – 0 .. kDelayCtrl_MaxDelaySamples (int32, Samples)
//   ChDelay1..8  – 0 .. kDelayCtrl_MaxDelaySamples (int32, Samples, per ch)
//   UnitMode     – 0 = Samples, 1 = Milliseconds
//   MasterBypass – standard bypass
//
// Display:
//   In Samples mode  → "1234 Smp"
//   In ms mode       → "  12.3 ms"
//
// The coefficient packet sent to the DSP contains the TOTAL delay per channel
// (master + per-channel), so the DSP doesn't need to know about units.
//
// Sample rate is read from the Controller once on EffectInit and on
// NotificationReceived(AAX_eNotificationEvent_SampleRateChanged).
// ============================================================================

#include "AAX_CEffectParameters.h"
#include "DelayCtrl_Defs.h"

#include <string>
#include <atomic>

class DelayCtrl_Parameters : public AAX_CEffectParameters
{
public:
    static AAX_CEffectParameters* AAX_CALLBACK Create();

    DelayCtrl_Parameters();
    ~DelayCtrl_Parameters() AAX_OVERRIDE;

    AAX_Result EffectInit()    AAX_OVERRIDE;
    AAX_Result TimerWakeup()   AAX_OVERRIDE;

    AAX_Result NotificationReceived(AAX_CTypeID  iType,
                                    const void*  iData,
                                    uint32_t     iSize) AAX_OVERRIDE;

    // Packet callback: builds coefficient packet for DSP
    AAX_Result UpdateCoefs(AAX_CPacket& ioPacket);

    // Helpers
    static std::string FormatDelay(int32_t samples, float sampleRate, bool isMs);

private:
    void RefreshSampleRate();

    int         mNumChannels;
    float       mSampleRate;    // cached, updated on SR-change notification
    bool        mIsStereoOrMore;
};

// ============================================================================
// Custom display delegate: shows Samples OR ms depending on UnitMode param
// ============================================================================
#include "AAX_IDisplayDelegate.h"
#include "AAX_CEffectParameters.h"
#include "AAX_CString.h"
#include <sstream>
#include <iomanip>

class DelayDisplayDelegate : public AAX_IDisplayDelegate<int32_t>
{
public:
    // isMaster: master delay or per-channel (same logic, just label differs)
    DelayDisplayDelegate(const AAX_CEffectParameters* params, float* sampleRateRef)
        : mParams(params), mSampleRateRef(sampleRateRef) {}

    AAX_IDisplayDelegate<int32_t>* Clone() const AAX_OVERRIDE
    {
        return new DelayDisplayDelegate(mParams, mSampleRateRef);
    }

    bool ValueToString(int32_t iValue, AAX_CString* oString) const AAX_OVERRIDE
    {
        // Read unit mode
        bool isMs = false;
        {
            AAX_IParameter* p =
                const_cast<AAX_CEffectParameters*>(mParams)
                ->GetParameterManager().GetParameterByID(kDelayCtrl_UnitModeID);
            if (p) { bool v = false; p->GetValueAsBool(&v); isMs = v; }
        }

        float sr = mSampleRateRef ? *mSampleRateRef : 48000.0f;
        if (sr <= 0.0f) sr = 48000.0f;

        std::ostringstream oss;
        if (!isMs)
        {
            oss << iValue << " Smp";
        }
        else
        {
            float ms = (iValue / sr) * 1000.0f;
            oss << std::fixed << std::setprecision(1) << ms << " ms";
        }
        *oString = AAX_CString(oss.str().c_str());
        return true;
    }

    bool ValueToString(int32_t iValue, int32_t, AAX_CString* oString) const AAX_OVERRIDE
    {
        return ValueToString(iValue, oString);
    }

    bool StringToValue(const AAX_CString& iString, int32_t* oValue) const AAX_OVERRIDE
    {
        // Try to parse typed input: accept plain number (samples or ms)
        bool isMs = false;
        {
            AAX_IParameter* p =
                const_cast<AAX_CEffectParameters*>(mParams)
                ->GetParameterManager().GetParameterByID(kDelayCtrl_UnitModeID);
            if (p) { bool v = false; p->GetValueAsBool(&v); isMs = v; }
        }

        float sr = mSampleRateRef ? *mSampleRateRef : 48000.0f;
        if (sr <= 0.0f) sr = 48000.0f;

        float parsed = 0.0f;
        try { parsed = std::stof(iString.Get()); }
        catch (...) { return false; }

        if (isMs)
            *oValue = static_cast<int32_t>((parsed / 1000.0f) * sr + 0.5f);
        else
            *oValue = static_cast<int32_t>(parsed + 0.5f);

        if (*oValue < 0) *oValue = 0;
        if (*oValue > kDelayCtrl_MaxDelaySamples) *oValue = kDelayCtrl_MaxDelaySamples;
        return true;
    }

private:
    const AAX_CEffectParameters* mParams;
    float*                        mSampleRateRef;
};

#endif // DelayCtrl_PARAMETERS_H
