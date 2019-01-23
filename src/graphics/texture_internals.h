#ifndef KRR_TEXTURE_internals_h_
#define KRR_TEXTURE_internals_h_

#include "texture.h"
#include <stdbool.h>

/// these API meant to be used internally by library only (not limited to only KRR_TEXTURE.c), not expose to user.
/// yeah technically, users can use if include this header, but you need to know what you're doing with these APIs.
/// Implementation source file is in KRR_TEXTURE.c.

///
/// Free internal texture.
///
/// \param texture Pointer to KRR_TEXTURE
///
extern void KRR_TEXTURE_free_internal_texture(KRR_TEXTURE* texture);

///
/// Load pixels data from file and set such pixel data into input KRR_TEXTURE
/// Use this to load 32-bit pixel image.
/// After texture is loaded then pixel data is set to texture, you have no need to lock it first.
///
/// \param texture Pointer to KRR_TEXTURE
/// \param path Image path to load pixels
/// \return True if successfully load, otherwise return false
///
extern bool KRR_TEXTURE_load_pixels_from_file(KRR_TEXTURE* texture, const char* path);

///
/// Load pixels data from file and set such pixel data into input KRR_TEXTURE.
/// Use this to load 8-bit pixel image.
/// After texture is loaded then pixel data is set to texture, you have no need to lock it first.
///
/// \param texture Pointer to KRR_TEXTURE
/// \param path Image path to load pixels
/// \return True if successfully load, otherwise return false.
///
extern bool KRR_TEXTURE_load_pixels_from_file8(KRR_TEXTURE* texture, const char* path);

///
/// Load texture from pre-created pixels (via KRR_TEXTURE_load_pixels_from_file())
/// This function will create texture from loaded pixels as already set into KRR_TEXTURE internally.
///
/// \parama texture Pointer to KRR_TEXTURE
/// \return True if successfully load, otherwise return false.
///
extern bool KRR_TEXTURE_load_texture_from_precreated_pixels32(KRR_TEXTURE* texture);

///
/// Load texture from pre-created pixels 8-bit (via KRR_TEXTURE_load_pixels_from_file8())
///
/// \param texture Pointer to KRR_TEXTURE
/// \return True if successfully load, otherwise return false.
///
extern bool KRR_TEXTURE_load_texture_from_precreated_pixels8(KRR_TEXTURE* texture);

#endif
