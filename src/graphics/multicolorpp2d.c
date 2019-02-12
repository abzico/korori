#include "multicolorpp2d.h"
#include <stdlib.h>
#include "foundation/log.h"

static void init_defaults(KRR_MULTCSHADERPROG2D* program);

void init_defaults(KRR_MULTCSHADERPROG2D* program)
{
  program->program = NULL;
  program->vertex_pos2d_location = -1;
  program->multi_color_location = -1;
  glm_mat4_identity(program->projection_matrix);
  program->projection_matrix_location = -1;
  glm_mat4_identity(program->modelview_matrix);
  program->modelview_matrix_location = -1;
}

KRR_MULTCSHADERPROG2D* KRR_MULTCSHADERPROG2D_new(KRR_SHADERPROG* program)
{
  KRR_MULTCSHADERPROG2D* out = malloc(sizeof(KRR_MULTCSHADERPROG2D));

  // init defaults
  init_defaults(out);

  // init from parmeters
  out->program = program;

  return out;
}

void KRR_MULTCSHADERPROG2D_free(KRR_MULTCSHADERPROG2D* program)
{
  // free underlying program
  KRR_SHADERPROG_free(program->program);

  // free source
  free(program);
  program = NULL;
}

bool KRR_MULTCSHADERPROG2D_load_program(KRR_MULTCSHADERPROG2D* program)
{
  // get underlying program
  KRR_SHADERPROG* uprog = program->program;

  // generate program
  uprog->program_id = glCreateProgram();

  // load vertex shader
  GLuint vertex_shader = KRR_SHADERPROG_load_shader_from_file("res/shaders/multicolorpp2d.vert", GL_VERTEX_SHADER);
  // check for errors
  if (vertex_shader == 0)
  {
    glDeleteProgram(uprog->program_id);
    uprog->program_id = 0;
    return false;
  }

  // attach vertex shader to program
  glAttachShader(uprog->program_id, vertex_shader);

  // create fragment shader
  GLuint fragment_shader = KRR_SHADERPROG_load_shader_from_file("res/shaders/multicolorpp2d.frag", GL_FRAGMENT_SHADER);
  // check for errors
  if (fragment_shader == 0)
  {
    // delete vertex shader
    glDeleteShader(vertex_shader);
    vertex_shader = 0;

    // delete program
    glDeleteShader(uprog->program_id);
    uprog->program_id = 0;
    return false;
  }

  // attach fragment shader to program
  glAttachShader(uprog->program_id, fragment_shader);

  // link program
  glLinkProgram(uprog->program_id);
  // check for errors
  GLint link_status = GL_FALSE;
  glGetProgramiv(uprog->program_id, GL_LINK_STATUS, &link_status);
  if (link_status != GL_TRUE)
  {
    // print log from this program
    KRR_LOGE("Unable to link program %d.", uprog->program_id);
    KRR_SHADERPROG_print_program_log(uprog->program_id);

    // delete vertex shader
    glDeleteShader(vertex_shader);
    vertex_shader = 0;

    // delete fragment shader
    glDeleteShader(fragment_shader);
    fragment_shader = 0;

    // delete program
    glDeleteProgram(uprog->program_id);
    uprog->program_id = 0;

    return false;
  }

  // clean up excess shader references
  glDeleteShader(vertex_shader);
  vertex_shader = -1;
  glDeleteShader(fragment_shader);
  fragment_shader = -1;

  // get variable locations
  program->vertex_pos2d_location = glGetAttribLocation(uprog->program_id, "vertex_pos2d");
  if (program->vertex_pos2d_location == -1)
  {
    KRR_LOGW("Warning: %s is not valid glsl program variable", "vertex_pos2d");
  }
  program->multi_color_location = glGetAttribLocation(uprog->program_id, "multi_color");
  if (program->multi_color_location == -1)
  {
    KRR_LOGW("Warning: %s is not valid glsl program variable", "multi_color");
  }
  program->projection_matrix_location = glGetUniformLocation(uprog->program_id, "projection_matrix");
  if (program->projection_matrix_location == -1)
  {
    KRR_LOGW("Warning: %s is not valid glsl program variable", "projection_matrix");
  }
  program->modelview_matrix_location = glGetUniformLocation(uprog->program_id, "modelview_matrix");
  if (program->modelview_matrix_location == -1)
  {
    KRR_LOGW("Warning: %s is not valid glsl program variable", "modelview_matrix");
  }

  return true;
}

void KRR_MULTCSHADERPROG2D_update_projection_matrix(KRR_MULTCSHADERPROG2D* program)
{
  glUniformMatrix4fv(program->projection_matrix_location, 1, GL_FALSE, program->projection_matrix[0]); 
}

void KRR_MULTCSHADERPROG2D_update_modelview_matrix(KRR_MULTCSHADERPROG2D* program)
{
  glUniformMatrix4fv(program->modelview_matrix_location, 1, GL_FALSE, program->modelview_matrix[0]);
}

void KRR_MULTCSHADERPROG2D_set_vertex_pointer(KRR_MULTCSHADERPROG2D* program, GLsizei stride, const GLvoid* data)
{
  glVertexAttribPointer(program->vertex_pos2d_location, 2, GL_FLOAT, GL_FALSE, stride, data);
}

void KRR_MULTCSHADERPROG2D_set_color_pointer(KRR_MULTCSHADERPROG2D* program, GLsizei stride, const GLvoid* data)
{
  glVertexAttribPointer(program->multi_color_location, 4, GL_FLOAT, GL_FALSE, stride, data);
}

void KRR_MULTCSHADERPROG2D_enable_attrib_pointers(KRR_MULTCSHADERPROG2D* program)
{
  glEnableVertexAttribArray(program->vertex_pos2d_location);
  glEnableVertexAttribArray(program->multi_color_location);
}

void KRR_MULTCSHADERPROG2D_disable_attrib_pointers(KRR_MULTCSHADERPROG2D* program)
{
  glDisableVertexAttribArray(program->vertex_pos2d_location);
  glDisableVertexAttribArray(program->multi_color_location);
}
