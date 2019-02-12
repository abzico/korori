/*
 * Contains common variables used across multiple source files.
 */

#ifndef KRR_common_h_
#define KRR_common_h_

#include <stdbool.h>
#include "window.h"

// use library call
// for inline
#include <cglm/cglm.h>
// for function call
//#include <cglm/call.h>

#include "foundation/log.h"

/*
 * The window we'll be rendering to.
 */
extern KRR_WINDOW* gWindow;

extern double common_frameTime;
#ifndef DISABLE_FPS_CALC
/*
 * Variables used in calculate frame rate per second.
 */
extern int common_frameCount;
extern double common_frameAccumTime;
extern double common_avgFPS;
#endif

#endif
