// demo custom shader
// note that shader is based on orthographic projection matrix
// note2: don't be afraid for long number of lines in this sample as I included doublemulticolor shader code (320 lines in total) in this source code as it's super custom and not really suitable to be provided by engine itself. Just look for comment of for start and end of such section.

#include "usercode.h"
#include "functs.h"
#include "foundation/common.h"
#include "foundation/window.h"
#include "foundation/util.h"
#include "foundation/cam.h"
#include "graphics/util.h"
#include "graphics/texturedpp2d.h"
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
// -- section of variables for maintaining aspect ratio -- //

// -- section of function signatures -- //
static void usercode_app_went_windowed_mode();
static void usercode_app_went_fullscreen();
// -- end of section of function signatures -- //

// basic shaders and font
static KRR_TEXSHADERPROG2D* texture_shader = NULL;
static KRR_FONTSHADERPROG2D* font_shader = NULL;
static KRR_FONT* font = NULL;
static KRR_CAM cam;

// double multicolor
static GLuint vertex_vbo = 0;
static GLuint rgby_vbo = 0;
static GLuint cymw_vbo = 0;
static GLuint gray_vbo = 0;
static GLuint ibo = 0;
static GLuint left_vao = 0;
static GLuint right_vao = 0;

// -- start of doublemulticolor shader code --
// -- except the line of define our variable to use it immediately next line after next typedef --
typedef struct KRR_DMULTICSHADERPROG2D_
{
  /// underlying shader program
  KRR_SHADERPROG* program;

  /// attribute location
  GLint vertex_pos2d_location;
  GLint multicolor1_location;
  GLint multicolor2_location;

  /// uniform location
  /// (internal use)
  GLint projection_matrix_location;
  GLint view_matrix_location;
  GLint model_matrix_location;

  // matrices
  mat4 projection_matrix;
  mat4 view_matrix;
  mat4 model_matrix;

} KRR_DMULTICSHADERPROG2D;
static KRR_DMULTICSHADERPROG2D* multicolor_shader = NULL;

///
/// create a new double multi-color shader program.
/// it will also create and manage underlying program (KRR_SHADERPROG).
/// user has no responsibility to free its underlying attribute again.
///
/// \return Newly created KRR_DMULTICSHADERPROG2D on heap.
///
static KRR_DMULTICSHADERPROG2D* KRR_DMULTICSHADERPROG2D_new(void);

///
/// Free internals
///
/// \param program pointer to KRR_DMULTICSHADERPROG2D
///
static void KRR_DMULTICSHADERPROG2D_free_internals(KRR_DMULTICSHADERPROG2D* program);

///
/// free double multi-color shader program.
///
/// \param program pointer to gl_ldouble_multicolor_polygon
static void KRR_DMULTICSHADERPROG2D_free(KRR_DMULTICSHADERPROG2D* program);

///
/// load program
///
/// \param program pointer to KRR_DMULTICSHADERPROG2D
///
static bool KRR_DMULTICSHADERPROG2D_load_program(KRR_DMULTICSHADERPROG2D* program);

///
/// Enable all vertex attribute pointers
///
/// \param program pointer to program
#define KRR_DMULTICSHADERPROG2D_enable_all_vertex_attrib_pointers(program) KRR_gputil_enable_vertex_attrib_pointers(program->vertex_pos2d_location, program->multicolor1_location, program->multicolor2_location, -1)

///
/// Disable all vertex attribute pointers
///
/// \param program pointer to program
///
#define KRR_DMULTICSHADERPROG2D_disable_all_vertex_attrib_pointers(program) KRR_gputil_disable_vertex_attrib_pointers(program->vertex_pos2d_location, program->multicolor1_location, program->multicolor2_location, -1)

/// set vertex pointer (packed version)
/// it will set stride as 0 as packed format.
/// if caller intend to use a single VBO combining several vertex data type together then this function is not the one you're looking for.
/// data - offset pointer to data
#define KRR_DMULTICSHADERPROG2D_set_attrib_vertex_pos2d_pointer_packed(program, data) glVertexAttribPointer(program->vertex_pos2d_location, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)data)

/// set attrib multicolor either 1 or 2 (packed version)
/// program - shader program
/// color - 1 for multicolor-1, 2 for multicolor-2
/// data - offset pointer to data
#define KRR_DMULTICSHADERPROG2D_set_attrib_multicolor_pointer_packed(program, color, data) glVertexAttribPointer(color == 1 ? program->multicolor1_location : program->multicolor2_location, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)data)

KRR_DMULTICSHADERPROG2D* KRR_DMULTICSHADERPROG2D_new(void)
{
  KRR_DMULTICSHADERPROG2D* out = malloc(sizeof(KRR_DMULTICSHADERPROG2D));

  // init zeros first
  out->program = NULL;
  out->vertex_pos2d_location = -1;
  out->multicolor1_location = -1;
  out->multicolor2_location = -1;
  out->projection_matrix_location = -1;
  out->view_matrix_location = -1;
  out->model_matrix_location = -1;
  glm_mat4_identity(out->projection_matrix);
  glm_mat4_identity(out->view_matrix);
  glm_mat4_identity(out->model_matrix);

  // init
  out->program = KRR_SHADERPROG_new();

  return out;
}

void KRR_DMULTICSHADERPROG2D_free_internals(KRR_DMULTICSHADERPROG2D* program)
{
  // free underlying shader program
  KRR_SHADERPROG_free(program->program);

  program->vertex_pos2d_location = -1;
  program->multicolor1_location = -1;
  program->multicolor2_location = -1;
  program->projection_matrix_location = -1;
  program->view_matrix_location = -1;
  program->model_matrix_location = -1;

  glm_mat4_identity(program->projection_matrix);
  glm_mat4_identity(program->view_matrix);
  glm_mat4_identity(program->model_matrix);
}

void KRR_DMULTICSHADERPROG2D_free(KRR_DMULTICSHADERPROG2D* program)
{
  // free internals
  KRR_DMULTICSHADERPROG2D_free_internals(program);

  // free source
  free(program);
  program = NULL;
}

bool KRR_DMULTICSHADERPROG2D_load_program(KRR_DMULTICSHADERPROG2D* program)
{
  // create a new program
  GLuint program_id = glCreateProgram();

  // set shader source
  const char* vert_shader_lines = "#version 330 core\n\
                             uniform mat4 projection_matrix;\n\
                             uniform mat4 view_matrix;\n\
                             uniform mat4 model_matrix;\n\
                             \n\
                             in vec2 vertex_pos2d;\n\
                             \n\
                             in vec4 multicolor1;\n\
                             in vec4 multicolor2;\n\
                             \n\
                             out vec4 multicolor;\n\
                             \n\
                             void main()\n\
                             {\n\
                               multicolor = multicolor1 * multicolor2;\n\
                               gl_Position = projection_matrix * view_matrix * model_matrix * vec4(vertex_pos2d.xy, 0.0, 1.0);\n\
                             }";
  // create shader id for vertex shader
  GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  // replace shader source
  glShaderSource(vertex_shader_id, 1, &vert_shader_lines, NULL);
  // compile shader source
  glCompileShader(vertex_shader_id);

  // check shader for errors
  GLint shader_compiled = GL_FALSE;
  glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &shader_compiled);
  if (shader_compiled != GL_TRUE)
  {
    KRR_LOGE("Unable to compile shader %d. Source: %s", vertex_shader_id, vert_shader_lines);
    KRR_SHADERPROG_print_shader_log(vertex_shader_id);

    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return 0;
  }

  // attach vertex shader to shader program
  glAttachShader(program_id, vertex_shader_id);
  // check for errors
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    KRR_LOGE("Error attaching vertex shader");
    KRR_SHADERPROG_print_shader_log(vertex_shader_id);

    // delete shader
    glDeleteShader(vertex_shader_id);
    vertex_shader_id = 0;

    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return false;
  }

  // load fragment shader
  //GLuint fragment_shader_id = KRR_SHADERPROG_load_shader_from_file("_/_assets/doublemulticolorpp2d.frag", GL_FRAGMENT_SHADER);
  const char* frag_shader_lines = "#version 330 core\n\
                             in vec4 multicolor;\n\
                             \n\
                             out vec4 final_color;\n\
                             \n\
                             void main()\n\
                             {\n\
                               final_color = multicolor;\n\
                             }";
  // create shader id for fragment shader
  GLuint frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  // replace shader source
  glShaderSource(frag_shader_id, 1, &frag_shader_lines, NULL);
  // compile shader source
  glCompileShader(frag_shader_id);

  // check shader for errors
  shader_compiled = GL_FALSE;
  glGetShaderiv(frag_shader_id, GL_COMPILE_STATUS, &shader_compiled);
  if (shader_compiled != GL_TRUE)
  {
    KRR_LOGE("Unable to compile shader %d. Source: %s", frag_shader_id, frag_shader_lines);
    KRR_SHADERPROG_print_shader_log(frag_shader_id);

    // delete shader
    glDeleteShader(vertex_shader_id);
    vertex_shader_id = 0;
    
    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return false;
  }

  // attach fragment shader to program
  glAttachShader(program_id, frag_shader_id);
  // check for errors
  error = glGetError();
  if (error != GL_NO_ERROR)
  {
    KRR_LOGE("Error attaching fragment shader");
    KRR_SHADERPROG_print_shader_log(frag_shader_id);

    // delete fragment shader
    glDeleteShader(frag_shader_id);
    frag_shader_id = 0;

    // delete vertex shader
    glDeleteShader(vertex_shader_id);
    vertex_shader_id = 0;

    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return false;
  }

  // link program
  glLinkProgram(program_id);
  error = glGetError();
  if (error != GL_NO_ERROR)
  {
    KRR_LOGE("Error linking program");
    KRR_SHADERPROG_print_program_log(program_id);

    // delete vertex shader
    glDeleteShader(vertex_shader_id);
    vertex_shader_id = 0;

    // delete fragment shader
    glDeleteShader(frag_shader_id);
    frag_shader_id = 0;

    // delete program
    glDeleteProgram(program_id);
    program_id = 0;

    return false;
  }

  // set result program id to underlying program
  program->program->program_id = program_id;

  // mark shader for delete
  glDeleteShader(vertex_shader_id);
  glDeleteShader(frag_shader_id);

  // get attribute locations
  program->vertex_pos2d_location = glGetAttribLocation(program_id, "vertex_pos2d");
  if (program->vertex_pos2d_location == -1)
  {
    KRR_LOGW("Warning: cannot get location of vertex_pos2d");
  }
  program->multicolor1_location = glGetAttribLocation(program_id, "multicolor1");
  if (program->multicolor1_location == -1)
  {
    KRR_LOGW("Warning: cannot get location of multicolor1");
  }
  program->multicolor2_location = glGetAttribLocation(program_id, "multicolor2");
  if (program->multicolor2_location == -1)
  {
    KRR_LOGW("Warning: cannot get location of multicolor2");
  }

  // get uniform locations
  program->projection_matrix_location = glGetUniformLocation(program_id, "projection_matrix");
  if (program->projection_matrix_location == -1)
  {
    KRR_LOGW("Warning: cannot get location of projection_matrix");
  }
  program->view_matrix_location = glGetUniformLocation(program_id, "view_matrix");
  if (program->view_matrix_location == -1)
  {
    KRR_LOGW("Warning: cannot get location of view_matrix");
  }
  program->model_matrix_location = glGetUniformLocation(program_id, "model_matrix");
  if (program->model_matrix_location == -1)
  {
    KRR_LOGW("Warning: cannot get location of model_matrix");
  }

  return true;
}
// -- end of doublemulticolor shader code --

void usercode_app_went_windowed_mode()
{
  // for custom one we didn't have refactored code to provide just yet, so here
  // do it manually :)
  KRR_SHADERPROG_bind(multicolor_shader->program);
    glm_mat4_copy(g_ui_projection_matrix, multicolor_shader->projection_matrix);
    KRR_gputil_update_matrix(multicolor_shader->projection_matrix_location, multicolor_shader->projection_matrix);

    glm_mat4_copy(g_base_model_matrix, multicolor_shader->view_matrix);
    KRR_gputil_update_matrix(multicolor_shader->view_matrix_location, multicolor_shader->view_matrix);

    glm_mat4_copy(g_base_model_matrix, multicolor_shader->model_matrix);
    KRR_gputil_update_matrix(multicolor_shader->model_matrix_location, multicolor_shader->model_matrix);

  // use macros to help out
  SU_BEGIN(texture_shader)
    SU_TEXSHADERPROG2D(texture_shader)

  SU_BEGIN(font_shader)
    SU_FONTSHADER(font_shader)
  SU_END(font_shader);
}

void usercode_app_went_fullscreen()
{
  KRR_SHADERPROG_bind(multicolor_shader->program);
    glm_mat4_copy(g_ui_projection_matrix, multicolor_shader->projection_matrix);
    KRR_gputil_update_matrix(multicolor_shader->projection_matrix_location, multicolor_shader->projection_matrix);

    glm_mat4_copy(g_base_model_matrix, multicolor_shader->view_matrix);
    KRR_gputil_update_matrix(multicolor_shader->view_matrix_location, multicolor_shader->view_matrix);

    glm_mat4_copy(g_base_model_matrix, multicolor_shader->model_matrix);
    KRR_gputil_update_matrix(multicolor_shader->model_matrix_location, multicolor_shader->model_matrix);

  SU_BEGIN(texture_shader)
    SU_TEXSHADERPROG2D(texture_shader)

  SU_BEGIN(font_shader)
    SU_FONTSHADER(font_shader)
  SU_END(font_shader);
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
  glm_perspective(GLM_PI_4f, g_screen_width * 1.0f / g_screen_height, 0.01f, 100.0f, g_projection_matrix);
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

  // enable face culling
  glEnable(GL_CULL_FACE);

  // initially start user's camera looking at -z, and up with +y
  glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, cam.forward);
  glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, cam.up);
  glm_vec3_copy((vec3){0.0f, 0.0f, 3.0f}, cam.pos);

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

  // TODO: Load media here...
  multicolor_shader = KRR_DMULTICSHADERPROG2D_new();
  if (!KRR_DMULTICSHADERPROG2D_load_program(multicolor_shader))
  {
    return false;
  }

  // initially update matrices for multicolor shader
  KRR_SHADERPROG_bind(multicolor_shader->program);
    // set matrices
    glm_mat4_copy(g_ui_projection_matrix, multicolor_shader->projection_matrix);
    glm_mat4_copy(g_view_matrix, multicolor_shader->view_matrix);
    glm_mat4_copy(g_base_model_matrix, multicolor_shader->model_matrix);
    // issue update matrices to gpu
    KRR_gputil_update_matrix(multicolor_shader->projection_matrix_location, multicolor_shader->projection_matrix);
    KRR_gputil_update_matrix(multicolor_shader->view_matrix_location, multicolor_shader->view_matrix);
    KRR_gputil_update_matrix(multicolor_shader->model_matrix_location, multicolor_shader->model_matrix);

  // initially update all related matrices and related graphics stuf for both shaders
  SU_BEGIN(texture_shader)
    SU_TEXSHADERPROG2D(texture_shader)
    // set texture unit
    KRR_TEXSHADERPROG2D_set_texture_sampler(texture_shader, 0);

  SU_BEGIN(font_shader)
    SU_FONTSHADER(font_shader)
    // set texture unit
    KRR_FONTSHADERPROG2D_set_texture_sampler(font_shader, 0);
  SU_END(font_shader)

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
        glm_perspective(GLM_PI_4f, g_ri_view_width * 1.0f / g_ri_view_height, 0.01f, 100.0f, g_projection_matrix);

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
        glm_perspective(GLM_PI_4f, g_ri_view_width * 1.0f / g_ri_view_height, 0.01f, 100.0f, g_projection_matrix);

				// re-calculate base model matrix
				glm_mat4_identity(g_base_ui_model_matrix);
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

  // double color (custom shader)
  KRR_SHADERPROG_bind(multicolor_shader->program);
  glm_mat4_copy(g_base_model_matrix, multicolor_shader->view_matrix);
  KRR_gputil_update_matrix(multicolor_shader->view_matrix_location, multicolor_shader->view_matrix);

  // texture 2d
  KRR_SHADERPROG_bind(texture_shader->program);
  usercode_set_matrix_then_update_to_shader(USERCODE_MATRIXTYPE_VIEW_MATRIX, USERCODE_SHADERTYPE_TEXTURE_SHADER, texture_shader);
  KRR_SHADERPROG_unbind(texture_shader->program);
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

  // TODO: render code goes here...
  // bind left vao
  glBindVertexArray(left_vao);
    // bind shader
    KRR_SHADERPROG_bind(multicolor_shader->program);

    // start fresh with model matrix
    // note: see why we use ui-model matrix at the top-most comment
    glm_mat4_copy(g_base_ui_model_matrix, multicolor_shader->model_matrix);

    // transform matrix for left quad
    glm_translate(multicolor_shader->model_matrix, (vec3){g_logical_width * 1.f / 4.f, g_logical_height / 2.f, 0.f});
    KRR_gputil_update_matrix(multicolor_shader->model_matrix_location, multicolor_shader->model_matrix);

    // render left quad
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);

  // bind right vao
  glBindVertexArray(right_vao);
    // start fresh
    glm_mat4_copy(g_base_ui_model_matrix, multicolor_shader->model_matrix);
    // transform matrix for right quad
    glm_translate(multicolor_shader->model_matrix, (vec3){g_logical_width * 3.f / 4.f, g_logical_height / 2.f, 0.f});
    KRR_gputil_update_matrix(multicolor_shader->model_matrix_location, multicolor_shader->model_matrix);

    // render right quad
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);

    // unbind shader
    KRR_SHADERPROG_unbind(multicolor_shader->program);
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
  if (multicolor_shader != NULL)
  {
    KRR_DMULTICSHADERPROG2D_free(multicolor_shader);
    multicolor_shader = NULL;
  }

  if (vertex_vbo != 0)
    glDeleteBuffers(1, &vertex_vbo);
  if (rgby_vbo != 0)
    glDeleteBuffers(1, &rgby_vbo);
  if (cymw_vbo != 0)
    glDeleteBuffers(1, &cymw_vbo);
  if (gray_vbo != 0)
    glDeleteBuffers(1, &gray_vbo);
  if (ibo != 0)
    glDeleteBuffers(1, &ibo);
  if (left_vao != 0)
    glDeleteVertexArrays(1, &left_vao);
  if (right_vao != 0)
    glDeleteVertexArrays(1, &right_vao);
}
