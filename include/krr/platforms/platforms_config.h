#ifndef KRR_PLATFORMS_CONFIG_h_
#define KRR_PLATFORMS_CONFIG_h_

/// define of our supported platforms
/// target and focus on Android, Windows (32/64 bit) and Linux (Ubuntu first)
#define KRR_PLATFORM_UNKNOWN    0
#define KRR_PLATFORM_ANDROID    1
#define KRR_PLATFORM_WIN32      2
#define KRR_PLATFORM_WIN64      3
#define KRR_PLATFORM_LINUX      4

/// the variable to check which platform is it to use
/// this will determine which functionalities to be enabled and used in the engine
/// initially started with unknown platform but it should be able to detected
#define KRR_TARGET_PLATFORM     KRR_PLATFORM_UNKNOWN

/// detect and define for category of platform first
/// either mobile or PC
#ifdef __ANDROID__
  #define KRR_TARGET_PLATFORM_CATEGORY_MOBILE

  #undef KRR_TARGET_PLATFORM
  #define KRR_TARGET_PLATFORM   KRR_PLATFORM_ANDROID
#else
  #define KRR_TARGET_PLATFORM_CATEGORY_PC
#endif

/// detect Windows 32 bit
#if (defined(_WIN32) || defined(__CYGWIN32__) || defined(__MINGW32__)) && !defined(__ANDROID__)
  #undef KRR_TARGET_PLATFORM
  #define KRR_TARGET_PLATFORM   KRR_PLATFORM_WIN32
#endif

/// detect Windows 64 bit
#if (defined(_WIN64) || defined(__CYGWIN__) || defined(__MINGW64__)) && !defined(__ANDROID__)
  #undef KRR_TARGET_PLATFORM
  #define KRR_TARGET_PLATFORM   KRR_PLATFORM_WIN64
#endif

/// detect Linux
#if (defined(__linux__) || defined(__CYGWIN__) || defined(__MINGW64__)) && !defined(__ANDROID__)
  #undef KRR_TARGET_PLATFORM
  #define KRR_TARGET_PLATFORM   KRR_PLATFORM_LINUX
#endif

#endif  // header
