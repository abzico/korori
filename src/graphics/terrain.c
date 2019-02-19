#include "graphics/terrain.h"
#include "graphics/terrain_shader3d.h"
#include "graphics/objloader.h"
#include <stdlib.h>
#include "foundation/log.h"

static void init_defaults(TERRAIN* tr)
{
  tr->vertices = NULL;
  tr->vertices_count = 0;

  tr->indices = NULL;
  tr->indices_count = 0;

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

bool KRR_TERRAIN_load_from_generation(TERRAIN* tr, unsigned int grid_width_size, unsigned int grid_height_size, float size)
{
  // generate terrain's vertices and indices
  KRR_TERRAIN_generate(grid_width_size, grid_height_size, size, &tr->vertices, &tr->vertices_count, &tr->indices, &tr->indices_count);

  KRR_LOGI("vertices count = %d", tr->vertices_count);
  KRR_LOGI("indices count = %d", tr->indices_count);

  // create vbo
  glGenBuffers(1, &tr->vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, tr->vbo_id);
  glBufferData(GL_ARRAY_BUFFER, tr->vertices_count * sizeof(VERTEXTEXNORM3D), tr->vertices, GL_STATIC_DRAW);

  glGenBuffers(1, &tr->ibo_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tr->ibo_id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, tr->indices_count * sizeof(VERTEXTEXNORM3D), tr->indices, GL_STATIC_DRAW);

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

void KRR_TERRAIN_generate(unsigned int grid_width_size, unsigned int grid_height_size, float size, VERTEXTEXNORM3D** dst_vertices, int* vertices_count, GLuint** dst_indices, int* indices_count)
{
  const int verts_count = (grid_width_size + 1) * (grid_height_size + 1);
  const int ids_count = grid_width_size * grid_height_size * 2 * 3;

  VERTEXTEXNORM3D* vertices = malloc(sizeof(VERTEXTEXNORM3D) * verts_count);
  GLuint* indices = malloc(sizeof(GLuint) * ids_count);

  int idx = 0;

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

      // vertex
      v.position.x = i*size;
      v.position.y = 0.0f;
      v.position.z = j*size;

      // texcoord
      // repeating of texture coord will be set in shader code
      v.texcoord.s = (i*size) / (grid_width_size*size);
      v.texcoord.t = (j*size) / (grid_height_size*size);

      // normal
      // always point upwards (+y) for now
      v.normal.x = 0.0f;
      v.normal.y = 1.0f;
      v.normal.z = 0.0f;

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
}
