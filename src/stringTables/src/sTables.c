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

#include "sTables.h"
#include "stringManipulate.h"
#include "memManagement.h"
#include "hashtable.h"

/********* BEGIN: String table default entries ***************/

static const char URI_1[] = "http://www.w3.org/XML/1998/namespace";
static const char URI_2[] = "http://www.w3.org/2001/XMLSchema-instance";
static const char URI_3[] = "http://www.w3.org/2001/XMLSchema";

static const char URI_1_PREFIX[] = "xml";
static const char URI_2_PREFIX[] = "xsi";

#define URI_1_LOCALNAME_SIZE 4
static const char* URI_1_LOCALNAME[] = {"base", "id", "lang", "space"};

#define URI_2_LOCALNAME_SIZE 2
static const char* URI_2_LOCALNAME[] = {"nil", "type"};

/* ONLY USED WHEN SCHEMA IS DEFINED.
 * Make it conditional and document it*/
#define URI_3_LOCALNAME_SIZE 46 // #DOCUMENT#
static const char* URI_3_LOCALNAME[] = {  // #DOCUMENT#
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

errorCode createValueTable(ValueTable** vTable, AllocList* memList)
{
	*vTable = (ValueTable*) memManagedAllocate(memList, sizeof(ValueTable));
	if(*vTable == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*vTable)->rows = (struct ValueRow*) memManagedAllocatePtr(memList, sizeof(struct ValueRow)*DEFAULT_VALUE_ROWS_NUMBER, &(*vTable)->memPair);
	if((*vTable)->rows == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*vTable)->arrayDimension = DEFAULT_VALUE_ROWS_NUMBER;
	(*vTable)->rowCount = 0;
	(*vTable)->globalID = 0;
	(*vTable)->hashTbl = NULL;

	return ERR_OK;
}

errorCode createURITable(URITable** uTable, AllocList* memList)
{
	*uTable = (URITable*) memManagedAllocate(memList, sizeof(URITable));
	if(*uTable == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*uTable)->rows = (struct URIRow*) memManagedAllocatePtr(memList, sizeof(struct URIRow)*DEFAULT_URI_ROWS_NUMBER, &(*uTable)->memPair);
	if((*uTable)->rows == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*uTable)->arrayDimension = DEFAULT_URI_ROWS_NUMBER;
	(*uTable)->rowCount = 0;
	return ERR_OK;
}

errorCode createPrefixTable(PrefixTable** pTable, AllocList* memList)
{
	(*pTable) = (PrefixTable*) memManagedAllocate(memList, sizeof(PrefixTable));
	if(*pTable == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*pTable)->rowCount = 0;
	return ERR_OK;
}

errorCode createLocalNamesTable(LocalNamesTable** lTable, AllocList* memList)
{
	*lTable = (LocalNamesTable*) memManagedAllocate(memList, sizeof(LocalNamesTable));
	if(*lTable == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*lTable)->rows = (struct LocalNamesRow*) memManagedAllocatePtr(memList, sizeof(struct LocalNamesRow)*DEFAULT_LOCALNAMES_ROWS_NUMBER, &(*lTable)->memPair);
	if((*lTable)->rows == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*lTable)->arrayDimension = DEFAULT_LOCALNAMES_ROWS_NUMBER;
	(*lTable)->rowCount = 0;
	return ERR_OK;
}

errorCode createValueLocalCrossTable(ValueLocalCrossTable** vlTable, AllocList* memList)
{
	*vlTable = (ValueLocalCrossTable*) memManagedAllocate(memList, sizeof(ValueLocalCrossTable));
	if(*vlTable == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*vlTable)->valueRowIds = (size_t*) memManagedAllocatePtr(memList, sizeof(size_t)*DEFAULT_VALUE_LOCAL_CROSS_ROWS_NUMBER, &(*vlTable)->memPair);
	if((*vlTable)->valueRowIds == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*vlTable)->arrayDimension = DEFAULT_VALUE_LOCAL_CROSS_ROWS_NUMBER;
	(*vlTable)->rowCount = 0;
	return ERR_OK;
}

errorCode addURIRow(URITable* uTable, String uri, uint16_t* rowID, AllocList* memList)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	if(uTable == NULL)
		return NULL_POINTER_REF;

	if(uTable->arrayDimension == uTable->rowCount)   // The dynamic array must be extended first
	{
		tmp_err_code = memManagedReAllocate((void **) &uTable->rows, sizeof(struct URIRow)*(uTable->rowCount + DEFAULT_URI_ROWS_NUMBER), uTable->memPair);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		uTable->arrayDimension = uTable->arrayDimension + DEFAULT_URI_ROWS_NUMBER;
	}
	uTable->rows[uTable->rowCount].string_val.length = uri.length;
	uTable->rows[uTable->rowCount].string_val.str = uri.str;

	tmp_err_code = createLocalNamesTable(&(uTable->rows[uTable->rowCount].lTable), memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	uTable->rows[uTable->rowCount].pTable = NULL;

	*rowID = uTable->rowCount;

	uTable->rowCount += 1;
	return ERR_OK;
}

errorCode addLNRow(LocalNamesTable* lTable, String local_name, size_t* rowID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	if(lTable->arrayDimension == lTable->rowCount)   // The dynamic array must be extended first
	{
		tmp_err_code = memManagedReAllocate((void **) &lTable->rows, sizeof(struct LocalNamesRow)*(lTable->rowCount + DEFAULT_LOCALNAMES_ROWS_NUMBER), lTable->memPair);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		lTable->arrayDimension = lTable->arrayDimension + DEFAULT_LOCALNAMES_ROWS_NUMBER;
	}

	lTable->rows[lTable->rowCount].string_val.length = local_name.length;
	lTable->rows[lTable->rowCount].string_val.str = local_name.str;
	lTable->rows[lTable->rowCount].typeGrammar = NULL;
	lTable->rows[lTable->rowCount].typeEmptyGrammar = NULL;

	/* Additions to value cross table are done when a value is inserted
	 * in the value string table*/
	lTable->rows[lTable->rowCount].vCrossTable = NULL;

	*rowID = lTable->rowCount;
	lTable->rowCount += 1;
	return ERR_OK;
}

errorCode createInitialStringTables(EXIStream* strm)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	tmp_err_code = createValueTable(&(strm->vTable), &strm->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createURITable(&(strm->uriTable), &strm->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

    return createInitialEntries(&(strm->memList), strm->uriTable, FALSE);
}

errorCode createInitialEntries(AllocList* memList, URITable* uTable, unsigned char withSchema)
{
	unsigned int i = 0;
	uint16_t uriID = 0;
	size_t lnID = 0;
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	String emptyStr;
	String tmp_str;
	unsigned int prfxID;

	// Insert initial entries in the URI partition
	getEmptyString(&emptyStr);

	/**** URI	0	"" [empty string] */
	tmp_err_code = addURIRow(uTable, emptyStr, &uriID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	if(uTable->rows[uriID].pTable == NULL)
	{
		tmp_err_code = createPrefixTable(&uTable->rows[uriID].pTable, memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	tmp_err_code = addPrefixRow(uTable->rows[uriID].pTable, emptyStr, &prfxID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	/**** URI	1	"http://www.w3.org/XML/1998/namespace" */
	tmp_err_code = asciiToString(URI_1, &tmp_str, memList, FALSE);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	tmp_err_code = addURIRow(uTable, tmp_str, &uriID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = asciiToString(URI_1_PREFIX, &tmp_str, memList, FALSE);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	if(uTable->rows[uriID].pTable == NULL)
	{
		tmp_err_code = createPrefixTable(&uTable->rows[uriID].pTable, memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	tmp_err_code = addPrefixRow(uTable->rows[uriID].pTable, tmp_str, &prfxID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	for(i = 0; i < URI_1_LOCALNAME_SIZE; i++)
	{
		tmp_err_code = asciiToString(URI_1_LOCALNAME[i], &tmp_str, memList, FALSE);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addLNRow(uTable->rows[uriID].lTable, tmp_str, &lnID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	/**** URI	2	"http://www.w3.org/2001/XMLSchema-instance" */
	tmp_err_code = asciiToString(URI_2, &tmp_str, memList, FALSE);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	tmp_err_code = addURIRow(uTable, tmp_str, &uriID, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = asciiToString(URI_2_PREFIX, &tmp_str, memList, FALSE);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	if(uTable->rows[uriID].pTable == NULL)
	{
		tmp_err_code = createPrefixTable(&uTable->rows[uriID].pTable, memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	tmp_err_code = addPrefixRow(uTable->rows[uriID].pTable, tmp_str, &prfxID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	for(i = 0; i < URI_2_LOCALNAME_SIZE; i++)
	{
		tmp_err_code = asciiToString(URI_2_LOCALNAME[i], &tmp_str, memList, FALSE);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		addLNRow(uTable->rows[uriID].lTable, tmp_str, &lnID);
	}

	/**** URI	3	"http://www.w3.org/2001/XMLSchema"  */
	if(withSchema == TRUE)
	{
		tmp_err_code = asciiToString(URI_3, &tmp_str, memList, FALSE);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		addURIRow(uTable, tmp_str, &uriID, memList);

		for(i = 0; i < URI_3_LOCALNAME_SIZE; i++)
		{
			tmp_err_code = asciiToString(URI_3_LOCALNAME[i], &tmp_str, memList, FALSE);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			addLNRow(uTable->rows[uriID].lTable, tmp_str, &lnID);
		}
	}

	return ERR_OK;
}

errorCode addValueRows(EXIStream* strm, String* value)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	size_t* vLocalCrossTablePtr = NULL;
	struct LocalNamesRow* lnRow = &(strm->uriTable->rows[strm->context.curr_uriID].lTable->rows[strm->context.curr_lnID]);

	// Add entry to the local value table
	if(lnRow->vCrossTable == NULL)
	{
		tmp_err_code = createValueLocalCrossTable(&(lnRow->vCrossTable), &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else if(lnRow->vCrossTable->rowCount == lnRow->vCrossTable->arrayDimension)   // The dynamic array must be extended first
	{
		tmp_err_code = memManagedReAllocate((void **) &lnRow->vCrossTable->valueRowIds, sizeof(size_t)*(lnRow->vCrossTable->rowCount + DEFAULT_VALUE_LOCAL_CROSS_ROWS_NUMBER), lnRow->vCrossTable->memPair);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		lnRow->vCrossTable->arrayDimension += DEFAULT_VALUE_LOCAL_CROSS_ROWS_NUMBER;
	}

	vLocalCrossTablePtr = &(lnRow->vCrossTable->valueRowIds[lnRow->vCrossTable->rowCount]);
	lnRow->vCrossTable->rowCount += 1;

	// Add entry to the global value table
	if(strm->vTable->arrayDimension == strm->vTable->globalID)   // The dynamic array must be extended first
	{
		tmp_err_code = memManagedReAllocate((void **) &strm->vTable->rows, sizeof(struct ValueRow)*(strm->vTable->rowCount + DEFAULT_VALUE_ROWS_NUMBER), strm->vTable->memPair);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		strm->vTable->arrayDimension += DEFAULT_VALUE_ROWS_NUMBER;
	}

	if(strm->vTable->globalID < strm->vTable->rowCount)
	{
		if(strm->vTable->rows[strm->vTable->globalID].valueLocalCrossTableRowPointer != NULL)
			*(strm->vTable->rows[strm->vTable->globalID].valueLocalCrossTableRowPointer) = SIZE_MAX;

		if(strm->vTable->hashTbl != NULL)
			hashtable_remove(strm->vTable->hashTbl, &strm->vTable->rows[strm->vTable->globalID].string_val);
	}
	else
		strm->vTable->rowCount += 1;

	strm->vTable->rows[strm->vTable->globalID].string_val.length = value->length;
	strm->vTable->rows[strm->vTable->globalID].string_val.str = value->str;
	strm->vTable->rows[strm->vTable->globalID].valueLocalCrossTableRowPointer = vLocalCrossTablePtr;
	*vLocalCrossTablePtr = strm->vTable->globalID;

	if(strm->vTable->hashTbl != NULL)
	{
		tmp_err_code = hashtable_insert(strm->vTable->hashTbl, value, strm->vTable->globalID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	strm->vTable->globalID += 1;

	if(strm->vTable->globalID == strm->header.opts.valuePartitionCapacity)
		strm->vTable->globalID = 0;

	return ERR_OK;
}

errorCode addPrefixRow(PrefixTable* pTable, String px_value, unsigned int* prfxID)
{
	if(pTable->rowCount >= MAXIMUM_NUMBER_OF_PREFIXES_PER_URI)
		return TOO_MUCH_PREFIXES_PER_URI;

	pTable->string_val[pTable->rowCount].length = px_value.length;
	pTable->string_val[pTable->rowCount].str = px_value.str;
	*prfxID = pTable->rowCount;
	pTable->rowCount += 1;

	return ERR_OK;
}

char lookupURI(URITable* uTable, String value, uint16_t* rowID)
{
	uint16_t i = 0;
	if(uTable == NULL)
			return 0;
	for(i = 0; i < uTable->rowCount; i++)
	{
		if(stringEqual(uTable->rows[i].string_val, value))
		{
			*rowID = i;
			return 1;
		}
	}
	return 0;
}

char lookupLN(LocalNamesTable* lTable, String value, size_t* rowID)
{
	size_t i = 0;
	if(lTable == NULL)
		return 0;
	for(i = 0; i < lTable->rowCount; i++)
	{
		if(stringEqual(lTable->rows[i].string_val, value))
		{
			*rowID = i;
			return 1;
		}
	}
	return 0;
}

char lookupPrefix(PrefixTable* pTable, String value, size_t* rowID)
{
	unsigned int i = 0;
	if(pTable == NULL)
		return 0;
	for(i = 0; i < pTable->rowCount; i++)
	{
		if(stringEqual(pTable->string_val[i], value))
		{
			*rowID = i;
			return 1;
		}
	}
	return 0;
}

char lookupLV(ValueTable* vTable, ValueLocalCrossTable* lvTable, String value, uint16_t* rowID)
{
	uint16_t i = 0;
	if(lvTable == NULL)
		return 0;
	for(i = 0; i < lvTable->rowCount; i++)
	{
		if(stringEqual(vTable->rows[lvTable->valueRowIds[i]].string_val, value))
		{
			*rowID = i;
			return 1;
		}
	}
	return 0;
}

char lookupVal(ValueTable* vTable, String value, size_t* rowID)
{
	size_t i = 0;
	if(vTable == NULL)
		return 0;

	if(vTable->hashTbl != NULL)
	{
		i = hashtable_search(vTable->hashTbl, &value);
		if(i != SIZE_MAX)
		{
			*rowID = i;
			return 1;
		}
	}
	else
	{
		for(i = 0; i < vTable->rowCount; i++)
		{
			if(stringEqual(vTable->rows[i].string_val, value))
			{
				*rowID = i;
				return 1;
			}
		}
	}

	return 0;
}
