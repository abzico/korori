#ifndef KRR_util_h_
#define KRR_util_h_

#include <SDL2/SDL_rwops.h>

#ifdef __cplusplus
extern "C" {
#endif

///
/// Print callstack up to this point of calling this function.
/// Stack size is 32.
///
/// Only supported on Linux, Unix, and macOS. It will be no-op for Windows and other platforms.
///
extern void KRR_util_print_callstack();

/*
 * Equivalent to fgets() but operate on string instead of file.
 *
 * It will read string up to `dst_size` or stop reading whenever find newline character, then return string so far as read via `dist_str`.
 *
 * It will finally stop whenever it has read all the bytes from the `input_str`.
 *
 * Be careful if you dynamically allocate input string, you have to save original pointer somewhere in order to free it later. Calling this function will modify input string's starting location thus you cannot use it to free any further unless you save original pointer to do so.
 *
 * \param dst_str destination string as output
 * \param dst_size up to number of bytes to read for destination string
 * \param input_str input string to read. When this function returns, this input string's location will be updated to point to the 
 * \return string as read starting from the current location of input string to newline character but with maximum limit up to `dst_size`. It returns NULL when all of `input_str`'s bytes has been read and no more string can be found.
 */
extern char* KRR_util_sgets(char* dst_str, int dst_size, char** input_str);

/*
 * Read line from SDL_RWops and output to `dst_str` upto `dst_size`. Its reading location will be updated thus this function can be called repetitively to get each line from SDL_RWops.
 *
 * Note that output string doesn't include newline character.
 *
 * \param file input SDL_RWops
 * \param dst_str destination string to get a line of text to
 * \param dst_size maximum size in bytes to read into output string.
 * \return pointer to destination string, this just return `dst_str`.
 */
extern char* KRR_util_rwopsgets(SDL_RWops* file, char* dst_str, int dst_size);

#ifdef __cplusplus
}
#endif

#endif
