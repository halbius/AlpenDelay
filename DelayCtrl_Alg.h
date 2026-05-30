#pragma once
#ifndef DelayCtrl_ALG_H
#define DelayCtrl_ALG_H

// ============================================================================
// DelayCtrl_Alg.h
//
// Algorithm data structures for the DelayCtrl AAX plug-in.
//
// Delay implementation:
//   Each channel has its own ring buffer of kDelayCtrl_MaxDelaySamples.
//   Total delay per channel = masterDelay + channelDelay[ch]  (in samples).
//   The ring buffer is allocated as a data block by AAX.
// ============================================================================

#include "AAX.h"
#include "DelayCtrl_Defs.h"

// ============================================================================
// Meter taps
// ============================================================================
enum EDelayCtrl_MeterTaps
{
    eMeterTap_PreGain = 0,
    eMeterTap_PostGain,
    eMeterTap_Count
};

// ============================================================================
// Ring-buffer block (one per channel, allocated by AAX as private data)
// Size = kDelayCtrl_MaxDelaySamples floats + write-head index
// ============================================================================

// We store the ring buffer and write index together in a flat block.
// Layout: float buf[kDelayCtrl_MaxDelaySamples], int32_t writeHead
// Total bytes per channel: kDelayCtrl_MaxDelaySamples * sizeof(float) + sizeof(int32_t)
// Rounded up to 4-byte alignment (already the case).

constexpr int32_t kRingBufFloats    = kDelayCtrl_MaxDelaySamples;
constexpr int32_t kRingBufDataBytes = kRingBufFloats * (int32_t)sizeof(float) + (int32_t)sizeof(int32_t);

// Accessor helpers (inline, used only from AlgProc)
inline float* RingBuf_Buffer(void* block)
{
    return reinterpret_cast<float*>(block);
}
inline int32_t& RingBuf_WriteHead(void* block)
{
    return *reinterpret_cast<int32_t*>(
        reinterpret_cast<char*>(block) + kRingBufFloats * sizeof(float));
}

// ============================================================================
// Coefficient block (sent from Parameters thread to Alg thread)
// ============================================================================

#include AAX_ALIGN_FILE_BEGIN
#include AAX_ALIGN_FILE_ALG
#include AAX_ALIGN_FILE_END

struct SDelayCtrl_Coefs
{
    // Total effective delay per channel in samples (masterDelay + chDelay[i])
    // clamped to [0, kDelayCtrl_MaxDelaySamples]
    int32_t chDelaySamples[kDelayCtrl_MaxChannels];

    // Number of active channels for this instance
    int32_t numChannels;
};

#include AAX_ALIGN_FILE_BEGIN
#include AAX_ALIGN_FILE_RESET
#include AAX_ALIGN_FILE_END

// ============================================================================
// Algorithm context (one per plug-in instance)
// ============================================================================
struct SDelayCtrl_Alg_Context
{
    int32_t*         mCtrlBypassP;      // Bypass control port
    SDelayCtrl_Coefs* mCoefsP;          // Coefficient port
    float**          mInputPP;          // Audio input channels
    float**          mOutputPP;         // Audio output channels
    int32_t*         mBufferSize;       // Buffer size port
    float**          mMetersPP;         // Meter taps

    // Per-channel ring buffer private data blocks (up to 8)
    // These are private data ports, one per channel.
    // We declare 8 slots; unused ones will be nullptr for narrower formats.
    void*            mRingBuf[kDelayCtrl_MaxChannels];
};

// ============================================================================
// Port IDs (must match field order in SDelayCtrl_Alg_Context)
// ============================================================================
enum EDelayCtrl_Alg_PortID
{
     eAlgPortID_BypassIn     = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mCtrlBypassP)
    ,eAlgPortID_CoefsIn      = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mCoefsP)
    ,eAlgFieldID_AudioIn     = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mInputPP)
    ,eAlgFieldID_AudioOut    = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mOutputPP)
    ,eAlgFieldID_BufferSize  = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mBufferSize)
    ,eAlgFieldID_Meters      = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mMetersPP)
    ,eAlgFieldID_RingBuf0    = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mRingBuf[0])
    ,eAlgFieldID_RingBuf1    = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mRingBuf[1])
    ,eAlgFieldID_RingBuf2    = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mRingBuf[2])
    ,eAlgFieldID_RingBuf3    = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mRingBuf[3])
    ,eAlgFieldID_RingBuf4    = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mRingBuf[4])
    ,eAlgFieldID_RingBuf5    = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mRingBuf[5])
    ,eAlgFieldID_RingBuf6    = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mRingBuf[6])
    ,eAlgFieldID_RingBuf7    = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mRingBuf[7])
};

// ============================================================================
// Callback declaration
// ============================================================================
void AAX_CALLBACK DelayCtrl_AlgorithmProcessFunction(
    SDelayCtrl_Alg_Context* const inInstancesBegin[],
    const void*                   inInstancesEnd);

#endif // DelayCtrl_ALG_H
