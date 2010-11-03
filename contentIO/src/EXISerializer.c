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
 * @file EXISerializer.c
 * @brief Implementation of serializer of EXI streams
 *
 * @date Sep 30, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "../include/EXISerializer.h"
#include "grammars.h"

errorCode initStream(EXIStream* strm, unsigned int initialBufSize)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	strm->buffer = (char*) memManagedAllocate(sizeof(char)*initialBufSize);
	if(strm->buffer == NULL)
		return MEMORY_ALLOCATION_ERROR;

	strm->opts = (struct EXIOptions*) memManagedAllocate(sizeof(struct EXIOptions));
	if(strm->opts == NULL)
		return MEMORY_ALLOCATION_ERROR;
	strm->bitPointer = 0;
	strm->bufLen = initialBufSize;
	strm->bufferIndx = 0;
	strm->nonTermID = GR_DOCUMENT;

	strm->gStack = (EXIGrammarStack*) memManagedAllocate(sizeof(EXIGrammarStack));
	if(strm->gStack == NULL)
		return MEMORY_ALLOCATION_ERROR;
	tmp_err_code = getBuildInDocGrammar(strm->gStack, strm->opts);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	strm->gPool = (struct ElementGrammarPool*) memManagedAllocate(sizeof(struct ElementGrammarPool));
	if(strm->gPool == NULL)
		return MEMORY_ALLOCATION_ERROR;
	tmp_err_code = createElementGrammarPool(strm->gPool);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createInitialStringTables(strm);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

errorCode startDocumentSer(EXIStream* strm)
{

	return NOT_IMPLEMENTED_YET;
}

errorCode endDocumentSer(EXIStream* strm)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode startElementSer(EXIStream* strm, QName qname)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode endElementSer(EXIStream* strm)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode attributeSer(EXIStream* strm, QName qname)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode intDataSer(EXIStream* strm, int32_t int_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode bigIntDataSer(EXIStream* strm, const BigSignedInt int_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode booleanDataSer(EXIStream* strm, unsigned char bool_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode stringDataSer(EXIStream* strm, const StringType str_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode floatDataSer(EXIStream* strm, double float_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode bigFloatDataSer(EXIStream* strm, BigFloat float_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode binaryDataSer(EXIStream* strm, const char* binary_val, uint32_t nbytes)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode dateTimeDataSer(EXIStream* strm, struct tm dt_val, uint16_t presenceMask)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode decimalDataSer(EXIStream* strm, decimal dec_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode bigDecimalDataSer(EXIStream* strm, bigDecimal dec_val)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode processingInstructionSer(EXIStream* strm)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode selfContainedSer(EXIStream* strm)
{
	return NOT_IMPLEMENTED_YET;
}
