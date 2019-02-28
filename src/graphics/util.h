#ifndef KRR_gputil_h_
#define KRR_gputil_h_

#include "graphics/common.h"

/// Utility functions to work with OpenGL

///
/// Adapt projection matrix to normal.
///
/// \param screen_width Screen width
/// \param screen_height Screen height
///
extern void KRR_gputil_adapt_to_normal(int screen_width, int screen_heght);

///
/// Adapt projection matrix to letterbox.
/// It can do a letterbox either on horizontal or vertical direction.
/// It depends on user's input screen and logical dimentions, and code to interpret them.
///
/// \param screen_width Screen width
/// \param screen_height Screen height
/// \param logical_width Logical width for game logic
/// \param logical_height Logical height for game logic
/// \param view_width Result of new screen width
/// \param view_height Result of new screen height
/// \param offset_x Offset x on screen for main content to be rendered. If NULL, value won't get return.
/// \param offset_y Offset y on screen for main content ot be rendered. If NULL, value won't get return.
///
extern void KRR_gputil_adapt_to_letterbox(int screen_width, int screen_height, int logical_width, int logical_height, int* view_width, int* view_height, int* offset_x, int* offset_y);

///
/// Get error string.
///
/// \param error GLenum
/// \return Error string. You should not modify or free this string.
///
extern const char* KRR_gputil_error_string(GLenum error);

///
/// Map color from RGBA to ABGR.
///
/// \param color Input color in RGBA
/// \return Mapped color in ABGR
///
extern GLuint KRR_gputil_map_color_RGBA_to_ABGR(GLuint color);

///
/// check and print any error so far for opengl
/// if opengl has any error, it will print error message on screen then return such error code
///
/// \param prefix prefix text to print. can be NULL.
/// \return error if any, or GL_NO_ERROR if no
///
extern GLenum KRR_gputil_anyerror(const char* prefix);

///
/// Update matrix at the location then issue update to GPU.
///
/// \param location location of uniform variable in shader code
/// \param matrix modelview matrix to update
///
extern void KRR_gputil_update_matrix(GLint location, mat4 matrix);

///
/// Enable vertex attribute pointers from input variable of locations.
/// Specify -1 to end the variadic input.
///
/// \param location location to enable in variadic.
///
extern void KRR_gputil_enable_vertex_attrib_pointers(GLint location, ...);

///
/// Disable vertex attribute pointers from input variable of locations.
/// Specify -1 to end the variadic input.
///
/// \param location location to disable in variadic.
///
extern void KRR_gputil_disable_vertex_attrib_pointers(GLint location, ...);

///
/// Create view matrix from input parameters
/// Note: It won't update for roll rotation even if you supply rotation value there in rot.
///
/// \param trans translation
/// \param rot rotation angle, in degrees
/// \param scale uniformed scale
/// \param dst destination matrix to receive result
///
extern void KRR_gputil_create_view_matrix(vec3 trans, vec3 rot, float scale, mat4 dst);

///
/// Generate mipmaps for the `target`.
/// This will also set the proper texture filtering to get good balanced result.
/// This will take into effect for the current active texture unit.
///
/// The reason why this function exists is to keep memory usage low, so call this
/// after loading texture as necessary to generate mipmap stack.
///
/// \param target target to generate mipmaps.
/// \param lod_bias level of detail bias value. This value will be set to GL_TEXTURE_LOD_BIAS. Normal value is 0.0, so if you don't want to configure this value, set it to 0.0.
///
extern void KRR_gputil_generate_mipmaps(GLenum target, float lod_bias);

#endif
