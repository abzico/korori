#ifndef KRR_TERRAINSHADERPROG3D_h_
#define KRR_TERRAINSHADERPROG3D_h_

#include "graphics/common.h"
#include "graphics/shaderprog.h"

typedef struct KRR_TERRAINSHADERPROG3D_
{
  // underlying shader program
  KRR_SHADERPROG* program;

  // attribute location
  GLint vertex_pos3d_location;
  GLint texcoord_location;
  GLint normal_location;

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

  // light
  GLint light_position_location;
  GLint light_color_location;
  LIGHT light;

  // specular
  GLint shine_damper_location;
  GLint reflectivity_location;
  GLfloat shine_damper;
  GLfloat reflectivity;

  // texture coord repeat
  GLint texcoord_repeat_location;
  float texcoord_repeat;

  // ambient color
  GLint ambient_color_location;
  vec3 ambient_color;

} KRR_TERRAINSHADERPROG3D;

// shared terrain 3d shader-program
extern KRR_TERRAINSHADERPROG3D* shared_terrain3d_shaderprogram;

///
/// create a new terrain polygon shader.
/// it will automatically create underlying KRR_SHADERPROG for us.
/// its underlying KRR_SHADERPROG will be managed automatically, use has no need to manually free it again.
///
/// \return Newly created KRR_TERRAINSHADERPROG3D on heap.
///
extern KRR_TERRAINSHADERPROG3D* KRR_TERRAINSHADERPROG3D_new(void);

///
/// Free KRR_TERRAINSHADERPROG3D.
/// after this its underlying KRR_SHADERPROG will be freed as well.
///
/// \param program pointer to KRR_TERRAINSHADERPROG3D
///
extern void KRR_TERRAINSHADERPROG3D_free(KRR_TERRAINSHADERPROG3D* program);

///
/// load program
///
/// \param program pointer to KRR_TERRAINSHADERPROG3D
/// \return true if load successfully, otherwise retrurn false.
///
extern bool KRR_TERRAINSHADERPROG3D_load_program(KRR_TERRAINSHADERPROG3D* program);

///
/// update projection matrix
///
/// \param program pTERRAINSHADERPROG3DEXSHADERPROG3D
///
extern void KRR_TERRAINSHADERPROG3D_update_projection_matrix(KRR_TERRAINSHADERPROG3D* program);

///
/// update view matrix
/// set view matrix information (see header) first then call this function to update to GPU
///
/// \param program pointer to KRR_TRRAINSHADERPROG3DD
///
extern void KRR_TERRAINSHADERPROG3D_update_view_matrix(KRR_TERRAINSHADERPROG3D* program);

///
/// update model matrix
/// set model matrix information (see header) first then call this function to update to GPU
///
/// \param program pointer to KRR_TERRAINSHADERPROG3D
///
extern void KRR_TERRAINSHADERPROG3D_update_model_matrix(KRR_TERRAINSHADERPROG3D* program);

///
/// update shininess variables
/// set shininess information (see header) first then call this function to update to GPU.
///
/// \param program pointer to KRR_TERRAINSHADERPROG3D
///
extern void KRR_TERRAINSHADERPROG3D_update_shininess(KRR_TERRAINSHADERPROG3D* program);

///
/// update light information
/// set light information first (see header) then call this function to update to GPU
///
/// \param program pointer to KRR_TERRAINSHADERPROG3D
///
extern void KRR_TERRAINSHADERPROG3D_update_light(KRR_TERRAINSHADERPROG3D* program);

///
/// update texture coord repeat
/// set texture coord repeat first (see header) then call this function to update to GPU
///
/// \param program pointer to KRR_TERRAINSHADERPROG3D
///
extern void KRR_TERRAINSHADERPROG3D_update_texcoord_repeat(KRR_TERRAINSHADERPROG3D* program);

///
/// update ambient color
/// set ambient color first (see header) then call this function to update to GPU
///
/// \param program pointer to KRR_TERRAINSHADERPROG3D
///
extern void KRR_TERRAINSHADERPROG3D_update_ambient_color(KRR_TERRAINSHADERPROG3D* program);

///
/// set vertex pointer
///
/// \param program pointer to KRR_TERRAINSHADERPROG3D
/// \param stride space in bytes to the next attribute in the next element
/// \param data opaque pointer to data buffer offset
///
extern void KRR_TERRAINSHADERPROG3D_set_vertex_pointer(KRR_TERRAINSHADERPROG3D* program, GLsizei stride, const GLvoid* data);

///
/// set texcoordinate pointer of attribute vertice
///
/// \param program pointer to KRR_TERRAINSHADERPROG3D
/// \param stride space in bytes to the next attribute in the next element
/// \param data opaque pointer to data buffer offset
///
extern void KRR_TERRAINSHADERPROG3D_set_texcoord_pointer(KRR_TERRAINSHADERPROG3D* program, GLsizei stride, const GLvoid* data);

///
/// set normal pointer of attribute vertice
///
/// \param program pointer to KRR_TERRAINSHADERPROG3D
/// \param stride space in bytes to the next attribute in the next element
/// \param data opaque pointer to data buffer offset
///
extern void KRR_TERRAINSHADERPROG3D_set_normal_pointer(KRR_TERRAINSHADERPROG3D* program, GLsizei stride, const GLvoid* data);

///
/// set texture sampler to shader
///
/// \param program pointer to KRR_TERRAINSHADERPROG3D
/// \param sampler texture sampler name
///
extern void KRR_TERRAINSHADERPROG3D_set_texture_sampler(KRR_TERRAINSHADERPROG3D* program, GLuint sampler);

///
/// enable all attribute pointers
///
/// \param program pointer to KRR_TERRAINSHADERPROG3D
///
extern void KRR_TERRAINSHADERPROG3D_enable_attrib_pointers(KRR_TERRAINSHADERPROG3D* program);

///
/// disable all attribute pointers
///
/// \param program pointer to KRR_TERRAINSHADERPROG3D
///
extern void KRR_TERRAINSHADERPROG3D_disable_attrib_pointers(KRR_TERRAINSHADERPROG3D* program);

#endif
