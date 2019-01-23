#ifndef KRR_TEXTURE_h_
#define KRR_TEXTURE_h_

#include <stdbool.h>
#include "graphics/common.h"
#include "graphics/types.h"

/// global shared variable that all instance of KRR_TEXTURE will use
struct KRR_TEXSHADERPROG2D_;
extern struct KRR_TEXSHADERPROG2D_* shared_textured_shaderprogram;

typedef struct 
{
  /// texture id
  GLuint texture_id;

  /// texture width
  /// it's real width used for this texture if texture is not POT (power-of-two)
  int width;

  /// texture height
  /// it's real width used for this texture if texture is not POT (power-of-two)
  int height;

  /// pixels data of 32-bit in RBGA  of this texture
  GLuint* pixels;
  /// pixels data of 8-bit of this texture
  GLubyte* pixels8;

  // pixel format
  GLuint pixel_format;

  /// (read-only)
  /// real physical texture width in memory
  /// note: if texture is not POT then value will be different from 'width' as it will be in POT
  /// Internal system will determine next POT to assign to width and this is what 'physical_width_' is in
  /// case of NPOT texture
  int physical_width_;

  /// (read-only)
  /// real physical texture height in memory
  /// note: if texture is not POT then value will be different from 'height' as it will be in POT
  /// Internal system will determine next POT to assign to width and this is what 'physical_width_' is in
  /// case of NPOT texture
  int physical_height_;

  // VBO
  GLuint VBO_id;

  // IBO
  GLuint IBO_id;
} KRR_TEXTURE;

///
/// Create a new texture.
///
/// \return Newly created KRR_TEXTURE on heap.
///
extern KRR_TEXTURE* KRR_TEXTURE_new();

///
/// Free texture.
///
/// \param texture KRR_TEXTURE to be freed.
///
extern void KRR_TEXTURE_free(KRR_TEXTURE* texture);

///
/// Load texture from file.
/// Currently only load in format of RGBA8888 for now.
/// Make sure input file format is in RGBA8888.
///
/// \param texture Pointer to KRR_TEXTURE
/// \param path Path to texture file to load
/// \return True if load successfully, otherwise return false.
///
extern bool KRR_TEXTURE_load_texture_from_file(KRR_TEXTURE* texture, const char* path);

///
/// Load texture with extra parameters
/// Currently only load in format of RGBA8 for now.
///
/// \param texture Pointer to KRR_TEXTURE
/// \param path Path to texture file to load
/// \param color_key Packed color key value in RGBA. This is a target color to replace it with transparent color in texture.
/// \return True if load successfully, otherwise return false.
///
extern bool KRR_TEXTURE_load_texture_from_file_ex(KRR_TEXTURE* texture, const char* path, GLuint color_key);

///
/// Load compressed texture, DDS from file.
/// Input file needs to be in .dds (DXT1, DXT3, or DXT5) format.
/// To be loaded texture needs to be in POT (power-of-two) texture, and square.
///
/// \param texture Pointer to KRR_TEXTURE
/// \param path Path to texture file to load
/// \return True if load successfully, otherwise return false.
///
extern bool KRR_TEXTURE_load_dds_texture_from_file(KRR_TEXTURE* texture, const char* path);

///
/// Load pixels 32-bit (8 bit per pixel) data into texture.
///
/// \param texture KRR_TEXTURE to load input pixels data into it.
/// \param pixels Pixels data; 32-bit, 8-bit per pixel
/// \param width Width of input pixels data
/// \param height Height of input pixels data
/// \return True if loading is successful, otherwise return false.
///
extern bool KRR_TEXTURE_load_texture_from_pixels32(KRR_TEXTURE* texture, GLuint* pixels, GLuint width, GLuint height);

///
/// Render texture.
///
/// \param texture KRR_TEXTURE to render
/// \param x Position x to render
/// \param y Position y to render
/// \param clip Clipping rectangle to render part of the texture. NULL to render fully of texture.
///
extern void KRR_TEXTURE_render(KRR_TEXTURE* texture, GLfloat x, GLfloat y, const RECT* clip);

///
/// Lock texture to manipulate pixel data.
/// Make sure your texture is in RGBA8 format. If it is not, then this will change your
/// image's format unexpectedly.
///
/// \param texture Pointer to KRR_TEXTURE
/// \return True if lock successfully, otherwise return false.
///
extern bool KRR_TEXTURE_lock(KRR_TEXTURE* texture);

///
/// Unlock texture, pushing back manipulated pixel data back to GPU.
/// Make sure your texture is in RGBA8 format. If it is not, then this will change your
/// image's format unexpectedly.
///
/// \param texture Pointer to KRR_TEXTURE
/// \return True if unlock successfully, otherwise return false.
///
extern bool KRR_TEXTURE_unlock(KRR_TEXTURE* texture);

///
/// Set pixel data at position x,y.
/// Only for RGBA8 pixel format.
///
/// \param texture Pointer to KRR_TEXTURE
/// \param x X position (index-based) to set pixel value at
/// \param y Y position (index-based) to set pixel value at
/// \param value Pixel value to set. It's packed in 32 bit value of RGBA.
///
extern void KRR_TEXTURE_set_pixel32(KRR_TEXTURE* texture, GLuint x, GLuint y, GLuint pixel);

///
/// Get pixel value at position x,y.
/// Only for RGBA8 format.
///
/// \param texture Pointer to KRR_TEXTURE
/// \param x Position x (index-based) to get pixel value from
/// \param y Position y (index-baesd) to get pixel value from
/// \return Pixel value packed as format RGBA8888
extern GLuint KRR_TEXTURE_get_pixel32(KRR_TEXTURE* texture, GLuint x, GLuint y);

///
/// Set pixel data at position x,y.
/// Only for 8-bit pixel format.
///
/// \param texture Pointer to KRR_TEXTURE
/// \param x X position (index-based) to set pixel value at
/// \param y Y position (index-based) to set pixel value at
/// \param value Pixel value to set. It's a byte color value.
///
extern void KRR_TEXTURE_set_pixel8(KRR_TEXTURE* texture, GLuint x, GLuint y, GLubyte pixel);

///
/// Get pixel value at position x,y.
/// Only for 8-bit format.
///
/// \param texture Pointer to KRR_TEXTURE
/// \param x Position x (index-based) to get pixel value from
/// \param y Position y (index-based) to get pixel value from
/// \return Pixel value as byte color.
///
extern GLubyte KRR_TEXTURE_get_pixel8(KRR_TEXTURE* texture, GLuint x, GLuint y);

///
/// Create blank canvas pixel space in format of RGBA 32-bit image.
///
/// \param texture Pointer to KRR_TEXTURE
/// \param image_width Image width for canvas space to create
/// \param image_height Image height for canvas space to create
///
extern void KRR_TEXTURE_create_pixels32(KRR_TEXTURE* texture, GLuint image_width, GLuint image_height);

///
/// Copy pixels into texture
/// This function will free previous texture data if any, then recreate it again.
/// Texture has no need to be pre-created in blank.
///
/// \param texture Pointer to KRR_TEXTURE
/// \param pixels Source pixels to copy into texture
/// \param image_width Image width of source pixels
/// \param image_height Image height of source pixels
///
extern void KRR_TEXTURE_copy_pixels32(KRR_TEXTURE* texture, GLuint* pixels, GLuint image_widtth, GLuint image_height);

///
/// Pad texture's pixels data to be in power-of-two (POT) dimensions.
///
/// \param texture Pointer to KRR_TEXTURE
///
extern void KRR_TEXTURE_pad_pixels32(KRR_TEXTURE* texture);

///
/// Create blank canvas of pixels space in fromat of 8-bit grayscale color.
///
/// \param texture Pointer to KRR_TEXTURE
/// \param image_width Image width for canvas space to create
/// \param image_height Image height for canvas space to create
///
extern void KRR_TEXTURE_create_pixels8(KRR_TEXTURE* texture, GLuint image_width, GLuint imgae_height);

///
/// Copy pixels (in 8-bit grayscale format) into texture.
/// This function will free previous texture data if any, then recreate it again.
///
/// \param texture Pointer to KRR_TEXTURE
/// \param pixels Source pixels to copy into texture. It should be in 8-bit grayscale format.
/// \param image_width Image width of source pixels
/// \param image_height Image height of source pixels
///
extern void KRR_TEXTURE_copy_pixels8(KRR_TEXTURE* texture, GLubyte* pixels, GLuint image_width, GLuint image_height);

///
/// Pad texture's pixels data to be in power-of-two (POT) dimensions.
///
/// \param Pointer to texture
///
extern void KRR_TEXTURE_pad_pixels8(KRR_TEXTURE* texture);

///
/// Copy source pixels into destination texture at position x, and y.
/// Required that both texture and dst_texture are loaded with pixel data.
///
/// \param texture Pointer to KRR_TEXTURE of source texture to copy pixels from
/// \param dst_x Destination x of destination texture to place source pixels at
/// \param dst_y Destination y of destination texture to place source pixels at
/// \parm dst_texture Destination texture to place pixels
///
extern void KRR_TEXTURE_blit_pixels32(KRR_TEXTURE* texture, GLuint dst_x, GLuint dst_y, const KRR_TEXTURE* dst_texture);

///
/// Copy source pixels into destination texture at position x, and y.
/// This function works with 8-bit grayscale image format.
/// Required that both texture and dst_texture are loaded with pixel data.
///
/// \param texture Pointer to KRR_TEXTURE of source texture to copy pixels from
/// \param dst_x Destination x of destination texture to place source pixels at
/// \param dst_y Destination y of destination texture to place source pixels at
/// \param dst_texture Destination texture to place pixels
///
extern void KRR_TEXTURE_blit_pixels8(KRR_TEXTURE* texture, GLuint dst_x, GLuint dst_y, const KRR_TEXTURE* dst_texture);

#endif
