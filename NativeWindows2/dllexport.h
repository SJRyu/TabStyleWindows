#pragma once

#ifdef NATIVEWINDOWS2_EXPORTS
#define NATIVEWINDOWS2_API	__declspec(dllexport)
#else
#define NATIVEWINDOWS2_API	__declspec(dllimport)
#endif

