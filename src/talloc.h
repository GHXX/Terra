
#ifndef __included_terra_alloc_h
#define __included_terra_alloc_h

#include "tdefine.h"

/**
* Terra Memory Allocation
*
*   The purpose of this file is to set up accessors to memory
*   allocation functions.
*
*/

/**
* Allocate a block of memory
*
* @param size                The block size.
*
* @return                    A pointer to the memory allocated
*
*/
extern TPtr (*TAlloc)(TSize size);

/**
* Re-allocate a block of memory
*
* @param size                The new block size
*
* @return                    A pointer to the memory allocated
*
*/
extern TPtr (*TRAlloc)(TPtr ptr, TSize size);

/**
* De-allocate a block of memory
*
* @param ptr                 The block pointer
*
*/
extern void (*TFree)(TPtr ptr);

#define TAllocData(T) (T *) TAlloc(sizeof(T))
#define TAllocNData(T,N) (T *) TAlloc(sizeof(T) * N)

#endif
