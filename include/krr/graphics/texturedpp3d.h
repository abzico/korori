#ifndef KRR_TEXSHADERPROG3D_h_
#define KRR_TEXSHADERPROG3D_h_

#include "krr/graphics/common.h"
#include "krr/graphics/shaderprog.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct KRR_TEXSHADERPROG3D_
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
  GLint light_position_locations[KRR_SHADERPROG_MAX_LIGHTS];
  GLint light_color_locations[KRR_SHADERPROG_MAX_LIGHTS];
  GLint light_attenuation_locations[KRR_SHADERPROG_MAX_LIGHTS];
  GLint light_num_location;
  LIGHT lights[KRR_SHADERPROG_MAX_LIGHTS];

  // specular
  GLint shine_damper_location;
  GLint reflectivity_location;
  GLfloat shine_damper;
  GLfloat reflectivity;

  // ambient
  GLint ambient_color_location;
  vec3 ambient_color; 

  // fog
  GLint fog_enabled_location;
  bool fog_enabled; // default to false
  GLint fog_density_location;
  GLint fog_gradient_location;
  float fog_density;
  float fog_gradient;
  
  // sky color
  GLint sky_color_location;
  vec3 sky_color; // default to (0.5, 0.5, 0.5)

} KRR_TEXSHADERPROG3D;

// shared textured 3d shader-program
extern KRR_TEXSHADERPROG3D* shared_textured3d_shaderprogram;

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
/// set projection matrix (see header) first then call this function to update to GPU
///
/// \param program pointer to KRR_TEXSHADERPROG3D
///
extern void KRR_TEXSHADERPROG3D_update_projection_matrix(KRR_TEXSHADERPROG3D* program);

///
/// update view matrix
/// set view matrix information (see header) first then call this function to update to GPU
///
/// \param program pointer to KRR_TEXSHADERPROG3D
///
extern void KRR_TEXSHADERPROG3D_update_view_matrix(KRR_TEXSHADERPROG3D* program);

///
/// update model matrix
/// set model matrix information (see header) first then call this function to update to GPU
///
/// \param program pointer to KRR_TEXSHADERPROG3D
///
extern void KRR_TEXSHADERPROG3D_update_model_matrix(KRR_TEXSHADERPROG3D* program);

///
/// update shininess variables
/// set shininess information (see header) first then call this function to update to GPU.
///
/// \param program pointer to KRR_TEXSHADERPROG3D
///
extern void KRR_TEXSHADERPROG3D_update_shininess(KRR_TEXSHADERPROG3D* program);

///
/// update light information
/// this will update all lights up to maximum as defined via `KRR_SHADERPROG_MAX_LIGHTS`.
/// set light information first (see header) then call this function to update to GPU
///
/// It will also automatically update number of light as will be processed by shader.
/// For this case, it will use all lights as defined maximally.
///
/// \param program pointer to 
///
extern void KRR_TEXSHADERPROG3D_update_lights(KRR_TEXSHADERPROG3D* program);

///
/// update light information up to `num_lights`.
/// set light information first (see header) then call this function to update to GPU
///
/// It will also automatically update number of light as will be processed by shader.
/// For this case, it will set `num_lights` to be used by shader.
///
/// \param program pointer to
/// \param num_lights number of lights to update
///
extern void KRR_TEXSHADERPROG3D_update_lights_num(KRR_TEXSHADERPROG3D* program, int num_lights);

///
/// update ambient color
/// set ambient color first (see header) then call this function to update to GPU
///
/// \param program pointer to KRR_TEXSHADERPROG3D
///
extern void KRR_TEXSHADERPROG3D_update_ambient_color(KRR_TEXSHADERPROG3D* program);

///
/// update fog enabled
/// set fog enabled first (see header) then call this function to update to GPU
///
/// \param program pointer to KRR_TEXSHADERPROG3D
///
extern void KRR_TEXSHADERPROG3D_update_fog_enabled(KRR_TEXSHADERPROG3D* program);

///
/// update fog density
/// set fog density first (see header) then call this function to update to GPU
///
/// \param program pointer to KRR_TEXSHADERPROG3D
///
extern void KRR_TEXSHADERPROG3D_update_fog_density(KRR_TEXSHADERPROG3D* program);

///
/// update fog gradient
/// set fog gradient first (see header) then call this function to update to GPU
///
/// \param program poitner to KRR_TEXSHADERPROG3D
///
extern void KRR_TEXSHADERPROG3D_update_fog_gradient(KRR_TEXSHADERPROG3D* program);

///
/// update sky color
/// set sky color first (see header) then call this function to update to GPU
///
/// \param program pointer to KRR_TEXSHADERPROG3D
///
extern void KRR_TEXSHADERPROG3D_update_sky_color(KRR_TEXSHADERPROG3D* program);

///
/// set vertex pointer
///
/// \param program pointer to KRR_TEXSHADERPROG3D
/// \param stride space in bytes to the next attribute in the next element
/// \param data opaque pointer to data buffer offset
///
extern void KRR_TEXSHADERPROG3D_set_vertex_pointer(KRR_TEXSHADERPROG3D* program, GLsizei stride, const GLvoid* data);

///
/// set texcoordinate pointer of attribute vertice
///
/// \param program pointer to KRR_TEXSHADERPROG3D
/// \param stride space in bytes to the next attribute in the next element
/// \param data opaque pointer to data buffer offset
///
extern void KRR_TEXSHADERPROG3D_set_texcoord_pointer(KRR_TEXSHADERPROG3D* program, GLsizei stride, const GLvoid* data);

///
/// set normal pointer of attribute vertice
///
/// \param program pointer to KRR_TEXSHADERPROG3D
/// \param stride space in bytes to the next attribute in the next element
/// \param data opaque pointer to data buffer offset
///
extern void KRR_TEXSHADERPROG3D_set_normal_pointer(KRR_TEXSHADERPROG3D* program, GLsizei stride, const GLvoid* data);

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

#ifdef __cplusplus
}
#endif

#endif
