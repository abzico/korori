#include "krr/foundation/util.h"
#include "krr/platforms/platforms_config.h"
#include <stdio.h>
#include <stdlib.h>

// FIXME: On arm refer to https://stackoverflow.com/questions/3398664/how-to-get-a-call-stack-backtrace-deeply-embedded-no-library-support to use unwind.h header with similar functionality. For now when calling KRR_util_print_callstack() on unsupported platform,
// it will do nothing as it's an empty function.
#if (defined __APPLE__ || defined __linux__) && defined(KRR_TARGET_PLATFORM_CATEGORY_PC)
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

char* KRR_util_sgets(char* dst_str, int dst_size, char** input_str)
{
  // safety check, probably removed out in performance-centric code
  if (input_str == NULL || *input_str == NULL)
    return NULL;

  // if end of the string from previous iteration(s)
  if ((*input_str)[0] == '\0')
  {
    return NULL;
  }

	int size_count = 0;
  // access value-only from input string
  const char* input_str_val = *input_str;

	for (int i = 0; i<dst_size; ++i)
	{
		char c = input_str_val[i];

		if (c == '\n')
		{
			dst_str[i] = c;
			dst_str[i+1] = '\0';
      ++size_count;
			break;
		}
		else if (c == '\0')
		{
			dst_str[i] = c;
			break;
		}
		else
		{
			dst_str[i] = c;
			++size_count;
		}
	}

  // update input str with number of bytes we've read
  // effect address back to the caller
  *input_str = (char*)((long)(*input_str) + size_count);

	return dst_str;
}

char* KRR_util_rwopsgets(SDL_RWops* file, char* dst_str, int dst_size)
{
	int size_count = 0;

	for (int i = 0; i<dst_size; ++i)
	{
    char c;
    // read a single byte from SDL_RWops
    // watchout: might be perf bottleneck
    if (SDL_RWread(file, &c, 1, 1) != 1)
    {
      // end of reading
      return NULL;
    }

		if (c == '\n')
		{
			dst_str[i] = c;
			dst_str[i+1] = '\0';
      ++size_count;
			break;
		}
		else if (c == '\0' && i == 0)
		{
      return NULL;
		}
		else if (c == '\0')
		{
			dst_str[i] = c;
			break;
		}
		else
		{
			dst_str[i] = c;
			++size_count;
		}
	}

  // SDL_RWops updates its own buffer automatically

	return dst_str;
}
