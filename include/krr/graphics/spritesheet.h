#ifndef KRR_TEXTURE_spritesheet_h_
#define KRR_TEXTURE_spritesheet_h_

#include "krr/graphics/common.h"
#include "krr/graphics/types.h"
#include "krr/graphics/texture.h"
#include <vector/vector.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  KRR_TEXTURE* ltexture;
  vector* clips;
  
  /// (internal use)
  GLuint vertex_data_buffer;
  /// (internal use)
  GLuint* index_buffers;
  /// (internal use)
  GLuint vao;
} KRR_SPRITESHEET;

///
/// Create a new KRR_SPRITESHEET.
/// Underlying KRR_TEXTURE will be taken care of then done using this via call KRR_SPRITESHEET_free()
///
/// \return Newly created KRR_SPRITESHEET
///
extern KRR_SPRITESHEET* KRR_SPRITESHEET_new(void);

///
/// Free spritesheeet.
/// It will also free its KRR_TEXTURE.
///
/// \param spritesheet Pointer to spritesheet
///
extern void KRR_SPRITESHEET_free(KRR_SPRITESHEET* spritesheet);

///
/// Add a new clipping for sprite inside spritesheet.
///
/// \param spritesheet Pointer to KRR_SPRITESHEET
/// \param new_clip New clipping rectangle to get sprite inside spritesheet
/// \return Index for newly added clipping sprite
///
extern int KRR_SPRITESHEET_add_clipsprite(KRR_SPRITESHEET* spritesheet, const RECT* new_clip);

///
/// Get clipping from specified index.
///
/// \param spritesheet Pointer to KRR_SPRITESHEET
/// \param index Index to get clipping rectangle
/// \return Clipping rectangle as RECT for specified index
///
extern RECT KRR_SPRITESHEET_get_clip(KRR_SPRITESHEET* spritesheet, int index);

///
/// Generate data buffer preparing for rendering.
///
/// \param spritesheet Pointer to KRR_SPRITESHEET
/// \return True if successfully generated, otherwise return false.
///
extern bool KRR_SPRITESHEET_generate_databuffer(KRR_SPRITESHEET* spritesheet);

///
/// Free VBO, IBO and all clipping array that used in rendering by the sheet.
///
/// \param spriteshet Pointer to KRR_SPRITESHEET
///
extern void KRR_SPRITESHEET_free_sheet(KRR_SPRITESHEET* spritesheet);

///
/// Bind vao.
///
/// It's not necessary to always call KRR_TEXTURE_unbind_vao() if next call is KRR_TEXTURE_bind_vao() with different KRR_TEXTURE.
/// It will be overwritely set the active VAO.
///
/// \param spritesheet pointer to KRR_SPRITESHEET
///
extern void KRR_SPRITESHEET_bind_vao(KRR_SPRITESHEET* spritesheet);

///
/// Render sprite from specified index.
///
/// \param spritesheet Pointer to KRR_SPRITESHEET
/// \param index Index representing sprite to render
/// \param x X position to render
/// \param y Y position to render
///
extern void KRR_SPRITESHEET_render_sprite(KRR_SPRITESHEET* spritesheet, int index, GLfloat x, GLfloat y);

///
/// Unbind vao.
///
/// \param spritesheet pointer to KRR_SPRITESHEET
///
extern void KRR_SPRITESHEET_unbind_vao(KRR_SPRITESHEET* spriteshet);

#ifdef __cplusplus
}
#endif

#endif
