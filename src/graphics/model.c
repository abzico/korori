#include "graphics/model.h"
#include "graphics/objloader.h"
#include "graphics/texturedpp3d.h"
#include <stdlib.h>

static void init_defaults(SIMPLEMODEL* sm)
{
  sm->vertices = NULL;
  sm->vertices_count = 0;

  sm->indices = NULL;
  sm->indices_count = 0;

  sm->vbo_id = 0;
  sm->ibo_id = 0;
  sm->vao_id = 0;
}

SIMPLEMODEL* SIMPLEMODEL_new()
{
  SIMPLEMODEL* out = malloc(sizeof(SIMPLEMODEL));
  init_defaults(out);

  return out;
}

void SIMPLEMODEL_free_internals(SIMPLEMODEL* sm)
{
  if (sm->vertices != NULL)
  {
    free(sm->vertices);
    sm->vertices = NULL;
    sm->vertices_count = 0;
  }

  if (sm->indices != NULL)
  {
    free(sm->indices);
    sm->indices = NULL;
    sm->indices_count = 0;
  }

  if (sm->vbo_id != 0)
  {
    glDeleteBuffers(1, &sm->vbo_id);
    sm->vbo_id = 0;
  }
  if (sm->ibo_id != 0)
  {
    glDeleteBuffers(1, &sm->ibo_id);
    sm->ibo_id = 0;
  }
  if (sm->vao_id != 0)
  {
    glDeleteBuffers(1, &sm->vao_id);
    sm->vao_id = 0;
  }
}

bool SIMPLEMODEL_load_objfile(SIMPLEMODEL* sm, const char* filepath)
{
  // unload first
  SIMPLEMODEL_unload(sm);

  // load .obj file
  KRR_load_objfile(filepath, &sm->vertices, &sm->vertices_count, &sm->indices, &sm->indices_count);

  // create vbo
  glGenBuffers(1, &sm->vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, sm->vbo_id);
  glBufferData(GL_ARRAY_BUFFER, sm->vertices_count * sizeof(VERTEXTEXNORM3D), sm->vertices, GL_STATIC_DRAW);

  glGenBuffers(1, &sm->ibo_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sm->ibo_id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sm->indices_count * sizeof(GLuint), sm->indices, GL_STATIC_DRAW);

  // free vertices and indices as we loaded into opengl buffer now
  free(sm->vertices);
  sm->vertices = NULL;
  free(sm->indices);
  sm->indices = NULL;

  // vao
  glGenVertexArrays(1, &sm->vao_id);
  glBindVertexArray(sm->vao_id);

    // enable vertex attributes
    KRR_TEXSHADERPROG3D_enable_attrib_pointers(shared_textured3d_shaderprogram);

    // set vertex data
    glBindBuffer(GL_ARRAY_BUFFER, sm->vbo_id);
    KRR_TEXSHADERPROG3D_set_vertex_pointer(shared_textured3d_shaderprogram, sizeof(VERTEXTEXNORM3D), (GLvoid*)offsetof(VERTEXTEXNORM3D, position));
    KRR_TEXSHADERPROG3D_set_texcoord_pointer(shared_textured3d_shaderprogram, sizeof(VERTEXTEXNORM3D), (GLvoid*)offsetof(VERTEXTEXNORM3D, texcoord));
    KRR_TEXSHADERPROG3D_set_normal_pointer(shared_textured3d_shaderprogram, sizeof(VERTEXTEXNORM3D), (GLvoid*)offsetof(VERTEXTEXNORM3D, normal));

    // ibo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sm->ibo_id);

  // unbind vao
  glBindVertexArray(0);

  return true;
}

void SIMPLEMODEL_render(SIMPLEMODEL* sm)
{
  glDrawElements(GL_TRIANGLES, sm->indices_count, GL_UNSIGNED_INT, NULL);
}

void SIMPLEMODEL_unload(SIMPLEMODEL* sm)
{
  // just call internal freeing
  // with this we don't free the source itself
  // as we will need it for future loading
  SIMPLEMODEL_free_internals(sm);
}

void SIMPLEMODEL_free(SIMPLEMODEL* sm)
{
  SIMPLEMODEL_free_internals(sm);
  
  // free source
  free(sm);
  sm = NULL;
}


