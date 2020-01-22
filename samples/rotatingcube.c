/// sameple for rotatingcube creating VBO, IBO, and VAO by hands in which position and texture coordinate are separated in two VBOs
/// note: to have a clear texture mapping on the top and bottom face, you need to duplicate define vertices as
///       a vertex can have only one texture coordinate,  but it's not the case in this example.

#include "usercode.h"
#include "functs.h"
#include "krr/foundation/common.h"
#include "krr/foundation/window.h"
#include "krr/foundation/util.h"
#include "krr/foundation/cam.h"
#include "krr/graphics/util.h"
#include "krr/graphics/texturedpp2d.h"
#include "krr/graphics/texturedpp3d.h"
#include "krr/graphics/font.h"
#include "krr/graphics/fontpp2d.h"

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
// -- section of variables for maintaining aspect ratio -- //

// -- section of function signatures -- //
static void usercode_app_went_windowed_mode();
static void usercode_app_went_fullscreen();
// -- end of section of function signatures -- //

// basic shaders and font
static KRR_TEXSHADERPROG2D* texture_shader = NULL;
static KRR_TEXSHADERPROG3D* texture3d_shader = NULL;
static KRR_FONTSHADERPROG2D* font_shader = NULL;
static KRR_FONT* font = NULL;
static KRR_CAM cam;

// TODO: define variables here...
static float texture_ratio; // will be filled
static KRR_TEXTURE* texture = NULL;
static GLuint vao = 0;
static GLuint vertex_vbo = 0;
static GLuint texcoord_vbo = 0;
static GLuint normal_vbo = 0;
static GLuint ibo = 0;

static float rotx = 0.f, roty = 0.f, rotz = 0.f;

void usercode_app_went_windowed_mode()
{
  SU_BEGIN(texture_shader)
    SU_TEXSHADERPROG2D(texture_shader)
  SU_BEGIN(texture3d_shader)
    SU_TEXSHADERPROG3D(texture3d_shader)
  SU_BEGIN(font_shader)
    SU_FONTSHADER(font_shader)
  SU_END(font_shader)
}

void usercode_app_went_fullscreen()
{
  SU_BEGIN(texture_shader)
    SU_TEXSHADERPROG2D(texture_shader)
  SU_BEGIN(texture3d_shader)
    SU_TEXSHADERPROG3D(texture3d_shader)
  SU_BEGIN(font_shader)
    SU_FONTSHADER(font_shader)
  SU_END(font_shader)
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
	glm_ortho(0.0f, (float)g_screen_width, (float)g_screen_height, 0.0f, -300.0f, 600.0f, g_ui_projection_matrix);
  glm_perspective(GLM_PI_4f, g_screen_width * 1.0f / g_screen_height, 0.01f, 10000.0f, g_projection_matrix);
  // calculate view matrix
  glm_mat4_identity(g_view_matrix);
	// calculate base model matrix (to reduce some of operations cost)
	glm_mat4_identity(g_base_model_matrix);

  // calculate base model for ui model matrix, and scale it
  glm_mat4_identity(g_base_ui_model_matrix);
	glm_scale(g_base_ui_model_matrix, (vec3){ g_ri_scale_x, g_ri_scale_y, 1.f});

  // initialize the viewport
  // define the area where to render, for now full screen
  glViewport(0, 0, g_screen_width, g_screen_height);

  // initialize clear color
  glClearColor(0.f, 0.f, 0.f, 1.f);

  // enable blending with default blend function
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // enable cull face, and depth test
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  // initially start user's camera looking at -z, and up with +y
  glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, cam.forward);
  glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, cam.up);
  glm_vec3_copy((vec3){0.0f, 1.0f, 20.0f}, cam.pos);

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
  SU_BEGIN(texture_shader)
    SU_TEXSHADERPROG2D(texture_shader)
    // set texture unit
    KRR_TEXSHADERPROG2D_set_texture_sampler(texture_shader, 0);

  SU_BEGIN(texture3d_shader)
    SU_TEXSHADERPROG3D(texture3d_shader)
    // set texture unit
    KRR_TEXSHADERPROG3D_set_texture_sampler(texture3d_shader, 0);
    // set lighting
    vec3 light_pos = {0.0f, 2.0f, 6.0f};
    vec3 light_color = {1.0f, 1.f, 1.f};
    memcpy(&texture3d_shader->lights[0].pos, &light_pos, sizeof(VERTEXPOS3D));
    memcpy(&texture3d_shader->lights[0].color, &light_color, sizeof(COLOR3F));
    KRR_TEXSHADERPROG3D_update_lights(texture3d_shader);
    // set specular lighting
    texture3d_shader->shine_damper = 10.0f;
    texture3d_shader->reflectivity = 0.5f;
    KRR_TEXSHADERPROG3D_update_shininess(texture3d_shader);

  SU_BEGIN(font_shader)
    SU_FONTSHADER(font_shader)
    // set texture unit
    KRR_FONTSHADERPROG2D_set_texture_sampler(font_shader, 0);
  SU_END(font_shader)

  // cube representation
  //
  // number is indices-index
  // faces are as follows (defined counter-clockwise; CCW, and divide quad into 2 triangles from top-left to bottom-right)
  // normal is calculated based on CCW per face
  //
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
  // - top-face
  //    - 4-0-3
  //    - 4-3-7
  // - bottom-face
  //    - 1-5-6
  //    - 1-6-2
  //
  //    but imagine above specification for duplicated vertices
  //    this means we have 24 vertices instead of just 8 to satisfy
  //    normal vector to be generated and able to be used in shader
  //    for each face as well as texture coordinate especially on top
  //    and bottom face.
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

  #define POS0 { -1.0f,  1.0f,  1.0f }
  #define POS1 { -1.0f, -1.0f,  1.0f }
  #define POS2 {  1.0f, -1.0f,  1.0f }
  #define POS3 {  1.0f,  1.0f,  1.0f }
  #define POS4 { -1.0f,  1.0f, -1.0f }
  #define POS5 { -1.0f, -1.0f, -1.0f }
  #define POS6 {  1.0f, -1.0f, -1.0f }
  #define POS7 {  1.0f,  1.0f, -1.0f }

  VERTEXPOS3D quad_pos[24] = {
    POS0, POS1, POS2, POS3, // front-face
    POS3, POS2, POS6, POS7, // right-face
    POS7, POS6, POS5, POS4, // back-face
    POS4, POS5, POS1, POS0, // left-face
    POS4, POS0, POS3, POS7, // top-face
    POS1, POS5, POS6, POS2  // bottom-face
  };

  // texture coordinate taken into account of padded POT texture, and un-aligned texture pixel
  const float texcoord_width_offset_start = 0.5f/texture->physical_width_;
  const float texcoord_width_offset_end = -0.5f/texture->physical_width_;
  const float texcoord_height_offset_start = 0.5f/texture->physical_height_;
  const float texcoord_height_offset_end = -0.5f/texture->physical_height_;

  #define TEXCOORD_WS texcoord_width_offset_start
  #define TEXCOORD_WE texture->width*1.0f/texture->physical_width_ + texcoord_width_offset_end
  #define TEXCOORD_HS texcoord_height_offset_start
  #define TEXCOORD_HE texture->height*1.0f/texture->physical_height_ + texcoord_height_offset_end
  #define TEXCOORD_WE_HALF (texture->width*1.0f/texture->physical_width_ + texcoord_width_offset_end)/2.2f
  #define TEXCOORD_HE_HALF (texture->height*1.0f/texture->physical_height_ + texcoord_height_offset_end)/2.2f
  
  // show the same texture on each face
  TEXCOORD2D texcoord[24] = {
    { TEXCOORD_WS, TEXCOORD_HS },   // front-face
    { TEXCOORD_WS, TEXCOORD_HE },
    { TEXCOORD_WE, TEXCOORD_HE },
    { TEXCOORD_WE, TEXCOORD_HS },

    { TEXCOORD_WS, TEXCOORD_HS },   // right-face
    { TEXCOORD_WS, TEXCOORD_HE },
    { TEXCOORD_WE, TEXCOORD_HE },
    { TEXCOORD_WE, TEXCOORD_HS },

    { TEXCOORD_WS, TEXCOORD_HS },   // back-face
    { TEXCOORD_WS, TEXCOORD_HE },
    { TEXCOORD_WE, TEXCOORD_HE },
    { TEXCOORD_WE, TEXCOORD_HS },

    { TEXCOORD_WS, TEXCOORD_HS },   // left-face
    { TEXCOORD_WS, TEXCOORD_HE },
    { TEXCOORD_WE, TEXCOORD_HE },
    { TEXCOORD_WE, TEXCOORD_HS },

    { TEXCOORD_WS, TEXCOORD_HS },   // top-face (only less than half of texture)
    { TEXCOORD_WS, TEXCOORD_HE_HALF },
    { TEXCOORD_WE_HALF, TEXCOORD_HE_HALF },
    { TEXCOORD_WE_HALF, TEXCOORD_HS },

    { TEXCOORD_WS, TEXCOORD_HS },   // bottom-face (only less than half of texture)
    { TEXCOORD_WS, TEXCOORD_HE_HALF },
    { TEXCOORD_WE_HALF, TEXCOORD_HE_HALF },
    { TEXCOORD_WE_HALF, TEXCOORD_HS },
  };

  #define NORM_ZUP {0.0f, 0.0f, 1.0f}
  #define NORM_ZDOWN {0.0f, 0.0f, -1.0f}
  #define NORM_XUP {1.0f, 0.0f, 0.0f}
  #define NORM_XDOWN {-1.0f, 0.0f, 0.0f}
  #define NORM_YUP {0.0f, 1.0f, 0.0f}
  #define NORM_YDOWN {0.0f, -1.0f, 0.0f}

  NORMAL normals[24] = {
    NORM_ZUP,
    NORM_ZUP,
    NORM_ZUP,
    NORM_ZUP,

    NORM_XUP,
    NORM_XUP,
    NORM_XUP,
    NORM_XUP,

    NORM_ZDOWN,
    NORM_ZDOWN,
    NORM_ZDOWN,
    NORM_ZDOWN,

    NORM_XDOWN,
    NORM_XDOWN,
    NORM_XDOWN,
    NORM_XDOWN,

    NORM_YUP,
    NORM_YUP,
    NORM_YUP,
    NORM_YUP,

    NORM_YDOWN,
    NORM_YDOWN,
    NORM_YDOWN,
    NORM_YDOWN
  };

  GLuint indices[36] = { 
    0, 1, 2,
    0, 2, 3,

    4, 5, 6,
    4, 6, 7,

    8, 9, 10,
    8, 10, 11,

    12, 13, 14,
    12, 14, 15,

    16, 17, 18,
    16, 18, 19,

    20, 21, 22,
    20, 22, 23
  };

  // create VBOs
  glGenBuffers(1, &vertex_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
  glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(VERTEXPOS3D), quad_pos, GL_STATIC_DRAW);

  glGenBuffers(1, &texcoord_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo);
  glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(TEXCOORD2D), texcoord, GL_STATIC_DRAW);

  glGenBuffers(1, &normal_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, normal_vbo);
  glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(NORMAL), normals, GL_STATIC_DRAW);

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
				glm_ortho(0.0f, (float)g_ri_view_width, (float)g_ri_view_height, 0.0f, -300.0f, 600.0f, g_ui_projection_matrix);
        glm_perspective(GLM_PI_4f, g_ri_view_width * 1.0f / g_ri_view_height, 0.01f, 10000.0f, g_projection_matrix);

				// re-calculate base model matrix
				// no need to scale as it's uniform 1.0 now
				glm_mat4_identity(g_base_ui_model_matrix);

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
				glm_ortho(0.0f, (float)g_ri_view_width, (float)g_ri_view_height, 0.0f, -300.0f, 600.0f, g_ui_projection_matrix);
        glm_perspective(GLM_PI_4f, g_ri_view_width * 1.0f / g_ri_view_height, 0.01f, 10000.0f, g_projection_matrix);

				// re-calculate base model matrix
				glm_mat4_identity(g_base_model_matrix);
				// also scale
				glm_scale(g_base_ui_model_matrix, (vec3){ g_ri_scale_x, g_ri_scale_y, 1.f});

				// signal that app went fullscreen mode
				usercode_app_went_fullscreen();
      }
    }
  }
}

void update_camera(float delta_time)
{
  vec3 cam_target;
  glm_vec3_add(cam.pos, cam.forward, cam_target);
  glm_lookat(cam.pos, cam_target, cam.up, g_view_matrix);

  // texture 3d
  KRR_SHADERPROG_bind(texture3d_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_VIEW_MATRIX, USERCODE_SHADERTYPE_TEXTURE3D_SHADER, texture3d_shader);
  KRR_SHADERPROG_unbind(texture3d_shader->program);
}

void usercode_update(float delta_time)
{
  update_camera(delta_time);

  rotx += 10.f * delta_time;
  if (rotx >= 360.f)
  {
    rotx -= 360.f;
  }

  roty += 12.5f * delta_time;
  if (roty >= 360.f)
  {
    roty -= 360.f;
  }

  rotz += 10.f * delta_time;
  if (rotz >= 360.0f)
  {
    rotz -= 360.0f;
  }
}

void usercode_render_ui_text()
{
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

    // rotate
    glm_mat4_copy(g_base_model_matrix, texture3d_shader->model_matrix);
    glm_scale(texture3d_shader->model_matrix, (vec3){2.0f, 2.0f, 2.0f});
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
    glm_mat4_copy(g_base_ui_model_matrix, shared_font_shaderprogram->model_matrix);
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
