#ifndef KRR_SKYBOX_h_
#define KRR_SKYBOX_h_

#include "krr/graphics/common.h"

typedef struct KRR_SKYBOX_S
{
  GLuint vbo_id;
  GLuint vao_id;

  GLuint cubemap_id;

} KRR_SKYBOX;

///
/// Create a new skybox.
///
/// \return newly created skybox on heap
///
extern KRR_SKYBOX* KRR_SKYBOX_new(void);

///
/// Load
///
/// \param right null-terminated string right-side texture path
/// \param left null-terminated string left-side texture path
/// \param top null-terminated string top-side texture path
/// \param bottom bull-terminated string bottom-side texture path
/// \param back null-terminated string back-side texture path
/// \param front null-terminated string front-side texture path
/// \return true if load successfully, otherwise false.
///
extern bool KRR_SKYBOX_load(KRR_SKYBOX* sb, const char* right, const char* left, const char* top, const char* bottom, const char* back, const char* front);

///
/// Free skybox's internals.
///
/// \param sb pointer to skybox
///
extern void KRR_SKYBOX_free_internals(KRR_SKYBOX* sb);

///
/// Render skybox
///
/// \param sb pointer to skybox
///
extern void KRR_SKYBOX_render(KRR_SKYBOX* sb);

///
/// Free skybox
///
/// \param sb pointer to skybox
///
extern void KRR_SKYBOX_free(KRR_SKYBOX* sb);

#endif
