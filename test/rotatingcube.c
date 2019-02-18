/// sameple for rotatingcube creating VBO, IBO, and VAO by hands in which position and texture coordinate are separated in two VBOs
/// note: to have a clear texture mapping on the top and bottom face, you need to duplicate define vertices as
///       a vertex can have only one texture coordinate,  but it's not the case in this example.

#include "usercode.h"
#include "foundation/common.h"
#include "foundation/window.h"
#include "foundation/util.h"
#include "graphics/util.h"
#include "graphics/texturedpp2d.h"
#include "graphics/texturedpp3d.h"
#include "graphics/font.h"
#include "graphics/fontpp2d.h"

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
static mat4 g_view_matrix;
// base model matrix to reduce some of mathematics operation initially
static mat4 g_base_model_matrix;
// -- section of variables for maintaining aspect ratio -- //

// -- section of function signatures -- //
static void usercode_app_went_windowed_mode();
static void usercode_app_went_fullscreen();

enum USERCODE_MATRIXTYPE
{
  USERCODE_MATRIXTYPE_PROJECTION_MATRIX,
  USERCODE_MATRIXTYPE_VIEW_MATRIX,
  USERCODE_MATRIXTYPE_MODEL_MATRIX
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

// basic shaders and font
static KRR_TEXSHADERPROG2D* texture_shader = NULL;
static KRR_TEXSHADERPROG3D* texture3d_shader = NULL;
static KRR_FONTSHADERPROG2D* font_shader = NULL;
static KRR_FONT* font = NULL;

// TODO: define variables here...
static float texture_ratio; // will be filled
static KRR_TEXTURE* texture = NULL;
static GLuint vao = 0;
static GLuint vertex_vbo = 0;
static GLuint texcoord_vbo = 0;
static GLuint normal_vbo = 0;
static GLuint ibo = 0;

static float rotx = 0.f, roty = 0.f, rotz = 0.f;

void usercode_set_matrix_then_update_to_shader(enum USERCODE_MATRIXTYPE matrix_type, enum USERCODE_SHADERTYPE shader_program, void* program)
{
  // projection matrix
  if (matrix_type == USERCODE_MATRIXTYPE_PROJECTION_MATRIX)
  {
    // texture shader
    if (shader_program == USERCODE_SHADERTYPE_TEXTURE_SHADER)
    {
      // convert to right type of program shader
      KRR_TEXSHADERPROG3D* shader_ptr = (KRR_TEXSHADERPROG3D*)program;

      // copy calculated projection matrix to shader's then update to shader
      glm_mat4_copy(g_projection_matrix, shader_ptr->projection_matrix);

      KRR_TEXSHADERPROG3D_update_projection_matrix(shader_ptr);
    }
    // font shader
    else if (shader_program == USERCODE_SHADERTYPE_FONT_SHADER)
    {
      KRR_FONTSHADERPROG2D* shader_ptr = (KRR_FONTSHADERPROG2D*)program;
      glm_mat4_copy(g_projection_matrix, shader_ptr->projection_matrix);

      KRR_FONTSHADERPROG2D_update_projection_matrix(shader_ptr);
    }
  }
  // view matrix
  else if (matrix_type == USERCODE_MATRIXTYPE_VIEW_MATRIX)
  {
    // texture shader
    if (shader_program == USERCODE_SHADERTYPE_TEXTURE_SHADER)
    {
      KRR_TEXSHADERPROG3D* shader_ptr = (KRR_TEXSHADERPROG3D*)program;
      glm_mat4_copy(g_view_matrix, shader_ptr->view_matrix);

      KRR_TEXSHADERPROG3D_update_view_matrix(shader_ptr);
    }
  }
  // model matrix
  else if (matrix_type == USERCODE_MATRIXTYPE_MODEL_MATRIX)
  {
    // texture shader
    if (shader_program == USERCODE_SHADERTYPE_TEXTURE_SHADER)
    {
      KRR_TEXSHADERPROG3D* shader_ptr = (KRR_TEXSHADERPROG3D*)program;
      glm_mat4_copy(g_base_model_matrix, shader_ptr->model_matrix);

      KRR_TEXSHADERPROG3D_update_model_matrix(shader_ptr);
    }
    // font shader
    else if (shader_program == USERCODE_SHADERTYPE_FONT_SHADER)
    {
      KRR_FONTSHADERPROG2D* shader_ptr = (KRR_FONTSHADERPROG2D*)program;
      glm_mat4_copy(g_base_model_matrix, shader_ptr->model_matrix);

      KRR_FONTSHADERPROG2D_update_model_matrix(shader_ptr);
    }
  }
}

void usercode_app_went_windowed_mode()
{
  KRR_SHADERPROG_bind(texture_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_VIEW_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODEL_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture_shader);

  KRR_SHADERPROG_bind(texture3d_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture3d_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_VIEW_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture3d_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODEL_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture3d_shader);
  // no need to unbind as we will bind a new one soon

  KRR_SHADERPROG_bind(font_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_FONT_SHADER, font_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODEL_MATRIX, USERCODE_SHADERTYPE_FONT_SHADER, font_shader);
  KRR_SHADERPROG_unbind(font_shader->program);
}

void usercode_app_went_fullscreen()
{
  KRR_SHADERPROG_bind(texture_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_VIEW_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODEL_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture_shader);

  KRR_SHADERPROG_bind(texture3d_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture3d_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_VIEW_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture3d_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODEL_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture3d_shader);

  KRR_SHADERPROG_bind(font_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_FONT_SHADER, font_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODEL_MATRIX, USERCODE_SHADERTYPE_FONT_SHADER, font_shader);
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
	glm_ortho(0.0f, (float)g_screen_width, (float)g_screen_height, 0.0f, -300.0f, 600.0f, g_projection_matrix);
  // calculate view matrix
  glm_mat4_identity(g_view_matrix);
	// calculate base model matrix (to reduce some of operations cost)
	glm_mat4_identity(g_base_model_matrix);
	glm_scale(g_base_model_matrix, (vec3){ g_ri_scale_x, g_ri_scale_y, 1.f});

  // initialize the viewport
  // define the area where to render, for now full screen
  glViewport(0, 0, g_screen_width, g_screen_height);

  // initialize clear color
  glClearColor(0.f, 0.f, 0.f, 1.f);

  // enable blending with default blend function
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_CULL_FACE);
  
  glEnable(GL_DEPTH_TEST);

  // check for errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    KRR_util_print_callstack();
    KRR_LOGE("Error initializing OpenGL! %s", KRR_gputil_error_string(error));
    return false;
  }

  return true;
}

bool usercode_loadmedia()
{
  // load texture shader
  texture_shader = KRR_TEXSHADERPROG2D_new();
  if (!KRR_TEXSHADERPROG2D_load_program(texture_shader))
  {
    KRR_LOGE("Error loading texture shader");
    return false;
  }
  shared_textured_shaderprogram = texture_shader;

  // load texture3d shader
  texture3d_shader = KRR_TEXSHADERPROG3D_new();
  if (!KRR_TEXSHADERPROG3D_load_program(texture3d_shader))
  {
    KRR_LOGE("Error loading texture3d shader");
    return false;
  }
  
  // load font shader
  font_shader = KRR_FONTSHADERPROG2D_new();
  if (!KRR_FONTSHADERPROG2D_load_program(font_shader))
  {
    KRR_LOGE("Error loading font shader");
    return false;
  }
  // set font shader to all KRR_FONT as active
  shared_font_shaderprogram = font_shader;

  // create font
  font = KRR_FONT_new();
  if (!KRR_FONT_load_freetype(font, "res/fonts/Minecraft.ttf", 40))
  {
    KRR_LOGE("Error to load font");
    return false;
  }

  // load font to render framerate
#ifndef DISABLE_FPS_CALC
  {
    fps_font = KRR_FONT_new();
    if (!KRR_FONT_load_freetype(fps_font, "res/fonts/Minecraft.ttf", 14))
    {
      KRR_LOGE("Unable to load font for rendering framerate");
      return false;
    }
  }
#endif

  // TODO: Load media here...
  texture = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(texture, "res/alien-arcade.png"))
  {
    KRR_LOGE("Error loading alien-arcade texture");
    return false;
  }
  // set aspect ratio of this texture
  texture_ratio = texture->width * 1.0f / texture->height;

  // initially update all related matrices and related graphics stuf for both shaders
  KRR_SHADERPROG_bind(texture_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_VIEW_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODEL_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture_shader);
  KRR_TEXSHADERPROG2D_set_texture_sampler(texture_shader, 0);
  
  KRR_SHADERPROG_bind(texture3d_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture3d_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_VIEW_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture3d_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODEL_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture3d_shader);
  KRR_TEXSHADERPROG3D_set_texture_sampler(texture3d_shader, 0);

  KRR_SHADERPROG_bind(font_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_FONT_SHADER, font_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODEL_MATRIX, USERCODE_SHADERTYPE_FONT_SHADER, font_shader);
  KRR_FONTSHADERPROG2D_set_texture_sampler(font_shader, 0);
  KRR_SHADERPROG_unbind(font_shader->program);

  // cube representation
  //
  // number is indices-index
  // faces are as follows (defined counter-clockwise; CCW, and divide quad into 2 triangles from top-left to bottom-right)
  // - front-face
  //    - 0-1-2
  //    - 0-2-3
  // - right-face
  //    - 3-2-6
  //    - 3-6-7
  // - back-face
  //    - 7-6-5
  //    - 7-5-4
  // - left-face
  //    - 4-5-1
  //    - 4-1-0
  // - up-face
  //    - 4-0-3
  //    - 4-3-7
  // - bottom-face
  //    - 1-5-6
  //    - 1-6-2
  //
  //
  //        4--------7
  //       /|       /|
  //      / |      / |
  //     0--------3  |
  //     |  5-----|--6
  //     | /      | /
  //     1--------2
  //
  //
  // set up VBO data
  // front side
  VERTEXPOS3D quad_pos[8] = {
    { -1.0f, -1.0f,  1.0f },
    { -1.0f,  1.0f,  1.0f },
    {  1.0f,  1.0f,  1.0f },
    {  1.0f, -1.0f,  1.0f },
    { -1.0f, -1.0f, -1.0f },
    { -1.0f,  1.0f, -1.0f },
    {  1.0f,  1.0f, -1.0f },
    {  1.0f, -1.0f, -1.0f }, 
  };

  // texture coordinate taken into account of padded POT texture, and un-aligned texture pixel
  const float texcoord_width_offset_start = 0.5f/texture->physical_width_;
  const float texcoord_width_offset_end = -0.5f/texture->physical_width_;
  const float texcoord_height_offset_start = 0.5f/texture->physical_height_;
  const float texcoord_height_offset_end = -0.5f/texture->physical_height_;

  #define TEXCOORD_WOS texcoord_width_offset_start
  #define TEXCOORD_WOE texcoord_width_offset_end
  #define TEXCOORD_HOS texcoord_height_offset_start
  #define TEXCOORD_HOE texcoord_height_offset_end
  
  TEXCOORD2D texcoord[8] = {
    { 0.0f + TEXCOORD_WOS, 0.0f + TEXCOORD_HOS },
    { 0.0f + TEXCOORD_WOS, texture->height*1.0f/texture->physical_height_ + TEXCOORD_HOE },
    { texture->width*1.0f/texture->physical_width_ + TEXCOORD_WOE, texture->height*1.0f/texture->physical_height_ + TEXCOORD_HOE },
    { texture->width*1.0f/texture->physical_width_ + TEXCOORD_WOE, 0.0f + TEXCOORD_HOS },
    { texture->width*1.0f/texture->physical_width_ + TEXCOORD_WOE, 0.0f + TEXCOORD_HOS },
    { texture->width*1.0f/texture->physical_width_ + TEXCOORD_WOE, texture->height*1.0f/texture->physical_height_ + TEXCOORD_HOE },
    { 0.0f + TEXCOORD_WOS, texture->height*1.0f/texture->physical_height_ + TEXCOORD_HOE },
    { 0.0f + TEXCOORD_WOS, 0.0f + TEXCOORD_HOS }
  };

  NORMAL normal_dup = {1.0f, 1.0f, 1.0f};
  NORMAL normals[8] = {
    normal_dup,
    normal_dup,
    normal_dup,
    normal_dup,
    normal_dup,
    normal_dup,
    normal_dup,
    normal_dup
  };

  GLuint indices[36] = { 
    0, 1, 2,
    0, 2, 3,
    3, 2, 6,
    3, 6, 7,
    7, 6, 5,
    7, 5, 4,
    4, 5, 1,
    4, 1, 0,
    4, 0, 3,
    4, 3, 7,
    1, 5, 6,
    1, 6, 2 
  };

  // create VBOs
  glGenBuffers(1, &vertex_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(VERTEXPOS3D), quad_pos, GL_STATIC_DRAW);

  glGenBuffers(1, &texcoord_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo);
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(TEXCOORD2D), texcoord, GL_STATIC_DRAW);

  // trick that we create normals that give good result of lighting
  // as we need it to have the size elements (size) of vertice array
  // it's fake
  glGenBuffers(1, &normal_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, normal_vbo);
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(NORMAL), normals, GL_STATIC_DRAW);

  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(GLuint), indices, GL_STATIC_DRAW);

  // vao, then bind
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // enable vertex attributes
  KRR_TEXSHADERPROG3D_enable_attrib_pointers(texture3d_shader);

  // set vertex data
  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
  KRR_TEXSHADERPROG3D_set_vertex_pointer(texture3d_shader, sizeof(VERTEXPOS3D), NULL);

  // set texcoord data
  glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo);
  KRR_TEXSHADERPROG3D_set_texcoord_pointer(texture3d_shader, sizeof(TEXCOORD2D), NULL);

  // set normal data
  glBindBuffer(GL_ARRAY_BUFFER, normal_vbo);
  KRR_TEXSHADERPROG3D_set_normal_pointer(texture3d_shader, sizeof(NORMAL), NULL);

  // ibo
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

  // unbind vao
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
				glm_ortho(0.0f, (float)g_ri_view_width, (float)g_ri_view_height, 0.0f, -300.0f, 600.0f, g_projection_matrix);

				// re-calculate base model matrix
				// no need to scale as it's uniform 1.0 now
				glm_mat4_identity(g_base_model_matrix);

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
				glm_ortho(0.0f, (float)g_ri_view_width, (float)g_ri_view_height, 0.0f, -300.0f, 600.0f, g_projection_matrix);

				// re-calculate base model matrix
				glm_mat4_identity(g_base_model_matrix);
				// also scale
				glm_scale(g_base_model_matrix, (vec3){ g_ri_scale_x, g_ri_scale_y, 1.f});

				// signal that app went fullscreen mode
				usercode_app_went_fullscreen();
      }
    }
  }
}

void usercode_update(float delta_time)
{
  rotx += 1.f;
  if (rotx >= 360.f)
  {
    rotx -= 360.f;
  }

  roty += 1.5f;
  if (roty >= 360.f)
  {
    roty -= 360.f;
  }

  rotz += 2.f;
  if (rotz >= 360.0f)
  {
    rotz -= 360.0f;
  }
}

void usercode_render()
{
  // clear color buffer
  if (g_need_clipping)
    glClearColor(0.f, 0.f, 0.f, 1.f);
  else
    glClearColor(CONTENT_BG_COLOR);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
  // bind vao
  glBindVertexArray(vao);

  // bind shader
  KRR_SHADERPROG_bind(texture3d_shader->program);

  // bind texture
  glBindTexture(GL_TEXTURE_2D, texture->texture_id);

  // update position of light to base on the original position of model first then offset further
  VERTEXPOS3D light_pos = {g_logical_width/2.0f * g_ri_scale_x, g_logical_height/2.0f * g_ri_scale_y + 300.0f, 50.0f};
  COLOR3F light_color = {1.0f, 1.f, 1.f};

  memcpy(&texture3d_shader->light.pos, &light_pos, sizeof(light_pos));
  memcpy(&texture3d_shader->light.color, &light_color, sizeof(light_color));
  KRR_TEXSHADERPROG3D_update_light(texture3d_shader);

  // rotate
  glm_mat4_copy(g_base_model_matrix, texture3d_shader->model_matrix);
  glm_translate(texture3d_shader->model_matrix, (vec3){g_logical_width/2.f, g_logical_height/2.f, 0.f});
  glm_scale(texture3d_shader->model_matrix, (vec3){80.0f, 80.0f, 80.0f});
  glm_rotate(texture3d_shader->model_matrix, glm_rad(roty), (vec3){0.f, 1.f, 0.f});
  glm_rotate(texture3d_shader->model_matrix, glm_rad(rotx), (vec3){1.f, 0.f, 0.f});
  glm_rotate(texture3d_shader->model_matrix, glm_rad(rotz), (vec3){0.f, 0.f, 1.f});
  // update model matrix
  KRR_TEXSHADERPROG3D_update_model_matrix(texture3d_shader);

  // render quad
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);


  // unbind shader
  KRR_SHADERPROG_unbind(texture3d_shader->program);

  // unbind vao
  glBindVertexArray(0);

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

  // bind font's vao
  KRR_FONT_bind_vao(fps_font);

  // use shared font shader
  KRR_SHADERPROG_bind(shared_font_shaderprogram->program);
    // start with clean state of model matrix
    glm_mat4_copy(g_base_model_matrix, shared_font_shaderprogram->model_matrix);
    KRR_FONTSHADERPROG2D_update_model_matrix(shared_font_shaderprogram);

    // render text on top right
    KRR_FONT_render_textex(fps_font, fps_text, 0.f, 4.f, &(SIZE){g_logical_width, g_logical_height}, KRR_FONT_TEXTALIGNMENT_RIGHT | KRR_FONT_TEXTALIGNMENT_TOP);
  KRR_SHADERPROG_unbind(shared_font_shaderprogram->program);

  // unbind fps-vao
  KRR_FONT_unbind_vao(fps_font);
#endif 
}

void usercode_close()
{
#ifndef DISABLE_FPS_CALC
  if (fps_font != NULL)
  {
    KRR_FONT_free(fps_font);
    fps_font = NULL;
  }
#endif
  if (font != NULL)
  {
    KRR_FONT_free(font);
    font = NULL;
  }
  if (font_shader != NULL)
  {
    KRR_FONTSHADERPROG2D_free(font_shader);
    font_shader = NULL;
  }
  if (texture_shader != NULL)
  {
    KRR_TEXSHADERPROG2D_free(texture_shader);
    texture_shader = NULL;
  }
  if (texture3d_shader != NULL)
  {
    KRR_TEXSHADERPROG3D_free(texture3d_shader);
    texture3d_shader = NULL;
  }

  if (texture != NULL)
  {
    KRR_TEXTURE_free(texture);
    texture = NULL;
  }

  if (vertex_vbo != 0)
    glDeleteBuffers(1, &vertex_vbo);
  if (texcoord_vbo != 0) 
    glDeleteBuffers(1, &texcoord_vbo);
  if (normal_vbo != 0)
    glDeleteBuffers(1, &normal_vbo);
  if (ibo != 0)
    glDeleteBuffers(1, &ibo);
  if (vao != 0)
    glDeleteVertexArrays(1, &vao);
}
