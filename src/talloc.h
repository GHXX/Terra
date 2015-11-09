
#ifndef __included_terra_alloc_h
#define __included_terra_alloc_h

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
* @param _alloc              The accessor function for allocating memory
* @param _ralloc             The accessor function for re-allocating memory
* @param _free               The accessor function for de-allocating memory
*
*/
void TAllocSet(TPtr(*_alloc)(uint64_t), TPtr(*_ralloc)(TPtr, TSize), void(*_free) (TPtr));

#define TAllocData(T) (T *) TAlloc(sizeof(T))

#endif
