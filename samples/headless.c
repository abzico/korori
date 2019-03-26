// similar to readobjfile sample, but headless and write frame snapshot to .tga file after 1st frame drew

#include "usercode.h"
#include "functs.h"
#include "krr/foundation/common.h"
#include "krr/foundation/window.h"
#include "krr/foundation/util.h"
#include "krr/foundation/cam.h"
#include "krr/graphics/util.h"
#include "krr/graphics/texturedpp2d.h"
#include "krr/graphics/texturedpp3d.h"
#include "krr/graphics/model.h"
#include "krr/graphics/font.h"
#include "krr/graphics/fontpp2d.h"
#include "krr/graphics/objloader.h"

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

// TODO: define variables here
static KRR_TEXTURE* texture = NULL;
static SIMPLEMODEL* sm = NULL;

static int num_frame = 0;
static bool took_snapshot = false;

static bool take_frame_snapshot_tofile(const char* dst_filepath)
{
  const int number_of_pixels = g_logical_width * g_logical_height * 3;
  unsigned char pixels[number_of_pixels];

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadBuffer(GL_FRONT);
  // use GL_BGR because TGA stores its data in file in little-endian
  glReadPixels(0, 0, g_logical_width, g_logical_height, GL_BGR, GL_UNSIGNED_BYTE, pixels);

  FILE *out_file = fopen(dst_filepath, "w");
  if (out_file == NULL)
  {
    KRR_LOGE("Error attempting to open file for writing");
    return false;
  }

  // check the header format of tga at http://www.paulbourke.net/dataformats/tga/
  unsigned char header[18];
  memset(header, 0, 18);
  // now we selectively set only the interested fields
  header[2] = 2;  // data type code, it's uncompressed RGB image
  header[12] = g_logical_width & 0x00FF;  // low-order bytes for width
  header[13] = (g_logical_width & 0xFF00) >> 8;  // high-order bytes for width
  header[14] = g_logical_height & 0x00FF; // low-order bytes for height
  header[15] = (g_logical_height & 0xFF00) >> 8; // high-order bytes for height
  header[16] = 24;  // number of bits per pixel

  if (fwrite(header, sizeof(header), 1, out_file) != 1)
  {
    KRR_LOGE("Error writing header section for .tga file");
    fclose(out_file);
    return false;
  }
  if (fwrite(pixels, sizeof(pixels), 1, out_file) != 1)
  {
    KRR_LOGE("Error writing image data section for .tga file");
    fclose(out_file);
    return false;
  }

  // close file
  fclose(out_file);

  return true;
}

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
	glm_ortho(0.0, g_screen_width, g_screen_height, 0.0, -300.0, 600.0, g_ui_projection_matrix);
  glm_perspective(GLM_PI_4f, g_screen_width * 1.0f / g_screen_height, 0.01f, 10000.0f, g_projection_matrix);
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

  // initially start user's camera looking at -z, and up with +y
  glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, cam.forward);
  glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, cam.up);
  glm_vec3_copy((vec3){0.0f, 1.0f, 50.0f}, cam.pos);

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
  if (!KRR_TEXTURE_load_texture_from_file(texture, "res/models/Horse.png"))
  {
    KRR_LOGE("Error loading model's texture");
    return false;
  }

  // initially update all related matrices and related graphics stuff for both basic shaders
  SU_BEGIN(texture_shader)
    SU_TEXSHADERPROG2D(texture_shader)
    // set texture unit
    KRR_TEXSHADERPROG2D_set_texture_sampler(texture_shader, 0);

  SU_BEGIN(texture3d_shader)
    SU_TEXSHADERPROG3D(texture3d_shader)
    // set texture unit
    KRR_TEXSHADERPROG3D_set_texture_sampler(texture3d_shader, 0);
    // set lighting
    vec3 light_pos = {0.0f, 6.0f, 10.0f};
    vec3 light_color = {1.0f, 1.f, 1.f};
    memcpy(&texture3d_shader->light.pos, &light_pos, sizeof(VERTEXPOS3D));
    memcpy(&texture3d_shader->light.color, &light_color, sizeof(COLOR3F));
    KRR_TEXSHADERPROG3D_update_light(texture3d_shader);
    // set specular lighting
    texture3d_shader->shine_damper = 10.0f;
    texture3d_shader->reflectivity = 0.1f;
    KRR_TEXSHADERPROG3D_update_shininess(texture3d_shader);
    
  SU_BEGIN(font_shader)
    SU_FONTSHADER(font_shader)
    // set texture unit
    KRR_FONTSHADERPROG2D_set_texture_sampler(font_shader, 0);
  SU_END(font_shader)

  // load .obj model
  sm = SIMPLEMODEL_new();
  if (!SIMPLEMODEL_load_objfile(sm, "res/models/horse.obj"))
  {
    KRR_LOGE("Error loading .obj file");
    return false;
  }

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
				glm_ortho(0.0, g_ri_view_width, g_ri_view_height, 0.0, -300.0, 600.0, g_ui_projection_matrix);
        glm_perspective(GLM_PI_4f, g_ri_view_width * 1.0f / g_ri_view_height, 0.01f, 10000.0f, g_projection_matrix);

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
				glm_ortho(0.0, g_ri_view_width, g_ri_view_height, 0.0, -300.0, 600.0, g_ui_projection_matrix);
        glm_perspective(GLM_PI_4f, g_ri_view_width * 1.0f / g_ri_view_height, 0.01f, 10000.0f, g_projection_matrix);

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
}

void usercode_render_ui_text()
{
}

void usercode_render()
{
  // take a snapshot and write into .tga file
  if (num_frame >= 10 && !took_snapshot)
  {
    took_snapshot = true;

    if (take_frame_snapshot_tofile("headless-snapshot.tga"))
    {
      KRR_LOG("Successfully took snapshot and wrote into .tga file");

      // then let's call it a day, we're done
      // manually push key press event to simulate quit as handling app quit is handled
      // in main.c
      SDL_Event event;
      event.type = SDL_KEYDOWN;
      event.key.keysym.sym = SDLK_ESCAPE;
      SDL_PushEvent(&event);
    }
  }

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

  // TODO: render code goes here...
  // bind vao
  glBindVertexArray(sm->vao_id);
    // bind shader
    KRR_SHADERPROG_bind(texture3d_shader->program);

    // bind texture
    glBindTexture(GL_TEXTURE_2D, texture->texture_id);

    // transform model matrix
    glm_mat4_copy(g_base_model_matrix, texture3d_shader->model_matrix);
    glm_translate(texture3d_shader->model_matrix, (vec3){0.0f, -6.0f, 0.f});
    KRR_TEXSHADERPROG3D_update_model_matrix(texture3d_shader);

    // render
    SIMPLEMODEL_render(sm);

    // unbind shader
    KRR_SHADERPROG_unbind(texture3d_shader->program);

  // unbind vao
  glBindVertexArray(0);

  // disable scissor (if needed)
  if (g_need_clipping)
  {
    glDisable(GL_SCISSOR_TEST);
  }

  ++num_frame;
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

  if (sm != NULL)
  {
    SIMPLEMODEL_free(sm);
    sm = NULL;
  }
}
