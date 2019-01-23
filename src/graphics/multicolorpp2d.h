#ifndef KRR_MULTCSHADERPROG2D_h_
#define KRR_MULTCSHADERPROG2D_h_

#include "graphics/common.h"
#include "graphics/shaderprog.h"

typedef struct
{
  // underlying shader program
  KRR_SHADERPROG* program;

  // vertex position 2d location
  // (internal use)
  GLint vertex_pos2d_location;

  // multi color location
  // (internal use)
  GLint multi_color_location;

  // projection matrix
  mat4 projection_matrix;

  // projection matrix location
  // (internal use)
  GLint projection_matrix_location;

  // modelview matrix
  mat4 modelview_matrix;

  // modelview matrix location
  // (internal use)
  GLint modelview_matrix_location;

} KRR_MULTCSHADERPROG2D;

///
/// Create a new multi-color polygon program.
/// Input KRR_SHADERPROG will be automatically managed its memory freeing. User has no responsible to manually free it later.
///
/// \param program Pointer to KRR_SHADERPROG
/// \return Newly created KRR_MULTCSHADERPROG2D on heap
///
extern KRR_MULTCSHADERPROG2D* KRR_MULTCSHADERPROG2D_new(KRR_SHADERPROG* program);

///
/// Free multi-color program
///
/// \param program Pointer to KRR_MULTCSHADERPROG2D
///
extern void KRR_MULTCSHADERPROG2D_free(KRR_MULTCSHADERPROG2D* program);

///
/// Load program
///
/// \param Pointer to KRR_MULTCSHADERPROG2D
/// \return True if load successfully, otherwise return false.
///
extern bool KRR_MULTCSHADERPROG2D_load_program(KRR_MULTCSHADERPROG2D* program);

///
/// Update projection matrix by sending to shader.
///
/// \param program Pointer to KRR_MULTCSHADERPROG2D
///
extern void KRR_MULTCSHADERPROG2D_update_projection_matrix(KRR_MULTCSHADERPROG2D* program);

///
/// Update modelview matrix by sending to shader.
///
/// \param program Pointer to KRR_MULTCSHADERPROG2D
///
extern void KRR_MULTCSHADERPROG2D_update_modelview_matrix(KRR_MULTCSHADERPROG2D* program);

///
/// Set vertex pointer.
///
/// \param program Pointer to LMultiColorPolygonProgram2D
/// \param stride Size in bytes of each vertex element
/// \param data Opaque pointer to data buffer
///
extern void KRR_MULTCSHADERPROG2D_set_vertex_pointer(KRR_MULTCSHADERPROG2D* program, GLsizei stride, const GLvoid* data);

///
/// set color pointer
///
/// \param program pointer to KRR_MULTCSHADERPROG2D
/// \param size in bytes of each vertex element
/// \param opaque pointer to data buffer
///
extern void KRR_MULTCSHADERPROG2D_set_color_pointer(KRR_MULTCSHADERPROG2D* program, GLsizei stride, const GLvoid* data);

///
/// enable all relevant generic vertex array pointers to this program
///
/// \param pointer to KRR_MULTCSHADERPROG2D
///
extern void KRR_MULTCSHADERPROG2D_enable_attrib_pointers(KRR_MULTCSHADERPROG2D* program);

///
/// disable all relevant generic vertex array  pointers to this program
///
/// \param program pointer to KRR_MULTCSHADERPROG2D
///
extern void KRR_MULTCSHADERPROG2D_disable_attrib_pointers(KRR_MULTCSHADERPROG2D* program);

#endif
