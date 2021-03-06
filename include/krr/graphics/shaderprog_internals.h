#ifndef KRR_SHADERPROG_internals_h_
#define KRR_SHADERPROG_internals_h_

/// this header is meant to be used internally by library or if you know what you're doing

#include "krr/graphics/shaderprog.h"

#ifdef __cplusplus
extern "C" {
#endif

///
/// Init defaults
///
/// \param pointer to KRR_SHADERPROG
///
extern void KRR_SHADERPROG_init_defaults(KRR_SHADERPROG* shader_program);

#ifdef __cplusplus
}
#endif

#endif
