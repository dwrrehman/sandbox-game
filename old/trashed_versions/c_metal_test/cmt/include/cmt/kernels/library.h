/*
 * Copyright (c), Recep Aslantas.
 * MIT License (MIT), http://opensource.org/licenses/MIT
 */

#ifndef cmt_library_h
#define cmt_library_h
#ifdef __cplusplus
extern "C" {
#endif

#include "cmt/common.h"
#include "cmt/types.h"
#include "cmt/enums.h"
#include "cmt/error.h"

MT_EXPORT
MT_API_AVAILABLE(mt_macos(10.11), mt_ios(8.0))
MtLibrary*
mtNewDefaultLibrary(MtDevice *device);

MT_EXPORT
MT_API_AVAILABLE(mt_macos(10.11), mt_ios(8.0))
MtLibrary*
mtNewLibraryWithFile(MtDevice *device, char *filepath, NsError *error);

MT_EXPORT
MT_API_AVAILABLE(mt_macos(10.11), mt_ios(8.0))
MtLibrary*
mtNewLibraryWithSource(MtDevice *device, char *source, MtCompileOptions *Opts, NsError **error);

MT_EXPORT
MT_API_AVAILABLE(mt_macos(10.11), mt_ios(8.0))
MtDevice*
mtLibraryDevice(MtLibrary *device);

MT_EXPORT
MT_API_AVAILABLE(mt_macos(10.11), mt_ios(8.0))
const char*
mtLibraryLabel(MtLibrary *device);

MT_EXPORT
MT_API_AVAILABLE(mt_macos(10.11), mt_ios(8.0))
const char **
mtLibraryFunctionNames(MtLibrary *device);

#ifdef __cplusplus
}
#endif
#endif /* cmt_library_h */
