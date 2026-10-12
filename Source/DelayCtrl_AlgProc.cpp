// ============================================================================
// DelayCtrl_AlgProc.cpp
// ============================================================================

#include "DelayCtrl_Alg.h"
#include "AAX.h"
#include <cmath>

void AAX_CALLBACK DelayCtrl_AlgorithmProcessFunction(
    SDelayCtrl_Alg_Context* const inInstancesBegin[],
    const void*                   inInstancesEnd)
{
    for (SDelayCtrl_Alg_Context* const* walk = inInstancesBegin;
         walk < inInstancesEnd; ++walk)
    {
        SDelayCtrl_Alg_Context* AAX_RESTRICT ctx = *walk;

        const int32_t   bypass  = *ctx->mCtrlBypassP;
        const int32_t   bufSize = *ctx->mBufferSize;
        const SDelayCtrl_Coefs* AAX_RESTRICT coefs = ctx->mCoefsP;
        const int32_t   numCh   = coefs->numChannels;
        float* AAX_RESTRICT meterTaps = *ctx->mMetersPP;

        for (int ch = 0; ch < numCh; ++ch)
        {
            const float* AAX_RESTRICT pIn  = ctx->mInputPP[ch];
            float*       AAX_RESTRICT pOut = ctx->mOutputPP[ch];

            if (bypass)
            {
                for (int t = 0; t < bufSize; ++t) pOut[t] = pIn[t];
                if (ch == 0)
                    for (int t = 0; t < bufSize; ++t)
                    {
                        float a = fabsf(pIn[t]);
                        if (a > meterTaps[eMeterTap_PreGain])  meterTaps[eMeterTap_PreGain]  = a;
                        if (a > meterTaps[eMeterTap_PostGain]) meterTaps[eMeterTap_PostGain] = a;
                    }
                continue;
            }

            // Round float delay → nearest integer sample, clamp
            int32_t D = static_cast<int32_t>(coefs->chDelaySamples[ch] + 0.5f);
            if (D < 0)              D = 0;
            if (D >= kRingBufFloats) D = kRingBufFloats - 1;

            void*    block     = ctx->mRingBuf[ch];
            float*   ringBuf   = RingBuf_Buffer(block);
            int32_t& writeHead = RingBuf_WriteHead(block);

            for (int t = 0; t < bufSize; ++t)
            {
                ringBuf[writeHead] = pIn[t];
                int32_t readHead = writeHead - D;
                if (readHead < 0) readHead += kRingBufFloats;
                pOut[t] = ringBuf[readHead];
                if (++writeHead >= kRingBufFloats) writeHead = 0;
            }

            if (ch == 0)
                for (int t = 0; t < bufSize; ++t)
                {
                    float ai = fabsf(pIn[t]),  ao = fabsf(pOut[t]);
                    if (ai > meterTaps[eMeterTap_PreGain])  meterTaps[eMeterTap_PreGain]  = ai;
                    if (ao > meterTaps[eMeterTap_PostGain]) meterTaps[eMeterTap_PostGain] = ao;
                }
        }
    }
}
