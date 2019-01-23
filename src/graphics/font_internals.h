#ifndef KRR_FONT_internals_h_
#define KRR_FONT_internals_h_

#include "graphics/common.h"
#include "graphics/font.h"

/// This header is meant to be used internally by library itself.
/// If you include this, you should know what you're doing.

///
/// Return the width of input string according to the current loaded font.
/// It will return the width until it reaches '\n' or '\0'.
///
/// \param font Pointer to KRR_FONT
/// \param string Input string
/// \return String width
///
extern GLfloat KRR_FONT_string_width(KRR_FONT* font, const char* string);

///
/// Return string's height required to render it according to current loaded font.
///
/// \param font Pointer to KRR_FONT
/// \param string Input string
/// \return String's height
///
extern GLfloat KRR_FONT_string_height(KRR_FONT* font, const char* string);

#endif
