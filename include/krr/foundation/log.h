#ifndef KRR_LOG_h_
#define KRR_LOG_h_

#include <SDL2/SDL_log.h>
#define KRR_LOG(fmt, ...) SDL_Log(fmt, ##__VA_ARGS__)
#define KRR_LOGV(fmt, ...) SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, fmt, ##__VA_ARGS__)
#define KRR_LOGD(fmt, ...) SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, fmt, ##__VA_ARGS__)
#define KRR_LOGI(fmt, ...) SDL_Log(fmt, ##__VA_ARGS__)
#define KRR_LOGW(fmt, ...) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, fmt, ##__VA_ARGS__)
#define KRR_LOGE(fmt, ...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, fmt, ##__VA_ARGS__)
#define KRR_LOGC(fmt, ...) SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, fmt, ##__VA_ARGS__)

#endif
