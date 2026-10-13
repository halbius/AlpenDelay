#pragma once
#ifndef DelayCtrl_ALG_H
#define DelayCtrl_ALG_H

#include "AAX.h"
#include "DelayCtrl_Defs.h"

enum EDelayCtrl_MeterTaps
{
    eMeterTap_PreGain = 0,
    eMeterTap_PostGain,
    eMeterTap_Count
};

// ============================================================================
// Ringpuffer: ein einziger Private-Data-Block für alle Kanäle.
// Pro Kanal: float buf[kRingBufFloats] + int32_t writeHead (16-Byte-aligned).
// ============================================================================
constexpr int32_t kRingBufFloats  = kDelayCtrl_MaxDelaySamples + 1;
constexpr int32_t kRingBlockBytes = ((kRingBufFloats * (int32_t)sizeof(float)
                                      + (int32_t)sizeof(int32_t)) + 15) / 16 * 16;

inline int32_t DelayCtrl_RingBytes(int numChannels)
{
    return numChannels * kRingBlockBytes;
}
inline float* Ring_Buffer(void* block, int ch)
{
    return reinterpret_cast<float*>(static_cast<char*>(block) + (size_t)ch * kRingBlockBytes);
}
inline int32_t& Ring_WriteHead(void* block, int ch)
{
    return *reinterpret_cast<int32_t*>(static_cast<char*>(block)
           + (size_t)ch * kRingBlockBytes + (size_t)kRingBufFloats * sizeof(float));
}

// ============================================================================
// Koeffizienten (Parameters → DSP)
// ============================================================================
#include AAX_ALIGN_FILE_BEGIN
#include AAX_ALIGN_FILE_ALG
#include AAX_ALIGN_FILE_END

struct SDelayCtrl_Coefs
{
    float   chDelaySamples[kDelayCtrl_MaxChannels]; // float-Samples, DSP rundet
    int32_t numChannels;
};

#include AAX_ALIGN_FILE_BEGIN
#include AAX_ALIGN_FILE_RESET
#include AAX_ALIGN_FILE_END

struct SDelayCtrl_Alg_Context
{
    int32_t*          mCtrlBypassP;
    SDelayCtrl_Coefs* mCoefsP;
    float**           mInputPP;
    float**           mOutputPP;
    int32_t*          mBufferSize;
    float**           mMetersPP;
    void*             mRingBuf;      // Private-Data-Block (alle Kanäle)
};

enum EDelayCtrl_Alg_PortID
{
     eAlgPortID_BypassIn    = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mCtrlBypassP)
    ,eAlgPortID_CoefsIn     = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mCoefsP)
    ,eAlgFieldID_AudioIn    = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mInputPP)
    ,eAlgFieldID_AudioOut   = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mOutputPP)
    ,eAlgFieldID_BufferSize = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mBufferSize)
    ,eAlgFieldID_Meters     = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mMetersPP)
    ,eAlgFieldID_RingBuf    = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mRingBuf)
};

void AAX_CALLBACK DelayCtrl_AlgorithmProcessFunction(
    SDelayCtrl_Alg_Context* const inInstancesBegin[],
    const void*                   inInstancesEnd);

#endif // DelayCtrl_ALG_H
