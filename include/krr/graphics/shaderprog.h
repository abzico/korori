#ifndef KRR_SHADERPROG_h_
#define KRR_SHADERPROG_h_

#include "krr/graphics/common.h"
#include <stdbool.h>

typedef struct
{
  // program id
  GLuint program_id;
} KRR_SHADERPROG;

///
/// Create a new shader program.
///
/// \return Newly created KRR_SHADERPROG returned as pointer to KRR_SHADERPROG
///
extern KRR_SHADERPROG* KRR_SHADERPROG_new(void);

///
/// Free shader program.
///
/// \param shader_program Pointer to KRR_SHADERPROG
///
extern void KRR_SHADERPROG_free(KRR_SHADERPROG* shader_program);

///
/// Load shader from file according to type.
///
/// \param path Path to shader file
/// \param shader_type Type of shader
/// \return Return id of a compiled shader, or 0 if failed.
///
extern GLuint KRR_SHADERPROG_load_shader_from_file(const char* path, GLenum shader_type);

///
/// Free shader program
///
/// \param shader_program Pointer to KRR_SHADERPROG
///
extern void KRR_SHADERPROG_free_program(KRR_SHADERPROG* shader_program);

///
/// Bind this shader program, thus set this shader program as the current shader program
///
/// \param shader_program Pointer to KRR_SHADERPROG
/// \return True if bind successfully or such program is already bound but no attempt to re-bind again, otherwise return false
///
extern bool KRR_SHADERPROG_bind(KRR_SHADERPROG* shader_program);

///
/// Unbind this shader program, thus unset it as the current shader program.
///
/// \param shader_program Pointer to KRR_SHADERPROG
///
extern void KRR_SHADERPROG_unbind(KRR_SHADERPROG* shader_program);

///
/// Print out log for input program id (or say program name).
///
/// \param program_id Program id to print log
///
extern void KRR_SHADERPROG_print_program_log(GLuint program_id);

///
/// Print out log for input shader id (or say shader name).
///
/// \param shader_id Shader id to print log
///
extern void KRR_SHADERPROG_print_shader_log(GLuint shader_id);

#endif
