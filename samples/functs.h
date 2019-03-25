#ifndef KRR_TEST_FUNCTS_h_
#define KRR_TEST_FUNCTS_h_

#include "krr/foundation/common.h"

#define SU_BEGIN(x) \
  KRR_SHADERPROG_bind(x->program);

#define SU_END(x) \
  KRR_SHADERPROG_unbind(x->program);

#define SU_TEXSHADERPROG2D(x) \
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, x); \
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_VIEW_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, x); \
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODEL_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, x);

#define SU_TEXSHADERPROG3D(x) \
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_TEXTURE3D_SHADER, x); \
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_VIEW_MATRIX, USERCODE_SHADERTYPE_TEXTURE3D_SHADER, x); \
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODEL_MATRIX, USERCODE_SHADERTYPE_TEXTURE3D_SHADER, x);

#define SU_TEXALPHASHADERPROG3D(x) \
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_TEXTUREALPHA3D_SHADER, x); \
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_VIEW_MATRIX, USERCODE_SHADERTYPE_TEXTUREALPHA3D_SHADER, x); \
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODEL_MATRIX, USERCODE_SHADERTYPE_TEXTUREALPHA3D_SHADER, x);

#define SU_TERRAINSHADER(x) \
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_TERRAIN_SHADER, x);  \
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_VIEW_MATRIX, USERCODE_SHADERTYPE_TERRAIN_SHADER, x);  \
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODEL_MATRIX, USERCODE_SHADERTYPE_TERRAIN_SHADER, x);

#define SU_FONTSHADER(x)  \
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_FONT_SHADER, x); \
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODEL_MATRIX, USERCODE_SHADERTYPE_FONT_SHADER, x);

enum USERCODE_MATRIXTYPE
{
  USERCODE_MATRIXTYPE_PROJECTION_MATRIX,
  USERCODE_MATRIXTYPE_VIEW_MATRIX,
  USERCODE_MATRIXTYPE_MODEL_MATRIX
};
enum USERCODE_SHADERTYPE
{
  USERCODE_SHADERTYPE_TEXTURE_SHADER,
  USERCODE_SHADERTYPE_TEXTURE3D_SHADER,
  USERCODE_SHADERTYPE_TEXTUREALPHA3D_SHADER,
  USERCODE_SHADERTYPE_TERRAIN_SHADER,
  USERCODE_SHADERTYPE_FONT_SHADER
};

extern mat4 g_ui_projection_matrix;
extern mat4 g_projection_matrix;
extern mat4 g_view_matrix;
extern mat4 g_base_ui_model_matrix;
extern mat4 g_base_model_matrix;

///
/// set matrix then update to shader
/// required user to bind the shader before calling this function.
///
/// \param matrix_type type of matrix to copy to dst. Value is enum USERCODE_MATRIXTYPE.
/// \param shader_type type of shader. Value is enum USERCODE_SHADERTYPE.
/// \param program pointer to shader program.
///
extern void usercode_set_matrix_then_update_to_shader(enum USERCODE_MATRIXTYPE matrix_type, enum USERCODE_SHADERTYPE shader_type, void* program);

#endif
