/* terrain sample
 - demonstrate switching and wrapping rendering code properly with objects which are in different type (transparent, opaque)
 - rendering ui text with font
 - camera movement and manipulation
 
 Key Control
 - TAB - to show/hide debugging text on the left
 - z - to switch between fixed moselook and freelook mode
 - w/s/a/d and q/e to move foward/backward/strafe-left/strafe-right and move-down/move-up
 - enter switch between fullscreen and windowed mode

 stall, and tree model uses the same (texture 3d) shader
 terrain uses terrain shader
 grass uses texture alpha 3d shader
*/

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
#include "graphics/texturedalphapp3d.h"
#include "graphics/terrain_shader3d.h"
#include "graphics/terrain.h"
#include "graphics/model.h"
#include "graphics/font.h"
#include "graphics/fontpp2d.h"
#include <math.h>

#define CONTENT_BG_COLOR 155.0f/255.0f, 202.0f/255.0f, 192.0f/255.0f, 1.0f
// the color should be the same as content bg color
#define SKY_COLOR_INIT (vec3){155.0f/255.0f, 202.0f/255.0f, 192.0f/255.0f}

#define TARGET_POS_LOOKAT_INIT (vec3){0.0f, 2.0f, -40.0f}

#define TEXT_RES_FREELOOK_DISABLED "Freelook mode: disabled"
#define TEXT_RES_FREELOOK_ENABLED "Freelook mode: enabled"

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
static KRR_TEXALPHASHADERPROG3D* texturealpha3d_shader = NULL;
static KRR_TERRAINSHADERPROG3D* terrain3d_shader = NULL;
static KRR_FONTSHADERPROG2D* font_shader = NULL;
static KRR_FONT* font = NULL;

// TODO: define variables here
static KRR_TEXTURE* terrain_texture = NULL;
static KRR_TEXTURE* grass_texture = NULL;
static KRR_TEXTURE* stall_texture = NULL;
static KRR_TEXTURE* tree_texture = NULL;
static KRR_TEXTURE* fern_texture = NULL;
static KRR_TEXTURE* player_texture = NULL;

static KRR_TEXTURE* mt_r_texture = NULL;
static KRR_TEXTURE* mt_g_texture = NULL;
static KRR_TEXTURE* mt_b_texture = NULL;
static KRR_TEXTURE* mt_blendmap = NULL;

static SIMPLEMODEL* stall = NULL;
static SIMPLEMODEL* grass = NULL;
static SIMPLEMODEL* tree = NULL;
static SIMPLEMODEL* fern = NULL;
static SIMPLEMODEL* player = NULL;
static TERRAIN* tr = NULL;

static KRR_CAM cam;
static float roty = 0.0f;
static bool is_freelook_mode_enabled = false;
static bool is_leftmouse_click = false;
static bool is_show_debugging_text = true;
static CGLM_ALIGN(8) vec3 player_position;
static float player_forward_rotation = 0.0f;
static CGLM_ALIGN(8) vec3 player_jump_velocity;
static bool is_player_inair = false;
static CGLM_ALIGN(8) vec3 player_dummy_pos;
static versor player_dummy_object_rot;

#define NUM_GRASS_UNIT 10
#define GRASS_RANDOM_SIZE 30
static CGLM_ALIGN(8) vec3 randomized_grass_pos[NUM_GRASS_UNIT];

#define NUM_TREE 3
#define TREE_RANDOM_SIZE 50
static CGLM_ALIGN(8) vec3 randomized_tree_pos[NUM_TREE];

#define NUM_FERN 10
#define FERN_RANDOM_SIZE 50
static CGLM_ALIGN(8) vec3 randomized_fern_pos[NUM_FERN];

#define TERRAIN_GRID_WIDTH 10
#define TERRAIN_GRID_HEIGHT 10
#define TERRAIN_SLOT_SIZE 200

// all in per second
#define MOVE_SPEED 60.f
#define PLAYER_SPEED 30.f
#define PLAYER_TURN_SPEED 150.f
#define GRAVITY -5.f
#define JUMP_POWER 75.0f

void usercode_app_went_windowed_mode()
{
  SU_BEGIN(texture_shader)
    SU_TEXSHADERPROG2D(texture_shader)
  SU_BEGIN(texture3d_shader)
    SU_TEXSHADERPROG3D(texture3d_shader)
  SU_BEGIN(texturealpha3d_shader)
    SU_TEXALPHASHADERPROG3D(texturealpha3d_shader)
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
  SU_BEGIN(texturealpha3d_shader)
    SU_TEXALPHASHADERPROG3D(texturealpha3d_shader)
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
  glm_vec3_copy((vec3){0.0f, 30.0f, 30.0f}, cam.pos);

  // initially set position to player
  glm_vec3_copy((vec3){0.0f, 0.0f, -15.0f}, player_position);

  // define dummy object's position and rotation as quaternion
  // camera will follow relatively to dummy object here
  glm_vec3_copy(cam.pos, player_dummy_pos);
  glm_quat_identity(player_dummy_object_rot);

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

  // load texture alpha 3d shader
  texturealpha3d_shader = KRR_TEXALPHASHADERPROG3D_new();
  if (!KRR_TEXALPHASHADERPROG3D_load_program(texturealpha3d_shader))
  {
    KRR_LOGE("Error loading texturealpha3d shader");
    return false;
  }
  // set texture alpha 3d shader
  shared_texturedalpha3d_shaderprogram = texturealpha3d_shader;

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
  if (!KRR_FONT_load_freetype(font, "res/fonts/Minecraft.ttf", 14))
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
    KRR_LOGE("Error loading tree texture");
    return false;
  }
  // fern texture
  fern_texture = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(fern_texture, "res/models/fern.png"))
  {
    KRR_LOGE("Error loading fern texture");
    return false;
  }
  // player texture
  player_texture = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(player_texture, "res/models/playerTexture-flip.png"))
  {
    KRR_LOGE("Error loading player texture");
    return false;
  }
  // multitexture r
  mt_r_texture = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(mt_r_texture, "res/models/mud.png"))
  {
    KRR_LOGE("Error loading multitexture r texture");
    return false;
  }
  // multitexture g
  mt_g_texture = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(mt_g_texture, "res/models/grassFlowers.png"))
  {
    KRR_LOGE("Error loading multitexture g texture");
    return false;
  }
  // multitexture b
  mt_b_texture = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(mt_b_texture, "res/models/path.png"))
  {
    KRR_LOGE("Error loading multitexture b texture");
    return false;
  }
  // multitexture blendmap
  mt_blendmap = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(mt_blendmap, "res/models/blendMap.png"))
  {
    KRR_LOGE("Error loading multitexture blendmap");
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

  for (int i=0; i<NUM_FERN; ++i)
  {
    glm_vec3_copy((vec3){KRR_math_rand_float2(-FERN_RANDOM_SIZE, FERN_RANDOM_SIZE), -1.0f, KRR_math_rand_float2(-FERN_RANDOM_SIZE, FERN_RANDOM_SIZE)}, randomized_fern_pos[i]);
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
    // sky color (affect to fog)
    glm_vec3_copy(SKY_COLOR_INIT, texture3d_shader->sky_color);
    KRR_TEXSHADERPROG3D_update_sky_color(texture3d_shader);
    // enable fog
    texture3d_shader->fog_enabled = true;
    KRR_TEXSHADERPROG3D_update_fog_enabled(texture3d_shader);
    // configure fog
    texture3d_shader->fog_density = 0.0055f;
    texture3d_shader->fog_gradient = 1.5f;
    KRR_TEXSHADERPROG3D_update_fog_density(texture3d_shader);
    KRR_TEXSHADERPROG3D_update_fog_gradient(texture3d_shader);

  SU_BEGIN(texturealpha3d_shader)
    SU_TEXALPHASHADERPROG3D(texturealpha3d_shader)
    // update ambient color
    glm_vec3_copy((vec3){0.1f, 0.1f, 0.1f}, texturealpha3d_shader->ambient_color);
    KRR_TEXALPHASHADERPROG3D_update_ambient_color(texturealpha3d_shader);
    // set texture unit
    KRR_TEXALPHASHADERPROG3D_set_texture_sampler(texturealpha3d_shader, 0);
    // set specular lighting
    texturealpha3d_shader->shine_damper = 10.0f;
    texturealpha3d_shader->reflectivity = 0.2f;
    KRR_TEXALPHASHADERPROG3D_update_shininess(texturealpha3d_shader);
    // set light info
    memcpy(&texturealpha3d_shader->light.pos, &light_pos, sizeof(light_pos));
    memcpy(&texturealpha3d_shader->light.color, &light_color, sizeof(light_color));
    KRR_TEXALPHASHADERPROG3D_update_light(texturealpha3d_shader);
    // sky color (affect to fog)
    glm_vec3_copy(SKY_COLOR_INIT, texturealpha3d_shader->sky_color);
    KRR_TEXALPHASHADERPROG3D_update_sky_color(texturealpha3d_shader);
    // enable fog
    texturealpha3d_shader->fog_enabled = true;
    KRR_TEXALPHASHADERPROG3D_update_fog_enabled(texturealpha3d_shader);
    // configure fog
    texturealpha3d_shader->fog_density = 0.0055f;
    texturealpha3d_shader->fog_gradient = 1.5f;
    KRR_TEXALPHASHADERPROG3D_update_fog_density(texturealpha3d_shader);
    KRR_TEXALPHASHADERPROG3D_update_fog_gradient(texturealpha3d_shader);

  SU_BEGIN(terrain3d_shader)
    SU_TERRAINSHADER(terrain3d_shader)
    // set ambient color
    glm_vec3_copy((vec3){0.7f, 0.7f, 0.7f}, terrain3d_shader->ambient_color);
    KRR_TERRAINSHADERPROG3D_update_ambient_color(terrain3d_shader);
    // set texture unit (at the same time this is multiteture background texture)
    KRR_TERRAINSHADERPROG3D_set_texture_sampler(terrain3d_shader, 0);
    // enabled multitexture
    terrain3d_shader->multitexture_enabled = true;
    KRR_TERRAINSHADERPROG3D_update_multitexture_enabled(terrain3d_shader);
    // set multitextures' texture unit
    KRR_TERRAINSHADERPROG3D_set_multitexture_texture_r_sampler(terrain3d_shader, 1);
    KRR_TERRAINSHADERPROG3D_set_multitexture_texture_g_sampler(terrain3d_shader, 2);
    KRR_TERRAINSHADERPROG3D_set_multitexture_texture_b_sampler(terrain3d_shader, 3);
    KRR_TERRAINSHADERPROG3D_set_multitexture_blendmap_sampler(terrain3d_shader, 4);
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
    // sky color (affect to fog)
    glm_vec3_copy(SKY_COLOR_INIT, terrain3d_shader->sky_color);
    KRR_TERRAINSHADERPROG3D_update_sky_color(terrain3d_shader);
    // enable fog
    terrain3d_shader->fog_enabled = true;
    KRR_TERRAINSHADERPROG3D_update_fog_enabled(terrain3d_shader);
    // configure fog
    terrain3d_shader->fog_density = 0.0055f;
    terrain3d_shader->fog_gradient = 1.5f;
    KRR_TERRAINSHADERPROG3D_update_fog_density(terrain3d_shader);
    KRR_TERRAINSHADERPROG3D_update_fog_gradient(terrain3d_shader);

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

  // load tree model
  tree = SIMPLEMODEL_new();
  if (!SIMPLEMODEL_load_objfile(tree, "res/models/lowPolyTree.obj"))
  {
    KRR_LOGE("Error loading tree model file");
    return false;
  }
  
  // load fern model
  fern = SIMPLEMODEL_new();
  if (!SIMPLEMODEL_load_objfile(fern, "res/models/fern.obj"))
  {
    KRR_LOGE("Error loading fern model");
    return false;
  }

  // load player model
  player = SIMPLEMODEL_new();
  if (!SIMPLEMODEL_load_objfile(player, "res/models/person.obj"))
  {
    KRR_LOGE("Error loading player model");
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
    else if (k == SDLK_TAB)
    {
      is_show_debugging_text = !is_show_debugging_text;
    }
    else if (k == SDLK_z)
    {
      is_freelook_mode_enabled = !is_freelook_mode_enabled;
    }
    else if (k == SDLK_SPACE)
    {
      if (!is_player_inair)
      {
        // update player's position in game loop
        is_player_inair = true;
        // define jump velocity
        glm_vec3_copy(GLM_YUP, player_jump_velocity);
      }
    }
  }
  else if (e->type == SDL_MOUSEBUTTONDOWN)
  {
    if (e->button.button == SDL_BUTTON_LEFT)
    {
      if (is_freelook_mode_enabled)
      {
        // allow to change forward, and up vector of user
        is_leftmouse_click = true;
      }
    }
    if (e->button.button == SDL_BUTTON_RIGHT)
    {
      if (is_freelook_mode_enabled)
      {
        // reset rotation of player's dummy object
        // this will stabilize camera's rotation in 3d space
        // note: just set it to identity, nullify additional rotation calculation done from mouse-move
        glm_quat_identity(player_dummy_object_rot);
      }
    }
  }
  else if (e->type == SDL_MOUSEBUTTONUP)
  {
    if (e->button.button == SDL_BUTTON_LEFT)
    {
      if (is_freelook_mode_enabled)
      {
        // disable allowance to chagne direction of user's vectors
        is_leftmouse_click = false;
      }
    }
  }

  if (is_leftmouse_click && is_freelook_mode_enabled && e->type == SDL_MOUSEMOTION)
  {
    SDL_MouseMotionEvent motion = e->motion;

    // if there's change along x-axis
    // then we rotate user's y-axis
    if (motion.xrel != 0)
    {
      float sign = motion.xrel > 0 ? 1.0f : -1.0f;
      int d_amount = abs(motion.xrel);
      if (d_amount > 5)
        d_amount = 5;
      
      // re-compute dummy's quaternion
      versor dummy_additional_rot;
      glm_quatv(dummy_additional_rot, -glm_rad(sign * d_amount * delta_time * 30.0f), GLM_YUP);
      glm_quat_mul(player_dummy_object_rot, dummy_additional_rot, player_dummy_object_rot);
    }

    // if there's change along y-axis
    // then we rotate user's x-axis
    if (motion.yrel != 0)
    {
      float sign = motion.yrel > 0 ? 1.0f : -1.0f;
      int d_amount = abs(motion.yrel);
      if (d_amount > 5)
        d_amount = 5;
      
      // re-compute dummy's quaternion
      versor dummy_additional_rot;
      glm_quatv(dummy_additional_rot, -glm_rad(sign * d_amount * delta_time * 30.0f), GLM_XUP);
      glm_quat_mul(player_dummy_object_rot, dummy_additional_rot, player_dummy_object_rot);
    }
  }
}

void update_camera(float delta_time)
{
  if (is_freelook_mode_enabled)
  {
    // define relative position to place our camera right behind the dummy object
    CGLM_ALIGN(8) vec3 campos;
    glm_vec3_copy((vec3){0.0f, 0.0f, 0.2f}, campos);

    // get matrix from quaternion
    CGLM_ALIGN_MAT mat4 dummy_transform;
    glm_quat_mat4(player_dummy_object_rot, dummy_transform);

    // transform position to place camera behind
    glm_vec3_rotate_m4(dummy_transform, campos, campos);
    glm_vec3_add(campos, player_dummy_pos, campos);

    // calcaulte up vector (to compute view matrix)
    CGLM_ALIGN(8) vec3 up;
    glm_vec3_copy(GLM_YUP, up);
    glm_vec3_rotate_m4(dummy_transform, up, up);

    // compute view matrix (lookat vector)
    glm_lookat(campos, player_dummy_pos, up, g_view_matrix);
  }
  else
  {
    glm_lookat(cam.pos, TARGET_POS_LOOKAT_INIT, cam.up, g_view_matrix);
  }

  // texture 3d
  KRR_SHADERPROG_bind(texture3d_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_VIEW_MATRIX, USERCODE_SHADERTYPE_TEXTURE3D_SHADER, texture3d_shader);

  // texture alpha 3d
  KRR_SHADERPROG_bind(texturealpha3d_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_VIEW_MATRIX, USERCODE_SHADERTYPE_TEXTUREALPHA3D_SHADER, texturealpha3d_shader);

  // terrain 3d
  KRR_SHADERPROG_bind(terrain3d_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_VIEW_MATRIX, USERCODE_SHADERTYPE_TERRAIN_SHADER, terrain3d_shader);
  KRR_SHADERPROG_unbind(terrain3d_shader->program);
}

void usercode_update(float delta_time)
{
  // note: update key press without relying on event should be put here
  // handle multiple key presses at once
  const Uint8* key_state = SDL_GetKeyboardState(NULL);

  if (key_state[SDL_SCANCODE_A])
  {
    if (is_freelook_mode_enabled)
    {
      // original forward vector
      CGLM_ALIGN(8) vec3 move;
      glm_vec3_copy((vec3){-1.0f, 0.0f, 0.0f}, move);

      // convert quaternion to matrix
      CGLM_ALIGN_MAT mat4 dummy_transform;
      glm_quat_mat4(player_dummy_object_rot, dummy_transform);
      
      // compute final of forward vector
      glm_vec3_rotate_m4(dummy_transform, move, move);

      // proceed distance along the line of forward vector
      glm_vec3_scale(move, MOVE_SPEED * delta_time, move);
      glm_vec3_add(player_dummy_pos, move, player_dummy_pos);
    }
    else
    {
      player_forward_rotation += PLAYER_TURN_SPEED * delta_time;
      player_forward_rotation = lroundf(player_forward_rotation) % 360;
    }
  }
  if (key_state[SDL_SCANCODE_D])
  {
    if (is_freelook_mode_enabled)
    {
      // original forward vector
      CGLM_ALIGN(8) vec3 move;
      glm_vec3_copy(GLM_XUP, move);

      // convert quaternion to matrix
      CGLM_ALIGN_MAT mat4 dummy_transform;
      glm_quat_mat4(player_dummy_object_rot, dummy_transform);
      
      // compute final of forward vector
      glm_vec3_rotate_m4(dummy_transform, move, move);

      // proceed distance along the line of forward vector
      glm_vec3_scale(move, MOVE_SPEED * delta_time, move);
      glm_vec3_add(player_dummy_pos, move, player_dummy_pos);
    }
    else
    {
      player_forward_rotation -= PLAYER_TURN_SPEED * delta_time;
      player_forward_rotation = lroundf(player_forward_rotation) % 360;
    }
  }
  if (key_state[SDL_SCANCODE_W])
  {
    if (is_freelook_mode_enabled)
    {
      // original forward vector
      CGLM_ALIGN(8) vec3 forward;
      glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, forward);

      // convert quaternion to matrix
      CGLM_ALIGN_MAT mat4 dummy_transform;
      glm_quat_mat4(player_dummy_object_rot, dummy_transform);
      
      // compute final of forward vector
      glm_vec3_rotate_m4(dummy_transform, forward, forward);

      // proceed distance along the line of forward vector
      glm_vec3_scale(forward, MOVE_SPEED * delta_time, forward);
      glm_vec3_add(player_dummy_pos, forward, player_dummy_pos);
    }
    else
    {
      // TODO: migrate this to be global approach later, for now only with this mode
      float distance = PLAYER_SPEED * delta_time;
      float rot_rad = glm_rad(player_forward_rotation);
      float dx = distance * sinf(rot_rad);
      float dz = distance * cosf(rot_rad);

      glm_vec3_add((vec3){dx, 0.0f, dz}, player_position, player_position);
    }
  }
  if (key_state[SDL_SCANCODE_S])
  {
    if (is_freelook_mode_enabled)
    {
      // original forward vector
      CGLM_ALIGN(8) vec3 move;
      glm_vec3_copy(GLM_ZUP, move);

      // convert quaternion to matrix
      CGLM_ALIGN_MAT mat4 dummy_transform;
      glm_quat_mat4(player_dummy_object_rot, dummy_transform);
      
      // compute final of forward vector
      glm_vec3_rotate_m4(dummy_transform, move, move);

      // proceed distance along the line of forward vector
      glm_vec3_scale(move, MOVE_SPEED * delta_time, move);
      glm_vec3_add(player_dummy_pos, move, player_dummy_pos);
    }
    else
    {
      float distance = PLAYER_SPEED * delta_time;
      
      float rot_rad = glm_rad(player_forward_rotation);
      float dx = distance * sinf(GLM_PI + rot_rad);
      float dz = distance * cosf(GLM_PI + rot_rad);

      glm_vec3_add((vec3){dx, 0.0f, dz}, player_position, player_position);
    }
  }
  if (key_state[SDL_SCANCODE_E])
  {
    if (is_freelook_mode_enabled)
    {
      // original forward vector
      CGLM_ALIGN(8) vec3 move;
      glm_vec3_copy(GLM_YUP, move);

      // convert quaternion to matrix
      CGLM_ALIGN_MAT mat4 dummy_transform;
      glm_quat_mat4(player_dummy_object_rot, dummy_transform);
      
      // compute final of forward vector
      glm_vec3_rotate_m4(dummy_transform, move, move);

      // proceed distance along the line of forward vector
      glm_vec3_scale(move, MOVE_SPEED * delta_time, move);
      glm_vec3_add(player_dummy_pos, move, player_dummy_pos);
    }
  }
  if (key_state[SDL_SCANCODE_Q])
  {
    if (is_freelook_mode_enabled)
    {
      // original forward vector
      CGLM_ALIGN(8) vec3 move;
      glm_vec3_copy((vec3){0.0f, -1.0f, 0.0f}, move);

      // convert quaternion to matrix
      CGLM_ALIGN_MAT mat4 dummy_transform;
      glm_quat_mat4(player_dummy_object_rot, dummy_transform);
      
      // compute final of forward vector
      glm_vec3_rotate_m4(dummy_transform, move, move);

      // proceed distance along the line of forward vector
      glm_vec3_scale(move, MOVE_SPEED * delta_time, move);
      glm_vec3_add(player_dummy_pos, move, player_dummy_pos);
    }
  }

  // update position of user if user jumps
  if (is_player_inair)
  {
    // pos = pos + velocity*dt
    // velocity = velocity + gravity*dt
    CGLM_ALIGN(8) vec3 velocity;
    glm_vec3_scale(player_jump_velocity, JUMP_POWER * delta_time, velocity);
    CGLM_ALIGN(8) vec3 gravity;
    glm_vec3_scale(GLM_YUP, GRAVITY * delta_time, gravity);
    // update position
    glm_vec3_add(velocity, player_position, player_position);
    // update velocity
    glm_vec3_add(player_jump_velocity, gravity, player_jump_velocity);

    // check if player touches the ground
    if (player_position[1] < 0.0f)
    {
      is_player_inair = false;
      // set player on the ground
      player_position[1] = 0.0f;
      // no more jump velocity
      glm_vec3_zero(player_jump_velocity);
    }
  }

  update_camera(delta_time);

  roty += 0.3f;
  if (roty > 360.0f)
  {
    roty -= 360.0f;
  }
}

void usercode_render(void)
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

  // STALL & TREE & PLAYER
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

  // render player
  glBindVertexArray(player->vao_id);
    // bind texture
    glBindTexture(GL_TEXTURE_2D, player_texture->texture_id);

    // transform
    glm_mat4_copy(g_base_model_matrix, texture3d_shader->model_matrix);
    glm_translate(texture3d_shader->model_matrix, player_position);
    glm_rotate(texture3d_shader->model_matrix, glm_rad(player_forward_rotation), GLM_YUP);
    // update model matrix
    KRR_TEXSHADERPROG3D_update_model_matrix(texture3d_shader);

    // render
    SIMPLEMODEL_render(player);

  // unbind shader
  KRR_SHADERPROG_unbind(texture3d_shader->program);

  // TERRAIN
  KRR_SHADERPROG_bind(terrain3d_shader->program);
  // render terrain
  glBindVertexArray(tr->vao_id);
    // bind background texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, terrain_texture->texture_id);
    // wrap texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    // multitexture r
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mt_r_texture->texture_id);

    // multitexture g
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mt_g_texture->texture_id);

    // multitexture b
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mt_b_texture->texture_id);

    // blendmap
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, mt_blendmap->texture_id);

    // transform model matrix
    glm_mat4_copy(g_base_model_matrix, terrain3d_shader->model_matrix);
    // rotate around itself
    //glm_rotate(terrain3d_shader->model_matrix, glm_rad(roty), GLM_YUP);
    glm_translate(terrain3d_shader->model_matrix, (vec3){-TERRAIN_GRID_WIDTH*TERRAIN_SLOT_SIZE/2, 0.0f, -TERRAIN_GRID_HEIGHT*TERRAIN_SLOT_SIZE/2});
    //update model matrix
    KRR_TERRAINSHADERPROG3D_update_model_matrix(terrain3d_shader);

    // render
    KRR_TERRAIN_render(tr);

    // set back to default texture
    glActiveTexture(GL_TEXTURE0);
  // unbind shader
  KRR_SHADERPROG_unbind(terrain3d_shader->program);

  // TEXTURE ALPHA (grass & fern)
  KRR_SHADERPROG_bind(texturealpha3d_shader->program);
  // disable backface culling as grass made up of crossing polygon
  glDisable(GL_CULL_FACE);

  // render grass
  glBindVertexArray(grass->vao_id);
    // bind texture
    glBindTexture(GL_TEXTURE_2D, grass_texture->texture_id);
    // clamp texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    for (int i=0; i<NUM_GRASS_UNIT; ++i)
    {
      glm_mat4_copy(g_base_model_matrix, texturealpha3d_shader->model_matrix);
      glm_rotate(texturealpha3d_shader->model_matrix, GLM_PI, GLM_ZUP);
      glm_translate(texturealpha3d_shader->model_matrix, randomized_grass_pos[i]);
      KRR_TEXALPHASHADERPROG3D_update_model_matrix(texturealpha3d_shader);

      SIMPLEMODEL_render(grass);
    }

  // render fern
  glBindVertexArray(fern->vao_id);
    // bind texture
    glBindTexture(GL_TEXTURE_2D, fern_texture->texture_id);

    for (int i=0; i<NUM_FERN; ++i)
    {
      glm_mat4_copy(g_base_model_matrix, texturealpha3d_shader->model_matrix);
      glm_translate(texturealpha3d_shader->model_matrix, randomized_fern_pos[i]);
      KRR_TEXALPHASHADERPROG3D_update_model_matrix(texturealpha3d_shader);

      SIMPLEMODEL_render(fern);
    }

  // enable backface culling again
  glEnable(GL_CULL_FACE);
  // unbind vao
  glBindVertexArray(0);
  // unbind shader
  KRR_SHADERPROG_unbind(texturealpha3d_shader->program);

  // disable scissor (if needed)
  if (g_need_clipping)
  {
    glDisable(GL_SCISSOR_TEST);
  }
}

void usercode_render_ui_text(void)
{
  if (is_show_debugging_text)
  {
    KRR_SHADERPROG_bind(shared_font_shaderprogram->program);
    KRR_FONT_bind_vao(font);
    
      // enable blending with default blend function
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // transform
      glm_mat4_copy(g_base_ui_model_matrix, shared_font_shaderprogram->model_matrix);
      KRR_FONTSHADERPROG2D_update_model_matrix(shared_font_shaderprogram);

      // render starting at top left corner
      KRR_FONT_render_textex(font, is_freelook_mode_enabled ? TEXT_RES_FREELOOK_ENABLED : TEXT_RES_FREELOOK_DISABLED, 4.f, 4.0f, &(SIZE){g_logical_width, g_logical_height}, KRR_FONT_TEXTALIGNMENT_LEFT | KRR_FONT_TEXTALIGNMENT_TOP);

      // disable blending
      glDisable(GL_BLEND);
      
    KRR_FONT_unbind_vao(font);
    KRR_SHADERPROG_unbind(shared_font_shaderprogram->program);
  }
}

void usercode_render_fps(int avg_fps)
{
#ifndef DISABLE_FPS_CALC
  // form framerate string to render
  snprintf(fps_text, FPS_BUFFER-1, "%d", avg_fps);

  // use shared font shader
  KRR_SHADERPROG_bind(shared_font_shaderprogram->program);
  // bind fps-vao
  KRR_FONT_bind_vao(fps_font);

    // enable blending with default blend function
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // start with clean state of model matrix
    glm_mat4_copy(g_base_ui_model_matrix, shared_font_shaderprogram->model_matrix);
    KRR_FONTSHADERPROG2D_update_model_matrix(shared_font_shaderprogram);

    // render text on top right
    KRR_FONT_render_textex(fps_font, fps_text, 0.f, 4.f, &(SIZE){g_logical_width, g_logical_height}, KRR_FONT_TEXTALIGNMENT_RIGHT | KRR_FONT_TEXTALIGNMENT_TOP);

    // disable blending
    glDisable(GL_BLEND);

  // unbind fps-vao
  KRR_FONT_unbind_vao(fps_font);
  KRR_SHADERPROG_unbind(shared_font_shaderprogram->program);
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
  if (texturealpha3d_shader != NULL)
  {
    KRR_TEXALPHASHADERPROG3D_free(texturealpha3d_shader);
    texturealpha3d_shader = NULL;
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
  if (fern_texture != NULL)
  {
    KRR_TEXTURE_free(fern_texture);
    fern_texture = NULL;
  }
  if (player_texture != NULL)
  {
    KRR_TEXTURE_free(player_texture);
    player_texture = NULL;
  }
  if (mt_r_texture != NULL)
  {
    KRR_TEXTURE_free(mt_r_texture);
    mt_r_texture = NULL;
  }
  if (mt_g_texture != NULL)
  {
    KRR_TEXTURE_free(mt_g_texture);
    mt_g_texture = NULL;
  }
  if (mt_b_texture != NULL)
  {
    KRR_TEXTURE_free(mt_b_texture);
    mt_b_texture = NULL;
  }
  if (mt_blendmap != NULL)
  {
    KRR_TEXTURE_free(mt_blendmap);
    mt_blendmap = NULL;
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
  if (fern != NULL)
  {
    SIMPLEMODEL_free(fern);
    fern = NULL;
  }
  if (player != NULL)
  {
    SIMPLEMODEL_free(player);
    player = NULL;
  }
  if (tr != NULL)
  {
    KRR_TERRAIN_free(tr);
    tr = NULL;
  }
}
