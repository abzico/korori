#include "fontpp2d.h"
#include <stdlib.h>
#include "foundation/log.h"

static void free_internals_(KRR_FONTSHADERPROG2D* program);

void free_internals_(KRR_FONTSHADERPROG2D* program)
{
  // reset all locations
  program->vertex_pos2d_location = -1;
  program->texture_coord_location = -1;
  program->projection_matrix_location = -1;
  program->model_matrix_location = -1;
  program->texture_sampler_location = -1;
  program->text_color_location = -1;

  // set matrix to identity
  glm_mat4_identity(program->projection_matrix);
  glm_mat4_identity(program->model_matrix);
  
  // free underlying shader program
  KRR_SHADERPROG_free(program->program);
  program->program = NULL;
}

KRR_FONTSHADERPROG2D* KRR_FONTSHADERPROG2D_new(void)
{
  KRR_FONTSHADERPROG2D* out = malloc(sizeof(KRR_FONTSHADERPROG2D));

  // init defaults first
  out->program = NULL;
  out->vertex_pos2d_location = -1;
  out->texture_coord_location = -1;
  out->projection_matrix_location = -1;
  out->model_matrix_location = -1;
  out->texture_sampler_location = -1;
  out->text_color_location = -1;
  glm_mat4_identity(out->projection_matrix);
  glm_mat4_identity(out->model_matrix);

  // create underlying shader program
  // we will take care of this automatically when freeing
  KRR_SHADERPROG* shader_program = KRR_SHADERPROG_new();
  out->program = shader_program; 

  return out;
}

void KRR_FONTSHADERPROG2D_free(KRR_FONTSHADERPROG2D* program)
{
  // free internals
  free_internals_(program);

  // free source
  free(program);
  program = NULL;
}

bool KRR_FONTSHADERPROG2D_load_program(KRR_FONTSHADERPROG2D* program)
{
  // create a new program
  GLuint program_id = glCreateProgram();

  // load vertex shader
  GLuint vertex_shader_id = KRR_SHADERPROG_load_shader_from_file("res/shaders/fontpp2d.vert", GL_VERTEX_SHADER);
  if (vertex_shader_id == 0)
  {
    KRR_LOGE("Unable to load vertex shader from file");

    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return false;
  }

  // attach vertex shader to shader program
  glAttachShader(program_id, vertex_shader_id);
  // check for errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    KRR_LOGE("Error attaching vertex shader");
    KRR_SHADERPROG_print_shader_log(vertex_shader_id);

    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return false;
  }

  // load fragment shader
  GLuint fragment_shader_id = KRR_SHADERPROG_load_shader_from_file("res/shaders/fontpp2d.frag", GL_FRAGMENT_SHADER);
  if (fragment_shader_id == 0)
  {
    KRR_LOGE("Unable to load fragment shader from file");

    // delete vertex shader
    glDeleteShader(vertex_shader_id);
    vertex_shader_id = 0;

    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return false;
  }

  // attach fragment shader to program
  glAttachShader(program_id, fragment_shader_id);
  // check for errors
  error = glGetError();
  if (error != GL_NO_ERROR)
  {
    KRR_LOGE("Error attaching fragment shader");
    KRR_SHADERPROG_print_shader_log(fragment_shader_id);

    // delete vertex shader
    glDeleteShader(vertex_shader_id);
    vertex_shader_id = 0;

    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return false;
  }

  // link program
  glLinkProgram(program_id);
  error = glGetError();
  if (error != GL_NO_ERROR)
  {
    KRR_LOGE("Error linking program");
    KRR_SHADERPROG_print_program_log(program_id);

    // delete vertex shader
    glDeleteShader(vertex_shader_id);
    vertex_shader_id = 0;

    // delete fragment shader
    glDeleteShader(fragment_shader_id);
    fragment_shader_id = 0;

    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return false;
  }

  // set result program id to underlying program
  program->program->program_id = program_id;

  // mark shader for delete
  glDeleteShader(vertex_shader_id);
  glDeleteShader(fragment_shader_id);

  // get attribute locations
  program->vertex_pos2d_location = glGetAttribLocation(program_id, "vertex_pos2d");
  if (program->vertex_pos2d_location == -1)
  {
    KRR_LOGW("Warning: cannot get location of vertex_pos2d");
  }
  program->texture_coord_location = glGetAttribLocation(program_id, "texcoord");
  if (program->texture_coord_location == -1)
  {
    KRR_LOGW("Warning: cannot get location of texcoord");
  }

  // get uniform locations
  program->projection_matrix_location = glGetUniformLocation(program_id, "projection_matrix");
  if (program->projection_matrix_location == -1)
  {
    KRR_LOGW("Warning: cannot get location of projection_matrix");
  }
  program->model_matrix_location = glGetUniformLocation(program_id, "model_matrix");
  if (program->model_matrix_location == -1)
  {
    KRR_LOGW("Warning: cannot get location of model_matrix");
  }
  program->texture_sampler_location = glGetUniformLocation(program_id, "texture_sampler");
  if (program->texture_sampler_location == -1)
  {
    KRR_LOGW("Warning: cannot get location of texture_sampler");
  }
  program->text_color_location = glGetUniformLocation(program_id, "text_color");
  if (program->text_color_location == -1)
  {
    KRR_LOGW("Warning: cannot get location of text_color");
  }

  return true;
}

void KRR_FONTSHADERPROG2D_update_projection_matrix(KRR_FONTSHADERPROG2D* program)
{
  glUniformMatrix4fv(program->projection_matrix_location, 1, GL_FALSE, program->projection_matrix[0]);
}

void KRR_FONTSHADERPROG2D_update_model_matrix(KRR_FONTSHADERPROG2D* program)
{
  glUniformMatrix4fv(program->model_matrix_location, 1, GL_FALSE, program->model_matrix[0]);
}

void KRR_FONTSHADERPROG2D_set_vertex_pointer(KRR_FONTSHADERPROG2D* program, GLsizei stride, const GLvoid* data)
{
  glVertexAttribPointer(program->vertex_pos2d_location, 2, GL_FLOAT, GL_FALSE, stride, data);
}

void KRR_FONTSHADERPROG2D_set_texcoord_pointer(KRR_FONTSHADERPROG2D* program, GLsizei stride, const GLvoid* data)
{
  glVertexAttribPointer(program->texture_coord_location, 2, GL_FLOAT, GL_FALSE, stride, data);
}

void KRR_FONTSHADERPROG2D_set_texture_sampler(KRR_FONTSHADERPROG2D* program, GLuint sampler)
{
  glUniform1i(program->texture_sampler_location, sampler);
}

void KRR_FONTSHADERPROG2D_set_text_color(KRR_FONTSHADERPROG2D* program, COLOR32 color)
{
  glUniform4f(program->text_color_location, color.r, color.g, color.b, color.a);
}

void KRR_FONTSHADERPROG2D_enable_attrib_pointers(KRR_FONTSHADERPROG2D* program)
{
  glEnableVertexAttribArray(program->vertex_pos2d_location);
  glEnableVertexAttribArray(program->texture_coord_location); 
}

void KRR_FONTSHADERPROG2D_disable_attrib_pointers(KRR_FONTSHADERPROG2D* program)
{
  glDisableVertexAttribArray(program->vertex_pos2d_location);
  glDisableVertexAttribArray(program->texture_coord_location);
}
