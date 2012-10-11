/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file sTables.c
 * @brief Implementation of functions describing EXI sting tables operations
 * @date Sep 21, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "sTables.h"
#include "stringManipulate.h"
#include "memManagement.h"
#include "hashtable.h"
#include "dynamicArray.h"

/********* BEGIN: String table default entries ***************/

static const char URI_1[] = "http://www.w3.org/XML/1998/namespace";
static const char URI_2[] = XML_SCHEMA_INSTANCE;
static const char URI_3[] = XML_SCHEMA_NAMESPACE;

static const char URI_1_PFX[] = "xml";
static const char URI_2_PFX[] = "xsi";

#define URI_1_LN_SIZE 4
static const char* URI_1_LN[] = {"base", "id", "lang", "space"};

#define URI_2_LN_SIZE 2
static const char* URI_2_LN[] = {"nil", "type"};

/* ONLY USED WHEN SCHEMA IS DEFINED.
 * Make it conditional and document it*/
#define URI_3_LN_SIZE 46 // #DOCUMENT#
static const char* URI_3_LN[] = {  // #DOCUMENT#
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

errorCode createValueTable(ValueTable* valueTable, AllocList* memList)
{
	errorCode tmp_err_code;

	tmp_err_code = createDynArray(&valueTable->dynArray, sizeof(ValueEntry), DEFAULT_VALUE_ENTRIES_NUMBER, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	valueTable->globalId = 0;
#if HASH_TABLE_USE == ON
	valueTable->hashTbl = NULL;
#endif
	return ERR_OK;
}

errorCode createPfxTable(PfxTable** pfxTable, AllocList* memList)
{
	// Due to the small size of the prefix table, there is no need to 
	// use a DynArray
	(*pfxTable) = (PfxTable*) memManagedAllocate(memList, sizeof(PfxTable));
	if(*pfxTable == NULL)
		return MEMORY_ALLOCATION_ERROR;

	(*pfxTable)->count = 0;
	return ERR_OK;
}

errorCode addUriEntry(UriTable* uriTable, String uriStr, SmallIndex* uriEntryId, AllocList* memList)
{
	errorCode tmp_err_code;
	UriEntry* uriEntry;
	Index uriLEntryId;

	tmp_err_code = addEmptyDynEntry(&uriTable->dynArray, (void**)&uriEntry, &uriLEntryId, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Fill in URI entry
	uriEntry->uriStr = uriStr;
	// Prefix table is created independently
	uriEntry->pfxTable = NULL;
	// Create local names table for this URI
	// TODO RCC 20120201: Should this be separate (empty string URI has no local names)?
	tmp_err_code = createDynArray(&uriEntry->lnTable.dynArray, sizeof(LnEntry), DEFAULT_LN_ENTRIES_NUMBER, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	*uriEntryId = (SmallIndex)uriLEntryId;
	return ERR_OK;
}

errorCode addLnEntry(LnTable* lnTable, String lnStr, Index* lnEntryId, AllocList* memList)
{
	errorCode tmp_err_code;
	LnEntry* lnEntry;

	tmp_err_code = addEmptyDynEntry(&lnTable->dynArray, (void**)&lnEntry, lnEntryId, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Fill in local names entry
	lnEntry->lnStr = lnStr;
	lnEntry->elemGrammar = INDEX_MAX;
	lnEntry->typeGrammar = INDEX_MAX;
	// Additions to value cross table are done when a value is inserted in the value table
	lnEntry->vxTable.vx = NULL;
	lnEntry->vxTable.count = 0;

	return ERR_OK;
}

errorCode addValueEntry(EXIStream* strm, String valueStr, QNameID qnameID)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	VxEntry* vxEntry = NULL;
	ValueEntry* valueEntry = NULL;
	Index vxEntryId;
	Index valueEntryId;
	struct LnEntry* lnEntry;

	// Find the local name entry from QNameID
	lnEntry = &GET_LN_URI_QNAME(strm->schema->uriTable, qnameID);

	// Add entry to the local name entry's value cross table (vxTable)
	if(lnEntry->vxTable.vx == NULL)
	{
		// First value entry - create the vxTable
		tmp_err_code = createDynArray(&lnEntry->vxTable.dynArray, sizeof(VxEntry), DEFAULT_VX_ENTRIES_NUMBER, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	// Add an entry - will fill in later
	tmp_err_code = addEmptyDynEntry(&lnEntry->vxTable.dynArray, (void**)&vxEntry, &vxEntryId, &strm->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// If the global ID is less than the actual array size, we must have wrapped around
	// In this case, we must reuse an existing entry
	if(strm->valueTable.globalId < strm->valueTable.count)
	{
		// Get the existing value entry
		valueEntry = &strm->valueTable.value[strm->valueTable.globalId];

		// Null out the existing cross table entry
		GET_LN_URI_QNAME(strm->schema->uriTable, valueEntry->locValuePartition.forQNameId).vxTable.vx[valueEntry->locValuePartition.vxEntryId].globalId = INDEX_MAX;

#if HASH_TABLE_USE == ON
		// Remove existing value string from hash table (if present)
		if(strm->valueTable.hashTbl != NULL)
		{
			hashtable_remove(strm->valueTable.hashTbl, valueEntry->valueStr);
		}
#endif
		// Free the memory allocated by the previous string entry
		EXIP_MFREE(valueEntry->valueStr.str);
	}
	else
	{
		// We are filling up the array and have not wrapped round yet
		// See http://www.w3.org/TR/exi/#encodingOptimizedForMisses
		tmp_err_code = addEmptyDynEntry(&strm->valueTable.dynArray, (void**)&valueEntry, &valueEntryId, &strm->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

	}

	// Set the global ID in the value cross table entry
	vxEntry->globalId = strm->valueTable.globalId;

	// Set the value entry fields
	valueEntry->valueStr = valueStr;
	valueEntry->locValuePartition.forQNameId = qnameID;
	valueEntry->locValuePartition.vxEntryId = vxEntryId;

#if HASH_TABLE_USE == ON
	// Add value string to hash table (if present)
	if(strm->valueTable.hashTbl != NULL)
	{
		tmp_err_code = hashtable_insert(strm->valueTable.hashTbl, valueStr, strm->valueTable.globalId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
#endif

	// Increment global ID
	strm->valueTable.globalId++;

	// The value table is limited by valuePartitionCapacity. If we have exceeded, we wrap around
	// to the beginning of the value table and null out existing IDs in the corresponding
	// cross table IDs
	if(strm->valueTable.globalId == strm->header.opts.valuePartitionCapacity)
		strm->valueTable.globalId = 0;

	return ERR_OK;
}

errorCode addPfxEntry(PfxTable* pfxTable, String pfxStr, SmallIndex* pfxEntryId)
{
	if(pfxTable->count >= MAXIMUM_NUMBER_OF_PREFIXES_PER_URI)
		return TOO_MUCH_PREFIXES_PER_URI;

	pfxTable->pfxStr[pfxTable->count].length = pfxStr.length;
	pfxTable->pfxStr[pfxTable->count].str = pfxStr.str;
	*pfxEntryId = pfxTable->count++;

	return ERR_OK;
}

errorCode createUriTableEntry(UriTable* uriTable, const char* uri, int createPfx, const char* pfx, const char** lnBase, Index lnSize, AllocList* memList)
{
	errorCode tmp_err_code;
	Index i;
	SmallIndex pfxEntryId;
	SmallIndex uriEntryId;
	Index lnEntryId;
	String emptyStr;
	String tmpStr;
	UriEntry* uriEntry;

	// Insert initial entries in the URI partition
	getEmptyString(&emptyStr);

	if(uri != NULL)
	{
		// Convert char* to String
		tmp_err_code = asciiToString(uri, &tmpStr, memList, FALSE);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
	{
		tmpStr = emptyStr;
	}

	// Add resulting String to URI table (creates lnTable as well)
	tmp_err_code = addUriEntry(uriTable, tmpStr, &uriEntryId, memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Get ptr. to URI Entry
	uriEntry = &uriTable->uri[uriEntryId];

	if(createPfx)
	{
		if(pfx != NULL)
		{
			tmp_err_code = asciiToString(pfx, &tmpStr, memList, FALSE);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
		{
			tmpStr = emptyStr;
		}

		// Create the URI's prefix table and add the default prefix
		tmp_err_code = createPfxTable(&uriEntry->pfxTable, memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addPfxEntry(uriEntry->pfxTable, tmpStr, &pfxEntryId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	for(i = 0; i < lnSize; i++)
	{
		tmp_err_code = asciiToString(lnBase[i], &tmpStr, memList, FALSE);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addLnEntry(&uriEntry->lnTable, tmpStr, &lnEntryId, memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	return ERR_OK;
}

errorCode createUriTableEntries(UriTable* uriTable, unsigned char withSchema, AllocList* memList)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	// See http://www.w3.org/TR/exi/#initialUriValues

	// URI 0: "" (empty string)
	tmp_err_code = createUriTableEntry(uriTable,
									   NULL,   // Namespace - empty string
									   TRUE, // Create prefix entry
									   NULL,   // Prefix entry - empty string
									   NULL, // No local names
									   0,
									   memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// URI 1: "http://www.w3.org/XML/1998/namespace"
	tmp_err_code = createUriTableEntry(uriTable,
									   URI_1,     // URI: "http://www.w3.org/XML/1998/namespace"
									   TRUE,      // Create prefix entry
									   URI_1_PFX, // Prefix: "xml"
									   URI_1_LN,  // Add local names
									   URI_1_LN_SIZE,
									   memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// URI 2: "http://www.w3.org/2001/XMLSchema-instance"
	tmp_err_code = createUriTableEntry(uriTable,
									   URI_2,     // URI: "http://www.w3.org/2001/XMLSchema-instance"
									   TRUE,		// Create prefix entry
									   URI_2_PFX, // Prefix: "xsi"
									   URI_2_LN,  // Add local names
									   URI_2_LN_SIZE,
									   memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(withSchema == TRUE)
	{
		// URI 3: "http://www.w3.org/2001/XMLSchema"
		tmp_err_code = createUriTableEntry(uriTable,
										   URI_3,    // URI: "http://www.w3.org/2001/XMLSchema"
										   FALSE,    // No prefix entry (see http://www.w3.org/TR/exi/#initialPrefixValues) 
										   NULL,     // (no prefix)
										   URI_3_LN, // Add local names
										   URI_3_LN_SIZE,
										   memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

char lookupUri(UriTable* uriTable, String uriStr, SmallIndex* uriEntryId)
{
	SmallIndex i;

	if(uriTable == NULL)
		return 0;

	for(i = 0; i < uriTable->count; i++)
	{
		if(stringEqual(uriTable->uri[i].uriStr, uriStr))
		{
			*uriEntryId = i;
			return 1;
		}
	}
	return 0;
}

char lookupLn(LnTable* lnTable, String lnStr, Index* lnEntryId)
{
	Index i;

	if(lnTable == NULL)
		return 0;
	for(i = 0; i < lnTable->count; i++)
	{
		if(stringEqual(lnTable->ln[i].lnStr, lnStr))
		{
			*lnEntryId = i;
			return 1;
		}
	}
	return 0;
}

char lookupPfx(PfxTable* pfxTable, String pfxStr, SmallIndex* pfxEntryId)
{
	SmallIndex i;

	if(pfxTable == NULL)
		return 0;

	for(i = 0; i < pfxTable->count; i++)
	{
		if(stringEqual(pfxTable->pfxStr[i], pfxStr))
		{
			*pfxEntryId = i;
			return 1;
		}
	}
	return 0;
}

char lookupVx(ValueTable* valueTable, VxTable* vxTable, String valueStr, Index* vxEntryId)
{
	Index i;
	VxEntry* vxEntry;
	ValueEntry* valueEntry;

	if((vxTable == NULL) || (vxTable->vx == NULL))
		return 0;

	for(i = 0; i < vxTable->count; i++)
	{
		vxEntry = vxTable->vx + i;
		if(vxEntry->globalId == INDEX_MAX) // The value was removed from the local value partition
			continue;
		valueEntry = valueTable->value + vxEntry->globalId;
		if(stringEqual(valueEntry->valueStr, valueStr))
		{
			*vxEntryId = i;
			return 1;
		}
	}
	return 0;
}

char lookupValue(ValueTable* valueTable, String valueStr, Index* valueEntryId)
{
	Index i;
	ValueEntry* valueEntry;

	if(valueTable == NULL)
		return 0;

#if HASH_TABLE_USE == ON
	if(valueTable->hashTbl != NULL)
	{
		// Use hash table search
		i = hashtable_search(valueTable->hashTbl, valueStr);
		if(i != INDEX_MAX)
		{
			*valueEntryId = i;
			return 1;
		}
	}
	else
#endif
	{
		// No hash table - linear search
		for(i = 0; i < valueTable->count; i++)
		{
			valueEntry = valueTable->value + i;
			if(stringEqual(valueEntry->valueStr, valueStr))
			{
				*valueEntryId = i;
				return 1;
			}
		}
	}

	return 0;
}
