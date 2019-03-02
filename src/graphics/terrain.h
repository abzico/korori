#ifndef KRR_TERRAIN_h_
#define KRR_TERRAIN_h_

#include "graphics/common.h"

typedef struct
{
  /// (internally used)
  VERTEXTEXNORM3D* vertices;
  int vertices_count;

  /// (internally used)
  GLuint* indices;
  int indices_count;

  // will be set after loading completes
  // note: if load terrain via KRR_TERRAIN_load_objfile() function,
  // these information won't be available
  int grid_width;
  int grid_height;

  GLuint vbo_id;
  GLuint ibo_id;
  GLuint vao_id;
} TERRAIN;

///
/// Create a new TERRAIN on heap.
///
/// \return Pointer to newly created TERRAIN.
///
extern TERRAIN* KRR_TERRAIN_new();

///
/// Free internals of a simple model.
///
/// \param tr pointer to TERRAIN
///
extern void KRR_TERRAIN_free_internals(TERRAIN* tr);

///
/// Load model from .obj file.
///
/// \param tr a pointer to TERRAIN
/// \param filepath file path to an .obj file to load
/// \return true if load successfully, otherwise return false.
///
extern bool KRR_TERRAIN_load_objfile(TERRAIN* tr, const char* filepath);

///
/// Load terrain from generation algorithm from input specifications.
/// After this call, terrain is ready to be rendered.
///
/// \param tr pointer to TERRAIN
/// \param heightmap_path path to heightmap file
/// \param size distance between slot in pixels
/// \param hfactor height factor to be multiplied to height value
/// \return true if load successfully, otherwise return false.
///
extern bool KRR_TERRAIN_load_from_generation(TERRAIN* tr, const char* heightmap_path, float size, float hfactor);

///
/// Unload current loaded model.
/// This will make it ready for a next loading call.
///
/// \param tr a pointer to TERRAIN
///
extern void KRR_TERRAIN_unload(TERRAIN* tr);

///
/// Render
/// User needs to call glBindVertexArray(vao) before calling this function, to optimize the batch rendering. As well as necessary binding to relevant stuff before actual rendering.
///
/// \param pointer to TERRAIN
///
extern void KRR_TERRAIN_render(TERRAIN* tr);

///
/// Free a simple model.
///
/// \param tr pointer to pointer of TERRAIN
///
extern void KRR_TERRAIN_free(TERRAIN* tr);

///
/// Generate terrain
/// Get result for VERTEXTEXNORM3D and its indices.
///
/// \param heightmap_path path to heightmap file.
/// \param size distance between individual slot in grid to the next, in pixels
/// \param hfactor height factor to be multiplied with height value
/// \param dst_vertices dynamically created buffer for vertices. You should free it when done using it. The type depends on type flag set.
/// \param vertices_count number of vertices returned
/// \param dst_indices dynamically created buffer for indices. You should free it when done using it.
/// \param indices_count returned count of indices
/// \param rst_grid_width returned grid size in width
/// \param rst_grid_height returned grid size in height
/// \return return true for success, otherwise return false.
///
extern bool KRR_TERRAIN_generate(const char* heightmap_path, float size, float hfactor, VERTEXTEXNORM3D** dst_vertices, int* vertices_count, GLuint** dst_indices, int* indices_count, int* rst_grid_width, int* rst_grid_height);



#endif
