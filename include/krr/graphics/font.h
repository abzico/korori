#ifndef KRR_FONT_h_
#define KRR_FONT_h_

#include "krr/graphics/common.h"
#include "krr/graphics/spritesheet.h"
#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef __cplusplus
extern "C" {
#endif

enum KRR_FONT_TEXTALIGNMENT
{
  // horizontal set
  KRR_FONT_TEXTALIGNMENT_LEFT            = 0x1,
  KRR_FONT_TEXTALIGNMENT_CENTERED_H      = 0x2,
  KRR_FONT_TEXTALIGNMENT_RIGHT           = 0x4,
  // vertical set
  KRR_FONT_TEXTALIGNMENT_TOP             = 0x8,
  KRR_FONT_TEXTALIGNMENT_CENTERED_V      = 0x10,
  KRR_FONT_TEXTALIGNMENT_BOTTOM          = 0x20
};

// global shader program that is used by all of KRR_FONT instances
// user can set this variable in runtime to different shader program
// to achieve different rendering effect
// it's safe to retain the original one then set it back when done
struct KRR_FONTSHADERPROG2D_;
extern struct KRR_FONTSHADERPROG2D_* shared_font_shaderprogram;

typedef struct
{
  /// underlying spritesheet
  KRR_SPRITESHEET* spritesheet;

  // spacing variables
  /// how much spacing when found ' ' space
  GLfloat space;
  // how much spacing between highest text pixel to lowest text pixel
  GLfloat line_height;
  // how much spacing when found '\n' (newline)
  GLfloat newline;
} KRR_FONT;

///
/// Create a new bitmap font.
/// Underlying KRR_SPRITESHEET will be automatically creaed and managed when done using this.
///
/// \param spritesheet Pointer to KRR_SPRITESHEET
/// \return Newly created KRR_FONT allocated on heap. It can be NULL if underlying system initialization failed to do so.
///
extern KRR_FONT* KRR_FONT_new(void);

///
/// Free font.
/// After this call, KRR_FONT will be freed (destroyed).
///
/// \param font Pointer to KRR_FONT
///
extern void KRR_FONT_free(KRR_FONT* font);

///
/// Load bitmap
/// Expects 8-bit grayscale image with no alpha.
///
/// \param font Pointer to KRR_FONT
/// \param path Path to bitmap file to load
/// \return True if successfully load, otherwise return false
///
extern bool KRR_FONT_load_bitmap(KRR_FONT* font, const char* path);

///
/// Load FreeType font
///
/// \param font Pointer to KRR_FONT
/// \param path Path to load TTF file
/// \param pixel_size Pixel size of font to generate bitmap font from TTF file
/// \return True if load successfully, otherwise return false.
///
extern bool KRR_FONT_load_freetype(KRR_FONT* font, const char* path, GLuint pixel_size);

///
/// Free KRR_FONT's font.
/// This doesn't free or destroy KRR_FONT itself.
///
/// \param font Pointer to KRR_FONT
///
extern void KRR_FONT_free_font(KRR_FONT* font);

///
/// Bind vao.
///
/// \param font pointerto KRR_FONT
///
extern void KRR_FONT_bind_vao(KRR_FONT* font);

///
/// Render text
///
/// \param font Pointer to KRR_FONT
/// \param text Text to render
/// \param x Position x to render. Origin is at top-left.
/// \param y Position y to render. Origin is at top-left.
///
extern void KRR_FONT_render_text(KRR_FONT* font, const char* text, GLfloat x, GLfloat y);

///
/// Render text
///
/// \param font Pointer to KRR_FONT
/// \param text Text to render
/// \param x Position x to render.
/// \param y Position y to render.
/// \param area_size Area size to align text within it if given. It can be NULL.
/// \param align Alignment to align text within the given area. See KRR_FONT_TEXTALIGNMENT.
///
extern void KRR_FONT_render_textex(KRR_FONT* font, const char* text, GLfloat x, GLfloat y, const SIZE* area_size, int align);

///
/// Unbind vao.
///
/// \param font pointer to KRR_FONT
///
extern void KRR_FONT_unbind_vao(KRR_FONT* font);

///
/// Get rendering area size forinput text.
///
/// \param font Pointer to font
/// \param text Input text to get its rendering area size
/// \return Area in SIZE covering the rendering size
///
extern SIZE KRR_FONT_get_string_area_size(KRR_FONT* font, const char* text);

#ifdef __cplusplus
}
#endif

#endif
