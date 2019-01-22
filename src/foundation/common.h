/*
 * Contains common variables used across multiple source files.
 */

#ifndef krr_common_h_
#define krr_common_h_

#include <stdbool.h>
#include "window.h"

// CAUTION: It's user's resposibility to define these variables before use.

/*
 * The window we'll be rendering to.
 */
extern krr_WINDOW* gWindow;

extern float common_frameTime;
#ifndef DISABLE_FPS_CALC
/*
 * Variables used in calculate frame rate per second.
 */
extern int common_frameCount;
extern float common_frameAccumTime;
extern float common_avgFPS;
#endif

#endif /* common_h_ */
