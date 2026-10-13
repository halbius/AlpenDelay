#pragma once
#ifndef DelayCtrl_DEFS_H
#define DelayCtrl_DEFS_H

#include "AAX.h"
#include "DelayCtrl_Formats.h"

// ============================================================================
// Effect / Hersteller / Produkt
// ============================================================================
const AAX_CEffectID kEffectID_DelayCtrl = "com.ChristianAlpen.aax.alpendelay";

const AAX_CTypeID cDelayCtrl_ManufactureID = 'DyCo';
const AAX_CTypeID cDelayCtrl_ProductID     = 'DyCp';

// Meter IDs
const AAX_CTypeID cDelayCtrl_MeterID[2] = { 'DcIn', 'DcOt' };

// ============================================================================
// Parameter IDs
// (Kanal-Delay-IDs kommen aus DelayCtrl_Formats.h: "DlyL", "DlyLss", ...)
// ============================================================================
#define kDelayCtrl_MasterDelayID  "MasterDelay"
#define kDelayCtrl_UnitModeID     "UnitMode"

// ============================================================================
// Delay-Bereich. Interne Einheit: Samples (float); der DSP rundet auf ganze
// Samples. 10000 Smp ≈ 208 ms bei 48 kHz.
// ============================================================================
constexpr int32_t kDelayCtrl_MaxDelaySamples = 10000;
constexpr int32_t kDelayCtrl_MinDelaySamples = 0;
constexpr int32_t kDelayCtrl_DefaultDelay    = 0;
constexpr int32_t kDelayCtrl_NumSteps        = 10001; // 0..10000 inklusive

inline int DelayCtrl_ChannelCount(AAX_EStemFormat fmt)
{
    const SDelayCtrl_Format* f = DelayCtrl_FindFormat(fmt);
    return f ? f->numChannels : 1;
}

#endif // DelayCtrl_DEFS_H
