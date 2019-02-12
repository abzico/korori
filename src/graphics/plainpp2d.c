#include "plainpp2d.h"
#include <stdlib.h>
#include "graphics/shaderprog_internals.h"
#include "foundation/log.h"

static void init_defaults(KRR_PLAINSHADERPROG2D* program);

void init_defaults(KRR_PLAINSHADERPROG2D* program)
{
	program->program = NULL;
  program->polygon_color_location = -1;
	glm_mat4_identity(program->projection_matrix);
	program->projection_matrix_location = -1;
	glm_mat4_identity(program->modelview_matrix);
	program->modelview_matrix_location = -1;
}

KRR_PLAINSHADERPROG2D* KRR_PLAINSHADERPROG2D_new(KRR_SHADERPROG* program)
{
  KRR_PLAINSHADERPROG2D* out = malloc(sizeof(KRR_PLAINSHADERPROG2D));

  // first init defaults value
  init_defaults(out);

	// init from input params
  out->program = program;
  KRR_SHADERPROG_init_defaults(out->program);

  return out;
}

void KRR_PLAINSHADERPROG2D_free(KRR_PLAINSHADERPROG2D* program)
{
  // free underlying program
  KRR_SHADERPROG_free(program->program);

  // free the source
  free(program);
  program = NULL;
}

bool KRR_PLAINSHADERPROG2D_load_program(KRR_PLAINSHADERPROG2D* program)
{
  // generate program
  program->program->program_id = glCreateProgram();

  // load vertex shader
  GLuint vertex_shader = KRR_SHADERPROG_load_shader_from_file("res/shaders/plainpp2d.vert", GL_VERTEX_SHADER);

  // check for errors
  if (vertex_shader == 0)
  {
    // delete program
    glDeleteProgram(program->program->program_id);
    program->program->program_id = 0;
    return false;
  }
  // attach vertex shader to program
  glAttachShader(program->program->program_id, vertex_shader);

  // create fragment shader
  GLuint fragment_shader = KRR_SHADERPROG_load_shader_from_file("res/shaders/plainpp2d.frag", GL_FRAGMENT_SHADER);

  // check for errors
  if (fragment_shader == 0)
  {
    // delete vertex shader
    glDeleteShader(vertex_shader);
    vertex_shader = 0;

    // delete program
    glDeleteProgram(program->program->program_id);
    program->program->program_id = 0;
    return false;
  }

  // attach fragment shader to program
  glAttachShader(program->program->program_id, fragment_shader);

  // link program
  glLinkProgram(program->program->program_id);

  // check for errors
  GLint link_status = GL_FALSE;
  glGetProgramiv(program->program->program_id, GL_LINK_STATUS, &link_status);
  if (link_status != GL_TRUE)
  {
    KRR_LOGE("Error linking program %d", program->program->program_id);
    KRR_SHADERPROG_print_program_log(program->program->program_id);

    // delete vertex shader
    glDeleteShader(vertex_shader);
    vertex_shader = 0;

    // delete fragment shader
    glDeleteShader(fragment_shader);
    fragment_shader = 0;

    // delete program
    glDeleteProgram(program->program->program_id);
    program->program->program_id = 0;
    return false;
  }

  // clean up
  // delete vertex shader
  glDeleteShader(vertex_shader);
  vertex_shader = 0;

  // delete fragment shader
  glDeleteShader(fragment_shader);
  fragment_shader = 0;

  // get variable locations
  // note: we can only query for location after link successfully
	
	// vertex
	program->projection_matrix_location = glGetUniformLocation(program->program->program_id, "projection_matrix");
	if (program->projection_matrix_location == -1)
	{
		KRR_LOGW("Warning: %s is not a valid glsl program variable", "projection_matrix");
	}
	program->modelview_matrix_location = glGetUniformLocation(program->program->program_id, "modelview_matrix");
	if (program->modelview_matrix_location == -1)
	{
		KRR_LOGW("Warning: %s is not a valid glsl program variable", "modelview_matrix");
	}
	
	// fragment
  program->polygon_color_location = glGetUniformLocation(program->program->program_id, "polygon_color");
  if (program->polygon_color_location == -1)
  {
    KRR_LOGW("Warning: %s is not a valid glsl program variable", "polygon_color");
  }

  return true;
}

void KRR_PLAINSHADERPROG2D_set_color(KRR_PLAINSHADERPROG2D* program, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  // update color in shader
  glUniform4f(program->polygon_color_location, r, g, b, a);
}

void KRR_PLAINSHADERPROG2D_update_projection_matrix(KRR_PLAINSHADERPROG2D* program)
{
	// can consult README.md of cglm on how to pass matrix to opengl function
	// note: it's column major
	glUniformMatrix4fv(program->projection_matrix_location, 1, GL_FALSE, program->projection_matrix[0]);
}

void KRR_PLAINSHADERPROG2D_update_modelview_matrix(KRR_PLAINSHADERPROG2D* program)
{
	glUniformMatrix4fv(program->modelview_matrix_location, 1, GL_FALSE, program->modelview_matrix[0]);
}
