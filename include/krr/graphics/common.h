#ifndef KRR_graphics_common_h_
#define KRR_graphics_common_h_

#include <GL/glew.h>
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__MINGW32__) || defined(__MINGW64__)
#include <GL/gl.h>
#elif defined(__APPLE__) || defined(__MACH__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

// use library call
// for inline
#include <cglm/cglm.h>
// for function call
//#include <cglm/call.h>

#include "krr/graphics/types.h"

#endif
