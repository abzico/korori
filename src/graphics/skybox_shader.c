#include "krr/graphics/skybox_shader.h"
#include <stdlib.h>
#include <string.h>
#include "krr/foundation/log.h"

// this should be set once in user's program
KRR_SKYBOXSHADERPROG* shared_skybox_shaderprogram = NULL;

KRR_SKYBOXSHADERPROG* KRR_SKYBOXSHADERPROG_new()
{
  KRR_SKYBOXSHADERPROG* out = malloc(sizeof(KRR_SKYBOXSHADERPROG));

  // init defaults first
  out->program = NULL;
  out->vertex_pos3d_location = -1;
  out->cubemap_sampler_location = -1;
  out->fog_color_location = -1;
  glm_vec3_zero(out->fog_color);
  out->ctrans_limits_location = -1;
  out->ctrans_limits[0] = 0.0f; // initially set to (0.0f, 1.0f) to avoid divide by zero in shader code
  out->ctrans_limits[1] = 1.0f;
  glm_mat4_identity(out->projection_matrix);
  out->projection_matrix_location = -1;
  glm_mat4_identity(out->view_matrix);
  out->view_matrix_location = -1;

  // create underlying shader program
  out->program = KRR_SHADERPROG_new();

  return out;
}

void KRR_SKYBOXSHADERPROG_free(KRR_SKYBOXSHADERPROG* program)
{
  // free underlying shader program
  KRR_SHADERPROG_free(program->program);

  // free source
  free(program);
  program = NULL;
}

bool KRR_SKYBOXSHADERPROG_load_program(KRR_SKYBOXSHADERPROG* program)
{
  // get underlying shader program
  KRR_SHADERPROG* uprog = program->program;

  // generate program
  uprog->program_id = glCreateProgram();

  // load vertex shader
  GLuint vertex_shader = KRR_SHADERPROG_load_shader_from_file("res/shaders/skybox.vert", GL_VERTEX_SHADER);
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
  GLuint fragment_shader = KRR_SHADERPROG_load_shader_from_file("res/shaders/skybox.frag", GL_FRAGMENT_SHADER);
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
  program->cubemap_sampler_location = glGetUniformLocation(uprog->program_id, "cubemap_sampler");
  if (program->cubemap_sampler_location == -1)
  {
    KRR_LOGW("Warning: cubemap_sampler is invalid glsl variable name");
  }
  program->fog_color_location = glGetUniformLocation(uprog->program_id, "fog_color");
  if (program->fog_color_location == -1)
  {
    KRR_LOGW("Warning: fog_color is invalid glsl variable name");
  }
  program->ctrans_limits_location = glGetUniformLocation(uprog->program_id, "ctrans_limits");
  if (program->ctrans_limits_location == -1)
  {
    KRR_LOGW("Warning: ctrans_limits is invalid glsl variable name");
  }
  program->vertex_pos3d_location = glGetAttribLocation(uprog->program_id, "vertex_pos3d");
  if (program->vertex_pos3d_location == -1)
  {
    KRR_LOGW("Warning: vertex_pos3d is invalid glsl variable name");
  }

  return true;
}

void KRR_SKYBOXSHADERPROG_update_projection_matrix(KRR_SKYBOXSHADERPROG* program)
{
  glUniformMatrix4fv(program->projection_matrix_location, 1, GL_FALSE, program->projection_matrix[0]);
}

void KRR_SKYBOXSHADERPROG_update_view_matrix(KRR_SKYBOXSHADERPROG* program)
{
  // clear translation out before we upload to GPU
  // we don't want skybox to move relative to the camera, but just be there
  program->view_matrix[3][0] = 0.0f;
  program->view_matrix[3][1] = 0.0f;
  program->view_matrix[3][2] = 0.0f;
  glUniformMatrix4fv(program->view_matrix_location, 1, GL_FALSE, program->view_matrix[0]);
}

void KRR_SKYBOXSHADERPROG_set_vertex_pointer(KRR_SKYBOXSHADERPROG* program, GLsizei stride, const GLvoid* data)
{
  glVertexAttribPointer(program->vertex_pos3d_location, 3, GL_FLOAT, GL_FALSE, stride, data);
}

void KRR_SKYBOXSHADERPROG_set_cubemap_sampler(KRR_SKYBOXSHADERPROG* program, GLuint sampler)
{
  glUniform1i(program->cubemap_sampler_location, sampler);
}

void KRR_SKYBOXSHADERPROG_update_fog_color(KRR_SKYBOXSHADERPROG* program)
{
  glUniform3fv(program->fog_color_location, 1, program->fog_color);
}

void KRR_SKYBOXSHADERPROG_update_ctrans_limits(KRR_SKYBOXSHADERPROG* program)
{
  glUniform2fv(program->ctrans_limits_location, 1, program->ctrans_limits);
}

void KRR_SKYBOXSHADERPROG_enable_attrib_pointers(KRR_SKYBOXSHADERPROG* program)
{
  glEnableVertexAttribArray(program->vertex_pos3d_location);
}

void KRR_SKYBOXSHADERPROG_disable_attrib_pointers(KRR_SKYBOXSHADERPROG* program)
{
  glDisableVertexAttribArray(program->vertex_pos3d_location);
}
