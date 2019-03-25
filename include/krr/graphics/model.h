#ifndef KRR_MODEL_h_
#define KRR_MODEL_h_

#include "krr/graphics/common.h"

typedef struct
{
  /// (internally used)
  VERTEXTEXNORM3D* vertices;
  int vertices_count;

  /// (internally used)
  GLuint* indices;
  int indices_count;

  GLuint vbo_id;
  GLuint ibo_id;
  GLuint vao_id;
} SIMPLEMODEL;

///
/// Create a new SIMPLEMODEL on heap.
///
/// \return Pointer to newly created SIMPLEMODEL.
///
extern SIMPLEMODEL* SIMPLEMODEL_new();

///
/// Free internals of a simple model.
///
/// \param sm pointer to SIMPLEMODEL
///
extern void SIMPLEMODEL_free_internals(SIMPLEMODEL* sm);

///
/// Load model from .obj file.
///
/// \param sm a pointer to SIMPLEMODEL
/// \param filepath file path to an .obj file to load
/// \return true if load successfully, otherwise return false.
///
extern bool SIMPLEMODEL_load_objfile(SIMPLEMODEL* sm, const char* filepath);

///
/// Unload current loaded model.
/// This will make it ready for a next loading call.
///
/// \param sm a pointer to SIMPLEMODEL
///
extern void SIMPLEMODEL_unload(SIMPLEMODEL* sm);

///
/// Render
/// User needs to call glBindVertexArray(vao) before calling this function, to optimize the batch rendering. As well as necessary binding to relevant stuff before actual rendering.
///
/// \param pointer to SIMPLEMODEL
///
extern void SIMPLEMODEL_render(SIMPLEMODEL* sm);

///
/// Free a simple model.
///
/// \param sm pointer to pointer of SIMPLEMODEL
///
extern void SIMPLEMODEL_free(SIMPLEMODEL* sm);

#endif
