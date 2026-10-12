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

// Ring buffer: float buf[kRingBufFloats] + int32_t writeHead
constexpr int32_t kRingBufFloats    = kDelayCtrl_MaxDelaySamples + 1;
constexpr int32_t kRingBufDataBytes = kRingBufFloats * (int32_t)sizeof(float)
                                    + (int32_t)sizeof(int32_t);

inline float*   RingBuf_Buffer(void* block)
{
    return reinterpret_cast<float*>(block);
}
inline int32_t& RingBuf_WriteHead(void* block)
{
    return *reinterpret_cast<int32_t*>(
        reinterpret_cast<char*>(block) + kRingBufFloats * sizeof(float));
}

#include AAX_ALIGN_FILE_BEGIN
#include AAX_ALIGN_FILE_ALG
#include AAX_ALIGN_FILE_END

struct SDelayCtrl_Coefs
{
    float   chDelaySamples[kDelayCtrl_MaxChannels];
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
    void*             mRingBuf[kDelayCtrl_MaxChannels];
};

enum EDelayCtrl_Alg_PortID
{
     eAlgPortID_BypassIn    = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mCtrlBypassP)
    ,eAlgPortID_CoefsIn     = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mCoefsP)
    ,eAlgFieldID_AudioIn    = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mInputPP)
    ,eAlgFieldID_AudioOut   = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mOutputPP)
    ,eAlgFieldID_BufferSize = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mBufferSize)
    ,eAlgFieldID_Meters     = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mMetersPP)
    ,eAlgFieldID_RingBuf0   = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mRingBuf[0])
    ,eAlgFieldID_RingBuf1   = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mRingBuf[1])
    ,eAlgFieldID_RingBuf2   = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mRingBuf[2])
    ,eAlgFieldID_RingBuf3   = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mRingBuf[3])
    ,eAlgFieldID_RingBuf4   = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mRingBuf[4])
    ,eAlgFieldID_RingBuf5   = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mRingBuf[5])
    ,eAlgFieldID_RingBuf6   = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mRingBuf[6])
    ,eAlgFieldID_RingBuf7   = AAX_FIELD_INDEX(SDelayCtrl_Alg_Context, mRingBuf[7])
};

void AAX_CALLBACK DelayCtrl_AlgorithmProcessFunction(
    SDelayCtrl_Alg_Context* const inInstancesBegin[],
    const void*                   inInstancesEnd);

#endif // DelayCtrl_ALG_H
