#include "krr/graphics/skybox.h"
#include "krr/graphics/util.h"
#include "krr/graphics/skybox_shader.h"
#include <stdlib.h>

/// fixed number of vertices used to render skybox
/// we don't use indices buffer, thus all 36 vertices needed to be defined
#define NUM_VERTS 36

static void init_defaults(KRR_SKYBOX* sb)
{
  sb->vbo_id = 0;
  sb->vao_id = 0;
}

KRR_SKYBOX* KRR_SKYBOX_new()
{
  KRR_SKYBOX* out = malloc(sizeof(KRR_SKYBOX));
  init_defaults(out);

  return out;
}

bool KRR_SKYBOX_load(KRR_SKYBOX* sb, const char* right, const char* left, const char* top, const char* bottom, const char* back, const char* front)
{
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
  // set vertices
  
  // customize this size for how big of skybox
  #define SIZE 2000.0f

  // skybox's vertices
  #define POS0 -1.0f * SIZE,  1.0f * SIZE,  1.0f * SIZE
  #define POS1 -1.0f * SIZE, -1.0f * SIZE,  1.0f * SIZE
  #define POS2  1.0f * SIZE, -1.0f * SIZE,  1.0f * SIZE
  #define POS3  1.0f * SIZE,  1.0f * SIZE,  1.0f * SIZE
  #define POS4 -1.0f * SIZE,  1.0f * SIZE, -1.0f * SIZE
  #define POS5 -1.0f * SIZE, -1.0f * SIZE, -1.0f * SIZE
  #define POS6  1.0f * SIZE, -1.0f * SIZE, -1.0f * SIZE
  #define POS7  1.0f * SIZE,  1.0f * SIZE, -1.0f * SIZE

  // make sure faces are visible from inside the cube
  // as we're inside the cube
  float vertices[] = {
    // front
    POS4, POS5, POS7,
    POS5, POS6, POS7,
    // left
    POS0, POS1, POS4,
    POS1, POS5, POS4,
    // back
    POS3, POS2, POS0,
    POS2, POS1, POS0,
    // right
    POS7, POS6, POS3,
    POS6, POS2, POS3,
    // up
    POS0, POS4, POS3,
    POS4, POS7, POS3,
    // bottom
    POS5, POS1, POS6,
    POS1, POS2, POS6
  };

  // create vbo
  glGenBuffers(1, &sb->vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_id);
  glBufferData(GL_ARRAY_BUFFER, 3 * NUM_VERTS * sizeof(float), &vertices, GL_STATIC_DRAW);

  // load cubemap
  sb->cubemap_id = KRR_gputil_load_cubemap(right, left, top, bottom, back, front);
  if (sb->cubemap_id == -1)
  {
    glDeleteBuffers(1, &sb->vbo_id);
    sb->vbo_id = 0;
    return false;
  }

  // vao
  glGenVertexArrays(1, &sb->vao_id);
  glBindVertexArray(sb->vao_id);

    // enable vertex attributes
    // as all models use the same shader, we operate on shared shader here
    KRR_SKYBOXSHADERPROG_enable_attrib_pointers(shared_skybox_shaderprogram);

    // set vertex data
    glBindBuffer(GL_ARRAY_BUFFER, sb->vbo_id);
    // set to 0, as it's tightly packed together
    KRR_SKYBOXSHADERPROG_set_vertex_pointer(shared_skybox_shaderprogram, 0, 0);

    // bind cubemap
    glBindTexture(GL_TEXTURE_CUBE_MAP, sb->cubemap_id);
    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // unbind vao
  glBindVertexArray(0);

  return true;
}

void KRR_SKYBOX_free_internals(KRR_SKYBOX* sb)
{
  if (sb->vbo_id != 0)
  {
    glDeleteBuffers(1, &sb->vbo_id);
    sb->vbo_id = 0;
  }
  if (sb->vao_id != 0)
  {
    glDeleteBuffers(1, &sb->vao_id);
    sb->vao_id = 0;
  }
  if (sb->cubemap_id != 0)
  {
    glDeleteTextures(1, &sb->cubemap_id);
    sb->cubemap_id = 0;
  }
}

void KRR_SKYBOX_render(KRR_SKYBOX* sb)
{
  // set to less than or equal to be able to render on deepest depth value or empty pixel
  // as default depth value is 1.0f
  glDepthFunc(GL_LEQUAL);
  glDrawArrays(GL_TRIANGLES, 0, NUM_VERTS);
  // set depth function back to normal
  glDepthFunc(GL_LESS);
}

void KRR_SKYBOX_free(KRR_SKYBOX* sb)
{
  KRR_SKYBOX_free_internals(sb);

  // free source
  free(sb);
  sb = NULL;
}
