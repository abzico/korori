/* terrain sample
 - demonstrate switching and wrapping rendering code properly with objects which are in different type (transparent, opaque)
 - rendering ui text with font
 - camera movement and manipulation
 - fps camera implementation via quaternion
 
 Key Control
 - TAB - to show/hide debugging text on the left
 - z - to switch between fixed moselook and freelook mode
 - w/s/a/d and q/e to move foward/backward/strafe-left/strafe-right and move-down/move-up
 - enter switch between fullscreen and windowed mode

 stall, and tree model uses the same (texture 3d) shader
 terrain uses terrain shader
*/

#include "usercode.h"
#include "functs.h"
#include "krr/foundation/common.h"
#include "krr/foundation/window.h"
#include "krr/foundation/util.h"
#include "krr/foundation/cam.h"
#include "krr/foundation/math.h"
#include "krr/graphics/util.h"
#include "krr/graphics/texturedpp2d.h"
#include "krr/graphics/texturedpp3d.h"
#include "krr/graphics/texturedalphapp3d.h"
#include "krr/graphics/terrain_shader3d.h"
#include "krr/graphics/terrain.h"
#include "krr/graphics/model.h"
#include "krr/graphics/font.h"
#include "krr/graphics/fontpp2d.h"
#include <texpackr/texpackr.h>
#include <math.h>

#define CONTENT_BG_COLOR 0xDC/255.0f, 0xE4/255.0f, 0xF1/255.0f, 1.0f
// note: the color should be the same as content bg color
#define SKY_COLOR_INIT (vec3){0xDC/255.0f, 0xE4/255.0f, 0xF1/255.0f}

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
static KRR_TEXTURE* stall_texture = NULL;
static KRR_TEXTURE* tree_texture = NULL;
static KRR_TEXTURE* fern_texture = NULL;
static KRR_TEXTURE* player_texture = NULL;
static KRR_TEXTURE* lamp_texture = NULL;

static KRR_TEXTURE* mt_r_texture = NULL;
static KRR_TEXTURE* mt_g_texture = NULL;
static KRR_TEXTURE* mt_b_texture = NULL;
static KRR_TEXTURE* mt_blendmap = NULL;

static SIMPLEMODEL* stall = NULL;
static SIMPLEMODEL* tree = NULL;
static SIMPLEMODEL* fern = NULL;
static SIMPLEMODEL* player = NULL;
static SIMPLEMODEL* lamp = NULL;
static TERRAIN* tr = NULL;

static KRR_CAM cam;
static float roty = 0.0f;
static bool is_freelook_mode_enabled = false;   // not 100% freedom
static bool is_leftmouse_click = false;
static bool is_rightmouse_click = false;
static bool is_show_debugging_text = true;

#define PLAYER_CAM_MIN_DISTANCE 30.0f
#define PLAYER_CAM_MAX_DISTANCE 250.0f

// player's positining variables
static CGLM_ALIGN(8) vec3 player_position;
static float player_forward_rotation = 180.0f;
static CGLM_ALIGN(8) vec3 player_jump_velocity;
static bool is_player_inair = false;
static float player_cam_distance;
static float player_cam_pitch_angle;  // in degrees
static float player_cam_yaw_angle;    // in degrees

// player's dummy position and rotation used in freelook as a placeholder object
// for camera to keeps following
static CGLM_ALIGN(8) vec3 player_dummy_pos;
static CGLM_ALIGN(16) versor player_dummy_object_rot;

static CGLM_ALIGN(16) versor cam_rot;

static CGLM_ALIGN(8) vec3 stall_pos;
static CGLM_ALIGN(16) versor stall_rot;

#define NUM_LAMP 3
#define LAMP_RANDOM_SIZE 200
static CGLM_ALIGN(8)  vec3 lamp_pos[NUM_LAMP];

#define NUM_TREE 10
#define TREE_RANDOM_SIZE 200
static CGLM_ALIGN(8) vec3 randomized_tree_pos[NUM_TREE];
static CGLM_ALIGN(16) versor tree_rots[NUM_TREE];

#define NUM_FERN 20
#define FERN_RANDOM_SIZE 200
static CGLM_ALIGN(8) vec3 randomized_fern_pos[NUM_FERN];
static CGLM_ALIGN(16) versor fern_rots[NUM_FERN];
static CGLM_ALIGN(8) vec4 fern_clipped_texcoords[4];
static int fern_texcoord_is[NUM_FERN];

#define TERRAIN_SLOT_SIZE 10
#define TERRAIN_HFACTOR 3.0f

// all in per second
#define MOVE_SPEED 120.f
#define PLAYER_SPEED 50.f
#define PLAYER_TURN_SPEED 150.f
#define GRAVITY -5.f
#define JUMP_POWER 2.5f

static void update_camera(float delta_time);
/// compute position y from world position in xz plane
static float compute_posy(float x, float z);
/// compute to get normal of terrain at the world position in xz plane
static void compute_terrain_normal(float x, float y, float z, vec3 dest);

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

  // enable gamma correction
  glEnable(GL_FRAMEBUFFER_SRGB);

  // initially start user's camera looking at -z, and up with +y
  glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, cam.forward);
  glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, cam.up);
  glm_vec3_copy((vec3){0.0f, 30.0f, 30.0f}, cam.pos);

  // initially set position to player
  glm_vec3_copy((vec3){0.0f, 0.0f, -15.0f}, player_position);
  
  // initialize the distance at the startup
  player_cam_distance = KRR_math_lerp(PLAYER_CAM_MIN_DISTANCE, PLAYER_CAM_MAX_DISTANCE, 0.2f);
  player_cam_pitch_angle = 30.0f;
  player_cam_yaw_angle = 0.0f;

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
  glBindTexture(GL_TEXTURE_2D, terrain_texture->texture_id);
  KRR_gputil_generate_mipmaps(GL_TEXTURE_2D, 0.0f);

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
  if (!KRR_TEXTURE_load_texture_from_file(fern_texture, "res/models/fern-sheet.png"))
  {
    KRR_LOGE("Error loading fern texture");
    return false;
  }
  // load sheet meta
  texpackr_sheetmeta* fern_sheetmeta = texpackr_parse("res/models/fern-sheet.tpr");
  if (fern_sheetmeta == NULL)
  {
    KRR_LOGE("Error loading fern-sheet.tpr");
    return false;
  }

  // player texture
  player_texture = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(player_texture, "res/models/playerTexture-flip.png"))
  {
    KRR_LOGE("Error loading player texture");
    return false;
  }
  // lamp texture
  lamp_texture = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(lamp_texture, "res/models/lamp-optimized.png"))
  {
    KRR_LOGE("Error loading lamp texture");
    return false;
  }
  // multitexture r
  mt_r_texture = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(mt_r_texture, "res/models/mud.png"))
  {
    KRR_LOGE("Error loading multitexture r texture");
    return false;
  }
  glBindTexture(GL_TEXTURE_2D, mt_r_texture->texture_id);
  KRR_gputil_generate_mipmaps(GL_TEXTURE_2D, 0.0f);

  // multitexture g
  mt_g_texture = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(mt_g_texture, "res/models/grassFlowers.png"))
  {
    KRR_LOGE("Error loading multitexture g texture");
    return false;
  }
  glBindTexture(GL_TEXTURE_2D, mt_g_texture->texture_id);
  KRR_gputil_generate_mipmaps(GL_TEXTURE_2D, 0.0f);

  // multitexture b
  mt_b_texture = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(mt_b_texture, "res/models/path.png"))
  {
    KRR_LOGE("Error loading multitexture b texture");
    return false;
  }
  // generate mipmap stack for multitexture b
  // note: bind texture first, then call util function
  glBindTexture(GL_TEXTURE_2D, mt_b_texture->texture_id);
  KRR_gputil_generate_mipmaps(GL_TEXTURE_2D, -1.2f);

  // multitexture blendmap
  mt_blendmap = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(mt_blendmap, "res/models/blendMap.png"))
  {
    KRR_LOGE("Error loading multitexture blendmap");
    return false;
  }

  // load from generation of terrain
  tr = KRR_TERRAIN_new();
  if (!KRR_TERRAIN_load_from_generation(tr, "res/models/heightmap.png", TERRAIN_SLOT_SIZE, TERRAIN_HFACTOR))
  {
    KRR_LOGE("Error loading terrain from generation");
    return false;
  }

  // pre-define lights
  // place light higher above the ground of terrain at that location of x/z
#define NUM_LIGHTS 4
  float light_yoffsets[] = {
    50.0f,
    50.0f,
    50.0f,
    50.0f
  };
  VERTEXPOS3D light_poss[] = {
    {30.0f, compute_posy(30.0f, 30.0f) + light_yoffsets[0], 30.0f},
    {100.0f, compute_posy(100.0f, -100.0f) + light_yoffsets[1], -100.0f},
    {200.0f, compute_posy(200.0f, -200.0f) + light_yoffsets[2], -200.0f},
    {-200.0f, compute_posy(-200.0f, -200.0f) + light_yoffsets[3], -200.0f}
  };
  COLOR3F light_colors[] = {
    {0.01f, 0.01f, 0.01f},
    {2.0f, 0.0f, 0.0f},     // set value more than 1.0f to accomodate the loss of light intensity when we use light calculation
    {0.0f, 2.0f, 2.0f},     // same
    {0.0f, 2.0f, 0.0f}      // same
  };
  float light_attenuation_factors[] = {
    0.0f,       // sun, no attenuation, just set to 0.0f
    0.0025f,    // point light
    0.0025f,    // point light
    0.0025f     // point light
  };

  // initially update all related matrices and related graphics stuff for both basic shaders
  SU_BEGIN(texture_shader)
    SU_TEXSHADERPROG2D(texture_shader)
    // set texture unit
    KRR_TEXSHADERPROG2D_set_texture_sampler(texture_shader, 0);

  SU_BEGIN(texture3d_shader)
    SU_TEXSHADERPROG3D(texture3d_shader)
    // update ambient color
    glm_vec3_copy((vec3){0.01f, 0.01f, 0.01f}, texture3d_shader->ambient_color);
    KRR_TEXSHADERPROG3D_update_ambient_color(texture3d_shader);
    // set texture unit
    KRR_TEXSHADERPROG3D_set_texture_sampler(texture3d_shader, 0);
    // set specular lighting
    texture3d_shader->shine_damper = 10.0f;
    texture3d_shader->reflectivity = 0.2f;
    KRR_TEXSHADERPROG3D_update_shininess(texture3d_shader);
    // set lights info
    for (int i=0; i<NUM_LIGHTS; ++i)
    {
      memcpy(&texture3d_shader->lights[i].pos, &light_poss[i], sizeof(VERTEXPOS3D));
      memcpy(&texture3d_shader->lights[i].color, &light_colors[i], sizeof(COLOR3F));
      texture3d_shader->lights[i].attenuation_factor = light_attenuation_factors[i];
    }
    // update lights we have
    KRR_TEXSHADERPROG3D_update_lights_num(texture3d_shader, NUM_LIGHTS);
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
    glm_vec3_copy((vec3){0.01f, 0.01f, 0.01f}, texturealpha3d_shader->ambient_color);
    KRR_TEXALPHASHADERPROG3D_update_ambient_color(texturealpha3d_shader);
    // set texture unit
    KRR_TEXALPHASHADERPROG3D_set_texture_sampler(texturealpha3d_shader, 0);
    // set specular lighting
    texturealpha3d_shader->shine_damper = 10.0f;
    texturealpha3d_shader->reflectivity = 0.2f;
    KRR_TEXALPHASHADERPROG3D_update_shininess(texturealpha3d_shader);
    // set light info - 1st
    for (int i=0; i<NUM_LIGHTS; ++i)
    {
      memcpy(&texturealpha3d_shader->lights[i].pos, &light_poss[i], sizeof(VERTEXPOS3D));
      memcpy(&texturealpha3d_shader->lights[i].color, &light_colors[i], sizeof(COLOR3F));
      texturealpha3d_shader->lights[i].attenuation_factor = light_attenuation_factors[i];
    }
    // update lights
    KRR_TEXALPHASHADERPROG3D_update_lights_num(texturealpha3d_shader, NUM_LIGHTS);
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
    glm_vec3_copy((vec3){0.01f, 0.01f, 0.01f}, terrain3d_shader->ambient_color);
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
    // set light info - 1st
    for (int i=0; i<NUM_LIGHTS; ++i) {
      memcpy(&terrain3d_shader->lights[i].pos, &light_poss[i], sizeof(VERTEXPOS3D));
      memcpy(&terrain3d_shader->lights[i].color, &light_colors[i], sizeof(COLOR3F));
      terrain3d_shader->lights[i].attenuation_factor = light_attenuation_factors[i];
    }
    // update light to GPU
    KRR_TERRAINSHADERPROG3D_update_lights_num(terrain3d_shader, NUM_LIGHTS);
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

  // load lamp model
  lamp = SIMPLEMODEL_new();
  if (!SIMPLEMODEL_load_objfile(lamp, "res/models/lamp.obj"))
  {
    KRR_LOGE("Error loading lamp model");
    return false;
  }

  // temp variables to hold randomized value for positioning
  float temp_x, temp_y, temp_z;
  CGLM_ALIGN(8) vec3 normal;

  // set stall's position
  temp_y = compute_posy(0.0f, -50.0f);
  glm_vec3_copy((vec3){0.0f, temp_y, -50.0f}, stall_pos);

  // find stall's rotation according to terrain's height
  compute_terrain_normal(0.0f, temp_y, -50.0f, normal);
  // nudge and give more weight to y-axis
  normal[1] *= 2.3f;
  glm_vec3_normalize(normal);
  KRR_math_quat_v2rot(GLM_YUP, normal, stall_rot);

  for (int i=0; i<NUM_TREE; ++i)
  {
    temp_x = KRR_math_rand_float2(-TREE_RANDOM_SIZE, TREE_RANDOM_SIZE);
    temp_z = KRR_math_rand_float2(-TREE_RANDOM_SIZE, TREE_RANDOM_SIZE);
    temp_y = compute_posy(temp_x, temp_z) - 1.0f;
    glm_vec3_copy((vec3){temp_x, temp_y, temp_z}, randomized_tree_pos[i]);

    // use y position to find terrain's normal at that position
    compute_terrain_normal(temp_x, temp_y, temp_z, normal);
    normal[1] *= 2.3f;
    glm_vec3_normalize(normal);
    
    // compute quaternion for rotation from UP vector to result normal
    KRR_math_quat_v2rot(GLM_YUP, normal, tree_rots[i]);
  }

  // get pre-computed texture coordinates from sprites and save it for rendering
  for (int i=0; i<4; ++i)
  {
    const texpackr_sprite* sprite = NULL;

    // calculate texcoord's clipping for this particular
    // optimize note: pre-cache calcuting this.
    if (i == 0)
    {
      sprite = hashmapc_get(fern_sheetmeta->sprites, "/Users/haxpor/Desktop/fern1.png");
    }
    else if (i == 1)
    {
      sprite = hashmapc_get(fern_sheetmeta->sprites, "/Users/haxpor/Desktop/fern2.png");
    }
    else if (i == 2)
    {
      sprite = hashmapc_get(fern_sheetmeta->sprites, "/Users/haxpor/Desktop/fern3.png");
    }
    else
    {
      sprite = hashmapc_get(fern_sheetmeta->sprites, "/Users/haxpor/Desktop/fern4.png");
    }

    // it's already pre-computed, we get it hre
    texpackr_vec2f tc_u = sprite->texcoord_u;
    texpackr_vec2f tc_v = sprite->texcoord_v;

    // copy to our cached variable
    glm_vec4_copy((vec4){
          tc_u.x, tc_u.y,
          tc_v.x, tc_v.y
        }, fern_clipped_texcoords[i]);

    KRR_LOG("ferp_clipped_texcoords[%d] = u(%f,%f), v(%f,%f)", i, fern_clipped_texcoords[i][0], fern_clipped_texcoords[i][1], fern_clipped_texcoords[i][2], fern_clipped_texcoords[i][3]);
  }

  for (int i=0; i<NUM_FERN; ++i)
  {
    temp_x = KRR_math_rand_float2(-FERN_RANDOM_SIZE, FERN_RANDOM_SIZE);
    temp_z = KRR_math_rand_float2(-FERN_RANDOM_SIZE, FERN_RANDOM_SIZE);
    temp_y = compute_posy(temp_x, temp_z) - 1.0f;
    glm_vec3_copy((vec3){temp_x, temp_y, temp_z}, randomized_fern_pos[i]);

    // use y position to find terrain's normal at that position
    compute_terrain_normal(temp_x, temp_y, temp_z, normal);
    normal[1] *= 2.3f;
    glm_vec3_normalize(normal);

    // compute quaternion for rotation from UP vector to result normal
    KRR_math_quat_v2rot(GLM_YUP, normal, fern_rots[i]);

    // random to get different texture for fern
    int rand_fern_i = KRR_math_rand_int(3);

    // save which fern texcoord this fern should be
    fern_texcoord_is[i] = rand_fern_i;
  }

  for (int i=0; i<NUM_LAMP; ++i)
  {
    // note: number of lights and number of lamps are not the same
    // sun (light at index 0) we don't use lamp to represent it, thus we have lamp less by 1
    //
    // FIXME: Additional texture workflow for clener solution to disable real light calcution to fully accept light effect from light source for a model
    // note for placing the position of lamp here is slightly under the actual light source's position
    // and we need to fake the normal of lamp in order to get the light effect from the light source as well
    // for now we pre-faked normal from .obj file by using only yUP as normal for all vertices
    // but for better we could use additional texture to identify which part of the model to *fully* accept and being influenced
    // by the light source without factor of light attenuation or light direction in dot product.
    glm_vec3_copy((vec3){light_poss[i+1].x, light_poss[i+1].y - light_yoffsets[i+1] - 2.0f, light_poss[i+1].z}, lamp_pos[i]);
  }

  // we have no need to continue using sheetmeta for fern anymore
  texpackr_sheetmeta_free(fern_sheetmeta);
  fern_sheetmeta = NULL;

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
        // immediately scale with JUMP_POWER
        glm_vec3_scale(player_jump_velocity, JUMP_POWER, player_jump_velocity);
      }
    }
    else if (k == SDLK_f)
    {
      // toggle fog
      texture3d_shader->fog_enabled = !texture3d_shader->fog_enabled;
      texturealpha3d_shader->fog_enabled = !texturealpha3d_shader->fog_enabled;
      terrain3d_shader->fog_enabled = !terrain3d_shader->fog_enabled;
      
      SU_BEGIN(texture3d_shader)
        KRR_TEXSHADERPROG3D_update_fog_enabled(texture3d_shader);
      SU_BEGIN(texturealpha3d_shader)
        KRR_TEXALPHASHADERPROG3D_update_fog_enabled(texturealpha3d_shader);
      SU_BEGIN(terrain3d_shader)
        KRR_TERRAINSHADERPROG3D_update_fog_enabled(terrain3d_shader);
      SU_END(terrain3d_shader)
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
      if (!is_freelook_mode_enabled)
      {
        // allow to change pitch of player's camera
        is_rightmouse_click = true;
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
    if (e->button.button == SDL_BUTTON_RIGHT)
    {
      // disable allowance to change pitch of camera
      is_rightmouse_click = false;
    }
  }
  else if (e->type == SDL_MOUSEWHEEL)
  {
    if (!is_freelook_mode_enabled)
    {
      // SDL doesn't keep consistent behavior across the platform
      // if direction is flipped, then we need to process opposite value
      int flip = e->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1.0 : 1.0;
      int sign = e->wheel.y < 0.0 ? -1.0 : 1.0;

      float abs_amount = fabsf(e->wheel.y * delta_time * 40.0f);
      if (abs_amount > 6.0f)
        abs_amount = 6.0f;

      // update to cam's relative pos for +y and +z axis
      float real_amount = abs_amount * flip * sign;
      // update to player's cam distance
      player_cam_distance += real_amount;
      // cap player amount
      if (player_cam_distance < PLAYER_CAM_MIN_DISTANCE)
        player_cam_distance = PLAYER_CAM_MIN_DISTANCE;
      else if (player_cam_distance > PLAYER_CAM_MAX_DISTANCE)
        player_cam_distance = PLAYER_CAM_MAX_DISTANCE;
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
      CGLM_ALIGN(16) versor dummy_additional_rot;
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
      CGLM_ALIGN(16) versor dummy_additional_rot;
      glm_quatv(dummy_additional_rot, -glm_rad(sign * d_amount * delta_time * 30.0f), GLM_XUP);
      glm_quat_mul(player_dummy_object_rot, dummy_additional_rot, player_dummy_object_rot);
    }
  }
  else if (is_rightmouse_click && !is_freelook_mode_enabled && e->type == SDL_MOUSEMOTION)
  {
    SDL_MouseMotionEvent motion = e->motion;

    // pitch
    if (motion.yrel != 0)
    {
      float sign = motion.yrel > 0 ? 1.0f : -1.0f;
      int d_amount = abs(motion.yrel);
      if (d_amount > 5)
        d_amount = 5;

      float angle_amount = sign * d_amount * delta_time * 30.0f;
      player_cam_pitch_angle += angle_amount;
      if (player_cam_pitch_angle < 5.0f)
        player_cam_pitch_angle = 5.0f;
      else if (player_cam_pitch_angle > 85.0f)
        player_cam_pitch_angle = 85.0f;
    }
    // yaw
    if (motion.xrel != 0)
    {
      float sign = motion.xrel > 0 ? 1.0f : -1.0f;
      int d_amount = abs(motion.xrel);

      if (d_amount > 5)
        d_amount = 5;

      float angle_amount = sign * d_amount * delta_time * 30.0f;
      player_cam_yaw_angle = (int)(player_cam_yaw_angle + angle_amount) % 360;
    }
  }
}

void compute_terrain_normal(float x, float y, float z, vec3 dest)
{
  // compute offset from terrain's position (world space)
  // this depends on how we translate such terrain in render() function
  float adjusted_x = x + tr->grid_width*TERRAIN_SLOT_SIZE/2;
  float adjusted_z = z + tr->grid_height*TERRAIN_SLOT_SIZE/2;

  int index_i = (int)floor(adjusted_x / TERRAIN_SLOT_SIZE);
  int index_z = (int)floor(adjusted_z / TERRAIN_SLOT_SIZE);

  // check if indexes are in the range of terrain
  // it might be player is outside of terrain area
  if (index_i < 0 || index_i >= tr->grid_width ||
      index_z < 0 || index_z >= tr->grid_height)
  {
    KRR_LOGI("return zero vector as result");
    glm_vec3_copy(GLM_VEC3_ZERO, dest);
    return;
  }

  // determine which triangle input position will be in
  //
  // ref from terrain which bases on this in its implementation,
  // with index denoted to work with barycentric coordinate
  //
  // ^
  // | Z-Axis
  //
  // A(0,0) ---- B(1,0)
  // |          /   |
  // |        /     |
  // |      /       |
  // |    /         |
  // |  /           |
  // C(0,1) ---- D(1,1)  --> X-Axis
  //
  // first get the unit coordinate of input position for that terrain slot
  float x_coord = (((int)adjusted_x) % tr->grid_width) * 1.0f / tr->grid_width;
  float z_coord = (((int)adjusted_z) % tr->grid_height) * 1.0f / tr->grid_height;

  // set next indexes, and bound them for this triangle
  int next_index_i = index_i+1; if (next_index_i > tr->grid_width) next_index_i = tr->grid_width;
  int next_index_z = index_z+1; if (next_index_z > tr->grid_height) next_index_z = tr->grid_height;

  // grab normal
  CGLM_ALIGN(8) vec3 n_topleft;
  CGLM_ALIGN(8) vec3 n_topright;
  CGLM_ALIGN(8) vec3 n_bottomleft;
  CGLM_ALIGN(8) vec3 n_bottomright;

  glm_vec3_copy(tr->normals[index_z*(tr->grid_width) + index_i], n_topleft);
  glm_vec3_copy(tr->normals[index_z*(tr->grid_width) + next_index_i], n_topright);
  glm_vec3_copy(tr->normals[next_index_z*(tr->grid_width) + index_i], n_bottomleft);
  glm_vec3_copy(tr->normals[next_index_z*(tr->grid_width) + next_index_i], n_bottomright);

  // determine which triangle of such slot input position is in
  if (x_coord <= 1.0f - z_coord)
  {
    // left triangle
    CGLM_ALIGN(8) vec3 p1;
    CGLM_ALIGN(8) vec3 p2;
    CGLM_ALIGN(8) vec3 p3;
    vec2 p;
    p[0] = x_coord;
    p[1] = z_coord;

    // x
    glm_vec3_copy((vec3){0.0f, n_topleft[0], 0.0f}, p1);
    glm_vec3_copy((vec3){1.0f, n_topright[0], 0.0f}, p2);
    glm_vec3_copy((vec3){0.0f, n_bottomleft[0], 1.0f}, p3);
    float normal_x = KRR_math_barycentric_xz(p1, p2, p3, p);

    // y
    glm_vec3_copy((vec3){0.0f, n_topleft[1], 0.0f}, p1);
    glm_vec3_copy((vec3){1.0f, n_topright[1], 0.0f}, p2);
    glm_vec3_copy((vec3){0.0f, n_bottomleft[1], 1.0f}, p3);
    float normal_y = KRR_math_barycentric_xz(p1, p2, p3, p);

    // z
    glm_vec3_copy((vec3){0.0f, n_topleft[2], 0.0f}, p1);
    glm_vec3_copy((vec3){1.0f, n_topright[2], 0.0f}, p2);
    glm_vec3_copy((vec3){0.0f, n_bottomleft[2], 1.0f}, p3);
    float normal_z = KRR_math_barycentric_xz(p1, p2, p3, p);

    // fill in result
    glm_vec3_copy((vec3){normal_x, normal_y, normal_z}, dest);
  }
  else
  {
    // right triangle
    CGLM_ALIGN(8) vec3 p1;
    CGLM_ALIGN(8) vec3 p2;
    CGLM_ALIGN(8) vec3 p3;
    vec2 p;
    p[0] = x_coord;
    p[1] = z_coord;

    // x
    glm_vec3_copy((vec3){1.0f, n_topright[0], 0.0f}, p1);
    glm_vec3_copy((vec3){0.0f, n_bottomleft[0], 1.0f}, p2);
    glm_vec3_copy((vec3){1.0f, n_bottomright[0], 1.0f}, p3);
    float normal_x = KRR_math_barycentric_xz(p1, p2, p3, p);

    // y
    glm_vec3_copy((vec3){1.0f, n_topright[1], 0.0f}, p1);
    glm_vec3_copy((vec3){0.0f, n_bottomleft[1], 1.0f}, p2);
    glm_vec3_copy((vec3){1.0f, n_bottomright[1], 1.0f}, p3);
    float normal_y = KRR_math_barycentric_xz(p1, p2, p3, p);

    // z
    glm_vec3_copy((vec3){1.0f, n_topright[2], 0.0f}, p1);
    glm_vec3_copy((vec3){0.0f, n_bottomleft[2], 1.0f}, p2);
    glm_vec3_copy((vec3){1.0f, n_bottomright[2], 1.0f}, p3);
    float normal_z = KRR_math_barycentric_xz(p1, p2, p3, p);

    // fill in result
    glm_vec3_copy((vec3){normal_x, normal_y, normal_z}, dest);
  }
}

float compute_posy(float x, float z)
{
  // compute offset from terrain's position (world space)
  // this depends on how we translate such terrain in render() function
  float adjusted_x = x + tr->grid_width*TERRAIN_SLOT_SIZE/2;
  float adjusted_z = z + tr->grid_height*TERRAIN_SLOT_SIZE/2;

  int index_i = (int)floor(adjusted_x / TERRAIN_SLOT_SIZE);
  int index_z = (int)floor(adjusted_z / TERRAIN_SLOT_SIZE);

  // check if indexes are in the range of terrain
  // it might be player is outside of terrain area
  if (index_i < 0 || index_i >= tr->grid_width ||
      index_z < 0 || index_z >= tr->grid_height)
  {
    KRR_LOGI("return 0.0f");
    return 0.0f;
  }

  // determine which triangle input position will be in
  //
  // ref from terrain which bases on this in its implementation,
  // with index denoted to work with barycentric coordinate
  //
  // ^
  // | Z-Axis
  //
  // A(0,0) ---- B(1,0)
  // |          /   |
  // |        /     |
  // |      /       |
  // |    /         |
  // |  /           |
  // C(0,1) ---- D(1,1)  --> X-Axis
  //
  // first get the unit coordinate of input position for that terrain slot
  float x_coord = (((int)adjusted_x) % tr->grid_width) * 1.0f / tr->grid_width;
  float z_coord = (((int)adjusted_z) % tr->grid_height) * 1.0f / tr->grid_height;

  // set next indexes, and bound them for this triangle
  int next_index_i = index_i+1; if (next_index_i > tr->grid_width) next_index_i = tr->grid_width;
  int next_index_z = index_z+1; if (next_index_z > tr->grid_height) next_index_z = tr->grid_height;

  // determine which triangle of such slot input position is in
  if (x_coord <= 1.0f - z_coord)
  {
    // left triangle
    CGLM_ALIGN(8) vec3 p1;
    CGLM_ALIGN(8) vec3 p2;
    CGLM_ALIGN(8) vec3 p3;
    vec2 p;
    p[0] = x_coord;
    p[1] = z_coord;

    glm_vec3_copy((vec3){0.0f, tr->heights[index_z*(tr->grid_width) + index_i], 0.0f}, p1);
    glm_vec3_copy((vec3){1.0f, tr->heights[index_z*(tr->grid_width) + next_index_i], 0.0f}, p2);
    glm_vec3_copy((vec3){0.0f, tr->heights[next_index_z*(tr->grid_width) + index_i], 1.0f}, p3);

    return KRR_math_barycentric_xz(p1, p2, p3, p);
  }
  else
  {
    // right triangle
    CGLM_ALIGN(8) vec3 p1;
    CGLM_ALIGN(8) vec3 p2;
    CGLM_ALIGN(8) vec3 p3;
    vec2 p;
    p[0] = x_coord;
    p[1] = z_coord;

    glm_vec3_copy((vec3){1.0f, tr->heights[index_z*(tr->grid_width) + next_index_i], 0.0f}, p1);
    glm_vec3_copy((vec3){0.0f, tr->heights[next_index_z*(tr->grid_width) + index_i], 1.0f}, p2);
    glm_vec3_copy((vec3){1.0f, tr->heights[next_index_z*(tr->grid_width) + next_index_i], 1.0f}, p3);

    return KRR_math_barycentric_xz(p1, p2, p3, p);
  }
}

void update_camera(float delta_time)
{
  if (is_freelook_mode_enabled)
  {
    // lerp 10% to target of player's dummy object rotation
    glm_quat_lerp(cam_rot, player_dummy_object_rot, 0.45f, cam_rot);

    // define relative position to place our camera right behind the dummy object
    CGLM_ALIGN(8) vec3 campos;
    glm_vec3_copy((vec3){0.0f, 0.0f, 0.2f}, campos);

    // get matrix from quaternion
    CGLM_ALIGN_MAT mat4 cam_transform;
    glm_quat_mat4(cam_rot, cam_transform);

    // transform position to place camera behind
    glm_vec3_rotate_m4(cam_transform, campos, campos);
    glm_vec3_add(campos, player_dummy_pos, cam.pos);

    // compute view matrix (lookat vector)
    // always use +y as UP 
    glm_lookat(cam.pos, player_dummy_pos, GLM_YUP, g_view_matrix);
  }
  else
  {
    // define relative position to place our camera right behind the player's position
    CGLM_ALIGN(8) vec3 campos;
    glm_vec3_copy((vec3){0.0f, 0.0f, 1.0f}, campos);

    // update player's position y, only when player is on the ground
    if (!is_player_inair)
    {
      player_position[1] = KRR_math_lerp(player_position[1], compute_posy(player_position[0], player_position[2]), 0.15f);
    }

    // cam modes
    // 1: addition: zoom in/out
    // compute direction vector to move camera back
    CGLM_ALIGN(8) vec3 back;
    glm_vec3_sub(cam.pos, player_position, back);
    // scale * unit(v)
    glm_vec3_scale_as(back, player_cam_distance, back);
    glm_vec3_add(cam.pos, back, cam.pos);

    // 2: addition: pitch cam
    // note: rotate around x-axis
    float angle_rad = glm_rad(player_cam_pitch_angle);
    float y_dst = player_cam_distance * sin(angle_rad);
    float z_dst = player_cam_distance * cos(angle_rad); // used as a distance for yaw rotation

    // transform relative cam position
    // 3: addition: angle around player
    // note: rotate around y-axis
    float angle2_rad = GLM_PI - glm_rad(-player_forward_rotation + player_cam_yaw_angle);
    float z2_dst = z_dst * cos(angle2_rad);
    float x2_dst = z_dst * sin(angle2_rad);
    glm_vec3_add(campos, (vec3){x2_dst, y_dst, z2_dst}, campos);

    // update to cam.pos
    glm_vec3_add(campos, player_position, cam.pos);

    // adjust look at position to be at the head of player
    CGLM_ALIGN(8) vec3 lookat_pos;
    glm_vec3_copy((vec3){0.0f, 10.0f, 0.0f}, lookat_pos);
    glm_vec3_add(lookat_pos, player_position, lookat_pos);

    // create view matrix
    glm_lookat(cam.pos, lookat_pos, GLM_YUP, g_view_matrix);
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
      move[1] = 0.0f;
      glm_vec3_normalize(move);

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
      move[1] = 0.0f;
      glm_vec3_normalize(move);

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
      forward[1] = 0.0f;
      glm_vec3_normalize(forward);

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
      move[1] = 0.0f;
      glm_vec3_normalize(move);

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
      move[0] = 0.0f;
      move[2] = 0.0f;
      glm_vec3_normalize(move);

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
      move[0] = 0.0f;
      move[2] = 0.0f;
      glm_vec3_normalize(move);

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
    CGLM_ALIGN(8) vec3 gravity;
    glm_vec3_scale(GLM_YUP, GRAVITY * delta_time, gravity);
    // update position
    glm_vec3_add(player_jump_velocity, player_position, player_position);
    // update velocity
    glm_vec3_add(player_jump_velocity, gravity, player_jump_velocity);

    // define ground offset to be slightly below the ground
    // to have the effect of bouncing back (as lerping will further work in update_camera())
    const float ground_offset = 1.5f;
    // get the current player's position y
    float curr_player_posy = compute_posy(player_position[0], player_position[2]);

    // check if player touches the ground
    if (player_position[1] < curr_player_posy - ground_offset && player_jump_velocity[1] < 0.0f)
    {
      is_player_inair = false;
      // set player on the ground
      player_position[1] = curr_player_posy - ground_offset;
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
  CGLM_ALIGN_MAT mat4 t_mat;  // for temp converted from quaternion, rotate object according
                              // to current terrain's normal

  // STALL & TREE & PLAYER
  KRR_SHADERPROG_bind(texture3d_shader->program);
  // render stall
  glBindVertexArray(stall->vao_id);
    // bind texture
    glBindTexture(GL_TEXTURE_2D, stall_texture->texture_id);

    // transform model matrix
    glm_mat4_copy(g_base_model_matrix, texture3d_shader->model_matrix);
    glm_translate(texture3d_shader->model_matrix, stall_pos);

    // convert from quaternion to matrix
    glm_quat_mat4(stall_rot, t_mat);
    // (t_mat must be on the right of multiplication as we want it to happen first!)
    glm_mat4_mul(texture3d_shader->model_matrix, t_mat, texture3d_shader->model_matrix);

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
      
      // convert from quaternion to matrix
      glm_quat_mat4(tree_rots[i], t_mat);
      // (t_mat must be on the right of multiplication as we want it to happen first!)
      glm_mat4_mul(texture3d_shader->model_matrix, t_mat, texture3d_shader->model_matrix);
      
      // update model matrix to GPU
      KRR_TEXSHADERPROG3D_update_model_matrix(texture3d_shader);

      // render
      SIMPLEMODEL_render(tree);
    }

  // render lamp
  glBindVertexArray(lamp->vao_id);
    glBindTexture(GL_TEXTURE_2D, lamp_texture->texture_id);

    for (int i=0; i<NUM_LAMP; ++i)
    {
      glm_mat4_copy(g_base_model_matrix, texture3d_shader->model_matrix);
      glm_translate(texture3d_shader->model_matrix, lamp_pos[i]);
      KRR_TEXSHADERPROG3D_update_model_matrix(texture3d_shader);
      SIMPLEMODEL_render(lamp);
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
    glm_translate(terrain3d_shader->model_matrix, (vec3){-tr->grid_width*TERRAIN_SLOT_SIZE/2, 0.0f, -tr->grid_height*TERRAIN_SLOT_SIZE/2});
    //update model matrix
    KRR_TERRAINSHADERPROG3D_update_model_matrix(terrain3d_shader);

    // render
    KRR_TERRAIN_render(tr);

    // set back to default texture
    glActiveTexture(GL_TEXTURE0);
  // unbind shader
  KRR_SHADERPROG_unbind(terrain3d_shader->program);

  // TEXTURE ALPHA fern
  KRR_SHADERPROG_bind(texturealpha3d_shader->program);
  // disable backface culling as fern made up of crossing polygon
  glDisable(GL_CULL_FACE);

  // render fern
  glBindVertexArray(fern->vao_id);
    // bind texture
    glBindTexture(GL_TEXTURE_2D, fern_texture->texture_id);

    for (int i=0; i<NUM_FERN; ++i)
    {
      // set computed result to shader
      glm_vec4_copy(fern_clipped_texcoords[fern_texcoord_is[i]], texturealpha3d_shader->clipped_texcoord);
      // update to GPU
      KRR_TEXALPHASHADERPROG3D_update_clipped_texcoord(texturealpha3d_shader);
      
      glm_mat4_copy(g_base_model_matrix, texturealpha3d_shader->model_matrix);
      glm_translate(texturealpha3d_shader->model_matrix, randomized_fern_pos[i]);

      // convert from quaternion to matrix
      glm_quat_mat4(fern_rots[i], t_mat);
      // t_mat must be on the right!
      glm_mat4_mul(texturealpha3d_shader->model_matrix, t_mat, texturealpha3d_shader->model_matrix);

      // update matrix to GPU
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
  if (lamp_texture != NULL)
  {
    KRR_TEXTURE_free(lamp_texture);
    lamp_texture = NULL;
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
  if (lamp != NULL)
  {
    SIMPLEMODEL_free(lamp);
    lamp = NULL;
  }
  if (tr != NULL)
  {
    KRR_TERRAIN_free(tr);
    tr = NULL;
  }
}
