
#ifndef __included_terra_alloc_h
#define __included_terra_alloc_h

#include "tdefine.h"

/**
* Terra Memory Allocation
*
*   The purpose of this file is to set up accessors to memory
*   allocation functions. This can be used to improve memory
*   management, track memory allocation, ...
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
TPtr TAlloc(TSize size);

/**
* Re-allocate a block of memory
*
* @param size                The new block size
*
* @return                    A pointer to the memory allocated
*
*/
TPtr TRAlloc(TPtr ptr, TSize size);

/**
* De-allocate a block of memory
*
* @param ptr                 The block pointer
*
*/
void TFree(TPtr ptr);

/**
* Set Accessors to memory allocation functions. Use default function
* if a parameter is set to null.
*
* @param allocFunc           The accessor function for allocating memory
* @param rallocFunc          The accessor function for re-allocating memory
* @param freeFunc            The accessor function for de-allocating memory
*
*/
void TAllocSet(TPtr(*allocFunc)(TSize), TPtr(*rallocFunc)(TPtr, TSize), void(*freeFunc) (TPtr));

#define TAllocData(T) (T *) TAlloc(sizeof(T))

#endif
