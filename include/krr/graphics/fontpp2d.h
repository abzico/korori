#ifndef KRR_FONTSHADERPROG2D_h_
#define KRR_FONTSHADERPROG2D_h_

#include "krr/graphics/common.h"
#include "krr/graphics/shaderprog.h"

typedef struct KRR_FONTSHADERPROG2D_
{
  // underlying shader program
  KRR_SHADERPROG *program;

  /// attribute locations
  GLint vertex_pos2d_location;
  GLint texture_coord_location;

  /// uniform location
  /// (internal use)
  GLint projection_matrix_location;
  GLint model_matrix_location;
  GLint texture_sampler_location;
  GLint text_color_location;

  /// matrices
  mat4 projection_matrix;
  mat4 model_matrix;

} KRR_FONTSHADERPROG2D;

///
/// create a new font shader program on heap.
/// it will also create underlying KRR_SHADERPROG and manage it automatically for its memory deallocation.
///
/// \return Newly created KRR_FONTSHADERPROG2D
///
extern KRR_FONTSHADERPROG2D* KRR_FONTSHADERPROG2D_new(void);

///
/// free font shader program
///
/// \param program pointer to KRR_FONTSHADERPROG2D
///
extern void KRR_FONTSHADERPROG2D_free(KRR_FONTSHADERPROG2D* program);

///
/// load program
///
/// \param program pointer to program
/// \return true if load successfully, otherwise false
///
extern bool KRR_FONTSHADERPROG2D_load_program(KRR_FONTSHADERPROG2D* program);

///
/// update projection matrix then to update to gpu.
///
/// \param program pointer to program
///
extern void KRR_FONTSHADERPROG2D_update_projection_matrix(KRR_FONTSHADERPROG2D* program);

///
/// update model matrix then to update to gpu.
///
/// \param program pointer to program
///
extern void KRR_FONTSHADERPROG2D_update_model_matrix(KRR_FONTSHADERPROG2D* program);

///
/// set vertex pointer then to update to gpu
///
/// \param program pointer to program
/// \param stride byte spaces til next element
/// \param data pointer to data buffer offset
///
extern void KRR_FONTSHADERPROG2D_set_vertex_pointer(KRR_FONTSHADERPROG2D* program, GLsizei stride, const GLvoid* data);

///
/// set texture coordinate pointer then to update to gpu
///
/// \param program pointer to program
/// \param stride byte spaces til next element
/// \param data pointer to data buffer offset
///
extern void KRR_FONTSHADERPROG2D_set_texcoord_pointer(KRR_FONTSHADERPROG2D* program, GLsizei stride, const GLvoid* data);

///
/// set texture sampler name then to update to gpu
///
/// \param program pointer to program
/// \param sampler sampler name to bind texture
///
extern void KRR_FONTSHADERPROG2D_set_texture_sampler(KRR_FONTSHADERPROG2D* program, GLuint sampler);

///
/// set text color, then to update to gpu.
///
/// \param program pointer to program
/// \param color text color
///
extern void KRR_FONTSHADERPROG2D_set_text_color(KRR_FONTSHADERPROG2D* program, COLOR32 color);

///
/// enable all attribute pointers
///
/// \parm program pointer to program
///
extern void KRR_FONTSHADERPROG2D_enable_attrib_pointers(KRR_FONTSHADERPROG2D* program);

///
/// disable all attribute pointers
///
/// \param program pointer to program
///
extern void KRR_FONTSHADERPROG2D_disable_attrib_pointers(KRR_FONTSHADERPROG2D* program);

#endif
