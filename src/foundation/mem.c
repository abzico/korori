#include "mem.h"
#include <stdlib.h>

/// generic free memory of N bytes alignment
static void KRR_MEM_free_generic(void* p)
{
  // convert to byte pointer
  unsigned char* ptr = (unsigned char*)p;

  // grab offset size
  size_t offset = *(ptr-1);
  // free (original) whole memory space
  free(ptr - offset);
}

// concept for allocating aligned memory space
// overflow it, then turncate the lower bits
// those two techniques will make sure we have aligned memory
//
// return NULL in case it cannot allocate memory due to out of memory
void* KRR_MEM_malloc16(size_t s)
{
  // allocate memory 16 bytes (15 bytes for alignment size + 1 byte more to store offset size)
  // this means at worst case, we can possibly waste memory for up to 16 bytes unused to
  // satisfy the memory alignment
  unsigned char* p_original = malloc(s + 0x10);

  // if not enough memory
  if (p_original == NULL)
    return NULL;

  unsigned char* p = p_original;

  // overflow (1.) then truncate (2.) the memory to guaruntee aligned allocation
  //
  // 1.) 0x10 means we offset upwards by 0x0F bytes for actual target byte alignment + 0x01 byte 
  // allocated to store our offset size just before the pointer. Offset upwards like this will 
  // make sure we will pass the boundary location in which we will truncate it next.
  //
  // 2.) & ~0x0F means we truncate lower 4 bits, 0x0F comes from 0x10 - 1 in order for its result 
  // to be negated with ~ thus ~(00001111) => 11110000, so the memory address always at boundary
  // note: for N byte alignment, we only need to offset the pointer from 0 to (N-1) bytes
  p = (unsigned char*)((size_t)(p + 0x10) & (~0x0F));
  // store offset size at the byte before the pointer
  *(p-1) = p - p_original;

  return p;
}

// it will find the original pointer that point to whole memory space before applying with offset
// then free it, thus it needs to grab such offset size at the -1 offset location of input pointer
void KRR_MEM_free16(void* p)
{
  KRR_MEM_free_generic(p);
}

/// similar implementation, see comments in KRR_MEM_malloc16() and KRR_MEM_free16()
void* KRR_MEM_malloc8(size_t s)
{
  unsigned char* p_original = malloc(s + 0x08);

  if (p_original == NULL)
    return NULL;

  unsigned char* p = p_original;

  p = (unsigned char*)((size_t)(p + 0x08) & (~0x07));
  *(p-1) = p - p_original;

  return p;
}

void KRR_MEM_free8(void* p)
{
  KRR_MEM_free_generic(p);
}

void* KRR_MEM_malloc32(size_t s)
{
  unsigned char* p_original = malloc(s + 0x20);

  if (p_original == NULL)
    return NULL;

  unsigned char* p = p_original;

  p = (unsigned char*)((size_t)(p + 0x20) & (~0x1F));
  *(p-1) = p - p_original;

  return p;
}

void KRR_MEM_free32(void* p)
{
  KRR_MEM_free_generic(p);
}
