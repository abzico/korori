#ifndef KRR_FONT_polygon_program2d_h_
#define KRR_FONT_polygon_program2d_h_

#include "graphics/common.h"
#include "graphics/shaderprog.h"

typedef struct KRR_FONT_polygon_program2d_
{
  // underlying shader program
  KRR_SHADERPROG *program;

  /// attribute locations
  GLint vertex_pos2d_location;
  GLint texture_coord_location;

  /// uniform location
  /// (internal use)
  GLint projection_matrix_location;
  GLint modelview_matrix_location;
  GLint texture_sampler_location;
  GLint text_color_location;

  /// matrices
  mat4 projection_matrix;
  mat4 modelview_matrix;

} KRR_FONT_polygon_program2d;

///
/// create a new font shader program on heap.
/// it will also create underlying KRR_SHADERPROG and manage it automatically for its memory deallocation.
///
/// \return Newly created KRR_FONT_polygon_program2d
///
extern KRR_FONT_polygon_program2d* KRR_FONT_polygon_program2d_new();

///
/// free font shader program
///
/// \param program pointer to KRR_FONT_polygon_program2d
///
extern void KRR_FONT_polygon_program2d_free(KRR_FONT_polygon_program2d* program);

///
/// load program
///
/// \param program pointer to program
/// \return true if load successfully, otherwise false
///
extern bool KRR_FONT_polygon_program2d_load_program(KRR_FONT_polygon_program2d* program);

///
/// update projection matrix then to update to gpu.
///
/// \param program pointer to program
///
extern void KRR_FONT_polygon_program2d_update_projection_matrix(KRR_FONT_polygon_program2d* program);

///
/// update modelview matrix then to update to gpu.
///
/// \param program pointer to program
///
extern void KRR_FONT_polygon_program2d_update_modelview_matrix(KRR_FONT_polygon_program2d* program);

///
/// set vertex pointer then to update to gpu
///
/// \param program pointer to program
/// \param stride byte spaces til next element
/// \param data pointer to data buffer offset
///
extern void KRR_FONT_polygon_program2d_set_vertex_pointer(KRR_FONT_polygon_program2d* program, GLsizei stride, const GLvoid* data);

///
/// set texture coordinate pointer then to update to gpu
///
/// \param program pointer to program
/// \param stride byte spaces til next element
/// \param data pointer to data buffer offset
///
extern void KRR_FONT_polygon_program2d_set_texcoord_pointer(KRR_FONT_polygon_program2d* program, GLsizei stride, const GLvoid* data);

///
/// set texture sampler name then to update to gpu
///
/// \param program pointer to program
/// \param sampler sampler name to bind texture
///
extern void KRR_FONT_polygon_program2d_set_texture_sampler(KRR_FONT_polygon_program2d* program, GLuint sampler);

///
/// set text color, then to update to gpu.
///
/// \param program pointer to program
/// \param color text color
///
extern void KRR_FONT_polygon_program2d_set_text_color(KRR_FONT_polygon_program2d* program, COLOR32 color);

///
/// enable all attribute pointers
///
/// \parm program pointer to program
///
extern void KRR_FONT_polygon_program2d_enable_attrib_pointers(KRR_FONT_polygon_program2d* program);

///
/// disable all attribute pointers
///
/// \param program pointer to program
///
extern void KRR_FONT_polygon_program2d_disable_attrib_pointers(KRR_FONT_polygon_program2d* program);

#endif
