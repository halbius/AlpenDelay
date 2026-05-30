// ============================================================================
// DelayCtrl_AlgProc.cpp
//
// Real-time DSP: sample-accurate delay using per-channel ring buffers.
//
// Each channel has a private ring buffer of kDelayCtrl_MaxDelaySamples floats
// plus a 32-bit write-head index, allocated by AAX as private data.
//
// Total delay for channel i = coefs.chDelaySamples[i]
//   (already the sum of master + per-channel offset, computed in Parameters)
//
// Delay = 0  → pass-through (ring buffer still advances, write == read)
// ============================================================================

#include "DelayCtrl_Alg.h"
#include "AAX.h"
#include <algorithm>
#include <cstring>
#include <cmath>

void AAX_CALLBACK DelayCtrl_AlgorithmProcessFunction(
    SDelayCtrl_Alg_Context* const inInstancesBegin[],
    const void*                   inInstancesEnd)
{
    for (SDelayCtrl_Alg_Context* const* walk = inInstancesBegin;
         walk < inInstancesEnd; ++walk)
    {
        SDelayCtrl_Alg_Context* AAX_RESTRICT ctx = *walk;

        const int32_t   bypass     = *ctx->mCtrlBypassP;
        const int32_t   bufSize    = *ctx->mBufferSize;
        const SDelayCtrl_Coefs* AAX_RESTRICT coefs = ctx->mCoefsP;
        const int32_t   numCh      = coefs->numChannels;

        // ----------------------------------------------------------------
        // Meter tap pointers
        // ----------------------------------------------------------------
        float* AAX_RESTRICT meterTaps = *ctx->mMetersPP;

        // ----------------------------------------------------------------
        // Process each channel
        // ----------------------------------------------------------------
        for (int ch = 0; ch < numCh; ++ch)
        {
            const float* AAX_RESTRICT pIn  = ctx->mInputPP[ch];
            float*       AAX_RESTRICT pOut = ctx->mOutputPP[ch];

            if (bypass)
            {
                // Bypass: straight copy, ring buffer is NOT advanced.
                // This keeps the buffer state intact so when bypass is
                // released there is no glitch caused by a stale buffer.
                for (int t = 0; t < bufSize; ++t)
                    pOut[t] = pIn[t];

                // Meter
                if (ch == 0)
                {
                    for (int t = 0; t < bufSize; ++t)
                    {
                        float abs = fabsf(pIn[t]);
                        if (abs > meterTaps[eMeterTap_PreGain])
                            meterTaps[eMeterTap_PreGain] = abs;
                        if (abs > meterTaps[eMeterTap_PostGain])
                            meterTaps[eMeterTap_PostGain] = abs;
                    }
                }
                continue;
            }

            // Get ring buffer for this channel
            void*    block     = ctx->mRingBuf[ch];
            float*   ringBuf   = RingBuf_Buffer(block);
            int32_t& writeHead = RingBuf_WriteHead(block);

            const int32_t delaySamples = coefs->chDelaySamples[ch];

            if (delaySamples <= 0)
            {
                // Zero delay: pass-through, still write into ring buf
                // so the buffer stays consistent if delay is changed later.
                for (int t = 0; t < bufSize; ++t)
                {
                    ringBuf[writeHead] = pIn[t];
                    writeHead = (writeHead + 1 >= kRingBufFloats) ? 0 : writeHead + 1;
                    pOut[t] = pIn[t];
                }
            }
            else
            {
                // Normal delay
                const int32_t D = delaySamples;

                for (int t = 0; t < bufSize; ++t)
                {
                    // Write current sample
                    ringBuf[writeHead] = pIn[t];

                    // Read sample D steps behind the write head
                    int32_t readHead = writeHead - D;
                    if (readHead < 0) readHead += kRingBufFloats;

                    pOut[t] = ringBuf[readHead];

                    // Advance write head
                    writeHead = (writeHead + 1 >= kRingBufFloats) ? 0 : writeHead + 1;
                }
            }

            // Meter (channel 0 only for efficiency)
            if (ch == 0)
            {
                for (int t = 0; t < bufSize; ++t)
                {
                    float absIn  = fabsf(pIn[t]);
                    float absOut = fabsf(pOut[t]);
                    if (absIn  > meterTaps[eMeterTap_PreGain])
                        meterTaps[eMeterTap_PreGain]  = absIn;
                    if (absOut > meterTaps[eMeterTap_PostGain])
                        meterTaps[eMeterTap_PostGain] = absOut;
                }
            }
        }
    }
}
