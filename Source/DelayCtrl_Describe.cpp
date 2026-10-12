// ============================================================================
// DelayCtrl_Describe.cpp
// ============================================================================

#include "DelayCtrl_Describe.h"
#include "DelayCtrl_Defs.h"
#include "DelayCtrl_Alg.h"
#include "DelayCtrl_Parameters.h"

#include "AAX_ICollection.h"
#include "AAX_IComponentDescriptor.h"
#include "AAX_IEffectDescriptor.h"
#include "AAX_IPropertyMap.h"
#include "AAX_Exception.h"
#include "AAX_Errors.h"
#include "AAX_Assert.h"

static AAX_CTypeID GetPlugInID(AAX_EStemFormat fmt)
{
    switch (fmt)
    {
        case AAX_eStemFormat_Mono:   return cDelayCtrl_PlugInID_Mono;
        case AAX_eStemFormat_Stereo: return cDelayCtrl_PlugInID_Stereo;
        case AAX_eStemFormat_LCR:    return cDelayCtrl_PlugInID_LCR;
        case AAX_eStemFormat_LCRS:   return cDelayCtrl_PlugInID_LCRS;
        case AAX_eStemFormat_Quad:   return cDelayCtrl_PlugInID_Quad;
        case AAX_eStemFormat_5_0:    return cDelayCtrl_PlugInID_5_0;
        case AAX_eStemFormat_5_1:    return cDelayCtrl_PlugInID_5_1;
        default:                     return cDelayCtrl_PlugInID_Mono;
    }
}

static void DescribeAlgorithmComponent(AAX_IComponentDescriptor* outDesc,
                                        AAX_EStemFormat            stemFormat)
{
    AAX_CheckedResult err;
    const int numCh = DelayCtrl_ChannelCount(stemFormat);

    err = outDesc->AddAudioIn(eAlgFieldID_AudioIn);
    err = outDesc->AddAudioOut(eAlgFieldID_AudioOut);
    err = outDesc->AddAudioBufferLength(eAlgFieldID_BufferSize);

    static_assert(eMeterTap_Count == sizeof(cDelayCtrl_MeterID) / sizeof(AAX_CTypeID),
                  "meter count mismatch");
    err = outDesc->AddMeters(eAlgFieldID_Meters, cDelayCtrl_MeterID, eMeterTap_Count);

    err = outDesc->AddDataInPort(eAlgPortID_BypassIn, sizeof(int32_t));
    err = outDesc->AddDataInPort(eAlgPortID_CoefsIn,  sizeof(SDelayCtrl_Coefs));

    // Per-channel ring buffers (only for active channels)
    static const int kRingBufIDs[kDelayCtrl_MaxChannels] = {
        eAlgFieldID_RingBuf0, eAlgFieldID_RingBuf1,
        eAlgFieldID_RingBuf2, eAlgFieldID_RingBuf3,
        eAlgFieldID_RingBuf4, eAlgFieldID_RingBuf5,
        eAlgFieldID_RingBuf6, eAlgFieldID_RingBuf7,
    };
    for (int ch = 0; ch < numCh; ++ch)
        err = outDesc->AddPrivateData(kRingBufIDs[ch], kRingBufDataBytes,
                                      AAX_ePrivateDataOptions_DefaultOptions);

    AAX_IPropertyMap* props = outDesc->NewPropertyMap();
    if (!props) { err = AAX_ERROR_NULL_OBJECT; return; }

    err = props->AddProperty(AAX_eProperty_ManufacturerID,   cDelayCtrl_ManufactureID);
    err = props->AddProperty(AAX_eProperty_ProductID,        cDelayCtrl_ProductID);
    err = props->AddProperty(AAX_eProperty_PlugInID_Native,  GetPlugInID(stemFormat));
    err = props->AddProperty(AAX_eProperty_CanBypass,        true);
    err = props->AddProperty(AAX_eProperty_UsesClientGUI,    true);
    err = props->AddProperty(AAX_eProperty_InputStemFormat,  stemFormat);
    err = props->AddProperty(AAX_eProperty_OutputStemFormat, stemFormat);
    err = props->AddPointerProperty(AAX_eProperty_NativeProcessProc,
        reinterpret_cast<const void*>(&DelayCtrl_AlgorithmProcessFunction));

    err = outDesc->AddProcessProc(props);
}

static AAX_Result DescribeEffect(AAX_IEffectDescriptor* outDesc)
{
    AAX_CheckedResult err;

    err = outDesc->AddName("AlpenDelay");
    err = outDesc->AddName("Alpen Dly");
    err = outDesc->AddName("AlpDly");
    err = outDesc->AddName("AlpD");
    err = outDesc->AddName("ADl");
    err = outDesc->AddName("AD");
    err = outDesc->AddCategory(AAX_ePlugInCategory_None);

    const AAX_EStemFormat kFormats[7] = {
        AAX_eStemFormat_Mono,
        AAX_eStemFormat_Stereo,
        AAX_eStemFormat_LCR,
        AAX_eStemFormat_LCRS,
        AAX_eStemFormat_Quad,
        AAX_eStemFormat_5_0,
        AAX_eStemFormat_5_1,
    };

    AAX_IComponentDescriptor* compDesc = outDesc->NewComponentDescriptor();
    if (!compDesc) { err = AAX_ERROR_NULL_OBJECT; return err; }

    for (int i = 0; i < 7; ++i)
    {
        err = compDesc->Clear();
        DescribeAlgorithmComponent(compDesc, kFormats[i]);
        err = outDesc->AddComponent(compDesc);
    }

    err = outDesc->AddProcPtr(
        reinterpret_cast<void*>(DelayCtrl_Parameters::Create),
        kAAX_ProcPtrID_Create_EffectParameters);
    err = outDesc->AddResourceInfo(AAX_eResourceType_PageTable, "DelayCtrlPages.xml");

    // Meters
    {
        AAX_IPropertyMap* mp = outDesc->NewPropertyMap();
        if (!mp) { err = AAX_ERROR_NULL_OBJECT; return err; }
        err = mp->AddProperty(AAX_eProperty_Meter_Type,        AAX_eMeterType_Input);
        err = mp->AddProperty(AAX_eProperty_Meter_Orientation, AAX_eMeterOrientation_Default);
        err = outDesc->AddMeterDescription(cDelayCtrl_MeterID[eMeterTap_PreGain], "Input", mp);
    }
    {
        AAX_IPropertyMap* mp = outDesc->NewPropertyMap();
        if (!mp) { err = AAX_ERROR_NULL_OBJECT; return err; }
        err = mp->AddProperty(AAX_eProperty_Meter_Type,        AAX_eMeterType_Output);
        err = mp->AddProperty(AAX_eProperty_Meter_Orientation, AAX_eMeterOrientation_Default);
        err = outDesc->AddMeterDescription(cDelayCtrl_MeterID[eMeterTap_PostGain], "Output", mp);
    }

    return err;
}

AAX_Result GetEffectDescriptions(AAX_ICollection* outCollection)
{
    AAX_CheckedResult err;
    AAX_IEffectDescriptor* plugInDesc = outCollection->NewDescriptor();
    if (plugInDesc)
    {
        AAX_SWALLOW_MULT(
            err = DescribeEffect(plugInDesc);
            err = outCollection->AddEffect(kEffectID_DelayCtrl, plugInDesc);
        );
    }
    else err = AAX_ERROR_NULL_OBJECT;

    err = outCollection->SetManufacturerName("Your Company");
    err = outCollection->AddPackageName("AlpenDelay");
    err = outCollection->AddPackageName("Alpen Delay");
    err = outCollection->AddPackageName("AlpDly");
    err = outCollection->SetPackageVersion(1);
    return err;
}
