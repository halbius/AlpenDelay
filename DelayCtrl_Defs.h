#pragma once
#ifndef DelayCtrl_DEFS_H
#define DelayCtrl_DEFS_H

#include "AAX.h"

// ============================================================================
// Effect ID
// ============================================================================
const AAX_CEffectID kEffectID_DelayCtrl = "com.yourcompany.aax.delayctrl";

// ============================================================================
// Type / Product / Plug-in IDs
// ============================================================================
const AAX_CTypeID cDelayCtrl_ManufactureID          = 'DyCo';
const AAX_CTypeID cDelayCtrl_ProductID              = 'DyCp';

// Native (real-time) IDs per channel count
const AAX_CTypeID cDelayCtrl_PlugInID_Mono          = 'DC01';
const AAX_CTypeID cDelayCtrl_PlugInID_Stereo        = 'DC02';
const AAX_CTypeID cDelayCtrl_PlugInID_LCR           = 'DC03';
const AAX_CTypeID cDelayCtrl_PlugInID_LCRS          = 'DC04';
const AAX_CTypeID cDelayCtrl_PlugInID_Quad          = 'DC05';
const AAX_CTypeID cDelayCtrl_PlugInID_5_0           = 'DC06';
const AAX_CTypeID cDelayCtrl_PlugInID_5_1           = 'DC07';
const AAX_CTypeID cDelayCtrl_PlugInID_7_0           = 'DC08';
const AAX_CTypeID cDelayCtrl_PlugInID_7_1           = 'DC09';

// Meter IDs (Pre/Post)
const AAX_CTypeID cDelayCtrl_MeterID[2] = { 'DcIn', 'DcOt' };

// ============================================================================
// Parameter IDs
// ============================================================================
// Master delay (affects all channels equally, added on top of per-channel)
#define kDelayCtrl_MasterDelayID    "MasterDelay"

// Per-channel delays Ch1..Ch8
#define kDelayCtrl_ChDelayID_1      "ChDelay1"
#define kDelayCtrl_ChDelayID_2      "ChDelay2"
#define kDelayCtrl_ChDelayID_3      "ChDelay3"
#define kDelayCtrl_ChDelayID_4      "ChDelay4"
#define kDelayCtrl_ChDelayID_5      "ChDelay5"
#define kDelayCtrl_ChDelayID_6      "ChDelay6"
#define kDelayCtrl_ChDelayID_7      "ChDelay7"
#define kDelayCtrl_ChDelayID_8      "ChDelay8"

// Unit toggle: 0 = Samples, 1 = Milliseconds
#define kDelayCtrl_UnitModeID       "UnitMode"

// ============================================================================
// Delay Range
// ============================================================================
// Maximum delay: 10000 samples (≈ 227 ms @ 44.1 kHz, ≈ 208 ms @ 48 kHz)
constexpr int32_t kDelayCtrl_MaxDelaySamples = 10000;
constexpr int32_t kDelayCtrl_MinDelaySamples = 0;
constexpr int32_t kDelayCtrl_DefaultDelay    = 0;

// Number of plugin channels supported
constexpr int kDelayCtrl_MaxChannels = 8;

// ============================================================================
// Helper: channel count from stem format
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
        case AAX_eStemFormat_7_0:    return 7;
        case AAX_eStemFormat_7_1:    return 8;
        default:                     return 1;
    }
}

// Per-channel parameter ID by index (0-based)
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
