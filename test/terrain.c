// terrain sample

#include "usercode.h"
#include "functs.h"
#include "foundation/common.h"
#include "foundation/window.h"
#include "foundation/util.h"
#include "foundation/cam.h"
#include "foundation/math.h"
#include "graphics/util.h"
#include "graphics/texturedpp2d.h"
#include "graphics/texturedpp3d.h"
#include "graphics/terrain_shader3d.h"
#include "graphics/terrain.h"
#include "graphics/model.h"
#include "graphics/font.h"
#include "graphics/fontpp2d.h"
#include <math.h>

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
static KRR_TERRAINSHADERPROG3D* terrain3d_shader = NULL;
static KRR_FONTSHADERPROG2D* font_shader = NULL;
static KRR_FONT* font = NULL;

// TODO: define variables here
static KRR_TEXTURE* terrain_texture = NULL;
static KRR_TEXTURE* grass_texture = NULL;
static KRR_TEXTURE* stall_texture = NULL;
static KRR_TEXTURE* tree_texture = NULL;
static SIMPLEMODEL* stall = NULL;
static SIMPLEMODEL* grass = NULL;
static SIMPLEMODEL* tree = NULL;
static TERRAIN* tr = NULL;
static KRR_CAM cam;
static float roty = 0.0f;

#define NUM_GRASS_UNIT 10
#define GRASS_RANDOM_SIZE 30
static vec3 randomized_grass_pos[NUM_GRASS_UNIT];

#define NUM_TREE 3
#define TREE_RANDOM_SIZE 50
static vec3 randomized_tree_pos[NUM_TREE];

#define TERRAIN_GRID_WIDTH 10
#define TERRAIN_GRID_HEIGHT 10
#define TERRAIN_SLOT_SIZE 200

static bool is_leftmouse_click = false;

void usercode_app_went_windowed_mode()
{
  SU_BEGIN(texture_shader)
    SU_TEXSHADERPROG2D(texture_shader)
  SU_BEGIN(texture3d_shader)
    SU_TEXSHADERPROG3D(texture3d_shader)
  SU_BEGIN(terrain3d_shader)
    SU_TERRAINSHADER(terrain3d_shader)
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
  SU_BEGIN(terrain3d_shader)
    SU_TERRAINSHADER(terrain3d_shader)
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
	glm_ortho(0.0, g_screen_width, g_screen_height, 0.0, -300.f, 6000.0, g_ui_projection_matrix);
  glm_perspective(GLM_PI_4f, g_screen_width * 1.0f / g_screen_height, 0.01f, 10000.0f, g_projection_matrix);
  // calcualte view matrix
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


  // enable face culling
  glEnable(GL_CULL_FACE);

  // enable depth test
  glEnable(GL_DEPTH_TEST);

  // initially start user's camera looking at -z, and up with +y
  glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, cam.forward);
  glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, cam.up);
  glm_vec3_copy((vec3){0.0f, 1.0f, 3.0f}, cam.pos);

  // seed random function with current time
  // we gonna use some random functions in this sample
  KRR_math_rand_seed_time();

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

  // load terrain3d shader
  terrain3d_shader = KRR_TERRAINSHADERPROG3D_new();
  if (!KRR_TERRAINSHADERPROG3D_load_program(terrain3d_shader))
  {
    KRR_LOGE("Error loading terrain3d shader");
    return false;
  }
  // set terrain shader
  shared_terrain3d_shaderprogram = terrain3d_shader;
  
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

  // terrain texture
  terrain_texture = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(terrain_texture, "res/models/grass.png"))
  {
    KRR_LOGE("Error loading terrain's texture");
    return false;
  }
  // grass texture
  grass_texture = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(grass_texture, "res/models/grassTexture.png"))
  {
    KRR_LOGE("Error loading grass's texture");
    return false;
  }
  // stall texture
  stall_texture = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(stall_texture, "res/models/stallTexture.png"))
  {
    KRR_LOGE("Error loading white texture");
    return false;
  }
  // tree texture
  tree_texture = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(tree_texture, "res/models/lowPolyTree.png"))
  {
    KRR_LOG("Error loading tree texture");
    return false;
  }

  // random all position of grass unit to show on terrain
  // **note: we have problem re-export grasssModel thus we need
  // to use original one which -y is UP
  for (int i=0; i<NUM_GRASS_UNIT; ++i)
  {
    glm_vec3_copy((vec3){KRR_math_rand_float2(-GRASS_RANDOM_SIZE, GRASS_RANDOM_SIZE), -1.0f, KRR_math_rand_float2(-GRASS_RANDOM_SIZE, GRASS_RANDOM_SIZE)}, randomized_grass_pos[i]);
  }

  for (int i=0; i<NUM_TREE; ++i)
  {
    glm_vec3_copy((vec3){KRR_math_rand_float2(-TREE_RANDOM_SIZE, TREE_RANDOM_SIZE), -1.0f, KRR_math_rand_float2(-TREE_RANDOM_SIZE, TREE_RANDOM_SIZE)}, randomized_tree_pos[i]);
  }

  // initially update all related matrices and related graphics stuff for both basic shaders
  SU_BEGIN(texture_shader)
    SU_TEXSHADERPROG2D(texture_shader)
    // set texture unit
    KRR_TEXSHADERPROG2D_set_texture_sampler(texture_shader, 0);

  SU_BEGIN(texture3d_shader)
    SU_TEXSHADERPROG3D(texture3d_shader)
    // update ambient color
    glm_vec3_copy((vec3){0.1f, 0.1f, 0.1f}, texture3d_shader->ambient_color);
    KRR_TEXSHADERPROG3D_update_ambient_color(texture3d_shader);
    // set texture unit
    KRR_TEXSHADERPROG3D_set_texture_sampler(texture3d_shader, 0);
    // set specular lighting
    texture3d_shader->shine_damper = 10.0f;
    texture3d_shader->reflectivity = 0.2f;
    KRR_TEXSHADERPROG3D_update_shininess(texture3d_shader);
    // set light info
    vec3 light_pos = {30.0f, 30.0f, 30.0f};
    vec3 light_color = {1.0f, 1.f, 1.f};
    memcpy(&texture3d_shader->light.pos, &light_pos, sizeof(light_pos));
    memcpy(&texture3d_shader->light.color, &light_color, sizeof(light_color));
    KRR_TEXSHADERPROG3D_update_light(texture3d_shader);

  SU_BEGIN(terrain3d_shader)
    SU_TERRAINSHADER(terrain3d_shader)
    // set ambient color
    glm_vec3_copy((vec3){0.7f, 0.7f, 0.7f}, terrain3d_shader->ambient_color);
    KRR_TERRAINSHADERPROG3D_update_ambient_color(terrain3d_shader);
    // set texture unit
    KRR_TERRAINSHADERPROG3D_set_texture_sampler(terrain3d_shader, 0);
    // set specular lighting
    terrain3d_shader->shine_damper = 10.0f;
    terrain3d_shader->reflectivity = 0.35f;
    KRR_TERRAINSHADERPROG3D_update_shininess(terrain3d_shader);
    // set repeatness over texture coord
    terrain3d_shader->texcoord_repeat = 30.0f;
    KRR_TERRAINSHADERPROG3D_update_texcoord_repeat(terrain3d_shader);
    // set light info
    memcpy(&terrain3d_shader->light.pos, &light_pos, sizeof(light_pos));
    memcpy(&terrain3d_shader->light.color, &light_color, sizeof(light_color));
    KRR_TERRAINSHADERPROG3D_update_light(terrain3d_shader);

  SU_BEGIN(font_shader)
    SU_FONTSHADER(font_shader)
    // set texture unit
    KRR_FONTSHADERPROG2D_set_texture_sampler(font_shader, 0);
  SU_END(font_shader)

  // load .obj model
  stall = SIMPLEMODEL_new();
  if (!SIMPLEMODEL_load_objfile(stall, "res/models/stall.obj"))
  {
    KRR_LOGE("Error loading stall model file");
    return false;
  }

  // load grass model
  grass = SIMPLEMODEL_new();
  if (!SIMPLEMODEL_load_objfile(grass, "res/models/grassModel.obj"))
  {
    KRR_LOGE("Error loading grass model file");
    return false;
  }

  // load tree mode
  tree = SIMPLEMODEL_new();
  if (!SIMPLEMODEL_load_objfile(tree, "res/models/lowPolyTree.obj"))
  {
    KRR_LOGE("Error loading tree model file");
    return false;
  }
  
  // load from generation of terrain
  tr = KRR_TERRAIN_new();
  if (!KRR_TERRAIN_load_from_generation(tr, TERRAIN_GRID_WIDTH, TERRAIN_GRID_HEIGHT, TERRAIN_SLOT_SIZE))
  {
    KRR_LOGE("Error loading terrain from generation");
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
				
				// re-calculate projection matrices for both ui and 3d view
				glm_ortho(0.0, g_ri_view_width, g_ri_view_height, 0.0, -300.0f, 6000.0, g_ui_projection_matrix);
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

				// re-calculate projection matrices for both ui and 3d view
				glm_ortho(0.0, g_ri_view_width, g_ri_view_height, 0.0, -300.0f, 6000.0, g_ui_projection_matrix);
        glm_perspective(GLM_PI_4f, g_ri_view_width * 1.0f / g_ri_view_height, 0.01f, 10000.0f, g_projection_matrix);

				// re-calculate base model matrix
				glm_mat4_identity(g_base_ui_model_matrix);
				// also scale
				glm_scale(g_base_ui_model_matrix, (vec3){ g_ri_scale_x, g_ri_scale_y, 1.f});

				// signal that app went fullscreen mode
				usercode_app_went_fullscreen();
      }
    }
  }
  else if (e->type == SDL_MOUSEBUTTONDOWN)
  {
    if (e->button.button == SDL_BUTTON_LEFT)
    {
      // allow to change forward, and up vector of user
      is_leftmouse_click = true;
    }
  }
  else if (e->type == SDL_MOUSEBUTTONUP)
  {
    if (e->button.button == SDL_BUTTON_LEFT)
    {
      // disable allowance to chagne direction of user's vectors
      is_leftmouse_click = false;
    }
  }

  if (is_leftmouse_click && e->type == SDL_MOUSEMOTION)
  {
    SDL_MouseMotionEvent motion = e->motion;

    bool need_update = false;

    // if there's change along x-axis
    // then we rotate user's y-axis
    if (motion.xrel != 0)
    {
      need_update = true;
    }

    // if there's change along y-axis
    // then we rotate user's x-axis
    if (motion.yrel != 0)
    {
      need_update = true;
    }

    // check if need to update view matrix
    if (need_update)
    {
      cam.rot[0] -= motion.xrel;
      cam.rot[1] -= motion.yrel;

      // clamp on increasing values
      // these will preserve the sign of cam.rot[x]
      cam.rot[0] = (int)cam.rot[0] % 360;
      cam.rot[1] = (int)cam.rot[1] % 360;

      // 1. rotate up & forward vector affected from changed in y-direction of mouse movement
      // rotate up vector
      vec3 up;
      glm_vec3_copy(GLM_YUP, up);
      glm_vec3_rotate(up, glm_rad(cam.rot[1]), GLM_XUP);
      glm_vec3_normalize(up);
      glm_vec3_copy(up, cam.up);

      // rotate forward vector
      vec3 forward;
      glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, forward);
      glm_vec3_rotate(forward, glm_rad(cam.rot[1]), GLM_XUP);
      glm_vec3_normalize(forward);

      // 2. rotate forward vector affected from change in x-direction of mouse movement
      glm_vec3_rotate(forward, glm_rad(cam.rot[0]), GLM_YUP);
      glm_vec3_normalize(forward);
      glm_vec3_copy(forward, cam.forward);
    }
  }

  // handle multiple key presses at once
  const Uint8* key_state = SDL_GetKeyboardState(NULL);

  // move speed is distance per second
  #define MOVE_SPEED 0.25f

  if (key_state[SDL_SCANCODE_A])
  {
    vec3 side;
    glm_vec3_cross(cam.forward, cam.up, side);
    vec3 temp;
    glm_vec3_scale(side, -MOVE_SPEED * delta_time, temp);
    glm_vec3_add(cam.pos, temp, cam.pos);
  }
  if (key_state[SDL_SCANCODE_D])
  {
    vec3 side;
    glm_vec3_cross(cam.forward, cam.up, side);
    vec3 temp;
    glm_vec3_scale(side, MOVE_SPEED * delta_time, temp);
    glm_vec3_add(cam.pos, temp, cam.pos);
  }
  if (key_state[SDL_SCANCODE_W])
  {
    vec3 temp;
    glm_vec3_scale(cam.forward, MOVE_SPEED * delta_time, temp);
    glm_vec3_add(cam.pos, temp, cam.pos);
  }
  if (key_state[SDL_SCANCODE_S])
  {
    vec3 temp;
    glm_vec3_scale(cam.forward, -MOVE_SPEED * delta_time, temp);
    glm_vec3_add(cam.pos, temp, cam.pos);
  }
  if (key_state[SDL_SCANCODE_E])
  {
    vec3 temp;
    glm_vec3_scale(cam.up, MOVE_SPEED * delta_time, temp);
    glm_vec3_add(cam.pos, temp, cam.pos);
  }
  if (key_state[SDL_SCANCODE_Q])
  {
    vec3 temp;
    glm_vec3_scale(cam.up, -MOVE_SPEED * delta_time, temp);
    glm_vec3_add(cam.pos, temp, cam.pos);
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

  // terrain 3d
  KRR_SHADERPROG_bind(terrain3d_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_VIEW_MATRIX, USERCODE_SHADERTYPE_TERRAIN_SHADER, terrain3d_shader);
  KRR_SHADERPROG_unbind(terrain3d_shader->program);
}

void usercode_update(float delta_time)
{
  update_camera(delta_time);

  roty += 0.3f;
  if (roty > 360.0f)
  {
    roty -= 360.0f;
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

  // TODO: render code goes here...

  // bind shader
  KRR_SHADERPROG_bind(texture3d_shader->program);
  // render stall
  glBindVertexArray(stall->vao_id);
    // bind texture
    glBindTexture(GL_TEXTURE_2D, stall_texture->texture_id);

    // transform model matrix
    glm_mat4_copy(g_base_model_matrix, texture3d_shader->model_matrix);
    glm_translate(texture3d_shader->model_matrix, (vec3){0.0f, 0.0f, -50.0f});
    //update model matrix
    KRR_TEXSHADERPROG3D_update_model_matrix(texture3d_shader);

    // render
    SIMPLEMODEL_render(stall);

  // render tree
  glBindVertexArray(tree->vao_id);
    // bind texture
    glBindTexture(GL_TEXTURE_2D, tree_texture->texture_id);

    for (int i=0; i<NUM_TREE; ++i)
    {
      // transform model matrix
      glm_mat4_copy(g_base_model_matrix, texture3d_shader->model_matrix);
      glm_translate(texture3d_shader->model_matrix, randomized_tree_pos[i]);
      // update model matrix
      KRR_TEXSHADERPROG3D_update_model_matrix(texture3d_shader);

      // render
      SIMPLEMODEL_render(tree);
    }

  // unbind shader
  KRR_SHADERPROG_unbind(texture3d_shader->program);

  // bind shader
  KRR_SHADERPROG_bind(terrain3d_shader->program);
  // render terrain
  glBindVertexArray(tr->vao_id);
    // bind texture
    glBindTexture(GL_TEXTURE_2D, terrain_texture->texture_id);
    // wrap texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    // transform model matrix
    glm_mat4_copy(g_base_model_matrix, terrain3d_shader->model_matrix);
    // rotate around itself
    //glm_rotate(terrain3d_shader->model_matrix, glm_rad(roty), GLM_YUP);
    glm_translate(terrain3d_shader->model_matrix, (vec3){-TERRAIN_GRID_WIDTH*TERRAIN_SLOT_SIZE/2, 0.0f, -TERRAIN_GRID_HEIGHT*TERRAIN_SLOT_SIZE/2});
    //update model matrix
    KRR_TERRAINSHADERPROG3D_update_model_matrix(terrain3d_shader);

    // render
    KRR_TERRAIN_render(tr);
  // unbind shader
  KRR_SHADERPROG_unbind(terrain3d_shader->program);

  // bind shader
  KRR_SHADERPROG_bind(texture3d_shader->program);
  // render grass
  glBindVertexArray(grass->vao_id);
    // bind texture
    glBindTexture(GL_TEXTURE_2D, grass_texture->texture_id);
    // clamp texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // disable backface culling as grass made up of crossing polygon
    glDisable(GL_CULL_FACE);
    // enable blending with default blend function
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i=0; i<NUM_GRASS_UNIT; ++i)
    {
      // transform
      // **note: we have problem re-export grasssModel thus we need
      // to use original one which -y is UP
      glm_mat4_copy(g_base_model_matrix, texture3d_shader->model_matrix);
      glm_rotate(texture3d_shader->model_matrix, GLM_PI, GLM_ZUP);
      glm_translate(texture3d_shader->model_matrix, randomized_grass_pos[i]);
      KRR_TEXSHADERPROG3D_update_model_matrix(texture3d_shader);

      SIMPLEMODEL_render(grass);
    }

    // diable blending
    glDisable(GL_BLEND);
    // enable backface culling again
    glEnable(GL_CULL_FACE);
  // unbind vao
  glBindVertexArray(0);
  // unbind shader
  KRR_SHADERPROG_unbind(texture3d_shader->program);

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

    // enable blending with default blend function
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // start with clean state of model matrix
    glm_mat4_copy(g_base_ui_model_matrix, shared_font_shaderprogram->model_matrix);
    KRR_FONTSHADERPROG2D_update_model_matrix(shared_font_shaderprogram);

    // render text on top right
    KRR_FONT_render_textex(fps_font, fps_text, 0.f, 4.f, &(SIZE){g_logical_width, g_logical_height}, KRR_FONT_TEXTALIGNMENT_RIGHT | KRR_FONT_TEXTALIGNMENT_TOP);
  KRR_SHADERPROG_unbind(shared_font_shaderprogram->program);

    // disable blending
    glDisable(GL_BLEND);

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
  if (terrain3d_shader != NULL)
  {
    KRR_TERRAINSHADERPROG3D_free(terrain3d_shader);
    terrain3d_shader = NULL;
  }

  if (terrain_texture != NULL)
  {
    KRR_TEXTURE_free(terrain_texture);
    terrain_texture = NULL;
  }
  if (grass_texture != NULL)
  {
    KRR_TEXTURE_free(grass_texture);
    grass_texture = NULL;
  }
  if (stall_texture != NULL)
  {
    KRR_TEXTURE_free(stall_texture);
    stall_texture = NULL;
  }
  if (tree_texture != NULL)
  {
    KRR_TEXTURE_free(tree_texture);
    tree_texture = NULL;
  }

  if (stall != NULL)
  {
    SIMPLEMODEL_free(stall);
    stall = NULL;
  }
  if (grass != NULL)
  {
    SIMPLEMODEL_free(grass);
    grass = NULL;
  }
  if (tree != NULL)
  {
    SIMPLEMODEL_free(tree);
    tree = NULL;
  }
  if (tr != NULL)
  {
    KRR_TERRAIN_free(tr);
    tr = NULL;
  }
}
