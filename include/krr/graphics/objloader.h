#ifndef KRR_OBJLOADER_h_
#define KRR_OBJLOADER_h_

#include "krr/graphics/common.h"

#ifdef __cplusplus
extern "C" {
#endif

///
/// Load .obj file then return result of formed vertices.
/// Load vertex, texture coordinate and normals.
///
/// \param filepath file path of .obj file to parse
/// \param dst_vertices dynamically created buffer for vertices. You should free it when done using it. The type depends on type flag set.
/// \param vertices_count number of vertices returned
/// \param dst_indices dynamically created buffer for indices. You should free it when done using it.
/// \param indices_count returned count of indices
/// \return return 0 for success. Returned -1 if there's any error occurred.
///
extern int KRR_load_objfile(const char* filepath, VERTEXTEXNORM3D** dst_vertices, int* vertices_count, GLuint** dst_indices, int* indices_count);

#ifdef __cplusplus
}
#endif

#endif
