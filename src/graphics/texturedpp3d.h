#ifndef KRR_TEXSHADERPROG3D_h_
#define KRR_TEXSHADERPROG3D_h_

#include "graphics/common.h"
#include "graphics/shaderprog.h"

typedef struct KRR_TEXSHADERPROG3D_
{
  // underlying shader program
  KRR_SHADERPROG* program;

  // attribute location
  GLint vertex_pos3d_location;
  GLint texcoord_location;

  // uniform color to apply to texture's pixels
  GLint texture_color_location;
  // uniform texture
  GLint texture_sampler_location;

  // projection matrix
  mat4 projection_matrix;
  GLint projection_matrix_location;

  // modelview matrix
  mat4 modelview_matrix;
  GLint modelview_matrix_location;

} KRR_TEXSHADERPROG3D;

///
/// create a new textured polygon shader.
/// it will automatically create underlying KRR_SHADERPROG for us.
/// its underlying KRR_SHADERPROG will be managed automatically, use has no need to manually free it again.
///
/// \return Newly created KRR_TEXSHADERPROG3D on heap.
///
extern KRR_TEXSHADERPROG3D* KRR_TEXSHADERPROG3D_new(void);

///
/// Free KRR_TEXSHADERPROG3D.
/// after this its underlying KRR_SHADERPROG will be freed as well.
///
/// \param program pointer to KRR_TEXSHADERPROG3D
///
extern void KRR_TEXSHADERPROG3D_free(KRR_TEXSHADERPROG3D* program);

///
/// load program
///
/// \param program pointer to KRR_TEXSHADERPROG3D
/// \return true if load successfully, otherwise retrurn false.
///
extern bool KRR_TEXSHADERPROG3D_load_program(KRR_TEXSHADERPROG3D* program);

///
/// update projection matrix
///
/// \param program pointer to KRR_TEXSHADERPROG3D
///
extern void KRR_TEXSHADERPROG3D_update_projection_matrix(KRR_TEXSHADERPROG3D* program);

///
/// update modelview matrix
///
/// \param program pointer to KRR_TEXSHADERPROG3D
///
extern void KRR_TEXSHADERPROG3D_update_modelview_matrix(KRR_TEXSHADERPROG3D* program);

///
/// set vertex pointer
///
/// \param program pointer to KRR_TEXSHADERPROG3D
/// \param stride space in bytes to the next attribute in the next element
/// \param data opaque pointer to data buffer offset
///
extern void KRR_TEXSHADERPROG3D_set_vertex_pointer(KRR_TEXSHADERPROG3D* program, GLsizei stride, const GLvoid* data);

///
/// set texcoordinate pointer
///
/// \param program pointer to KRR_TEXSHADERPROG3D
/// \param stride space in bytes to the next attribute in the next element
/// \param data opaque pointer to data buffer offset
///
extern void KRR_TEXSHADERPROG3D_set_texcoord_pointer(KRR_TEXSHADERPROG3D* program, GLsizei stride, const GLvoid* data);

///
/// set texture color.
/// color will be apply to all of texture's color.
///
/// \param program pointer to KRR_TEXSHADERPROG3D
/// \param color color
///
extern void KRR_TEXSHADERPROG3D_set_texture_color(KRR_TEXSHADERPROG3D* program, COLOR32 color);

///
/// set texture sampler to shader
///
/// \param program pointer to KRR_TEXSHADERPROG3D
/// \param sampler texture sampler name
///
extern void KRR_TEXSHADERPROG3D_set_texture_sampler(KRR_TEXSHADERPROG3D* program, GLuint sampler);

///
/// enable all attribute pointers
///
/// \param program pointer to KRR_TEXSHADERPROG3D
///
extern void KRR_TEXSHADERPROG3D_enable_attrib_pointers(KRR_TEXSHADERPROG3D* program);

///
/// disable all attribute pointers
///
/// \param program pointer to KRR_TEXSHADERPROG3D
///
extern void KRR_TEXSHADERPROG3D_disable_attrib_pointers(KRR_TEXSHADERPROG3D* program);

#endif
