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
 * @file dynamicArray.c
 * @brief Implementation for untyped dynamic array
 *
 * @date Jan 25, 2011
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "dynamicArray.h"
#include "memManagement.h"

errorCode createDynArray(DynArray** dArray, size_t elSize, uint16_t defaultSize, EXIStream* strm)
{
	(*dArray) = (DynArray*) memManagedAllocate(strm, sizeof(DynArray));
	if(*dArray == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*dArray)->elements = memManagedAllocatePtr(strm, elSize*defaultSize, &((*dArray)->memNode));
	if((*dArray)->elements == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*dArray)->arrayDimension = defaultSize;
	(*dArray)->elementCount = 0;
	(*dArray)->defaultSize = defaultSize;
	(*dArray)->elSize = elSize;

	return ERR_OK;
}

errorCode addDynElement(DynArray* dArray, void* elem, uint32_t* elID, EXIStream* strm)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	if(dArray == NULL)
		return NULL_POINTER_REF;
	if(dArray->arrayDimension == dArray->elementCount)   // The dynamic array must be extended first
	{
		tmp_err_code = memManagedReAllocate(/*(void **)*/ &dArray->elements, dArray->elSize*(dArray->elementCount + dArray->defaultSize), dArray->memNode);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		dArray->arrayDimension = dArray->arrayDimension + dArray->defaultSize;
	}

	memcpy(((unsigned char *) dArray->elements) + dArray->elementCount*dArray->elSize, elem, dArray->elSize);

	*elID = dArray->elementCount;

	dArray->elementCount += 1;
	return ERR_OK;
}
