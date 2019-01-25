#include "usercode.h"
#include "foundation/common.h"
#include "foundation/window.h"
#include "foundation/util.h"
#include "graphics/util.h"
#include "graphics/texturedpp2d.h"
#include "graphics/font.h"
#include "graphics/fontpp2d.h"
#include "graphics/doublemulticolorpp2d.h"

// don't use this elsewhere
#define CONTENT_BG_COLOR 0.f, 0.f, 0.f, 1.f

#ifndef DISABLE_FPS_CALC
#define FPS_BUFFER 7+1
char fps_text[FPS_BUFFER];
static KRR_FONT* fps_font = NULL;
#endif

// -- section of variables for maintaining aspect ratio -- //
static int g_screen_width;
static int g_screen_height;

static int g_logical_width;
static int g_logical_height;
static float g_logical_aspect;

static int g_offset_x = 0;
static int g_offset_y = 0;
// resolution independent scale for x and y
static float g_ri_scale_x = 1.0f;
static float g_ri_scale_y = 1.0f;
// resolution independent dimensions
static int g_ri_view_width;
static int g_ri_view_height;

static bool g_need_clipping = false;

static mat4 g_projection_matrix;
// base modelview matrix to reduce some of mathematics operation initially
static mat4 g_base_modelview_matrix;
// -- section of variables for maintaining aspect ratio -- //

// -- section of function signatures -- //
static void usercode_app_went_windowed_mode();
static void usercode_app_went_fullscreen();

enum USERCODE_MATRIXTYPE
{
  USERCODE_MATRIXTYPE_PROJECTION_MATRIX,
  USERCODE_MATRIXTYPE_MODELVIEW_MATRIX
};
enum USERCODE_SHADERTYPE
{
  USERCODE_SHADERTYPE_TEXTURE_SHADER,
  USERCODE_SHADERTYPE_FONT_SHADER
};
///
/// set matrix then update to shader
/// required user to bind the shader before calling this function.
///
/// \param matrix_type type of matrix to copy to dst. Value is enum USERCODE_MATRIXTYPE.
/// \param shader_type type of shader. Value is enum USERCODE_SHADERTYPE.
/// \param program pointer to shader program.
///
static void usercode_set_matrix_then_update_to_shader(enum USERCODE_MATRIXTYPE matrix_type, enum USERCODE_SHADERTYPE shader_type, void* program);
// -- end of section of function signatures -- //

#ifndef DISABLE_FPS_CALC
static GLuint fps_vao = 0;
#endif

// basic shaders and font
static KRR_TEXSHADERPROG2D* texture_shader = NULL;
static KRR_FONTSHADERPROG2D* font_shader = NULL;
static KRR_FONT* font = NULL;

// double multicolor
static KRR_DMULTICSHADERPROG2D* multicolor_shader = NULL;
static GLuint vertex_vbo = 0;
static GLuint rgby_vbo = 0;
static GLuint cymw_vbo = 0;
static GLuint gray_vbo = 0;
static GLuint ibo = 0;
static GLuint left_vao = 0;
static GLuint right_vao = 0;

void usercode_set_matrix_then_update_to_shader(enum USERCODE_MATRIXTYPE matrix_type, enum USERCODE_SHADERTYPE shader_program, void* program)
{
  // projection matrix
  if (matrix_type == USERCODE_MATRIXTYPE_PROJECTION_MATRIX)
  {
    // texture shader
    if (shader_program == USERCODE_SHADERTYPE_TEXTURE_SHADER)
    {
      // convert to right type of program shader
      KRR_TEXSHADERPROG2D* shader_ptr = (KRR_TEXSHADERPROG2D*)program;

      // copy calculated projection matrix to shader's then update to shader
      glm_mat4_copy(g_projection_matrix, shader_ptr->projection_matrix);

      KRR_TEXSHADERPROG2D_update_projection_matrix(shader_ptr);
    }
    // font shader
    else if (shader_program == USERCODE_SHADERTYPE_FONT_SHADER)
    {
      KRR_FONTSHADERPROG2D* shader_ptr = (KRR_FONTSHADERPROG2D*)program;
      glm_mat4_copy(g_projection_matrix, shader_ptr->projection_matrix);

      KRR_FONTSHADERPROG2D_update_projection_matrix(shader_ptr);
    }
  }
  // modelview matrix
  else if (matrix_type == USERCODE_MATRIXTYPE_MODELVIEW_MATRIX)
  {
    // texture shader
    if (shader_program == USERCODE_SHADERTYPE_TEXTURE_SHADER)
    {
      KRR_TEXSHADERPROG2D* shader_ptr = (KRR_TEXSHADERPROG2D*)program;
      glm_mat4_copy(g_base_modelview_matrix, shader_ptr->modelview_matrix);

      KRR_TEXSHADERPROG2D_update_modelview_matrix(shader_ptr);
    }
    // font shader
    else if (shader_program == USERCODE_SHADERTYPE_FONT_SHADER)
    {
      KRR_FONTSHADERPROG2D* shader_ptr = (KRR_FONTSHADERPROG2D*)program;
      glm_mat4_copy(g_base_modelview_matrix, shader_ptr->modelview_matrix);

      KRR_FONTSHADERPROG2D_update_modelview_matrix(shader_ptr);
    }
  }
}

void usercode_app_went_windowed_mode()
{
  KRR_SHADERPROG_bind(multicolor_shader->program);
  glm_mat4_copy(g_projection_matrix, multicolor_shader->projection_matrix);
  KRR_gputil_update_projection_matrix(multicolor_shader->projection_matrix_location, multicolor_shader->projection_matrix);

  glm_mat4_copy(g_base_modelview_matrix, multicolor_shader->modelview_matrix);
  KRR_gputil_update_modelview_matrix(multicolor_shader->modelview_matrix_location, multicolor_shader->modelview_matrix);

  KRR_SHADERPROG_bind(texture_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODELVIEW_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture_shader);
  // no need to unbind as we will bind a new one soon

  KRR_SHADERPROG_bind(font_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_FONT_SHADER, font_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODELVIEW_MATRIX, USERCODE_SHADERTYPE_FONT_SHADER, font_shader);
  KRR_SHADERPROG_unbind(font_shader->program);
}

void usercode_app_went_fullscreen()
{
  KRR_SHADERPROG_bind(multicolor_shader->program);
  glm_mat4_copy(g_projection_matrix, multicolor_shader->projection_matrix);
  KRR_gputil_update_projection_matrix(multicolor_shader->projection_matrix_location, multicolor_shader->projection_matrix);

  glm_mat4_copy(g_base_modelview_matrix, multicolor_shader->modelview_matrix);
  KRR_gputil_update_modelview_matrix(multicolor_shader->modelview_matrix_location, multicolor_shader->modelview_matrix);

  KRR_SHADERPROG_bind(texture_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODELVIEW_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture_shader);

  KRR_SHADERPROG_bind(font_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_FONT_SHADER, font_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODELVIEW_MATRIX, USERCODE_SHADERTYPE_FONT_SHADER, font_shader);
  KRR_SHADERPROG_unbind(font_shader->program);
}

bool usercode_init(int screen_width, int screen_height, int logical_width, int logical_height)
{
  // FIXME: This code would works only if user starts with windowed mode, didnt test yet if start with fullscreen mode ...
  // set input screen dimensions
  g_screen_width = screen_width;
  g_screen_height = screen_height;

  g_logical_width = logical_width;
  g_logical_height = logical_height;
  g_logical_aspect = g_screen_width * 1.0f / g_screen_height;

  // start off with resolution matching the screen
  g_ri_view_width = g_screen_width;
  g_ri_view_height = g_screen_height;

  // calculate orthographic projection matrix
	glm_ortho(0.0f, (float)g_screen_width, (float)g_screen_height, 0.0f, -1.0f, 1.0f, g_projection_matrix);
	// calculate base modelview matrix (to reduce some of operations cost)
	glm_mat4_identity(g_base_modelview_matrix);
	glm_scale(g_base_modelview_matrix, (vec3){ g_ri_scale_x, g_ri_scale_y, 1.f});

  // initialize the viewport
  // define the area where to render, for now full screen
  glViewport(0, 0, g_screen_width, g_screen_height);

  // initialize clear color
  glClearColor(0.f, 0.f, 0.f, 1.f);

  // enable blending with default blend function
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // enable face culling
  glEnable(GL_CULL_FACE);

  // check for errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    KRR_util_print_callstack();
    SDL_Log("Error initializing OpenGL! %s", KRR_gputil_error_string(error));
    return false;
  }

  return true;
}

bool usercode_loadmedia()
{
  // load font to render framerate
#ifndef DISABLE_FPS_CALC
  {
    glGenVertexArrays(1, &fps_vao);

    fps_font = KRR_FONT_new();
    if (!KRR_FONT_load_freetype(fps_font, "res/fonts/Minecraft.ttf", 14))
    {
      SDL_Log("Unable to load font for rendering framerate");
      return false;
    }
  }
#endif
  
  // create font
  font = KRR_FONT_new();
  if (!KRR_FONT_load_freetype(font, "res/fonts/Minecraft.ttf", 40))
  {
    SDL_Log("Error to load font");
    return false;
  }
  // load texture shader
  texture_shader = KRR_TEXSHADERPROG2D_new();
  if (!KRR_TEXSHADERPROG2D_load_program(texture_shader))
  {
    SDL_Log("Error loading texture shader");
    return false;
  }
  
  // load font shader
  font_shader = KRR_FONTSHADERPROG2D_new();
  if (!KRR_FONTSHADERPROG2D_load_program(font_shader))
  {
    SDL_Log("Error loading font shader");
    return false;
  }

  // TODO: Load media here...
  multicolor_shader = KRR_DMULTICSHADERPROG2D_new();
  if (!KRR_DMULTICSHADERPROG2D_load_program(multicolor_shader))
  {
    return false;
  }

  // initially update matrices for multicolor shader
  KRR_SHADERPROG_bind(multicolor_shader->program);
  // set matrices
  glm_mat4_copy(g_projection_matrix, multicolor_shader->projection_matrix);
  glm_mat4_copy(g_base_modelview_matrix, multicolor_shader->modelview_matrix);
  // issue update matrices to gpu
  KRR_gputil_update_projection_matrix(multicolor_shader->projection_matrix_location, multicolor_shader->projection_matrix);
  KRR_gputil_update_modelview_matrix(multicolor_shader->modelview_matrix_location, multicolor_shader->modelview_matrix);

  // initially update all related matrices and related graphics stuf for both shaders
  KRR_SHADERPROG_bind(texture_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODELVIEW_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture_shader);
  KRR_TEXSHADERPROG2D_set_texture_sampler(texture_shader, 0);
  // set texture shader to all KRR_TEXTURE as active
  shared_textured_shaderprogram = texture_shader;

  KRR_SHADERPROG_bind(font_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_FONT_SHADER, font_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODELVIEW_MATRIX, USERCODE_SHADERTYPE_FONT_SHADER, font_shader);
  KRR_FONTSHADERPROG2D_set_texture_sampler(font_shader, 0);
  // set font shader to all KRR_FONT as active
  shared_font_shaderprogram = font_shader;
  KRR_SHADERPROG_unbind(font_shader->program);

  // set up VBO data
  VERTEXPOS2D quad_pos[4] = {
    {-50.0f, -50.0f },
    {-50.0f, 50.0f },
    {50.0f, 50.0f },
    {50.0f, -50.0f }
  };

  COLOR32 quad_color_rgby[4] = {
    { 1.0f, 0.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 0.0f, 1.0f },
    { 0.0f, 1.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f, 1.0f }
  };

  COLOR32 quad_color_cymw[4] = {
    { 0.0f, 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f }
  };

  COLOR32 quad_color_gray[4] = {
    { 0.75f, 0.75f, 0.75f, 1.0f },
    { 0.50f, 0.50f, 0.50f, 0.50f },
    { 0.75f, 0.75f, 0.75f, 1.0f },
    { 0.50f, 0.50f, 0.50f, 1.0f }
  };

  GLuint indices[4] = { 0, 1, 2, 3 };

  // create VBOs
  glGenBuffers(1, &vertex_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
  glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(VERTEXPOS2D), quad_pos, GL_STATIC_DRAW);

  glGenBuffers(1, &rgby_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, rgby_vbo);
  glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(COLOR32), quad_color_rgby, GL_STATIC_DRAW);

  glGenBuffers(1, &cymw_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, cymw_vbo);
  glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(COLOR32), quad_color_cymw, GL_STATIC_DRAW);

  glGenBuffers(1, &gray_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, gray_vbo);
  glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(COLOR32), quad_color_gray, GL_STATIC_DRAW);

  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), indices, GL_STATIC_DRAW);

  // left vao
  glGenVertexArrays(1, &left_vao);

  // bind vertex array
  glBindVertexArray(left_vao);
  // enable vertex attributes
  KRR_DMULTICSHADERPROG2D_enable_all_vertex_attrib_pointers(multicolor_shader);

  // set vertex data
  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
  KRR_DMULTICSHADERPROG2D_set_attrib_vertex_pos2d_pointer_packed(multicolor_shader, NULL);

  glBindBuffer(GL_ARRAY_BUFFER, rgby_vbo);
  KRR_DMULTICSHADERPROG2D_set_attrib_multicolor_pointer_packed(multicolor_shader, 1, NULL);

  glBindBuffer(GL_ARRAY_BUFFER, gray_vbo);
  KRR_DMULTICSHADERPROG2D_set_attrib_multicolor_pointer_packed(multicolor_shader, 2, NULL);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

  // (note: not neccessary to unbin vao as setting a new one will overwrite the active vao, reduce cost in swithing vao)
  // right vao
  glGenVertexArrays(1, &right_vao);

  // bind vertex array
  glBindVertexArray(right_vao);
  // enable vertex attributes
  KRR_DMULTICSHADERPROG2D_enable_all_vertex_attrib_pointers(multicolor_shader);

  // set vertex data
  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
  KRR_DMULTICSHADERPROG2D_set_attrib_vertex_pos2d_pointer_packed(multicolor_shader, NULL);

  glBindBuffer(GL_ARRAY_BUFFER, cymw_vbo);
  KRR_DMULTICSHADERPROG2D_set_attrib_multicolor_pointer_packed(multicolor_shader, 1, NULL);

  glBindBuffer(GL_ARRAY_BUFFER, gray_vbo);
  KRR_DMULTICSHADERPROG2D_set_attrib_multicolor_pointer_packed(multicolor_shader, 2, NULL);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

  // unbine vao
  glBindVertexArray(0);

  return true;
}

void usercode_set_screen_dimension(Uint32 window_id, int screen_width, int screen_height)
{
  g_screen_width = screen_width;
  g_screen_height = screen_height;
}

void usercode_handle_event(SDL_Event *e, float delta_time)
{
  if (e->type == SDL_KEYDOWN)
  {
    int k = e->key.keysym.sym;

    // toggle fullscreen via enter key
    if (k == SDLK_RETURN)
    {
      // go windowed mode, currently in fullscreen mode
      if (gWindow->fullscreen)
      {
        KRR_WINDOW_set_fullscreen(gWindow, false);
        // set projection matrix back to normal
        KRR_gputil_adapt_to_normal(g_screen_width, g_screen_height);
        // reset relavant values back to normal
        g_offset_x = 0.0f;
        g_offset_y = 0.0f;
        g_ri_scale_x = 1.0f;
        g_ri_scale_y = 1.0f;
        g_ri_view_width = g_screen_width;
        g_ri_view_height = g_screen_height;
        g_need_clipping = false;
				
				// re-calculate orthographic projection matrix
				glm_ortho(0.0f, (float)g_ri_view_width, (float)g_ri_view_height, 0.0f, -1.0f, 1.0f, g_projection_matrix);

				// re-calculate base modelview matrix
				// no need to scale as it's uniform 1.0 now
				glm_mat4_identity(g_base_modelview_matrix);

				// signal that app went windowed mode
				usercode_app_went_windowed_mode();
      }
      else
      {
        KRR_WINDOW_set_fullscreen(gWindow, true);
        // get new window's size
        int w, h;
        SDL_GetWindowSize(gWindow->window, &w, &h);
        // also adapt to letterbox
        KRR_gputil_adapt_to_letterbox(w, h, g_logical_width, g_logical_height, &g_ri_view_width, &g_ri_view_height, &g_offset_x, &g_offset_y);
        // calculate scale 
        g_ri_scale_x = g_ri_view_width * 1.0f / g_logical_width;
        g_ri_scale_y = g_ri_view_height * 1.0f / g_logical_height;
        g_need_clipping = true;

				// re-calculate orthographic projection matrix
				glm_ortho(0.0f, (float)g_ri_view_width, (float)g_ri_view_height, 0.0f, -1.0f, 1.0f, g_projection_matrix);

				// re-calculate base modelview matrix
				glm_mat4_identity(g_base_modelview_matrix);
				// also scale
				glm_scale(g_base_modelview_matrix, (vec3){ g_ri_scale_x, g_ri_scale_y, 1.f});

				// signal that app went fullscreen mode
				usercode_app_went_fullscreen();
      }
    }
  }
}

void usercode_update(float delta_time)
{

}

void usercode_render()
{
  // clear color buffer
  if (g_need_clipping)
    glClearColor(0.f, 0.f, 0.f, 1.f);
  else
    glClearColor(CONTENT_BG_COLOR);
  glClear(GL_COLOR_BUFFER_BIT);

  // now clip content to be drawn only on content area (if needed)
  if (g_need_clipping)
  {
    // clear color for content area
    glEnable(GL_SCISSOR_TEST);
    glScissor(g_offset_x, g_offset_y, g_ri_view_width, g_ri_view_height);
    glClearColor(CONTENT_BG_COLOR);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  // note: set viewport via glViewport accordingly, you should start at g_offset_x, and g_offset_y
  // note2: glViewport coordinate still in world coordinate, but for individual object (vertices) to be drawn, it's local coordinate

  // TODO: render code goes here...
  // bind left vao
  glBindVertexArray(left_vao);

  // bind shader
  KRR_SHADERPROG_bind(multicolor_shader->program);

  // start fresh with modelview matrix
  glm_mat4_copy(g_base_modelview_matrix, multicolor_shader->modelview_matrix);

  // transform matrix for left quad
  glm_translate(multicolor_shader->modelview_matrix, (vec3){g_logical_width * 1.f / 4.f, g_logical_height / 2.f, 0.f});
  KRR_gputil_update_modelview_matrix(multicolor_shader->modelview_matrix_location, multicolor_shader->modelview_matrix);

  // render left quad
  glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);

  // bind right vao
  glBindVertexArray(right_vao);

  // start fresh
  glm_mat4_copy(g_base_modelview_matrix, multicolor_shader->modelview_matrix);
  // transform matrix for right quad
  glm_translate(multicolor_shader->modelview_matrix, (vec3){g_logical_width * 3.f / 4.f, g_logical_height / 2.f, 0.f });
  KRR_gputil_update_modelview_matrix(multicolor_shader->modelview_matrix_location, multicolor_shader->modelview_matrix);

  // render right quad
  glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);

  // unbind shader
  KRR_SHADERPROG_unbind(multicolor_shader->program);

  // disable scissor (if needed)
  if (g_need_clipping)
  {
    glDisable(GL_SCISSOR_TEST);
  }
}

void usercode_render_fps(int avg_fps)
{
#ifndef DISABLE_FPS_CALC
  // form framerate string to render
  snprintf(fps_text, FPS_BUFFER-1, "%d", avg_fps);

  // bind fps-vao
  glBindVertexArray(fps_vao);

  // use shared font shader
  KRR_SHADERPROG_bind(shared_font_shaderprogram->program);
    // start with clean state of modelview matrix
    glm_mat4_copy(g_base_modelview_matrix, shared_font_shaderprogram->modelview_matrix);
    KRR_FONTSHADERPROG2D_update_modelview_matrix(shared_font_shaderprogram);

    // render text on top right
    KRR_FONT_render_textex(fps_font, fps_text, 0.f, 4.f, &(SIZE){g_logical_width, g_logical_height}, KRR_FONT_TEXTALIGNMENT_RIGHT | KRR_FONT_TEXTALIGNMENT_TOP);
  KRR_SHADERPROG_unbind(shared_font_shaderprogram->program);

  // unbind fps-vao
  glBindVertexArray(0);
#endif 
}

void usercode_close()
{
#ifndef DISABLE_FPS_CALC
  if (fps_vao == 0)
  {
    glDeleteVertexArrays(1, &fps_vao);
    fps_vao = 0;
  }
  if (fps_font != NULL)
    KRR_FONT_free(fps_font);
#endif
  if (font != NULL)
    KRR_FONT_free(font);
  if (font_shader != NULL)
    KRR_FONTSHADERPROG2D_free(font_shader);
  if (texture_shader != NULL)
    KRR_TEXSHADERPROG2D_free(texture_shader);

  if (multicolor_shader != NULL)
    KRR_DMULTICSHADERPROG2D_free(multicolor_shader);

  if (vertex_vbo != 0)
    glDeleteBuffers(1, &vertex_vbo);
  if (rgby_vbo != 0)
    glDeleteBuffers(1, &rgby_vbo);
  if (cymw_vbo != 0)
    glDeleteBuffers(1, &cymw_vbo);
  if (gray_vbo != 0)
    glDeleteBuffers(1, &gray_vbo);
  if (left_vao != 0)
    glDeleteVertexArrays(1, &left_vao);
  if (right_vao != 0)
    glDeleteVertexArrays(1, &right_vao);
}