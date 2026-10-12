#pragma once
#ifndef DelayCtrl_PARAMETERS_H
#define DelayCtrl_PARAMETERS_H

#include "AAX_CEffectParameters.h"
#include "AAX_IDisplayDelegate.h"
#include "AAX_CString.h"
#include "DelayCtrl_Defs.h"

#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstdlib>

// ============================================================================
// DelayDisplayDelegate
// Internal value: float samples.
// Display: "1234 Smp"  or  " 25.7 ms" (0.1 ms resolution)
// ============================================================================
class DelayDisplayDelegate : public AAX_IDisplayDelegate<float>
{
public:
    DelayDisplayDelegate(const bool* isMsRef, const float* srRef)
        : mIsMsRef(isMsRef), mSrRef(srRef) {}

    AAX_IDisplayDelegate<float>* Clone() const AAX_OVERRIDE
    {
        return new DelayDisplayDelegate(mIsMsRef, mSrRef);
    }

    bool ValueToString(float iValue, AAX_CString* oString) const AAX_OVERRIDE
    {
        const bool  isMs = mIsMsRef && *mIsMsRef;
        const float sr   = (mSrRef && *mSrRef > 0.f) ? *mSrRef : 48000.f;
        std::ostringstream oss;
        if (!isMs)
        {
            oss << static_cast<int32_t>(iValue + 0.5f) << " Smp";
        }
        else
        {
            float ms = std::roundf((iValue / sr) * 10000.f) / 10.f; // 0.1 ms
            oss << std::fixed << std::setprecision(1) << ms << " ms";
        }
        *oString = AAX_CString(oss.str().c_str());
        return true;
    }

    bool ValueToString(float iValue, int32_t, AAX_CString* oString) const AAX_OVERRIDE
    {
        return ValueToString(iValue, oString);
    }

    bool StringToValue(const AAX_CString& iString, float* oValue) const AAX_OVERRIDE
    {
        const bool  isMs = mIsMsRef && *mIsMsRef;
        const float sr   = (mSrRef && *mSrRef > 0.f) ? *mSrRef : 48000.f;

        // Strip non-numeric characters, then parse
        std::string num;
        for (const char* p = iString.Get(); *p; ++p)
            if ((*p >= '0' && *p <= '9') || *p == '.' || *p == '-')
                num += *p;
        if (num.empty()) return false;

        char* end = nullptr;
        float parsed = std::strtof(num.c_str(), &end);
        if (end == num.c_str()) return false;

        float samples = isMs ? (std::roundf(parsed * 10.f) / 10.f / 1000.f * sr)
                              : std::roundf(parsed);
        if (samples < static_cast<float>(kDelayCtrl_MinDelaySamples))
            samples = static_cast<float>(kDelayCtrl_MinDelaySamples);
        if (samples > static_cast<float>(kDelayCtrl_MaxDelaySamples))
            samples = static_cast<float>(kDelayCtrl_MaxDelaySamples);
        *oValue = samples;
        return true;
    }

private:
    const bool*  mIsMsRef;
    const float* mSrRef;
};

// ============================================================================
// DelayCtrl_Parameters
// ============================================================================
class DelayCtrl_Parameters : public AAX_CEffectParameters
{
public:
    static AAX_CEffectParameters* AAX_CALLBACK Create();

    DelayCtrl_Parameters();
    ~DelayCtrl_Parameters() AAX_OVERRIDE;

    AAX_Result EffectInit()    AAX_OVERRIDE;
    AAX_Result TimerWakeup()   AAX_OVERRIDE;

    AAX_Result NotificationReceived(AAX_CTypeID iType,
                                    const void* iData,
                                    uint32_t    iSize) AAX_OVERRIDE;

    AAX_Result UpdateCoefs(AAX_CPacket& ioPacket);

private:
    void AddDelayParam(const char* paramID, const char* name, bool registerPacket);
    void RefreshSampleRate();
    void RefreshUnitMode();
    void MarkAllDelaysDirty();

    AAX_EStemFormat mStemFormat;
    int             mNumChannels;
    float           mSampleRate;   // referenced by display delegates
    bool            mUnitIsMs;     // referenced by display delegates
    bool            mLastUnitIsMs; // detect toggle in TimerWakeup
};

#endif // DelayCtrl_PARAMETERS_H
