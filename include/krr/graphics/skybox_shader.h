#ifndef KRR_SKYBOXSHADERPROG_h_
#define KRR_SKYBOXSHADERPROG_h_

#include "krr/graphics/common.h"
#include "krr/graphics/shaderprog.h"

typedef struct KRR_SKYBOXSHADERPROG_S
{
  // underlying shader program
  KRR_SHADERPROG* program;

  // attribute location
  GLint vertex_pos3d_location;

  // uniform cubemap texture
  GLint cubemap_sampler_location;
  // fog color
  GLint fog_color_location;
  vec3 fog_color;

  // color transitions limits
  // to make it seamless for scene and skybox
  GLint ctrans_limits_location;
  vec2 ctrans_limits;

  // projection matrix
  mat4 projection_matrix;
  GLint projection_matrix_location;

  // view matrix
  mat4 view_matrix;
  GLint view_matrix_location;

} KRR_SKYBOXSHADERPROG;

/// shared skybox shader-program
extern KRR_SKYBOXSHADERPROG* shared_skybox_shaderprogram;

///
/// Create a new skybox shader-program
///
/// \return newly created skybox shader-program on heap
///
extern KRR_SKYBOXSHADERPROG* KRR_SKYBOXSHADERPROG_new(void);

///
/// Free skybox shader-program.
///
/// \param program skybox shader-program to free
///
extern void KRR_SKYBOXSHADERPROG_free(KRR_SKYBOXSHADERPROG* program);

///
/// Load skybox shader-program
///
/// \param program skybox shader-program to load
/// \return true if load successfully, otherwise return false.
///
extern bool KRR_SKYBOXSHADERPROG_load_program(KRR_SKYBOXSHADERPROG* program);

///
/// Update projection matrix
/// set projection matrix (see header) first then call this function to update to GPU
///
/// \param program skybox shader-program
///
extern void KRR_SKYBOXSHADERPROG_update_projection_matrix(KRR_SKYBOXSHADERPROG* program);

///
/// update view matrix
/// set view matrix (see header) first then call this function to update to GPU
///
/// \param program skybox shader-program
///
extern void KRR_SKYBOXSHADERPROG_update_view_matrix(KRR_SKYBOXSHADERPROG* program);

///
/// set vertex pointer
///
/// \param program pointer to KRR_SKYBOXSHADERPROG
/// \param stride space in bytes to the next attribute in the next element
/// \param data opaque pointer to data buffer offset
///
extern void KRR_SKYBOXSHADERPROG_set_vertex_pointer(KRR_SKYBOXSHADERPROG* program, GLsizei stride, const GLvoid* data);

///
/// set cubemap sampler to shader
///
/// \param program pointer to KRR_SKYBOXSHADERPROG
/// \param sampler cubemap sampler name
///
extern void KRR_SKYBOXSHADERPROG_set_cubemap_sampler(KRR_SKYBOXSHADERPROG* program, GLuint sampler);

///
/// update fog color to GPU
/// set fog color (see header) first then call this function to update to GPU
///
/// \param program skybox shader-program
///
extern void KRR_SKYBOXSHADERPROG_update_fog_color(KRR_SKYBOXSHADERPROG* program);

///
/// update color transitioning limits (lower, and upper)
/// set color transitioning limits (see header) first then call this function to update to GPU
///
/// \param program skybox shader-program
///
extern void KRR_SKYBOXSHADERPROG_update_ctrans_limits(KRR_SKYBOXSHADERPROG* program);

///
/// enable all attribute pointers
///
/// \param program pointer to KRR_SKYBOXSHADERPROG
///
extern void KRR_SKYBOXSHADERPROG_enable_attrib_pointers(KRR_SKYBOXSHADERPROG* program);

/// disable all attribute pointers
///
/// \param program pointer to KRR_SKYBOXSHADERPROG
///
extern void KRR_SKYBOXSHADERPROG_disable_attrib_pointers(KRR_SKYBOXSHADERPROG* program);

#endif
