/*==================================================================================*\
|                                                                                    |
|                    EXIP - Efficient XML Interchange Processor                      |
|                                                                                    |
|------------------------------------------------------------------------------------|
| Copyright (c) 2010, EISLAB - Luleå University of Technology                        |
| All rights reserved.                                                               |
|                                                                                    |
| Redistribution and use in source and binary forms, with or without                 |
| modification, are permitted provided that the following conditions are met:        |
|     * Redistributions of source code must retain the above copyright               |
|       notice, this list of conditions and the following disclaimer.                |
|     * Redistributions in binary form must reproduce the above copyright            |
|       notice, this list of conditions and the following disclaimer in the          |
|       documentation and/or other materials provided with the distribution.         |
|     * Neither the name of the EISLAB - Luleå University of Technology nor the      |
|       names of its contributors may be used to endorse or promote products         |
|       derived from this software without specific prior written permission.        |
|                                                                                    |
| THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND    |
| ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED      |
| WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE             |
| DISCLAIMED. IN NO EVENT SHALL EISLAB - LULEÅ UNIVERSITY OF TECHNOLOGY BE LIABLE    |
| FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES |
| (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;       |
| LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND        |
| ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT         |
| (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS      |
| SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                       |
|                                                                                    |
|                                                                                    |
|                                                                                    |
\===================================================================================*/

/**
 * @file dynamicArray.h
 * @brief Declarations for untyped dynamic array
 *
 * @date Jan 25, 2011
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */


#ifndef DYNAMICARRAY_H_
#define DYNAMICARRAY_H_

#include "procTypes.h"

struct dynArray {
	void* elements; // Dynamic array elements
	size_t elSize;	// The size of a single array element in bytes
	uint16_t defaultSize; // Initial number of elements and the number of elements to be added each expansion time
	uint32_t elementCount; // The actual number of elements
	uint32_t arrayDimension; // The size of the Dynamic array
	void* memNode; // Used by the memoryManager when there is reallocation
};

typedef struct dynArray DynArray;

/**
 * @brief Creates fresh empty Untyped Dynamic Array
 * This operation includes allocation of memory for DEFAULT_VALUE_ROWS_NUMBER number of value rows
 * @param[out] dArray Untyped Dynamic Array
 * @param[in] elSize The size of a single array element in bytes
 * @param[in] defaultSize Initial number of elements and the number of elements to be added each expansion time
 * @param[in, out] strm EXI stream for which the allocation is made
 * @return Error handling code
 */
errorCode createDynArray(DynArray** dArray, size_t elSize, uint16_t defaultSize, EXIStream* strm);

/**
 * @brief Add new element into the dynamic array
 * NOTE that the new element is shallow copied!
 *
 * @param[in, out] dArray Untyped Dynamic Array
 * @param[in] elem the inserted element
 * @param[out] elID the ID of the element inserted
 * @param[in, out] strm EXI stream for which the allocation is made
 * @return Error handling code
 */
errorCode addDynElement(DynArray* dArray, void* elem, uint32_t* elID, EXIStream* strm);

#endif /* DYNAMICARRAY_H_ */