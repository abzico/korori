#include "functs.h"
#include "krr/graphics/texturedpp2d.h"
#include "krr/graphics/texturedpp3d.h"
#include "krr/graphics/texturedalphapp3d.h"
#include "krr/graphics/terrain_shader3d.h"
#include "krr/graphics/fontpp2d.h"
#include "krr/foundation/log.h"

mat4 g_ui_projection_matrix;
mat4 g_projection_matrix;
mat4 g_view_matrix;
mat4 g_base_ui_model_matrix;
mat4 g_base_model_matrix;

void usercode_set_matrix_then_update_to_shader(enum USERCODE_MATRIXTYPE matrix_type, enum USERCODE_SHADERTYPE shader_program, void* program)
{
  // projection matrix
  if (matrix_type == USERCODE_MATRIXTYPE_PROJECTION_MATRIX)
  {
    // texture shader
    if (shader_program == USERCODE_SHADERTYPE_TEXTURE_SHADER)
    {
      KRR_TEXSHADERPROG2D* shader_ptr = (KRR_TEXSHADERPROG2D*)program;
      glm_mat4_copy(g_ui_projection_matrix, shader_ptr->projection_matrix);
      KRR_TEXSHADERPROG2D_update_projection_matrix(shader_ptr);
    }
    // texture3d shader
    else if (shader_program == USERCODE_SHADERTYPE_TEXTURE3D_SHADER)
    {
      KRR_TEXSHADERPROG3D* shader_ptr = (KRR_TEXSHADERPROG3D*)program;
      glm_mat4_copy(g_projection_matrix, shader_ptr->projection_matrix);
      KRR_TEXSHADERPROG3D_update_projection_matrix(shader_ptr);
    }
    // texture alpha 3d shader
    else if (shader_program == USERCODE_SHADERTYPE_TEXTUREALPHA3D_SHADER)
    {
      KRR_TEXALPHASHADERPROG3D* shader_ptr = (KRR_TEXALPHASHADERPROG3D*)program;
      glm_mat4_copy(g_projection_matrix, shader_ptr->projection_matrix);
      KRR_TEXALPHASHADERPROG3D_update_projection_matrix(shader_ptr);
    }
    // terrain shader
    else if (shader_program == USERCODE_SHADERTYPE_TERRAIN_SHADER)
    {
      KRR_TERRAINSHADERPROG3D* shader_ptr = (KRR_TERRAINSHADERPROG3D*)program;
      glm_mat4_copy(g_projection_matrix, shader_ptr->projection_matrix);
      KRR_TERRAINSHADERPROG3D_update_projection_matrix(shader_ptr);
    }
    // font shader
    else if (shader_program == USERCODE_SHADERTYPE_FONT_SHADER)
    {
      KRR_FONTSHADERPROG2D* shader_ptr = (KRR_FONTSHADERPROG2D*)program;
      glm_mat4_copy(g_ui_projection_matrix, shader_ptr->projection_matrix);
      KRR_FONTSHADERPROG2D_update_projection_matrix(shader_ptr);
    }
  }
  // view matrix
  else if (matrix_type == USERCODE_MATRIXTYPE_VIEW_MATRIX)
  {
    // texture shader
    if (shader_program == USERCODE_SHADERTYPE_TEXTURE_SHADER)
    {
      KRR_TEXSHADERPROG2D* shader_ptr = (KRR_TEXSHADERPROG2D*)program;
      glm_mat4_copy(g_view_matrix, shader_ptr->view_matrix);
      KRR_TEXSHADERPROG2D_update_view_matrix(shader_ptr);
    }
    // texture 3d shader
    else if (shader_program == USERCODE_SHADERTYPE_TEXTURE3D_SHADER)
    {
      KRR_TEXSHADERPROG3D* shader_ptr = (KRR_TEXSHADERPROG3D*)program;
      glm_mat4_copy(g_view_matrix, shader_ptr->view_matrix);
      KRR_TEXSHADERPROG3D_update_view_matrix(shader_ptr);
    }
    // texture alpha 3d shader
    else if (shader_program == USERCODE_SHADERTYPE_TEXTUREALPHA3D_SHADER)
    {
      KRR_TEXALPHASHADERPROG3D* shader_ptr = (KRR_TEXALPHASHADERPROG3D*)program;
      glm_mat4_copy(g_view_matrix, shader_ptr->view_matrix);
      KRR_TEXALPHASHADERPROG3D_update_view_matrix(shader_ptr);
    }
    // terrain shader
    else if (shader_program == USERCODE_SHADERTYPE_TERRAIN_SHADER)
    {
      KRR_TERRAINSHADERPROG3D* shader_ptr = (KRR_TERRAINSHADERPROG3D*)program;
      glm_mat4_copy(g_view_matrix, shader_ptr->view_matrix);
      KRR_TERRAINSHADERPROG3D_update_view_matrix(shader_ptr);
    }
    // note: font shader doesn't have view matrix yet
  }
  // model matrix
  else if (matrix_type == USERCODE_MATRIXTYPE_MODEL_MATRIX)
  {
    // texture 2d shader
    if (shader_program == USERCODE_SHADERTYPE_TEXTURE_SHADER)
    {
      KRR_TEXSHADERPROG2D* shader_ptr = (KRR_TEXSHADERPROG2D*)program;
      glm_mat4_copy(g_base_ui_model_matrix, shader_ptr->model_matrix);
      KRR_TEXSHADERPROG2D_update_model_matrix(shader_ptr);
    }
    // texture 3d shader
    else if (shader_program == USERCODE_SHADERTYPE_TEXTURE3D_SHADER)
    {
      KRR_TEXSHADERPROG3D* shader_ptr = (KRR_TEXSHADERPROG3D*)program;
      glm_mat4_copy(g_base_model_matrix, shader_ptr->model_matrix);
      KRR_TEXSHADERPROG3D_update_model_matrix(shader_ptr);
    }
    // texture alpha 3d shader
    else if (shader_program == USERCODE_SHADERTYPE_TEXTUREALPHA3D_SHADER)
    {
      KRR_TEXALPHASHADERPROG3D* shader_ptr = (KRR_TEXALPHASHADERPROG3D*)program;
      glm_mat4_copy(g_base_model_matrix, shader_ptr->model_matrix);
      KRR_TEXALPHASHADERPROG3D_update_model_matrix(shader_ptr);
    }
    // terrain shader
    else if (shader_program == USERCODE_SHADERTYPE_TERRAIN_SHADER)
    {
      KRR_TERRAINSHADERPROG3D* shader_ptr = (KRR_TERRAINSHADERPROG3D*)program;
      glm_mat4_copy(g_base_model_matrix, shader_ptr->model_matrix);
      KRR_TERRAINSHADERPROG3D_update_model_matrix(shader_ptr);
    }
    // font shader
    else if (shader_program == USERCODE_SHADERTYPE_FONT_SHADER)
    {
      KRR_FONTSHADERPROG2D* shader_ptr = (KRR_FONTSHADERPROG2D*)program;
      glm_mat4_copy(g_base_ui_model_matrix, shader_ptr->model_matrix);
      KRR_FONTSHADERPROG2D_update_model_matrix(shader_ptr);
    }
  }
}
