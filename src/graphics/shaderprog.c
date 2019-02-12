#include "shaderprog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "foundation/log.h"
#include "foundation/util.h"
#include "graphics/shaderprog_internals.h"
#include "graphics/util.h"

void KRR_SHADERPROG_init_defaults(KRR_SHADERPROG* shader_program)
{
  shader_program->program_id = 0;
}

KRR_SHADERPROG* KRR_SHADERPROG_new(void)
{
  KRR_SHADERPROG* out = malloc(sizeof(KRR_SHADERPROG));
  KRR_SHADERPROG_init_defaults(out);

  return out;
}

void KRR_SHADERPROG_free(KRR_SHADERPROG* shader_program)
{
  // free program
  KRR_SHADERPROG_free_program(shader_program);

  // free the source
  free(shader_program);
  shader_program = NULL;
}

GLuint KRR_SHADERPROG_load_shader_from_file(const char* path, GLenum shader_type)
{
  // open file for read
  FILE* file = fopen(path, "r");
  if (file == NULL)
  {
    KRR_LOGE("Unable to open file for read %s", path);
    // return 0 for failed case
    return 0;
  }

  // find the total byte size of file
  fseek(file, 0, SEEK_END);
  const long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  // check if file size is zero
  if (file_size <= 0)
  {
    KRR_LOGE("Shader file has zero bytes");

    // close the file
    fclose(file);
    file = NULL;

    return 0;
  }

  // have enough space to hold file's content
  char shader_source[file_size+1];
  memset(shader_source, 0, file_size+1);

  // FIXME: should we fix this to read line-by-line or chunk by chunk in case the limitation of RAM?
  // read the whole file as once
  if (fread(shader_source, file_size, 1, file) != 1)
  {
    // something wrong happen
    // check if it's end-of-file error
    if (feof(file))
    {
      KRR_LOGE("Read error, end-of-file detected");
      // close the file
      fclose(file);
      file = NULL;

      return 0;
    }

    // there's something else
    if (ferror(file))
    {
      KRR_LOGE("Read error");
      // close the file
      fclose(file);
      file = NULL;

      return 0;
    }
  }

  // close the file
  fclose(file);
  file = NULL;

  // create shader id
  GLuint shader_id = glCreateShader(shader_type);

  // satisfy parameter type
  // glShaderSource needs 3rd parameter of const GLchar**
  const char* str_ptr = (const char*)shader_source;

  // set shader source
  glShaderSource(shader_id, 1, &str_ptr, NULL);

  // compile shader source
  glCompileShader(shader_id);

  // check shader for errors
  GLint shader_compiled = GL_FALSE;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &shader_compiled);
  if (shader_compiled != GL_TRUE)
  {
    KRR_LOGE("Unable to compile shader %d. Source: %s", shader_id, shader_source);
    KRR_SHADERPROG_print_shader_log(shader_id);
    
    // delete shader
    glDeleteShader(shader_id);
    shader_id = 0;
  }

  return shader_id;
}

void KRR_SHADERPROG_free_program(KRR_SHADERPROG* shader_program)
{
  // delete program
  glDeleteProgram(shader_program->program_id);
  shader_program->program_id = 0;
}

bool KRR_SHADERPROG_bind(KRR_SHADERPROG* shader_program)
{
	// check whether we need to bind again
	GLint current_bound;
	glGetIntegerv(GL_CURRENT_PROGRAM, &current_bound);

	// if such program is already bound, then return now
	if (current_bound == shader_program->program_id)
	{
		KRR_LOGE("Program is already bound, no need to bind again");
		return true;
	}

  // use shader
  glUseProgram(shader_program->program_id);

  // check for error
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    KRR_util_print_callstack();
    KRR_LOGE("Error use program %u: %s", shader_program->program_id, KRR_gputil_error_string(error));
    return false;
  }

  return true;
}

void KRR_SHADERPROG_unbind(KRR_SHADERPROG* shader_program)
{
  // use default program
  glUseProgram(0);
}

void KRR_SHADERPROG_print_program_log(GLuint program_id)
{
  // make sure name is shader
  if (glIsProgram(program_id))
  {
    // program log length
    int info_log_length = 0;
    int max_length = info_log_length;

    // get info string length
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &max_length);

    // allocate string
    char info_log[max_length];

    // get info log
    glGetProgramInfoLog(program_id, max_length, &info_log_length, info_log);
    if (info_log_length > 0)
    {
      // print log
      fprintf(stdout, "%s\n", info_log);
    }
  }
  else
  {
    KRR_LOGE("Name %d is not a program", program_id);
  }
}

void KRR_SHADERPROG_print_shader_log(GLuint shader_id)
{
  // make sure name is shader
  if (glIsShader(shader_id))
  {
    // shader log length
    int info_log_length = 0;
    int max_length = info_log_length;

    // get info string length
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &max_length);

    // allocate string
    char info_log[max_length];

    // get info log
    glGetShaderInfoLog(shader_id, max_length, &info_log_length, info_log);
    if (info_log_length > 0)
    {
      // print log
      fprintf(stderr, "%s\n", info_log);
    }
  }
  else
  {
    KRR_LOGE("Name %d is not a shader", shader_id);
  }
}
