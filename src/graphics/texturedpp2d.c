#include "texturedpp2d.h"
#include <stdlib.h>
#include "foundation/common.h"

KRR_TEXSHADERPROG2D* KRR_TEXSHADERPROG2D_new()
{
  KRR_TEXSHADERPROG2D* out = malloc(sizeof(KRR_TEXSHADERPROG2D));

  // init defaults first
  out->program = NULL;
  out->vertex_pos2d_location = -1;
  out->texcoord_location = -1;
  out->texture_color_location = -1;
  out->texture_sampler_location = -1;
  glm_mat4_identity(out->projection_matrix);
  out->projection_matrix_location = -1;
  glm_mat4_identity(out->modelview_matrix);
  out->modelview_matrix_location = -1;

  // create underlying shader program
  out->program = KRR_SHADERPROG_new();
  
  return out;
}

void KRR_TEXSHADERPROG2D_free(KRR_TEXSHADERPROG2D* program)
{
  // free underlying shader program
  KRR_SHADERPROG_free(program->program);

  // free source
  free(program);
  program = NULL;
}

bool KRR_TEXSHADERPROG2D_load_program(KRR_TEXSHADERPROG2D* program)
{
  // get underlying shader program
  KRR_SHADERPROG* uprog = program->program;

  // generate program
  uprog->program_id = glCreateProgram();

  // load vertex shader
  GLuint vertex_shader = KRR_SHADERPROG_load_shader_from_file("res/shaders/texturedpp2d.vert", GL_VERTEX_SHADER);
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
  GLuint fragment_shader = KRR_SHADERPROG_load_shader_from_file("res/shaders/texturedpp2d.frag", GL_FRAGMENT_SHADER);
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
  program->modelview_matrix_location = glGetUniformLocation(uprog->program_id, "modelview_matrix");
  if (program->modelview_matrix_location == -1)
  {
    KRR_LOGW("Warning: modelview_matrix is invalid glsl variable name");
  }

  program->vertex_pos2d_location = glGetAttribLocation(uprog->program_id, "vertex_pos2d");
  if (program->vertex_pos2d_location == -1)
  {
    KRR_LOGW("Warning: vertex_pos2d is invalid glsl variable name");
  }
  program->texcoord_location = glGetAttribLocation(uprog->program_id, "texcoord");
  if (program->texcoord_location == -1)
  {
    KRR_LOGW("Warning: texcoord_location is invalid glsl variable name");
  }
  program->texture_color_location = glGetUniformLocation(uprog->program_id, "texture_color");
  if (program->texture_color_location == -1)
  {
    KRR_LOGW("Warning: texture_color is invalid glsl variable name");
  }
  program->texture_sampler_location = glGetUniformLocation(uprog->program_id, "texture_sampler");
  if (program->texture_sampler_location == -1)
  {
    KRR_LOGW("Warning: texture_sampler is invalid glsl variable name");
  }

  return true;
}

void KRR_TEXSHADERPROG2D_update_projection_matrix(KRR_TEXSHADERPROG2D* program)
{
  glUniformMatrix4fv(program->projection_matrix_location, 1, GL_FALSE, program->projection_matrix[0]);
}

void KRR_TEXSHADERPROG2D_update_modelview_matrix(KRR_TEXSHADERPROG2D* program)
{
  glUniformMatrix4fv(program->modelview_matrix_location, 1, GL_FALSE, program->modelview_matrix[0]);
}

void KRR_TEXSHADERPROG2D_set_vertex_pointer(KRR_TEXSHADERPROG2D* program, GLsizei stride, const GLvoid* data)
{
  glVertexAttribPointer(program->vertex_pos2d_location, 2, GL_FLOAT, GL_FALSE, stride, data); 
}

void KRR_TEXSHADERPROG2D_set_texcoord_pointer(KRR_TEXSHADERPROG2D* program, GLsizei stride, const GLvoid* data)
{
  glVertexAttribPointer(program->texcoord_location, 2, GL_FLOAT, GL_FALSE, stride, data);
}

void KRR_TEXSHADERPROG2D_set_texture_color(KRR_TEXSHADERPROG2D* program, COLOR32 color)
{
  glUniform4fv(program->texture_color_location, 1, (const GLfloat*)&color);
}

void KRR_TEXSHADERPROG2D_set_texture_sampler(KRR_TEXSHADERPROG2D* program, GLuint sampler)
{
  glUniform1i(program->texture_sampler_location, sampler);
}

void KRR_TEXSHADERPROG2D_enable_attrib_pointers(KRR_TEXSHADERPROG2D* program)
{
  glEnableVertexAttribArray(program->vertex_pos2d_location);
  glEnableVertexAttribArray(program->texcoord_location);
}

void KRR_TEXSHADERPROG2D_disable_attrib_pointers(KRR_TEXSHADERPROG2D* program)
{
  glDisableVertexAttribArray(program->vertex_pos2d_location);
  glDisableVertexAttribArray(program->texcoord_location);
}
