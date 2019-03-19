#include "texturedalphapp3d.h"
#include <stdlib.h>
#include <string.h>
#include "foundation/log.h"

// this should be set once in user's program
KRR_TEXALPHASHADERPROG3D* shared_texturedalpha3d_shaderprogram = NULL;

KRR_TEXALPHASHADERPROG3D* KRR_TEXALPHASHADERPROG3D_new(void)
{
  KRR_TEXALPHASHADERPROG3D* out = malloc(sizeof(KRR_TEXALPHASHADERPROG3D));

  // init defaults first
  out->program = NULL;
  out->vertex_pos3d_location = -1;
  out->texcoord_location = -1;
  out->normal_location = -1;
  out->texture_sampler_location = -1;
  glm_mat4_identity(out->projection_matrix);
  out->projection_matrix_location = -1;
  glm_mat4_identity(out->view_matrix);
  out->view_matrix_location = -1;
  glm_mat4_identity(out->model_matrix);
  out->model_matrix_location = -1;
	out->clipped_texcoord_location = -1;
	// all zeros means we don't set such values in shader
	glm_vec4_zero(out->clipped_texcoord);
  out->light_position_location = -1;
  out->light_color_location = -1;
  memset(&out->light.pos, 0, sizeof(out->light.pos)); 
  out->light.color.r = 1.0f;
  out->light.color.g = 1.0f;
  out->light.color.b = 1.0f;
  out->shine_damper = 1.0f;
  out->reflectivity = 0.0f;
  out->ambient_color_location = -1;
  glm_vec3_one(out->ambient_color);
  out->sky_color_location = -1;
  glm_vec3_copy((vec3){0.5f, 0.5f, 0.5f}, out->sky_color);
  out->fog_enabled_location = -1;
  out->fog_enabled = false;
  out->fog_density_location = -1;
  out->fog_gradient_location = -1;
  out->fog_density = 0.0055;
  out->fog_gradient = 1.5;

  // create underlying shader program
  out->program = KRR_SHADERPROG_new();
  
  return out;
}

void KRR_TEXALPHASHADERPROG3D_free(KRR_TEXALPHASHADERPROG3D* program)
{
  // free underlying shader program
  KRR_SHADERPROG_free(program->program);

  // free source
  free(program);
  program = NULL;
}

bool KRR_TEXALPHASHADERPROG3D_load_program(KRR_TEXALPHASHADERPROG3D* program)
{
  // get underlying shader program
  KRR_SHADERPROG* uprog = program->program;

  // generate program
  uprog->program_id = glCreateProgram();

  // load vertex shader
  GLuint vertex_shader = KRR_SHADERPROG_load_shader_from_file("res/shaders/texturedalphapp3d.vert", GL_VERTEX_SHADER);
  // check errors
  if (vertex_shader == -1)
  {
    glDeleteProgram(uprog->program_id);
    uprog->program_id = 0;
    return false;
  }

  // attach vertex shader
  glAttachShader(uprog->program_id, vertex_shader);

  // create fragment shader
  GLuint fragment_shader = KRR_SHADERPROG_load_shader_from_file("res/shaders/texturedalphapp3d.frag", GL_FRAGMENT_SHADER);
  // check errors
  if (fragment_shader == -1)
  {
    // delete vertex shader
    glDeleteShader(vertex_shader);
    vertex_shader = -1;

    // delete program
    glDeleteProgram(uprog->program_id);
    uprog->program_id = 0;
    return false;
  }

  // attach fragment shader
  glAttachShader(uprog->program_id, fragment_shader);

  // link program
  glLinkProgram(uprog->program_id);
  // check errors
  GLint link_status = GL_FALSE;
  glGetProgramiv(uprog->program_id, GL_LINK_STATUS, &link_status);
  if (link_status != GL_TRUE)
  {
    KRR_LOGE("Link program error %d", uprog->program_id);
    KRR_SHADERPROG_print_program_log(uprog->program_id);

    // delete shaders
    glDeleteShader(vertex_shader);
    vertex_shader = -1;
    glDeleteShader(fragment_shader);
    fragment_shader = -1;
    // delete program
    glDeleteProgram(uprog->program_id);
    uprog->program_id = 0;

    return false;
  }

  // clean up
  glDeleteShader(vertex_shader);
  vertex_shader = -1;
  glDeleteShader(fragment_shader);
  fragment_shader = -1;

  // get variable locations
  program->projection_matrix_location = glGetUniformLocation(uprog->program_id, "projection_matrix");
  if (program->projection_matrix_location == -1)
  {
    KRR_LOGW("Warning: projection_matrix is invalid glsl variable name");
  }
  program->view_matrix_location = glGetUniformLocation(uprog->program_id, "view_matrix");
  if (program->view_matrix_location == -1)
  {
    KRR_LOGW("Warning: view_matrix is invalid glsl variable name");
  }
  program->model_matrix_location = glGetUniformLocation(uprog->program_id, "model_matrix");
  if (program->model_matrix_location == -1)
  {
    KRR_LOGW("Warning: model_matrix is invalid glsl variable name");
  }

  program->vertex_pos3d_location = glGetAttribLocation(uprog->program_id, "vertex_pos3d");
  if (program->vertex_pos3d_location == -1)
  {
    KRR_LOGW("Warning: vertex_pos3d is invalid glsl variable name");
  }
  program->texcoord_location = glGetAttribLocation(uprog->program_id, "texcoord");
  if (program->texcoord_location == -1)
  {
    KRR_LOGW("Warning: texcoord_location is invalid glsl variable name");
  }
  program->normal_location = glGetAttribLocation(uprog->program_id, "normal");
  if (program->normal_location == -1)
  {
    KRR_LOGW("Warning: normal_location is invalid glsl variable name");
  }
  program->texture_sampler_location = glGetUniformLocation(uprog->program_id, "texture_sampler");
  if (program->texture_sampler_location == -1)
  {
    KRR_LOGW("Warning: texture_sampler is invalid glsl variable name");
  }
	program->clipped_texcoord_location = glGetUniformLocation(uprog->program_id, "packed_clip_texture_uv");
	if (program->clipped_texcoord_location == -1)
	{
		KRR_LOGW("Warning: packed_clip_texture_uv is invalid glsl variable name");
	}
  program->light_position_location = glGetUniformLocation(uprog->program_id, "light_position");
  if (program->light_position_location == -1)
  {
    KRR_LOGW("Warning: light_position is invalid glsl variable name");
  }
  program->light_color_location = glGetUniformLocation(uprog->program_id, "light_color");
  if (program->light_color_location == -1)
  {
    KRR_LOGW("Warning: light_color is invalid glsl variable name");
  }
  program->shine_damper_location = glGetUniformLocation(uprog->program_id, "shine_damper");
  if (program->shine_damper_location == -1)
  {
    KRR_LOGW("Warning: shine_damper is invalid glsl variable name");
  }
  program->reflectivity_location = glGetUniformLocation(uprog->program_id, "reflectivity");
  if (program->reflectivity_location == -1)
  {
    KRR_LOGW("Warning: reflectivity is invalid glsl variable name");
  }
  program->ambient_color_location = glGetUniformLocation(uprog->program_id, "ambient_color");
  if (program->ambient_color_location == -1)
  {
    KRR_LOGW("Warning: ambient color is invalid glsl variable name");
  }
  program->sky_color_location = glGetUniformLocation(uprog->program_id, "sky_color");
  if (program->sky_color_location == -1)
  {
    KRR_LOGW("Warning: sky_color is invalid glsl variable name");
  }
  program->fog_enabled_location = glGetUniformLocation(uprog->program_id, "fog_enabled");
  if (program->fog_enabled_location == -1)
  {
    KRR_LOGW("Warning: fog_enabled is invalid glsl variable name");
  }
  program->fog_density_location = glGetUniformLocation(uprog->program_id, "fog_density");
  if (program->fog_density_location == -1)
  {
    KRR_LOGW("Warning: fog_density is invalid glsl variable name");
  }
  program->fog_gradient_location = glGetUniformLocation(uprog->program_id, "fog_gradient");
  if (program->fog_gradient_location == -1)
  {
    KRR_LOGW("Warning: fog_gradient is invalid glsl variable name");
  }

  return true;
}

void KRR_TEXALPHASHADERPROG3D_update_projection_matrix(KRR_TEXALPHASHADERPROG3D* program)
{
  glUniformMatrix4fv(program->projection_matrix_location, 1, GL_FALSE, program->projection_matrix[0]);
}

void KRR_TEXALPHASHADERPROG3D_update_view_matrix(KRR_TEXALPHASHADERPROG3D* program)
{
  glUniformMatrix4fv(program->view_matrix_location, 1, GL_FALSE, program->view_matrix[0]);
}

void KRR_TEXALPHASHADERPROG3D_update_model_matrix(KRR_TEXALPHASHADERPROG3D* program)
{
  glUniformMatrix4fv(program->model_matrix_location, 1, GL_FALSE, program->model_matrix[0]);
}

void KRR_TEXALPHASHADERPROG3D_update_clipped_texcoord(KRR_TEXALPHASHADERPROG3D* program)
{
	glUniform4fv(program->clipped_texcoord_location, 1, &program->clipped_texcoord[0]);
}

void KRR_TEXALPHASHADERPROG3D_update_light(KRR_TEXALPHASHADERPROG3D* program)
{
  glUniform3fv(program->light_position_location, 1, &program->light.pos.x);
  glUniform3fv(program->light_color_location, 1, &program->light.color.r);
}

void KRR_TEXALPHASHADERPROG3D_update_ambient_color(KRR_TEXALPHASHADERPROG3D* program)
{
  glUniform3fv(program->ambient_color_location, 1, program->ambient_color);
}

void KRR_TEXALPHASHADERPROG3D_update_fog_enabled(KRR_TEXALPHASHADERPROG3D* program)
{
  // avoid using boolean type as it's not guarunteed to be supported by graphics card
  // see https://stackoverflow.com/a/33690786/571227
  glUniform1f(program->fog_enabled_location, program->fog_enabled ? 1.0f : 0.0f);
}

void KRR_TEXALPHASHADERPROG3D_update_fog_density(KRR_TEXALPHASHADERPROG3D* program)
{
  glUniform1f(program->fog_density_location, program->fog_density);
}

void KRR_TEXALPHASHADERPROG3D_update_fog_gradient(KRR_TEXALPHASHADERPROG3D* program)
{
  glUniform1f(program->fog_gradient_location, program->fog_gradient);
}

void KRR_TEXALPHASHADERPROG3D_update_sky_color(KRR_TEXALPHASHADERPROG3D* program)
{
  glUniform3fv(program->sky_color_location, 1, program->sky_color);
}

void KRR_TEXALPHASHADERPROG3D_update_shininess(KRR_TEXALPHASHADERPROG3D* program)
{
  glUniform1f(program->shine_damper_location, program->shine_damper);
  glUniform1f(program->reflectivity_location, program->reflectivity);
}

void KRR_TEXALPHASHADERPROG3D_set_vertex_pointer(KRR_TEXALPHASHADERPROG3D* program, GLsizei stride, const GLvoid* data)
{
  glVertexAttribPointer(program->vertex_pos3d_location, 3, GL_FLOAT, GL_FALSE, stride, data); 
}

void KRR_TEXALPHASHADERPROG3D_set_texcoord_pointer(KRR_TEXALPHASHADERPROG3D* program, GLsizei stride, const GLvoid* data)
{
  glVertexAttribPointer(program->texcoord_location, 2, GL_FLOAT, GL_FALSE, stride, data);
}

void KRR_TEXALPHASHADERPROG3D_set_normal_pointer(KRR_TEXALPHASHADERPROG3D* program, GLsizei stride, const GLvoid* data)
{
  glVertexAttribPointer(program->normal_location, 3, GL_FLOAT, GL_FALSE, stride, data);
}

void KRR_TEXALPHASHADERPROG3D_set_texture_sampler(KRR_TEXALPHASHADERPROG3D* program, GLuint sampler)
{
  glUniform1i(program->texture_sampler_location, sampler);
}

void KRR_TEXALPHASHADERPROG3D_enable_attrib_pointers(KRR_TEXALPHASHADERPROG3D* program)
{
  glEnableVertexAttribArray(program->vertex_pos3d_location);
  glEnableVertexAttribArray(program->texcoord_location);
  glEnableVertexAttribArray(program->normal_location);
}

void KRR_TEXALPHASHADERPROG3D_disable_attrib_pointers(KRR_TEXALPHASHADERPROG3D* program)
{
  glDisableVertexAttribArray(program->vertex_pos3d_location);
  glDisableVertexAttribArray(program->texcoord_location);
  glDisableVertexAttribArray(program->normal_location);
}
