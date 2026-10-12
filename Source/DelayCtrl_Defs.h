#pragma once
#ifndef DelayCtrl_DEFS_H
#define DelayCtrl_DEFS_H

#include "AAX.h"

// ============================================================================
// Effect ID
// ============================================================================
const AAX_CEffectID kEffectID_DelayCtrl = "com.yourcompany.aax.alpendelay";

// ============================================================================
// Type / Product / Plug-in IDs
// ============================================================================
const AAX_CTypeID cDelayCtrl_ManufactureID   = 'DyCo';
const AAX_CTypeID cDelayCtrl_ProductID       = 'DyCp';

const AAX_CTypeID cDelayCtrl_PlugInID_Mono   = 'DC01';
const AAX_CTypeID cDelayCtrl_PlugInID_Stereo = 'DC02';
const AAX_CTypeID cDelayCtrl_PlugInID_LCR    = 'DC03';
const AAX_CTypeID cDelayCtrl_PlugInID_LCRS   = 'DC04';
const AAX_CTypeID cDelayCtrl_PlugInID_Quad   = 'DC05';
const AAX_CTypeID cDelayCtrl_PlugInID_5_0    = 'DC06';
const AAX_CTypeID cDelayCtrl_PlugInID_5_1    = 'DC07';

// Meter IDs
const AAX_CTypeID cDelayCtrl_MeterID[2] = { 'DcIn', 'DcOt' };

// ============================================================================
// Parameter IDs
// ============================================================================
#define kDelayCtrl_MasterDelayID  "MasterDelay"
#define kDelayCtrl_ChDelayID_1    "ChDelay1"
#define kDelayCtrl_ChDelayID_2    "ChDelay2"
#define kDelayCtrl_ChDelayID_3    "ChDelay3"
#define kDelayCtrl_ChDelayID_4    "ChDelay4"
#define kDelayCtrl_ChDelayID_5    "ChDelay5"
#define kDelayCtrl_ChDelayID_6    "ChDelay6"
#define kDelayCtrl_ChDelayID_7    "ChDelay7"
#define kDelayCtrl_ChDelayID_8    "ChDelay8"
#define kDelayCtrl_UnitModeID     "UnitMode"

// ============================================================================
// Delay Range
// Internal unit: Samples (float).  DSP rounds to nearest integer sample.
// At 48 kHz: 10000 Smp ≈ 208.3 ms
// ============================================================================
constexpr int32_t kDelayCtrl_MaxDelaySamples = 10000;
constexpr int32_t kDelayCtrl_MinDelaySamples = 0;
constexpr int32_t kDelayCtrl_DefaultDelay    = 0;
constexpr int32_t kDelayCtrl_NumSteps        = 10001; // 0..10000 inclusive
constexpr int     kDelayCtrl_MaxChannels     = 8;

// ============================================================================
// Helpers
// ============================================================================
inline int DelayCtrl_ChannelCount(AAX_EStemFormat fmt)
{
    switch (fmt)
    {
        case AAX_eStemFormat_Mono:   return 1;
        case AAX_eStemFormat_Stereo: return 2;
        case AAX_eStemFormat_LCR:    return 3;
        case AAX_eStemFormat_LCRS:   return 4;
        case AAX_eStemFormat_Quad:   return 4;
        case AAX_eStemFormat_5_0:    return 5;
        case AAX_eStemFormat_5_1:    return 6;
        default:                     return 1;
    }
}

inline const char* DelayCtrl_ChName(AAX_EStemFormat fmt, int ch)
{
    switch (fmt)
    {
        case AAX_eStemFormat_Stereo:
        case AAX_eStemFormat_Quad:
        { static const char* s[] = { "L", "R", "Ls", "Rs" }; return s[ch]; }
        case AAX_eStemFormat_LCR:
        { static const char* s[] = { "L", "C", "R" }; return s[ch]; }
        case AAX_eStemFormat_LCRS:
        { static const char* s[] = { "L", "C", "R", "S" }; return s[ch]; }
        case AAX_eStemFormat_5_0:
        { static const char* s[] = { "L", "C", "R", "Ls", "Rs" }; return s[ch]; }
        case AAX_eStemFormat_5_1:
        { static const char* s[] = { "L", "C", "R", "Ls", "Rs", "LFE" }; return s[ch]; }
        default:
        { static const char* s[] = {"1","2","3","4","5","6","7","8"}; return s[ch]; }
    }
}

inline const char* DelayCtrl_ChParamID(int ch)
{
    static const char* ids[kDelayCtrl_MaxChannels] = {
        kDelayCtrl_ChDelayID_1, kDelayCtrl_ChDelayID_2,
        kDelayCtrl_ChDelayID_3, kDelayCtrl_ChDelayID_4,
        kDelayCtrl_ChDelayID_5, kDelayCtrl_ChDelayID_6,
        kDelayCtrl_ChDelayID_7, kDelayCtrl_ChDelayID_8,
    };
    return (ch >= 0 && ch < kDelayCtrl_MaxChannels) ? ids[ch] : ids[0];
}

#endif // DelayCtrl_DEFS_H
