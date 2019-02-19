// sample to read .obj file then show textured model on screen
// demonstrate manual building of VBO as part of VAO to draw loaded .obj file, and another wrapped one via SIMPLEMODEL struct

#include "usercode.h"
#include "foundation/common.h"
#include "foundation/window.h"
#include "foundation/util.h"
#include "graphics/util.h"
#include "graphics/texturedpp2d.h"
#include "graphics/texturedpp3d.h"
#include "graphics/font.h"
#include "graphics/fontpp2d.h"
#include "graphics/objloader.h"

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

// TODO: define variables here
static KRR_TEXTURE* texture = NULL;

static GLuint vao = 0;
static GLuint vertex_vbo = 0;
static GLuint ibo = 0;
static int indices_count = 0;

static float roty = 0.f;
static float rotz = 0.f;

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
  }
  // model matrix
  else if (matrix_type == USERCODE_MATRIXTYPE_MODEL_MATRIX)
  {
    // texture shader
    if (shader_program == USERCODE_SHADERTYPE_TEXTURE_SHADER)
    {
      KRR_TEXSHADERPROG2D* shader_ptr = (KRR_TEXSHADERPROG2D*)program;
      glm_mat4_copy(g_base_model_matrix, shader_ptr->model_matrix);

      KRR_TEXSHADERPROG2D_update_model_matrix(shader_ptr);
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
	// set projection, view and model matrix to both of basic shaders
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

void usercode_app_went_fullscreen()
{
	// set projection, view and model matrix to both of basic shaders
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
	glm_ortho(0.0, g_screen_width, g_screen_height, 0.0, -300.0, 600.0, g_projection_matrix);
  // calcualte view matrix
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

  // enable face culling
  glEnable(GL_CULL_FACE);

  // enable depth test
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
  // set texture shader to all KRR_TEXTURE as active
  shared_textured_shaderprogram = texture_shader;

  // load texture3d shader
  texture3d_shader = KRR_TEXSHADERPROG3D_new();
  if (!KRR_TEXSHADERPROG3D_load_program(texture3d_shader))
  {
    KRR_LOGE("Error loading texture3d shader");
    return false;
  }
  // set texture shader
  shared_textured3d_shaderprogram = texture3d_shader;
  
  // load font shader
  font_shader = KRR_FONTSHADERPROG2D_new();
  if (!KRR_FONTSHADERPROG2D_load_program(font_shader))
  {
    KRR_LOGE("Error loading font shader");
    return false;
  }
  // set font shader to all KRR_FONT as active
  shared_font_shaderprogram = font_shader;

#ifndef DISABLE_FPS_CALC
  // load font to render framerate
  {
    fps_font = KRR_FONT_new();
    if (!KRR_FONT_load_freetype(fps_font, "res/fonts/Minecraft.ttf", 14))
    {
      KRR_LOGE("Unable to load font for rendering framerate");
      return false;
    }
  }
#endif
  
  // create font
  font = KRR_FONT_new();
  if (!KRR_FONT_load_freetype(font, "res/fonts/Minecraft.ttf", 40))
  {
    KRR_LOGE("Error to load font");
    return false;
  }

  // texture
  texture = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(texture, "res/models/stallTexture.png"))
  {
    KRR_LOGE("Error loading model's texture");
    return false;
  }

  // initially update all related matrices and related graphics stuff for both basic shaders
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
  // update shininess
  texture3d_shader->shine_damper = 10.0f;
  texture3d_shader->reflectivity = 0.07f;
  KRR_TEXSHADERPROG3D_update_shininess(texture3d_shader);

  KRR_SHADERPROG_bind(font_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_PROJECTION_MATRIX, USERCODE_SHADERTYPE_FONT_SHADER, font_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_VIEW_MATRIX, USERCODE_SHADERTYPE_FONT_SHADER, font_shader);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_MODEL_MATRIX, USERCODE_SHADERTYPE_FONT_SHADER, font_shader);
  KRR_FONTSHADERPROG2D_set_texture_sampler(font_shader, 0);
  KRR_SHADERPROG_unbind(font_shader->program);
  
  // load vertices and indices from .obj file
  VERTEXTEXNORM3D* vertices = NULL;
  int vertices_count = 0;

  GLuint* indices = NULL;

  KRR_load_objfile("res/models/stall.obj", &vertices, &vertices_count, &indices, &indices_count);
  //for (int i=0; i<vertices_count; i++)
  //{
  //  KRR_LOGI("[%d] = %.2f, %.2f, %.2f", i, vertices[i].position.x, vertices[i].position.y, vertices[i].position.z);
  //  KRR_LOGI("[%d] = %.2f, %.2f", i, vertices[i].texcoord.s, vertices[i].texcoord.t);
  //  KRR_LOGI("[%d] = %.2f, %.2f, %.2f", i, vertices[i].normal.x, vertices[i].normal.y, vertices[i].normal.z);
  //  KRR_LOGI("");
  //}

  // create vbo
  glGenBuffers(1, &vertex_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices_count * sizeof(VERTEXTEXNORM3D), vertices, GL_STATIC_DRAW);

  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_count * sizeof(GLuint), indices, GL_STATIC_DRAW);

  // free vertices and indices
  free(vertices);
  vertices = NULL;
  free(indices);
  indices = NULL;

  // vao
  glGenVertexArrays(1, &vao);
  // bind vao
  glBindVertexArray(vao);

    // enable vertex attributes
    KRR_TEXSHADERPROG3D_enable_attrib_pointers(texture3d_shader);

    // set vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
    KRR_TEXSHADERPROG3D_set_vertex_pointer(texture3d_shader, sizeof(VERTEXTEXNORM3D), (GLvoid*)offsetof(VERTEXTEXNORM3D, position));
    KRR_TEXSHADERPROG3D_set_texcoord_pointer(texture3d_shader, sizeof(VERTEXTEXNORM3D), (GLvoid*)offsetof(VERTEXTEXNORM3D, texcoord));
    KRR_TEXSHADERPROG3D_set_normal_pointer(texture3d_shader, sizeof(VERTEXTEXNORM3D), (GLvoid*)offsetof(VERTEXTEXNORM3D, normal));

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
				glm_ortho(0.0, g_ri_view_width, g_ri_view_height, 0.0, -300.0, 600.0, g_projection_matrix);

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
				glm_ortho(0.0, g_ri_view_width, g_ri_view_height, 0.0, -300.0, 600.0, g_projection_matrix);

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
  roty -= 1.0f;
  if (roty <= -360.f)
  {
    roty += 360.f;
  }
  
  rotz += 1.0f;
  if (rotz >= 360.f)
  {
    rotz -= 360.f;
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
    vec3 light_pos = {g_logical_width/2.0f * g_ri_scale_x, g_logical_height*3.0f/4.0f * g_ri_scale_y, 100.f};
    vec3 light_color = {1.0f, 1.f, 1.f};

    memcpy(&texture3d_shader->light.pos, &light_pos, sizeof(light_pos));
    memcpy(&texture3d_shader->light.color, &light_color, sizeof(light_color));

    KRR_TEXSHADERPROG3D_update_light(texture3d_shader);

    // transform model matrix
    glm_mat4_copy(g_base_model_matrix, texture3d_shader->model_matrix);
    glm_translate(texture3d_shader->model_matrix, (vec3){g_logical_width/2.f, g_logical_height/2.f, 0.f});
    glm_scale(texture3d_shader->model_matrix, (vec3){30.0f, 30.0f, 30.0f});
    glm_rotate(texture3d_shader->model_matrix, glm_rad(roty), (vec3){1.f, 1.f, 0.f});
    //update model matrix
    KRR_TEXSHADERPROG3D_update_model_matrix(texture3d_shader);

    // render
    glDrawElements(GL_TRIANGLES, indices_count, GL_UNSIGNED_INT, NULL);

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

  // bind fps-vao
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
  if (ibo != 0)
    glDeleteBuffers(1, &ibo);
  if (vao != 0)
    glDeleteVertexArrays(1, &vao);
}
