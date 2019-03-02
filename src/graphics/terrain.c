#include "graphics/terrain.h"
#include "graphics/terrain_shader3d.h"
#include "graphics/objloader.h"
#include "graphics/texture.h"
#include <stdlib.h>
#include "foundation/log.h"

#define MIN_TERRAIN_HEIGHT -50
#define MAX_TERRAIN_HEIGHT 100

static void init_defaults(TERRAIN* tr)
{
  tr->vertices = NULL;
  tr->vertices_count = 0;

  tr->indices = NULL;
  tr->indices_count = 0;

  tr->grid_width = 0;
  tr->grid_height = 0;

  tr->vbo_id = 0;
  tr->ibo_id = 0;
  tr->vao_id = 0;
}

TERRAIN* KRR_TERRAIN_new()
{
  TERRAIN* out = malloc(sizeof(TERRAIN));
  init_defaults(out);

  return out;
}

void KRR_TERRAIN_free_internals(TERRAIN* tr)
{
  if (tr->vertices != NULL)
  {
    free(tr->vertices);
    tr->vertices = NULL;
    tr->vertices_count = 0;
  }

  if (tr->indices != NULL)
  {
    free(tr->indices);
    tr->indices = NULL;
    tr->indices_count = 0;
  }

  if (tr->vbo_id != 0)
  {
    glDeleteBuffers(1, &tr->vbo_id);
    tr->vbo_id = 0;
  }
  if (tr->ibo_id != 0)
  {
    glDeleteBuffers(1, &tr->ibo_id);
    tr->ibo_id = 0;
  }
  if (tr->vao_id != 0)
  {
    glDeleteBuffers(1, &tr->vao_id);
    tr->vao_id = 0;
  }
}

bool KRR_TERRAIN_load_objfile(TERRAIN* tr, const char* filepath)
{
  // unload first
  KRR_TERRAIN_unload(tr);

  // load .obj file
  KRR_load_objfile(filepath, &tr->vertices, &tr->vertices_count, &tr->indices, &tr->indices_count);

  // create vbo
  glGenBuffers(1, &tr->vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, tr->vbo_id);
  glBufferData(GL_ARRAY_BUFFER, tr->vertices_count * sizeof(VERTEXTEXNORM3D), tr->vertices, GL_STATIC_DRAW);

  glGenBuffers(1, &tr->ibo_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tr->ibo_id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, tr->indices_count * sizeof(GLuint), tr->indices, GL_STATIC_DRAW);

  // free vertices and indices as we loaded into opengl buffer now
  free(tr->vertices);
  tr->vertices = NULL;
  free(tr->indices);
  tr->indices = NULL;

  // vao
  glGenVertexArrays(1, &tr->vao_id);
  glBindVertexArray(tr->vao_id);

    // enable vertex attributes
    KRR_TERRAINSHADERPROG3D_enable_attrib_pointers(shared_terrain3d_shaderprogram);

    // set vertex data
    glBindBuffer(GL_ARRAY_BUFFER, tr->vbo_id);
    KRR_TERRAINSHADERPROG3D_set_vertex_pointer(shared_terrain3d_shaderprogram, sizeof(VERTEXTEXNORM3D), (GLvoid*)offsetof(VERTEXTEXNORM3D, position));
    KRR_TERRAINSHADERPROG3D_set_texcoord_pointer(shared_terrain3d_shaderprogram, sizeof(VERTEXTEXNORM3D), (GLvoid*)offsetof(VERTEXTEXNORM3D, texcoord));
    KRR_TERRAINSHADERPROG3D_set_normal_pointer(shared_terrain3d_shaderprogram, sizeof(VERTEXTEXNORM3D), (GLvoid*)offsetof(VERTEXTEXNORM3D, normal));

    // ibo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tr->ibo_id);

  // unbind vao
  glBindVertexArray(0);

  return true;
}

bool KRR_TERRAIN_load_from_generation(TERRAIN* tr, const char* heightmap_path, float size, float hfactor)
{
  // generate terrain's vertices and indices
  if (!KRR_TERRAIN_generate(heightmap_path, size, hfactor, &tr->vertices, &tr->vertices_count, &tr->indices, &tr->indices_count, &tr->grid_width, &tr->grid_height))
  {
    return false;
  }

  KRR_LOGI("terrain vertices count = %d", tr->vertices_count);
  KRR_LOGI("terrain indices count = %d", tr->indices_count);

  // create vbo
  glGenBuffers(1, &tr->vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, tr->vbo_id);
  glBufferData(GL_ARRAY_BUFFER, tr->vertices_count * sizeof(VERTEXTEXNORM3D), tr->vertices, GL_STATIC_DRAW);

  glGenBuffers(1, &tr->ibo_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tr->ibo_id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, tr->indices_count * sizeof(GLuint), tr->indices, GL_STATIC_DRAW);

  // free vertices and indices as we loaded into opengl buffer now
  free(tr->vertices);
  tr->vertices = NULL;
  free(tr->indices);
  tr->indices = NULL;

  // vao
  glGenVertexArrays(1, &tr->vao_id);
  glBindVertexArray(tr->vao_id);

    // enable vertex attributes
    KRR_TERRAINSHADERPROG3D_enable_attrib_pointers(shared_terrain3d_shaderprogram);

    // set vertex data
    glBindBuffer(GL_ARRAY_BUFFER, tr->vbo_id);
    KRR_TERRAINSHADERPROG3D_set_vertex_pointer(shared_terrain3d_shaderprogram, sizeof(VERTEXTEXNORM3D), (GLvoid*)offsetof(VERTEXTEXNORM3D, position));
    KRR_TERRAINSHADERPROG3D_set_texcoord_pointer(shared_terrain3d_shaderprogram, sizeof(VERTEXTEXNORM3D), (GLvoid*)offsetof(VERTEXTEXNORM3D, texcoord));
    KRR_TERRAINSHADERPROG3D_set_normal_pointer(shared_terrain3d_shaderprogram, sizeof(VERTEXTEXNORM3D), (GLvoid*)offsetof(VERTEXTEXNORM3D, normal));

    // ibo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tr->ibo_id);

  // unbind vao
  glBindVertexArray(0);

  return true;
}

void KRR_TERRAIN_render(TERRAIN* tr)
{
  glDrawElements(GL_TRIANGLES, tr->indices_count, GL_UNSIGNED_INT, NULL);
}

void KRR_TERRAIN_unload(TERRAIN* tr)
{
  // just call internal freeing
  // with this we don't free the source itself
  // as we will need it for future loading
  KRR_TERRAIN_free_internals(tr);
}

void KRR_TERRAIN_free(TERRAIN* tr)
{
  KRR_TERRAIN_free_internals(tr);
  
  // free source
  free(tr);
  tr = NULL;
}

bool KRR_TERRAIN_generate(const char* heightmap_path, float size, float hfactor, VERTEXTEXNORM3D** dst_vertices, int* vertices_count, GLuint** dst_indices, int* indices_count, int* rst_grid_width, int* rst_grid_height)
{
  // load the heightmap
  KRR_TEXTURE* heightmap = KRR_TEXTURE_new();
  if (!KRR_TEXTURE_load_texture_from_file(heightmap, heightmap_path))
  {
    KRR_LOGE("Error loading height map file file");
    return false;
  }

  const int grid_width_size = heightmap->width;
  const int grid_height_size = heightmap->height;
  const int verts_count = (grid_width_size + 1) * (grid_height_size + 1);
  const int ids_count = grid_width_size * grid_height_size * 2 * 3;

  VERTEXTEXNORM3D* vertices = malloc(sizeof(VERTEXTEXNORM3D) * verts_count);
  GLuint* indices = malloc(sizeof(GLuint) * ids_count);

  int idx = 0;

  // lock texture to get access to its pixel data
  if (!KRR_TEXTURE_lock(heightmap))
  {
    KRR_LOGE("Error locking heightmap file");
    return false;
  }

  //
  // A ---- B
  // |    / |
  // |  /   |
  // C ---- D
  //
  // operate 4 vertices at the time starting from A-C-B then B-C-D
  for (int j=0; j<=grid_height_size; ++j)
  {
    for (int i=0; i<=grid_width_size; ++i)
    {
      VERTEXTEXNORM3D v;

      // get index to get height value from heightmap
      int heightmap_i = i;
      if (heightmap_i >= grid_width_size)
        heightmap_i = grid_width_size - 1;

      int heightmap_j = j;
      if (heightmap_j >= grid_height_size)
        heightmap_j = grid_height_size - 1;

      // convert to proper range of height value
      // as it's grayscale, each color component has the same value, thus we just get value
      // from a single component (not alpha)
      // then we try to convert it into range that has no need to be adjusted in-game later i.e. [-N,N]
#define H_VAL(i, j) (heightmap->pixels[i + j*heightmap->width] & 0xFF)
#define H_CONV(h) (MAX_TERRAIN_HEIGHT - MIN_TERRAIN_HEIGHT) * h / 255.0f + MIN_TERRAIN_HEIGHT
      float height_val = H_VAL(heightmap_i, heightmap_j);
      height_val = H_CONV(height_val);
      
      // vertex
      v.position.x = i*size;
      v.position.y = height_val*hfactor;
      v.position.z = j*size;

      // texcoord
      // repeating of texture coord will be set in shader code
      v.texcoord.s = (i*size) / (grid_width_size*size);
      v.texcoord.t = (j*size) / (grid_height_size*size);

      int hL_i = i-1; if (hL_i < 0) hL_i = 0;
      int hR_i = i+1; if (hR_i >= grid_width_size) hR_i = grid_width_size - 1;
      int hD_i = j-1; if (hD_i < 0) hD_i = 0;
      int hU_i = j+1; if (hU_i >= grid_height_size) hU_i = grid_height_size - 1;

      int normal_i = i;
      if (normal_i >= grid_width_size) normal_i = grid_width_size - 1;
      int normal_j = j;
      if (normal_j >= grid_height_size) normal_j = grid_height_size - 1;

      float hL = H_CONV(H_VAL(hL_i, normal_j));
      float hR = H_CONV(H_VAL(hR_i, normal_j));
      float hD = H_CONV(H_VAL(normal_i, hD_i));
      float hU = H_CONV(H_VAL(normal_i, hU_i));

      CGLM_ALIGN(8) vec3 n;
      glm_vec3_copy((vec3){hR - hL, hD - hU, 2.0f}, n);
      glm_vec3_normalize(n);

      // normal
      v.normal.x = n[0];
      v.normal.y = n[1];
      v.normal.z = n[2];

      // set to result vertices pointer
      memcpy(vertices + idx++, &v, sizeof(v));
    }
  }

  idx = 0;

  int real_width_size = grid_width_size+1;
  // set indices
  for (int j=0; j<grid_height_size; ++j)
  {
    for (int i=0; i<grid_width_size; ++i)
    {
      indices[idx++] = j*real_width_size + i;
      indices[idx++] = (j+1)*real_width_size + i;
      indices[idx++] = j*real_width_size + i + 1;

      indices[idx++] = j*real_width_size + i + 1;
      indices[idx++] = (j+1)*real_width_size + i;
      indices[idx++] = (j+1)*real_width_size + i + 1;
    }
  }

  // unlock texture
  KRR_TEXTURE_unlock(heightmap);

  // free heightmap
  KRR_TEXTURE_free(heightmap);
  heightmap = NULL;

  // return the results
  if (dst_vertices != NULL)
  {
    *dst_vertices = vertices;
  }
  if (vertices_count != NULL)
  {
    *vertices_count = verts_count;
  }
  if (dst_indices != NULL)
  {
    *dst_indices = indices;
  }
  if (indices_count != NULL)
  {
    *indices_count = ids_count;
  }
  if (rst_grid_width != NULL)
  {
    *rst_grid_width = grid_width_size;
  }
  if (rst_grid_height != NULL)
  {
    *rst_grid_height = grid_height_size;
  }

  return true;
}
