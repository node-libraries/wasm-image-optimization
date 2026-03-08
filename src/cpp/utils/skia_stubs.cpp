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
// -----------------------------------------------------------------------------

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

// Others
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

// ImageFilters & ColorFilters Registration
void _Z39SkRegisterPictureImageFilterFlattenablev() {}
void _Z39SkRegisterRuntimeImageFilterFlattenablev() {}
void _Z41SkRegisterMagnifierImageFilterFlattenablev() {}

// Filters & Effects
void _ZN10SkDashImpl10CreateProcER12SkReadBuffer() {}
void _ZN13SkColorMatrix13setSaturationEf() {}
void _ZN13SkColorMatrix8setScaleEffff() {}
void _ZN16SkDashPathEffect4MakeE6SkSpanIKfEf() {}
void _ZN17SkColorFilterPriv12MakeGaussianEv() {}
void _ZN17SkTableMaskFilter20RegisterFlattenablesEv() {}
void _ZN18SkCornerPathEffect20RegisterFlattenablesEv() {}
void _ZN18SkEmbossMaskFilter10CreateProcER12SkReadBuffer() {}
void _ZN18SkLine2DPathEffect20RegisterFlattenablesEv() {}
void _ZN18SkPath1DPathEffect20RegisterFlattenablesEv() {}
void _ZN18SkPath2DPathEffect20RegisterFlattenablesEv() {}
void _ZN18SkShaderMaskFilter20RegisterFlattenablesEv() {}
void _ZN20SkDiscretePathEffect20RegisterFlattenablesEv() {}
void _ZN8SkTrimPE10CreateProcER12SkReadBuffer() {}

} // extern "C"

#pragma clang diagnostic pop
