#include <stdint.h>
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/private/base/SkSemaphore.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wunused-parameter"

// SkSemaphore
SkSemaphore::~SkSemaphore() {}
void SkSemaphore::osSignal(int n) {}
void SkSemaphore::osWait() {}

extern "C" {

// Thread ID
int64_t SkGetThreadID() { return 0; }

// -----------------------------------------------------------------------------
// Skia Stubs for Aggressive Optimization (-Oz)
// Must match the EXACT signatures expected by the linker to avoid wasm-opt crash.
// -----------------------------------------------------------------------------

// (i32, i32) -> i32
int _ZNK15SkRuntimeEffect9findChildENSt3__217basic_string_viewIcNS0_11char_traitsIcEEEE(void* a,
                                                                                        void* b) {
    return 0;
}
int _ZNK15SkRuntimeEffect11findUniformENSt3__217basic_string_viewIcNS0_11char_traitsIcEEEE(
    void* a, void* b) {
    return 0;
}

// (i32, f32) -> i32
int _ZN11GrPathUtils15cubicPointCountEPK7SkPointf(void* a, float b) { return 0; }
int _ZN11GrPathUtils19quadraticPointCountEPK7SkPointf(void* a, float b) { return 0; }

// (i32, i32, i32, f32, i32, i32) -> i32
int _ZN11GrPathUtils23generateQuadraticPointsERK7SkPointS2_S2_fPPS0_j(void* a, void* b, void* c,
                                                                      float d, void* e, int f) {
    return 0;
}

// (i32, i32, i32, i32, f32, i32, i32) -> i32
int _ZN11GrPathUtils19generateCubicPointsERK7SkPointS2_S2_S2_fPPS0_j(void* a, void* b, void* c,
                                                                     void* d, float e, void* f,
                                                                     int g) {
    return 0;
}

// (i32) -> i32
int _ZNK13GrStyledShape15unstyledKeySizeEv(void* a) { return 0; }
int _ZN6SkMeshD1Ev(void* a) { return 0; }
int _ZNK15SkRuntimeEffect7Uniform11sizeInBytesEv(void* a) { return 0; }
int _ZNK15SkRuntimeEffect6sourceEv(void* a) { return 0; }
int _ZNK15SkRuntimeEffect11uniformSizeEv(void* a) { return 0; }

// Others
void _ZN15SkRuntimeEffect13MakeForShaderE8SkStringRKNS_7OptionsE() {}
void _ZN15SkRuntimeEffect14MakeForBlenderE8SkStringRKNS_7OptionsE() {}
void _ZN15SkRuntimeEffect18MakeForColorFilterE8SkStringRKNS_7OptionsE() {}
void _ZN15SkRuntimeEffect20RegisterFlattenablesEv() {}
void _ZNK15SkRuntimeEffect10makeShaderE5sk_spIK6SkDataE6SkSpanIKNS_8ChildPtrEEPK8SkMatrix() {}
void _ZNK15SkRuntimeEffect11makeBlenderE5sk_spIK6SkDataE6SkSpanIKNS_8ChildPtrEE() {}
void _ZNK15SkRuntimeEffect15makeColorFilterE5sk_spIK6SkDataE() {}
void _ZNK22SkRuntimeEffectBuilder10makeShaderEPK8SkMatrix() {}
void _Z25SkMakeCachedRuntimeEffectPFN15SkRuntimeEffect6ResultE8SkStringRKNS_7OptionsEES1_() {}
void _ZN6SkMeshC1ERKS_() {}
void _ZN13GrStyledShape8simplifyEv() {}
void _ZNK13GrStyledShape16writeUnstyledKeyEPj() {}

// SkOTUtils
void _ZN9SkOTUtils26LocalizedStrings_NameTable18MakeForFamilyNamesERK10SkTypeface(
    void* result_ptr, const void* typeface) {
    if (result_ptr) {
        *(void**)result_ptr = nullptr;
    }
}

// PathOps
bool _Z2OpRK6SkPathS1_8SkPathOp(void* one, void* two, int op, void* result) {
    if (result) {
        // Initialize the result path to be empty
        new (result) SkPath();
    }
    return true;
}

// ImageFilters & ColorFilters Registration
void _Z36SkRegisterBlurImageFilterFlattenablev() {}
void _Z36SkRegisterCropImageFilterFlattenablev() {}
void _Z36SkRegisterModeColorFilterFlattenablev() {}
void _Z37SkRegisterBlendImageFilterFlattenablev() {}
void _Z37SkRegisterImageImageFilterFlattenablev() {}
void _Z37SkRegisterMergeImageFilterFlattenablev() {}
void _Z37SkRegisterTableColorFilterFlattenablev() {}
void _Z38SkRegisterMatrixColorFilterFlattenablev() {}
void _Z38SkRegisterShaderImageFilterFlattenablev() {}
void _Z39SkRegisterComposeColorFilterFlattenablev() {}
void _Z39SkRegisterComposeImageFilterFlattenablev() {}
void _Z39SkRegisterPictureImageFilterFlattenablev() {}
void _Z39SkRegisterRuntimeImageFilterFlattenablev() {}
void _Z41SkRegisterLightingImageFilterFlattenablesv() {}
void _Z41SkRegisterMagnifierImageFilterFlattenablev() {}
void _Z43SkRegisterColorFilterImageFilterFlattenablev() {}
void _Z43SkRegisterMorphologyImageFilterFlattenablesv() {}
void _Z45SkRegisterWorkingFormatColorFilterFlattenablev() {}
void _Z47SkRegisterDisplacementMapImageFilterFlattenablev() {}
void _Z47SkRegisterMatrixTransformImageFilterFlattenablev() {}
void _Z48SkRegisterLegacyDropShadowImageFilterFlattenablev() {}
void _Z49SkRegisterMatrixConvolutionImageFilterFlattenablev() {}
void _Z49SkRegisterSkColorSpaceXformColorFilterFlattenablev() {}

// Filters & Effects
void _ZN10SkDashImpl10CreateProcER12SkReadBuffer() {}
void _ZN13SkColorMatrix13setSaturationEf() {}
void _ZN13SkColorMatrix8setScaleEffff() {}
void _ZN14SkColorFilters17LinearToSRGBGammaEv() {}
void _ZN14SkColorFilters17SRGBToLinearGammaEv() {}
void _ZN14SkColorFilters5BlendERK8SkRGBA4fIL11SkAlphaType3EE5sk_spI12SkColorSpaceE11SkBlendMode() {}
void _ZN14SkColorFilters5BlendEj11SkBlendMode() {}
void _ZN14SkColorFilters6MatrixERK13SkColorMatrixNS_5ClampE() {}
void _ZN14SkColorFilters9TableARGBEPKhS1_S1_S1_() {}
void _ZN14SkImageFilters10ArithmeticEffffb5sk_spI13SkImageFilterES2_RKNS_8CropRectE() {}
void _ZN14SkImageFilters11ColorFilterE5sk_spI13SkColorFilterES0_I13SkImageFilterERKNS_8CropRectE() {}
void _ZN14SkImageFilters14SpotLitDiffuseERK8SkPoint3S2_ffjff5sk_spI13SkImageFilterERKNS_8CropRectE() {}
void _ZN14SkImageFilters15DisplacementMapE14SkColorChannelS0_f5sk_spI13SkImageFilterES3_RKNS_8CropRectE() {}
void _ZN14SkImageFilters15PointLitDiffuseERK8SkPoint3jff5sk_spI13SkImageFilterERKNS_8CropRectE() {}
void _ZN14SkImageFilters15SpotLitSpecularERK8SkPoint3S2_ffjfff5sk_spI13SkImageFilterERKNS_8CropRectE() {}
void _ZN14SkImageFilters16PointLitSpecularERK8SkPoint3jfff5sk_spI13SkImageFilterERKNS_8CropRectE() {}
void _ZN14SkImageFilters17DistantLitDiffuseERK8SkPoint3jff5sk_spI13SkImageFilterERKNS_8CropRectE() {}
void _ZN14SkImageFilters18DistantLitSpecularERK8SkPoint3jfff5sk_spI13SkImageFilterERKNS_8CropRectE() {}
void _ZN14SkImageFilters4BlurEff10SkTileMode5sk_spI13SkImageFilterERKNS_8CropRectE(void* res, float f1, float f2, int tm, void* sp, void* cr) {
    if (res) {
        *(void**)res = nullptr;
    }
}
void _ZN14SkImageFilters5BlendE11SkBlendMode5sk_spI13SkImageFilterES3_RKNS_8CropRectE() {}
void _ZN14SkImageFilters5ErodeEff5sk_spI13SkImageFilterERKNS_8CropRectE() {}
void _ZN14SkImageFilters5ImageE5sk_spI7SkImageERK6SkRectS5_RK17SkSamplingOptions() {}
void _ZN14SkImageFilters5MergeEP5sk_spI13SkImageFilterEiRKNS_8CropRectE() {}
void _ZN14SkImageFilters6DilateEff5sk_spI13SkImageFilterERKNS_8CropRectE() {}
void _ZN14SkImageFilters6OffsetEff5sk_spI13SkImageFilterERKNS_8CropRectE() {}
void _ZN14SkImageFilters6ShaderE5sk_spI8SkShaderENS_6DitherERKNS_8CropRectE() {}
void _ZN16SkDashPathEffect4MakeE6SkSpanIKfEf() {}
void _ZN17SkColorFilterPriv12MakeGaussianEv() {}
void _ZN17SkLumaColorFilter4MakeEv() {}
void _ZN17SkTableMaskFilter20RegisterFlattenablesEv() {}
void _ZN18SkCornerPathEffect20RegisterFlattenablesEv() {}
void _ZN18SkEmbossMaskFilter10CreateProcER12SkReadBuffer() {}
void _ZN18SkLine2DPathEffect20RegisterFlattenablesEv() {}
void _ZN18SkPath1DPathEffect20RegisterFlattenablesEv() {}
void _ZN18SkPath2DPathEffect20RegisterFlattenablesEv() {}
void _ZN18SkShaderMaskFilter20RegisterFlattenablesEv() {}
void _ZN20SkDiscretePathEffect20RegisterFlattenablesEv() {}
void _ZN8SkTrimPE10CreateProcER12SkReadBuffer() {}
void _ZNK13SkColorFilter12makeComposedE5sk_spIS_E() {}

} // extern "C"

namespace SkSL {
class DebugTracePriv {
   public:
    virtual ~DebugTracePriv() {}
};
}  // namespace SkSL
SkSL::DebugTracePriv* force_vtable_sksl = nullptr;

#pragma clang diagnostic pop
