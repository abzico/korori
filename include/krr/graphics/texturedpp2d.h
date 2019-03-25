#ifndef KRR_TEXSHADERPROG2D_h_
#define KRR_TEXSHADERPROG2D_h_

#include "graphics/common.h"
#include "graphics/shaderprog.h"

typedef struct KRR_TEXSHADERPROG2D_
{
  // underlying shader program
  KRR_SHADERPROG* program;

  // attribute location
  GLint vertex_pos2d_location;
  GLint texcoord_location;

  // uniform color to apply to texture's pixels
  GLint texture_color_location;
  // uniform texture
  GLint texture_sampler_location;

  // projection matrix
  mat4 projection_matrix;
  GLint projection_matrix_location;

  // view matrix
  mat4 view_matrix;
  GLint view_matrix_location;

  // model matrix
  mat4 model_matrix;
  GLint model_matrix_location;

} KRR_TEXSHADERPROG2D;

///
/// create a new textured polygon shader.
/// it will automatically create underlying KRR_SHADERPROG for us.
/// its underlying KRR_SHADERPROG will be managed automatically, use has no need to manually free it again.
///
/// \return Newly created KRR_TEXSHADERPROG2D on heap.
///
extern KRR_TEXSHADERPROG2D* KRR_TEXSHADERPROG2D_new(void);

///
/// Free KRR_TEXSHADERPROG2D.
/// after this its underlying KRR_SHADERPROG will be freed as well.
///
/// \param program pointer to KRR_TEXSHADERPROG2D
///
extern void KRR_TEXSHADERPROG2D_free(KRR_TEXSHADERPROG2D* program);

///
/// load program
///
/// \param program pointer to KRR_TEXSHADERPROG2D
/// \return true if load successfully, otherwise retrurn false.
///
extern bool KRR_TEXSHADERPROG2D_load_program(KRR_TEXSHADERPROG2D* program);

///
/// update projection matrix
///
/// \param program pointer to KRR_TEXSHADERPROG2D
///
extern void KRR_TEXSHADERPROG2D_update_projection_matrix(KRR_TEXSHADERPROG2D* program);

///
/// update view matrix
///
/// \param program pointer to KRR_TEXSHADERPROG2D
///
extern void KRR_TEXSHADERPROG2D_update_view_matrix(KRR_TEXSHADERPROG2D* program);

///
/// update model matrix
///
/// \param program pointer to KRR_TEXSHADERPROG2D
///
extern void KRR_TEXSHADERPROG2D_update_model_matrix(KRR_TEXSHADERPROG2D* program);

///
/// set vertex pointer
///
/// \param program pointer to KRR_TEXSHADERPROG2D
/// \param stride space in bytes to the next attribute in the next element
/// \param data opaque pointer to data buffer offset
///
extern void KRR_TEXSHADERPROG2D_set_vertex_pointer(KRR_TEXSHADERPROG2D* program, GLsizei stride, const GLvoid* data);

///
/// set texcoordinate pointer
///
/// \param program pointer to KRR_TEXSHADERPROG2D
/// \param stride space in bytes to the next attribute in the next element
/// \param data opaque pointer to data buffer offset
///
extern void KRR_TEXSHADERPROG2D_set_texcoord_pointer(KRR_TEXSHADERPROG2D* program, GLsizei stride, const GLvoid* data);

///
/// set texture color.
/// color will be apply to all of texture's color.
///
/// \param program pointer to KRR_TEXSHADERPROG2D
/// \param color color
///
extern void KRR_TEXSHADERPROG2D_set_texture_color(KRR_TEXSHADERPROG2D* program, COLOR32 color);

///
/// set texture sampler to shader
///
/// \param program pointer to KRR_TEXSHADERPROG2D
/// \param sampler texture sampler name
///
extern void KRR_TEXSHADERPROG2D_set_texture_sampler(KRR_TEXSHADERPROG2D* program, GLuint sampler);

///
/// enable all attribute pointers
///
/// \param program pointer to KRR_TEXSHADERPROG2D
///
extern void KRR_TEXSHADERPROG2D_enable_attrib_pointers(KRR_TEXSHADERPROG2D* program);

///
/// disable all attribute pointers
///
/// \param program pointer to KRR_TEXSHADERPROG2D
///
extern void KRR_TEXSHADERPROG2D_disable_attrib_pointers(KRR_TEXSHADERPROG2D* program);

#endif
