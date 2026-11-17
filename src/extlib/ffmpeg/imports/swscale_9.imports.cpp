// Import declarations for swscale-9 library
// generated Tue Oct 14 08:24:26 2025 by DylibTool
#if STATIC_LIBSWSCALE_9

namespace Dylib { bool swscale_9_Loaded() { return true; } }
extern "C" {
}

#else

#include "DylibImports.h"
namespace { extern const Dylib::Import* functionImports; }

// Here you may specify a custom error message to be displayed when the library cannot be found.
static const char* notFoundMsg = "";
// Here you may specify an URL to some local or remote help resource.
static const char* notFoundURL = "";
RegisterDylibWithAliases( swscale_9, "swscale-9", functionImports, notFoundMsg, notFoundURL );

extern "C" {

// NOTE: When loading the library, function bodies will be overwritten with absolute jumps
// to imported functions, i.e. function bodies must be roughly 2 * sizeof(void*) bytes long.
// A call to a member function with a pointer argument should be large enough.
// 
// In addition, having distinct function bodies for each function stub
// is important -- that makes it impossible for size optimizers to
// collapse them into a single definition.
//
void sws_allocVec() { Dylib::swscale_9.UninitializedFunctionExport( "sws_allocVec" ); }
void sws_alloc_context() { Dylib::swscale_9.UninitializedFunctionExport( "sws_alloc_context" ); }
void sws_convertPalette8ToPacked24() { Dylib::swscale_9.UninitializedFunctionExport( "sws_convertPalette8ToPacked24" ); }
void sws_convertPalette8ToPacked32() { Dylib::swscale_9.UninitializedFunctionExport( "sws_convertPalette8ToPacked32" ); }
void sws_frame_end() { Dylib::swscale_9.UninitializedFunctionExport( "sws_frame_end" ); }
void sws_frame_setup() { Dylib::swscale_9.UninitializedFunctionExport( "sws_frame_setup" ); }
void sws_frame_start() { Dylib::swscale_9.UninitializedFunctionExport( "sws_frame_start" ); }
void sws_freeContext() { Dylib::swscale_9.UninitializedFunctionExport( "sws_freeContext" ); }
void sws_freeFilter() { Dylib::swscale_9.UninitializedFunctionExport( "sws_freeFilter" ); }
void sws_freeVec() { Dylib::swscale_9.UninitializedFunctionExport( "sws_freeVec" ); }
void sws_free_context() { Dylib::swscale_9.UninitializedFunctionExport( "sws_free_context" ); }
void sws_getCachedContext() { Dylib::swscale_9.UninitializedFunctionExport( "sws_getCachedContext" ); }
void sws_getCoefficients() { Dylib::swscale_9.UninitializedFunctionExport( "sws_getCoefficients" ); }
void sws_getColorspaceDetails() { Dylib::swscale_9.UninitializedFunctionExport( "sws_getColorspaceDetails" ); }
void sws_getContext() { Dylib::swscale_9.UninitializedFunctionExport( "sws_getContext" ); }
void sws_getDefaultFilter() { Dylib::swscale_9.UninitializedFunctionExport( "sws_getDefaultFilter" ); }
void sws_getGaussianVec() { Dylib::swscale_9.UninitializedFunctionExport( "sws_getGaussianVec" ); }
void sws_get_class() { Dylib::swscale_9.UninitializedFunctionExport( "sws_get_class" ); }
void sws_init_context() { Dylib::swscale_9.UninitializedFunctionExport( "sws_init_context" ); }
void sws_isSupportedEndiannessConversion() { Dylib::swscale_9.UninitializedFunctionExport( "sws_isSupportedEndiannessConversion" ); }
void sws_isSupportedInput() { Dylib::swscale_9.UninitializedFunctionExport( "sws_isSupportedInput" ); }
void sws_isSupportedOutput() { Dylib::swscale_9.UninitializedFunctionExport( "sws_isSupportedOutput" ); }
void sws_is_noop() { Dylib::swscale_9.UninitializedFunctionExport( "sws_is_noop" ); }
void sws_normalizeVec() { Dylib::swscale_9.UninitializedFunctionExport( "sws_normalizeVec" ); }
void sws_receive_slice() { Dylib::swscale_9.UninitializedFunctionExport( "sws_receive_slice" ); }
void sws_receive_slice_alignment() { Dylib::swscale_9.UninitializedFunctionExport( "sws_receive_slice_alignment" ); }
void sws_scale() { Dylib::swscale_9.UninitializedFunctionExport( "sws_scale" ); }
void sws_scaleVec() { Dylib::swscale_9.UninitializedFunctionExport( "sws_scaleVec" ); }
void sws_scale_frame() { Dylib::swscale_9.UninitializedFunctionExport( "sws_scale_frame" ); }
void sws_send_slice() { Dylib::swscale_9.UninitializedFunctionExport( "sws_send_slice" ); }
void sws_setColorspaceDetails() { Dylib::swscale_9.UninitializedFunctionExport( "sws_setColorspaceDetails" ); }
void sws_test_colorspace() { Dylib::swscale_9.UninitializedFunctionExport( "sws_test_colorspace" ); }
void sws_test_format() { Dylib::swscale_9.UninitializedFunctionExport( "sws_test_format" ); }
void sws_test_frame() { Dylib::swscale_9.UninitializedFunctionExport( "sws_test_frame" ); }
void sws_test_primaries() { Dylib::swscale_9.UninitializedFunctionExport( "sws_test_primaries" ); }
void sws_test_transfer() { Dylib::swscale_9.UninitializedFunctionExport( "sws_test_transfer" ); }
void swscale_configuration() { Dylib::swscale_9.UninitializedFunctionExport( "swscale_configuration" ); }
void swscale_license() { Dylib::swscale_9.UninitializedFunctionExport( "swscale_license" ); }
void swscale_version() { Dylib::swscale_9.UninitializedFunctionExport( "swscale_version" ); }
}

namespace {
const Dylib::Import functionImports_[] =
{
  { "sws_allocVec", (void**)&sws_allocVec, Dylib::Import::functionStub },
  { "sws_alloc_context", (void**)&sws_alloc_context, Dylib::Import::functionStub },
  { "sws_convertPalette8ToPacked24", (void**)&sws_convertPalette8ToPacked24, Dylib::Import::functionStub },
  { "sws_convertPalette8ToPacked32", (void**)&sws_convertPalette8ToPacked32, Dylib::Import::functionStub },
  { "sws_frame_end", (void**)&sws_frame_end, Dylib::Import::functionStub },
  { "sws_frame_setup", (void**)&sws_frame_setup, Dylib::Import::functionStub },
  { "sws_frame_start", (void**)&sws_frame_start, Dylib::Import::functionStub },
  { "sws_freeContext", (void**)&sws_freeContext, Dylib::Import::functionStub },
  { "sws_freeFilter", (void**)&sws_freeFilter, Dylib::Import::functionStub },
  { "sws_freeVec", (void**)&sws_freeVec, Dylib::Import::functionStub },
  { "sws_free_context", (void**)&sws_free_context, Dylib::Import::functionStub },
  { "sws_getCachedContext", (void**)&sws_getCachedContext, Dylib::Import::functionStub },
  { "sws_getCoefficients", (void**)&sws_getCoefficients, Dylib::Import::functionStub },
  { "sws_getColorspaceDetails", (void**)&sws_getColorspaceDetails, Dylib::Import::functionStub },
  { "sws_getContext", (void**)&sws_getContext, Dylib::Import::functionStub },
  { "sws_getDefaultFilter", (void**)&sws_getDefaultFilter, Dylib::Import::functionStub },
  { "sws_getGaussianVec", (void**)&sws_getGaussianVec, Dylib::Import::functionStub },
  { "sws_get_class", (void**)&sws_get_class, Dylib::Import::functionStub },
  { "sws_init_context", (void**)&sws_init_context, Dylib::Import::functionStub },
  { "sws_isSupportedEndiannessConversion", (void**)&sws_isSupportedEndiannessConversion, Dylib::Import::functionStub },
  { "sws_isSupportedInput", (void**)&sws_isSupportedInput, Dylib::Import::functionStub },
  { "sws_isSupportedOutput", (void**)&sws_isSupportedOutput, Dylib::Import::functionStub },
  { "sws_is_noop", (void**)&sws_is_noop, Dylib::Import::functionStub },
  { "sws_normalizeVec", (void**)&sws_normalizeVec, Dylib::Import::functionStub },
  { "sws_receive_slice", (void**)&sws_receive_slice, Dylib::Import::functionStub },
  { "sws_receive_slice_alignment", (void**)&sws_receive_slice_alignment, Dylib::Import::functionStub },
  { "sws_scale", (void**)&sws_scale, Dylib::Import::functionStub },
  { "sws_scaleVec", (void**)&sws_scaleVec, Dylib::Import::functionStub },
  { "sws_scale_frame", (void**)&sws_scale_frame, Dylib::Import::functionStub },
  { "sws_send_slice", (void**)&sws_send_slice, Dylib::Import::functionStub },
  { "sws_setColorspaceDetails", (void**)&sws_setColorspaceDetails, Dylib::Import::functionStub },
  { "sws_test_colorspace", (void**)&sws_test_colorspace, Dylib::Import::functionStub },
  { "sws_test_format", (void**)&sws_test_format, Dylib::Import::functionStub },
  { "sws_test_frame", (void**)&sws_test_frame, Dylib::Import::functionStub },
  { "sws_test_primaries", (void**)&sws_test_primaries, Dylib::Import::functionStub },
  { "sws_test_transfer", (void**)&sws_test_transfer, Dylib::Import::functionStub },
  { "swscale_configuration", (void**)&swscale_configuration, Dylib::Import::functionStub },
  { "swscale_license", (void**)&swscale_license, Dylib::Import::functionStub },
  { "swscale_version", (void**)&swscale_version, Dylib::Import::functionStub },
  { 0, 0, 0 }
};
const Dylib::Import* functionImports = functionImports_;
}

#endif // STATIC_LIBSWSCALE_9

