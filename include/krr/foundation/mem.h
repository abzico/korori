#ifndef KRR_MEM_h_
#define KRR_MEM_h_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

///
/// Dynamically allocate chunk of aligned memory in 8 bytes.
///
/// \param s size in bytes of memory to be allocated
/// \return opaque pointer to allocated aligned memory, if there's not enough memory then return NULL.
///
extern void* KRR_MEM_malloc8(size_t s);

///
/// Free allocated memory pointed to by `p` for 8 bytes alignment.
///
/// \param p pointer points to 8 bytes aligned memory
///
extern void KRR_MEM_free8(void* p);

///
/// Dynamically allocate chunk of aligned memory in 16 bytes.
///
/// \param s size in bytes of memory to be allocated
/// \return opaque pointer to allocated aligned memory, if there's not enough memory then return NULL.
///
extern void* KRR_MEM_malloc16(size_t s);

///
/// Free allocated memory pointed to by `p` for 16 bytes alignment.
///
/// \param p pointer points to 16 bytes aligned memory
///
extern void KRR_MEM_free16(void* p);

///
/// Dynamically allocate chunk of aligned memory in 32 bytes.
///
/// \param s size in bytes of memory to be allocated
/// \return opaque pointer to allocated aligned memory, if there's not enough memory then return NULL.
///
extern void* KRR_MEM_malloc32(size_t s);

///
/// Free allocated memory pointed to by `p` for 32 bytes alignment.
///
/// \param p pointer points to 32 bytes aligned memory
///
extern void KRR_MEM_free32(void* p);

#ifdef __cplusplus
}
#endif

#endif
