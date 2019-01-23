#ifndef KRR_DMULTICSHADERPROG2D_h_
#define KRR_DMULTICSHADERPROG2D_h_

#include "graphics/common.h"
#include "graphics/shaderprog.h"

typedef struct KRR_DMULTICSHADERPROG2D_
{
  /// underlying shader program
  KRR_SHADERPROG* program;

  /// attribute location
  GLint vertex_pos2d_location;
  GLint multicolor1_location;
  GLint multicolor2_location;

  /// uniform location
  /// (internal use)
  GLint projection_matrix_location;
  GLint modelview_matrix_location;

  // matrices
  mat4 projection_matrix;
  mat4 modelview_matrix;

} KRR_DMULTICSHADERPROG2D;

///
/// create a new double multi-color shader program.
/// it will also create and manage underlying program (KRR_SHADERPROG).
/// user has no responsibility to free its underlying attribute again.
///
/// \return Newly created KRR_DMULTICSHADERPROG2D on heap.
///
extern KRR_DMULTICSHADERPROG2D* KRR_DMULTICSHADERPROG2D_new();

///
/// Free internals
///
/// \param program pointer to KRR_DMULTICSHADERPROG2D
///
extern void KRR_DMULTICSHADERPROG2D_free_internals(KRR_DMULTICSHADERPROG2D* program);

///
/// free double multi-color shader program.
///
/// \param program pointer to gl_ldouble_multicolor_polygon
extern void KRR_DMULTICSHADERPROG2D_free(KRR_DMULTICSHADERPROG2D* program);

///
/// load program
///
/// \param program pointer to KRR_DMULTICSHADERPROG2D
///
extern bool KRR_DMULTICSHADERPROG2D_load_program(KRR_DMULTICSHADERPROG2D* program);

///
/// Enable all vertex attribute pointers
///
/// \param program pointer to program
#define KRR_DMULTICSHADERPROG2D_enable_all_vertex_attrib_pointers(program) KRR_gputil_enable_vertex_attrib_pointers(program->vertex_pos2d_location, program->multicolor1_location, program->multicolor2_location, -1)

///
/// Disable all vertex attribute pointers
///
/// \param program pointer to program
///
#define KRR_DMULTICSHADERPROG2D_disable_all_vertex_attrib_pointers(program) KRR_gputil_disable_vertex_attrib_pointers(program->vertex_pos2d_location, program->multicolor1_location, program->multicolor2_location, -1)

/// set vertex pointer (packed version)
/// it will set stride as 0 as packed format.
/// if caller intend to use a single VBO combining several vertex data type together then this function is not the one you're looking for.
/// data - offset pointer to data
#define KRR_DMULTICSHADERPROG2D_set_attrib_vertex_pos2d_pointer_packed(program, data) glVertexAttribPointer(program->vertex_pos2d_location, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)data)

/// set attrib multicolor either 1 or 2 (packed version)
/// program - shader program
/// color - 1 for multicolor-1, 2 for multicolor-2
/// data - offset pointer to data
#define KRR_DMULTICSHADERPROG2D_set_attrib_multicolor_pointer_packed(program, color, data) glVertexAttribPointer(color == 1 ? program->multicolor1_location : program->multicolor2_location, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)data)

#endif
