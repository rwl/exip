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
 * @file sTables.c
 * @brief Implementation of functions describing EXI sting tables operations
 * @date Sep 21, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "../include/sTables.h"

/********* BEGIN: String table default entries ***************/

#define URI_1 "http://www.w3.org/XML/1998/namespace"
#define URI_2 "http://www.w3.org/2001/XMLSchema-instance"
#define URI_3 "http://www.w3.org/2001/XMLSchema"

#define URI_1_PREFIX "xml"
#define URI_2_PREFIX "xsi"

#define URI_1_LOCALNAME_SIZE 4
const char* URI_1_LOCALNAME[] = {"base", "id", "lang", "space"};

#define URI_2_LOCALNAME_SIZE 2
const char* URI_2_LOCALNAME[] = {"nil", "type"};

/* ONLY USED WHEN SCHEMA IS DEFINED.
 * Make it conditional and document it*/
#define URI_3_LOCALNAME_SIZE 46 // #DOCUMENT#
const char* URI_3_LOCALNAME[] = {  // #DOCUMENT#
			"ENTITIES",
			"ENTITY",
			"ID",
			"IDREF",
			"IDREFS",
			"NCName",
			"NMTOKEN",
			"NMTOKENS",
			"NOTATION",
			"Name",
			"QName",
			"anySimpleType",
			"anyType",
			"anyURI",
			"base64Binary",
			"boolean",
			"byte",
			"date",
			"dateTime",
			"decimal",
			"double",
			"duration",
			"float",
			"gDay",
			"gMonth",
			"gMonthDay",
			"gYear",
			"gYearMonth",
			"hexBinary",
			"int",
			"integer",
			"language",
			"long",
			"negativeInteger",
			"nonNegativeInteger",
			"nonPositiveInteger",
			"normalizedString",
			"positiveInteger",
			"short",
			"string",
			"time",
			"token",
			"unsignedByte",
			"unsignedInt",
			"unsignedLong",
			"unsignedShort"
	};

/********* END: String table default entries ***************/

errorCode createValueTable(ValueTable** vTable)
{
	*vTable = (ValueTable*) memManagedAllocate(sizeof(ValueTable));
	if(*vTable == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*vTable)->rows = (struct ValueRow*) memManagedAllocatePtr(sizeof(struct ValueRow)*DEFAULT_VALUE_ROWS_NUMBER, &((*vTable)->memNode));
	if((*vTable)->rows == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*vTable)->arrayDimension = DEFAULT_VALUE_ROWS_NUMBER;
	(*vTable)->rowCount = 0;
	return ERR_OK;
}

errorCode createURITable(URITable** uTable)
{
	*uTable = (URITable*) memManagedAllocate(sizeof(URITable));
	if(*uTable == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*uTable)->rows = (struct URIRow*) memManagedAllocatePtr(sizeof(struct URIRow)*DEFAULT_URI_ROWS_NUMBER, &((*uTable)->memNode));
	if((*uTable)->rows == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*uTable)->arrayDimension = DEFAULT_URI_ROWS_NUMBER;
	(*uTable)->rowCount = 0;
	return ERR_OK;
}

errorCode createPrefixTable(PrefixTable** pTable)
{
	(*pTable) = (PrefixTable*) memManagedAllocate(sizeof(PrefixTable));
	if(pTable == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*pTable)->rows = (struct PrefixRow*) memManagedAllocatePtr(sizeof(struct PrefixRow)*DEFAULT_PREFIX_ROWS_NUMBER, &((*pTable)->memNode));
	if((*pTable)->rows == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*pTable)->arrayDimension = DEFAULT_PREFIX_ROWS_NUMBER;
	(*pTable)->rowCount = 0;
	return ERR_OK;
}

errorCode createLocalNamesTable(LocalNamesTable** lTable)
{
	*lTable = (LocalNamesTable*) memManagedAllocate(sizeof(LocalNamesTable));
	if(*lTable == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*lTable)->rows = (struct LocalNamesRow*) memManagedAllocatePtr(sizeof(struct LocalNamesRow)*DEFAULT_LOCALNAMES_ROWS_NUMBER, &((*lTable)->memNode));
	if((*lTable)->rows == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*lTable)->arrayDimension = DEFAULT_LOCALNAMES_ROWS_NUMBER;
	(*lTable)->rowCount = 0;
	return ERR_OK;
}

errorCode createValueLocalCrossTable(ValueLocalCrossTable** vlTable)
{
	*vlTable = (ValueLocalCrossTable*) memManagedAllocate(sizeof(ValueLocalCrossTable));
	if(*vlTable == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*vlTable)->valueRowIds = (uint32_t*) memManagedAllocatePtr(sizeof(uint32_t)*DEFAULT_VALUE_LOCAL_CROSS_ROWS_NUMBER, &((*vlTable)->memNode));
	if((*vlTable)->valueRowIds == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*vlTable)->arrayDimension = DEFAULT_VALUE_LOCAL_CROSS_ROWS_NUMBER;
	(*vlTable)->rowCount = 0;
	return ERR_OK;
}

errorCode addURIRow(URITable* uTable, StringType uri, uint32_t* rowID)
{
	if(uTable == NULL)
		return NULL_POINTER_REF;
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	if(uTable->arrayDimension == uTable->rowCount)   // The dynamic array must be extended first
	{
		tmp_err_code = memManagedReAllocate(&uTable->rows, sizeof(struct URIRow)*(uTable->rowCount + DEFAULT_URI_ROWS_NUMBER), uTable->memNode);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		uTable->arrayDimension = uTable->arrayDimension + DEFAULT_URI_ROWS_NUMBER;
	}
	uTable->rows[uTable->rowCount].string_val.length = uri.length;
	uTable->rows[uTable->rowCount].string_val.str = uri.str;

	tmp_err_code = createLocalNamesTable(&(uTable->rows[uTable->rowCount].lTable));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createPrefixTable(&(uTable->rows[uTable->rowCount].pTable));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	*rowID = uTable->rowCount;

	uTable->rowCount += 1;
	return ERR_OK;
}

errorCode addLNRow(LocalNamesTable* lTable, StringType local_name, uint32_t* rowID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	if(lTable->arrayDimension == lTable->rowCount)   // The dynamic array must be extended first
	{
		tmp_err_code = memManagedReAllocate(&lTable->rows, sizeof(struct LocalNamesRow)*(lTable->rowCount + DEFAULT_LOCALNAMES_ROWS_NUMBER), lTable->memNode);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		lTable->arrayDimension = lTable->arrayDimension + DEFAULT_LOCALNAMES_ROWS_NUMBER;
	}

	lTable->rows[lTable->rowCount].string_val.length = local_name.length;
	lTable->rows[lTable->rowCount].string_val.str = local_name.str;

	/* Additions to value cross table are done when a value is inserted
	 * in the value string table*/
	lTable->rows[lTable->rowCount].vCrossTable = NULL;

	*rowID = lTable->rowCount;
	lTable->rowCount += 1;
	return ERR_OK;
}

errorCode createInitialStringTables(EXIStream* strm)
{
	unsigned int i = 0;
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	tmp_err_code = createValueTable(&(strm->vTable));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createURITable(&(strm->uriTable));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

    // Insert initial entries in the URI partition
	StringType emptyStr;
	tmp_err_code = getEmptyString(&emptyStr);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	StringType tmp_str;

	/**** URI	0	"" [empty string] */
	strm->uriTable->rows[0].string_val.length = emptyStr.length;
	strm->uriTable->rows[0].string_val.str = emptyStr.str;

	tmp_err_code = createPrefixTable(&(strm->uriTable->rows[0].pTable));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	strm->uriTable->rows[0].pTable->rows[0].string_val.length = emptyStr.length;
	strm->uriTable->rows[0].pTable->rows[0].string_val.str = emptyStr.str;
	strm->uriTable->rows[0].pTable->rowCount = 1;

	strm->uriTable->rows[0].lTable = NULL; // There are no initial entries here

	strm->uriTable->rowCount += 1;

	/**** URI	1	"http://www.w3.org/XML/1998/namespace" */

	tmp_err_code = asciiToString(URI_1, &tmp_str);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	strm->uriTable->rows[1].string_val.str = tmp_str.str;
	strm->uriTable->rows[1].string_val.length = tmp_str.length;

	tmp_err_code = createPrefixTable(&(strm->uriTable->rows[1].pTable));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = asciiToString(URI_1_PREFIX, &tmp_str);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	strm->uriTable->rows[1].pTable->rows[0].string_val.length = tmp_str.length;
	strm->uriTable->rows[1].pTable->rows[0].string_val.str = tmp_str.str;
	strm->uriTable->rows[1].pTable->rowCount = 1;

	tmp_err_code = createLocalNamesTable(&(strm->uriTable->rows[1].lTable));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	for(i = 0; i < URI_1_LOCALNAME_SIZE; i++)
	{
		tmp_err_code = asciiToString(URI_1_LOCALNAME[i], &tmp_str);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		strm->uriTable->rows[1].lTable->rows[i].string_val.length = tmp_str.length;
		strm->uriTable->rows[1].lTable->rows[i].string_val.str = tmp_str.str;
		strm->uriTable->rows[1].lTable->rows[i].vCrossTable = NULL;

		strm->uriTable->rows[1].lTable->rowCount += 1;
	}

	strm->uriTable->rowCount += 1;

	/**** URI	2	"http://www.w3.org/2001/XMLSchema-instance" */

	tmp_err_code = asciiToString(URI_2, &tmp_str);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	strm->uriTable->rows[2].string_val.str = tmp_str.str;
	strm->uriTable->rows[2].string_val.length = tmp_str.length;

	tmp_err_code = createPrefixTable(&(strm->uriTable->rows[2].pTable));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = asciiToString(URI_2_PREFIX, &tmp_str);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	strm->uriTable->rows[2].pTable->rows[0].string_val.length = tmp_str.length;
	strm->uriTable->rows[2].pTable->rows[0].string_val.str = tmp_str.str;
	strm->uriTable->rows[2].pTable->rowCount = 1;

	tmp_err_code = createLocalNamesTable(&(strm->uriTable->rows[2].lTable));
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	for(i = 0; i < URI_2_LOCALNAME_SIZE; i++)
	{
		tmp_err_code = asciiToString(URI_2_LOCALNAME[i], &tmp_str);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		strm->uriTable->rows[2].lTable->rows[i].string_val.length = tmp_str.length;
		strm->uriTable->rows[2].lTable->rows[i].string_val.str = tmp_str.str;
		strm->uriTable->rows[2].lTable->rows[i].vCrossTable = NULL;

		strm->uriTable->rows[2].lTable->rowCount += 1;
	}

	strm->uriTable->rowCount += 1;

	/**** URI	3	"http://www.w3.org/2001/XMLSchema"  */
	if(strm->opts->schemaID != NULL)
	{
		tmp_err_code = asciiToString(URI_3, &tmp_str);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		strm->uriTable->rows[3].string_val.str = tmp_str.str;
		strm->uriTable->rows[3].string_val.length = tmp_str.length;

		strm->uriTable->rows[3].pTable = NULL;

		tmp_err_code = createLocalNamesTable(&(strm->uriTable->rows[3].lTable));
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		for(i = 0; i < URI_3_LOCALNAME_SIZE; i++)
		{
			tmp_err_code = asciiToString(URI_3_LOCALNAME[i], &tmp_str);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			strm->uriTable->rows[3].lTable->rows[i].string_val.length = tmp_str.length;
			strm->uriTable->rows[3].lTable->rows[i].string_val.str = tmp_str.str;
			strm->uriTable->rows[3].lTable->rows[i].vCrossTable = NULL;

			strm->uriTable->rows[3].lTable->rowCount += 1;
		}

		strm->uriTable->rowCount += 1;
	}

	return ERR_OK;
}

errorCode addGVRow(ValueTable* vTable, StringType global_value, uint32_t* rowID)
{
	if(vTable->arrayDimension == vTable->rowCount)   // The dynamic array must be extended first
	{
		errorCode tmp_err_code = UNEXPECTED_ERROR;
		tmp_err_code = memManagedReAllocate(&vTable->rows, sizeof(struct ValueRow)*(vTable->rowCount + DEFAULT_VALUE_ROWS_NUMBER), vTable->memNode);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		vTable->arrayDimension = vTable->arrayDimension + DEFAULT_VALUE_ROWS_NUMBER;
	}

	vTable->rows[vTable->rowCount].string_val.length = global_value.length;
	vTable->rows[vTable->rowCount].string_val.str = global_value.str;

	*rowID = vTable->rowCount;
	vTable->rowCount += 1;
	return ERR_OK;
}

errorCode addLVRow(struct LocalNamesRow* lnRow, uint32_t globalValueRowID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	if(lnRow->vCrossTable == NULL)
	{
		tmp_err_code = createValueLocalCrossTable(&(lnRow->vCrossTable));
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else if(lnRow->vCrossTable->rowCount == lnRow->vCrossTable->arrayDimension)   // The dynamic array must be extended first
	{
		tmp_err_code = memManagedReAllocate(&lnRow->vCrossTable->valueRowIds, sizeof(uint32_t)*(lnRow->vCrossTable->rowCount + DEFAULT_VALUE_LOCAL_CROSS_ROWS_NUMBER), lnRow->vCrossTable->memNode);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		lnRow->vCrossTable->arrayDimension += DEFAULT_VALUE_LOCAL_CROSS_ROWS_NUMBER;
	}

	lnRow->vCrossTable->valueRowIds[lnRow->vCrossTable->rowCount] = globalValueRowID;
	lnRow->vCrossTable->rowCount += 1;
	return ERR_OK;
}

char lookupURI(URITable* uTable, StringType value, uint32_t* rowID)
{
	if(uTable == NULL)
			return 0;
	uint32_t i = 0;
	for(; i < uTable->rowCount; i++)
	{
		if(str_equal(uTable->rows[i].string_val, value))
		{
			*rowID = i;
			return 1;
		}
	}
	return 0;
}

char lookupLN(LocalNamesTable* lTable, StringType value, uint32_t* rowID)
{
	if(lTable == NULL)
		return 0;
	uint32_t i = 0;
	for(; i < lTable->rowCount; i++)
	{
		if(str_equal(lTable->rows[i].string_val, value))
		{
			*rowID = i;
			return 1;
		}
	}
	return 0;
}
