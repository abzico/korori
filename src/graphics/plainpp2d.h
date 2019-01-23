#ifndef KRR_PLAINSHADERPROG2D_h_
#define KRR_PLAINSHADERPROG2D_h_

#include "graphics/common.h"
#include "graphics/shaderprog.h"
#include <stdlib.h>

typedef struct
{
  // underlying shader program pointer
  KRR_SHADERPROG* program;

  // color uniform location
  // (internal use, read-only)
  GLint polygon_color_location;

  // projection matrix
  mat4 projection_matrix;

  // (internal use)
  // projection matrix location in shader
  GLint projection_matrix_location;

  // modelview matrix
  mat4 modelview_matrix;

  // (internal use)
  // modelview matrix location in shader
  GLint modelview_matrix_location;

} KRR_PLAINSHADERPROG2D;

///
/// Create a new KRR_PLAINSHADERPROG2D
/// Underlying KRR_SHADERPROG will be automatically managed by clearing memory when done.
///
/// \param program Underlying pointer to KRR_SHADERPROG
/// \return Newly created KRR_PLAINSHADERPROG2D
///
extern KRR_PLAINSHADERPROG2D* KRR_PLAINSHADERPROG2D_new(KRR_SHADERPROG* program);

///
/// Free KRR_PLAINSHADERPROG2D
///
/// \param program Pointer to KRR_PLAINSHADERPROG2D
///
extern void KRR_PLAINSHADERPROG2D_free(KRR_PLAINSHADERPROG2D* program);

///
/// Load program
///
/// \param program Program id
///
extern bool KRR_PLAINSHADERPROG2D_load_program(KRR_PLAINSHADERPROG2D* program);

///
/// Set polygon color
///
/// \param program Pointer to KRR_PLAINSHADERPROG2D
/// \param r Red color component between 0.0-1.0
/// \param g Green color component between 0.0-1.0
/// \param b Blue color component between 0.0-1.0
/// \param a Alpha color component between 0.0-1.0
///
extern void KRR_PLAINSHADERPROG2D_set_color(KRR_PLAINSHADERPROG2D* program, GLfloat r, GLfloat g, GLfloat b, GLfloat a);

///
/// Update projection matrix to shader with projection matrix that has been set.
///
/// \param program Pointer to KRR_PLAINSHADERPROG2D
///
extern void KRR_PLAINSHADERPROG2D_update_projection_matrix(KRR_PLAINSHADERPROG2D* program);

///
/// Update modelview matrix to shader with modelview matrix that has been set.
///
/// \param program Pointer to KRR_PLAINSHADERPROG2D
///
extern void KRR_PLAINSHADERPROG2D_update_modelview_matrix(KRR_PLAINSHADERPROG2D* program);

#endif
