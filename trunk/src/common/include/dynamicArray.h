/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file dynamicArray.h
 * @brief Declarations for untyped dynamic array
 *
 * @date Jan 25, 2011
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */


#ifndef DYNAMICARRAY_H_
#define DYNAMICARRAY_H_

#include "procTypes.h"

/**
 * The size of a basic DynArray-derived structure:
 * @code
 *   struct [DerivedStructType]
 *   {
 *     DynArray dynArray;
 *     [DerivedStructEntryType]* base;
 *     Index count;
 *   };
 * @endcode
 */
#define DYN_ARRAY_SIZE (sizeof(DynArray) + sizeof(void*) + sizeof(Index))

/**
 * @brief Creates fresh empty Untyped Dynamic Array
 * This operation includes allocation of memory for DEFAULT_VALUE_ROWS_NUMBER number of value rows.
 * For every created array, destroyDynArray() must be invoked to release the allocated memory.
 * @param[in, out] dynArray Untyped Dynamic Array
 * @param[in] entrySize The size of a single array entry in bytes
 * @param[in] chunkSize Initial number of entries and the number of entries to be added each expansion time
 * @return Error handling code
 */
errorCode createDynArray(DynArray* dynArray, size_t entrySize, uint16_t chunkSize);

/**
 * @brief Add new empty entry into the dynamic array
 * The pointer to the entry is passed back to be filled in
 *
 * @param[in, out] dynArray Untyped Dynamic Array
 * @param[out] entry the empty inserted entry to be filled in
 * @param[out] entryID the ID of the entry inserted
 * @return Error handling code
 */
errorCode addEmptyDynEntry(DynArray* dynArray, void** entry, Index* entryID);

/**
 * @brief Add new entry into the dynamic array
 * NOTE that the new entry is shallow copied!
 *
 * @param[in, out] dynArray Untyped Dynamic Array
 * @param[in] entry the inserted entry
 * @param[out] entryID the ID of the entry inserted
 * @return Error handling code
 */
errorCode addDynEntry(DynArray* dynArray, void* entry, Index* entryID);

/**
 * @brief Removes an entry from the dynamic array with index elID
 *
 * @param[in, out] dynArray Untyped Dynamic Array
 * @param[in] entryID the ID of the entry to be deleted
 * @return Error handling code - EXIP_NULL_POINTER_REF if dArray is NULL; EXIP_OUT_OF_BOUND_BUFFER if the index is
 * bigger than the array size or the array size is 0
 */
errorCode delDynEntry(DynArray* dynArray, Index entryID);

/**
 * @brief Destroy a Dynamic Array
 * This operation frees the allocated memory
 * @param[in, out] dynArray Untyped Dynamic Array to be freed
 */
void destroyDynArray(DynArray* dynArray);

#endif /* DYNAMICARRAY_H_ */
