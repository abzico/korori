#include "krr/foundation/util.h"
#include "krr/platforms/platforms_config.h"
#include <stdio.h>
#include <stdlib.h>

// FIXME: On arm refer to https://stackoverflow.com/questions/3398664/how-to-get-a-call-stack-backtrace-deeply-embedded-no-library-support to use unwind.h header with similar functionality. For now when calling KRR_util_print_callstack() on unsupported platform,
// it will do nothing as it's an empty function.
#if (defined __APPLE__ || defined __linux__) && defined(KRR_PLATFORM_PC)
#define HAS_BACKTRACE
#include <execinfo.h>
#endif

#ifdef HAS_BACKTRACE
static void* callstack[32];
#endif

void KRR_util_print_callstack()
{
#ifdef HAS_BACKTRACE
  int frames = backtrace(callstack, 32);
  int i;
  char** strs = backtrace_symbols(callstack, frames);
  for (i=0; i<frames; i++)
  {
    fprintf(stdout, "%s\n", strs[i]);
  }
  free(strs);
#endif
}
