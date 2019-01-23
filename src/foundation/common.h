/*
 * Contains common variables used across multiple source files.
 */

#ifndef KRR_common_h_
#define KRR_common_h_

#include <stdbool.h>
#include "window.h"

#include "SDL_log.h"
#define KRR_LOG(fmt, ...) SDL_Log(fmt, ##__VA_ARGS__)
#define KRR_LOGV(fmt, ...) SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, fmt, ##__VA_ARGS__)
#define KRR_LOGD(fmt, ...) SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, fmt, ##__VA_ARGS__)
#define KRR_LOGI(fmt, ...) SDL_Log(fmt, ##__VA_ARGS__)
#define KRR_LOGW(fmt, ...) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, fmt, ##__VA_ARGS__)
#define KRR_LOGE(fmt, ...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, fmt, ##__VA_ARGS__)
#define KRR_LOGC(fmt, ...) SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, fmt, ##__VA_ARGS__)

/*
 * The window we'll be rendering to.
 */
extern KRR_WINDOW* gWindow;

extern float common_frameTime;
#ifndef DISABLE_FPS_CALC
/*
 * Variables used in calculate frame rate per second.
 */
extern int common_frameCount;
extern float common_frameAccumTime;
extern float common_avgFPS;
#endif

#endif
