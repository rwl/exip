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

extern CharType ops_URI_1[];
extern CharType ops_PFX_1_0[];
extern CharType ops_LN_1_0[];
extern CharType ops_LN_1_1[];
extern CharType ops_LN_1_2[];
extern CharType ops_LN_1_3[];
extern CharType ops_URI_2[];
extern CharType ops_PFX_2_0[];
extern CharType ops_LN_2_0[];
extern CharType ops_LN_2_1[];
extern CharType ops_URI_3[];
extern CharType ops_LN_3_0[];
extern CharType ops_LN_3_1[];
extern CharType ops_LN_3_2[];
extern CharType ops_LN_3_3[];
extern CharType ops_LN_3_4[];
extern CharType ops_LN_3_5[];
extern CharType ops_LN_3_6[];
extern CharType ops_LN_3_7[];
extern CharType ops_LN_3_8[];
extern CharType ops_LN_3_9[];
extern CharType ops_LN_3_10[];
extern CharType ops_LN_3_11[];
extern CharType ops_LN_3_12[];
extern CharType ops_LN_3_13[];
extern CharType ops_LN_3_14[];
extern CharType ops_LN_3_15[];
extern CharType ops_LN_3_16[];
extern CharType ops_LN_3_17[];
extern CharType ops_LN_3_18[];
extern CharType ops_LN_3_19[];
extern CharType ops_LN_3_20[];
extern CharType ops_LN_3_21[];
extern CharType ops_LN_3_22[];
extern CharType ops_LN_3_23[];
extern CharType ops_LN_3_24[];
extern CharType ops_LN_3_25[];
extern CharType ops_LN_3_26[];
extern CharType ops_LN_3_27[];
extern CharType ops_LN_3_28[];
extern CharType ops_LN_3_29[];
extern CharType ops_LN_3_30[];
extern CharType ops_LN_3_31[];
extern CharType ops_LN_3_32[];
extern CharType ops_LN_3_33[];
extern CharType ops_LN_3_34[];
extern CharType ops_LN_3_35[];
extern CharType ops_LN_3_36[];
extern CharType ops_LN_3_37[];
extern CharType ops_LN_3_38[];
extern CharType ops_LN_3_39[];
extern CharType ops_LN_3_40[];
extern CharType ops_LN_3_41[];
extern CharType ops_LN_3_42[];
extern CharType ops_LN_3_43[];
extern CharType ops_LN_3_44[];
extern CharType ops_LN_3_45[];

const String XML_NAMESPACE = {ops_URI_1, 36};
const String XML_SCHEMA_INSTANCE = {ops_URI_2, 41};
const String XML_SCHEMA_NAMESPACE = {ops_URI_3, 32};

const String URI_1_PFX = {ops_PFX_1_0, 3};
static const String URI_2_PFX = {ops_PFX_2_0, 3};

#define URI_1_LN_SIZE 4
static const String URI_1_LN[] = {{ops_LN_1_0, 4}, {ops_LN_1_1, 2}, {ops_LN_1_2, 4}, {ops_LN_1_3, 5}};

#define URI_2_LN_SIZE 2
const String URI_2_LN[] = {{ops_LN_2_0, 3}, {ops_LN_2_1, 4}};

#define URI_3_LN_SIZE 46
static const String URI_3_LN[] = {
			{ops_LN_3_0, 8},
			{ops_LN_3_1, 6},
			{ops_LN_3_2, 2},
			{ops_LN_3_3, 5},
			{ops_LN_3_4, 6},
			{ops_LN_3_5, 6},
			{ops_LN_3_6, 7},
			{ops_LN_3_7, 8},
			{ops_LN_3_8, 8},
			{ops_LN_3_9, 4},
			{ops_LN_3_10, 5},
			{ops_LN_3_11, 13},
			{ops_LN_3_12, 7},
			{ops_LN_3_13, 6},
			{ops_LN_3_14, 12},
			{ops_LN_3_15, 7},
			{ops_LN_3_16, 4},
			{ops_LN_3_17, 4},
			{ops_LN_3_18, 8},
			{ops_LN_3_19, 7},
			{ops_LN_3_20, 6},
			{ops_LN_3_21, 8},
			{ops_LN_3_22, 5},
			{ops_LN_3_23, 4},
			{ops_LN_3_24, 6},
			{ops_LN_3_25, 9},
			{ops_LN_3_26, 5},
			{ops_LN_3_27, 10},
			{ops_LN_3_28, 9},
			{ops_LN_3_29, 3},
			{ops_LN_3_30, 7},
			{ops_LN_3_31, 8},
			{ops_LN_3_32, 4},
			{ops_LN_3_33, 15},
			{ops_LN_3_34, 18},
			{ops_LN_3_35, 18},
			{ops_LN_3_36, 16},
			{ops_LN_3_37, 15},
			{ops_LN_3_38, 5},
			{ops_LN_3_39, 6},
			{ops_LN_3_40, 4},
			{ops_LN_3_41, 5},
			{ops_LN_3_42, 12},
			{ops_LN_3_43, 11},
			{ops_LN_3_44, 12},
			{ops_LN_3_45, 13}
	};

/********* END: String table default entries ***************/

errorCode createValueTable(ValueTable* valueTable)
{
	errorCode tmp_err_code;

	TRY(createDynArray(&valueTable->dynArray, sizeof(ValueEntry), DEFAULT_VALUE_ENTRIES_NUMBER));

	valueTable->globalId = 0;
#if HASH_TABLE_USE
	valueTable->hashTbl = NULL;
#endif
	return EXIP_ERR_OK;
}

errorCode createPfxTable(PfxTable** pfxTable)
{
	// Due to the small size of the prefix table, there is no need to 
	// use a DynArray
	(*pfxTable) = (PfxTable*) EXIP_MALLOC(sizeof(PfxTable));
	if(*pfxTable == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;

	(*pfxTable)->count = 0;
	return EXIP_ERR_OK;
}

errorCode addUriEntry(UriTable* uriTable, String uriStr, SmallIndex* uriEntryId)
{
	errorCode tmp_err_code;
	UriEntry* uriEntry;
	Index uriLEntryId;

	TRY(addEmptyDynEntry(&uriTable->dynArray, (void**)&uriEntry, &uriLEntryId));

	// Fill in URI entry
	uriEntry->uriStr = uriStr;
	// Prefix table is created independently
	uriEntry->pfxTable = NULL;
	// Create local names table for this URI
	// TODO RCC 20120201: Should this be separate (empty string URI has no local names)?
	TRY(createDynArray(&uriEntry->lnTable.dynArray, sizeof(LnEntry), DEFAULT_LN_ENTRIES_NUMBER));

	*uriEntryId = (SmallIndex)uriLEntryId;
	return EXIP_ERR_OK;
}

errorCode addLnEntry(LnTable* lnTable, String lnStr, Index* lnEntryId)
{
	errorCode tmp_err_code;
	LnEntry* lnEntry;

	TRY(addEmptyDynEntry(&lnTable->dynArray, (void**)&lnEntry, lnEntryId));

	// Fill in local names entry
	lnEntry->lnStr = lnStr;
	lnEntry->elemGrammar = INDEX_MAX;
	lnEntry->typeGrammar = INDEX_MAX;
#if VALUE_CROSSTABLE_USE
	// The Vx table is created on-demand (additions to value cross table are done when a value is inserted in the value table)
	lnEntry->vxTable = NULL;
#endif
	return EXIP_ERR_OK;
}

errorCode addValueEntry(EXIStream* strm, String valueStr, QNameID qnameID)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	ValueEntry* valueEntry = NULL;
	Index valueEntryId;

#if VALUE_CROSSTABLE_USE
	Index vxEntryId;
	{
		struct LnEntry* lnEntry;
		VxEntry vxEntry;

		// Find the local name entry from QNameID
		lnEntry = &GET_LN_URI_QNAME(strm->schema->uriTable, qnameID);

		// Add entry to the local name entry's value cross table (vxTable)
		if(lnEntry->vxTable == NULL)
		{
			lnEntry->vxTable = memManagedAllocate(&strm->memList, sizeof(VxTable));
			if(lnEntry->vxTable == NULL)
				return EXIP_MEMORY_ALLOCATION_ERROR;

			// First value entry - create the vxTable
			TRY(createDynArray(&lnEntry->vxTable->dynArray, sizeof(VxEntry), DEFAULT_VX_ENTRIES_NUMBER));
		}

		assert(lnEntry->vxTable->vx);

		// Set the global ID in the value cross table entry
		vxEntry.globalId = strm->valueTable.globalId;

		// Add the entry
		TRY(addDynEntry(&lnEntry->vxTable->dynArray, (void*) &vxEntry, &vxEntryId));
	}
#endif

	// If the global ID is less than the actual array size, we must have wrapped around
	// In this case, we must reuse an existing entry
	if(strm->valueTable.globalId < strm->valueTable.count)
	{
		// Get the existing value entry
		valueEntry = &strm->valueTable.value[strm->valueTable.globalId];

#if VALUE_CROSSTABLE_USE
		assert(GET_LN_URI_QNAME(strm->schema->uriTable, valueEntry->locValuePartition.forQNameId).vxTable);
		// Null out the existing cross table entry
		GET_LN_URI_QNAME(strm->schema->uriTable, valueEntry->locValuePartition.forQNameId).vxTable->vx[valueEntry->locValuePartition.vxEntryId].globalId = INDEX_MAX;
#endif

#if HASH_TABLE_USE
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
		TRY(addEmptyDynEntry(&strm->valueTable.dynArray, (void**)&valueEntry, &valueEntryId));
	}

	// Set the value entry fields
	valueEntry->valueStr = valueStr;
#if VALUE_CROSSTABLE_USE
	valueEntry->locValuePartition.forQNameId = qnameID;
	valueEntry->locValuePartition.vxEntryId = vxEntryId;
#endif

#if HASH_TABLE_USE
	// Add value string to hash table (if present)
	if(strm->valueTable.hashTbl != NULL)
	{
		TRY(hashtable_insert(strm->valueTable.hashTbl, valueStr, strm->valueTable.globalId));
	}
#endif

	// Increment global ID
	strm->valueTable.globalId++;

	// The value table is limited by valuePartitionCapacity. If we have exceeded, we wrap around
	// to the beginning of the value table and null out existing IDs in the corresponding
	// cross table IDs
	if(strm->valueTable.globalId == strm->header.opts.valuePartitionCapacity)
		strm->valueTable.globalId = 0;

	return EXIP_ERR_OK;
}

errorCode addPfxEntry(PfxTable* pfxTable, String pfxStr, SmallIndex* pfxEntryId)
{
	if(pfxTable->count >= MAXIMUM_NUMBER_OF_PREFIXES_PER_URI)
		return EXIP_TOO_MANY_PREFIXES_PER_URI;

	pfxTable->pfxStr[pfxTable->count].length = pfxStr.length;
	pfxTable->pfxStr[pfxTable->count].str = pfxStr.str;
	*pfxEntryId = pfxTable->count++;

	return EXIP_ERR_OK;
}

errorCode createUriTableEntry(UriTable* uriTable, const String uri, int createPfx, const String pfx, const String* lnBase, Index lnSize)
{
	errorCode tmp_err_code;
	Index i;
	SmallIndex pfxEntryId;
	SmallIndex uriEntryId;
	Index lnEntryId;
	UriEntry* uriEntry;

	// Add resulting String to URI table (creates lnTable as well)
	TRY(addUriEntry(uriTable, uri, &uriEntryId));

	// Get ptr. to URI Entry
	uriEntry = &uriTable->uri[uriEntryId];

	if(createPfx)
	{
		// Create the URI's prefix table and add the default prefix
		TRY(createPfxTable(&uriEntry->pfxTable));
		TRY(addPfxEntry(uriEntry->pfxTable, pfx, &pfxEntryId));
	}

	for(i = 0; i < lnSize; i++)
	{
		TRY(addLnEntry(&uriEntry->lnTable, lnBase[i], &lnEntryId));
	}
	return EXIP_ERR_OK;
}

errorCode createUriTableEntries(UriTable* uriTable, boolean withSchema)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	String emptyStr;
	getEmptyString(&emptyStr);

	// See http://www.w3.org/TR/exi/#initialUriValues

	// URI 0: "" (empty string)
	TRY(createUriTableEntry(uriTable,
									   emptyStr,   // Namespace - empty string
									   TRUE, // Create prefix entry
									   emptyStr,   // Prefix entry - empty string
									   NULL, // No local names
									   0));

	// URI 1: "http://www.w3.org/XML/1998/namespace"
	TRY(createUriTableEntry(uriTable,
									   XML_NAMESPACE,     // URI: "http://www.w3.org/XML/1998/namespace"
									   TRUE,      // Create prefix entry
									   URI_1_PFX, // Prefix: "xml"
									   URI_1_LN,  // Add local names
									   URI_1_LN_SIZE));

	// URI 2: "http://www.w3.org/2001/XMLSchema-instance"
	TRY(createUriTableEntry(uriTable,
									   XML_SCHEMA_INSTANCE,     // URI: "http://www.w3.org/2001/XMLSchema-instance"
									   TRUE,		// Create prefix entry
									   URI_2_PFX, // Prefix: "xsi"
									   URI_2_LN,  // Add local names
									   URI_2_LN_SIZE));

	if(withSchema == TRUE)
	{
		// URI 3: "http://www.w3.org/2001/XMLSchema"
		TRY(createUriTableEntry(uriTable,
										   XML_SCHEMA_NAMESPACE,    // URI: "http://www.w3.org/2001/XMLSchema"
										   FALSE,    // No prefix entry (see http://www.w3.org/TR/exi/#initialPrefixValues) 
										   emptyStr,     // (no prefix)
										   URI_3_LN, // Add local names
										   URI_3_LN_SIZE));
	}

	return EXIP_ERR_OK;
}

boolean lookupUri(UriTable* uriTable, String uriStr, SmallIndex* uriEntryId)
{
	SmallIndex i;

	if(uriTable == NULL)
		return FALSE;

	for(i = 0; i < uriTable->count; i++)
	{
		if(stringEqual(uriTable->uri[i].uriStr, uriStr))
		{
			*uriEntryId = i;
			return TRUE;
		}
	}
	return FALSE;
}

boolean lookupLn(LnTable* lnTable, String lnStr, Index* lnEntryId)
{
	Index i;

	if(lnTable == NULL)
		return FALSE;
	for(i = 0; i < lnTable->count; i++)
	{
		if(stringEqual(lnTable->ln[i].lnStr, lnStr))
		{
			*lnEntryId = i;
			return TRUE;
		}
	}
	return FALSE;
}

boolean lookupPfx(PfxTable* pfxTable, String pfxStr, SmallIndex* pfxEntryId)
{
	SmallIndex i;

	if(pfxTable == NULL)
		return FALSE;

	for(i = 0; i < pfxTable->count; i++)
	{
		if(stringEqual(pfxTable->pfxStr[i], pfxStr))
		{
			*pfxEntryId = i;
			return TRUE;
		}
	}
	return FALSE;
}

#if VALUE_CROSSTABLE_USE
boolean lookupVx(ValueTable* valueTable, VxTable* vxTable, String valueStr, Index* vxEntryId)
{
	Index i;
	VxEntry* vxEntry;
	ValueEntry* valueEntry;

	if(vxTable == NULL || vxTable->vx == NULL)
		return FALSE;

	for(i = 0; i < vxTable->count; i++)
	{
		vxEntry = vxTable->vx + i;
		if(vxEntry->globalId == INDEX_MAX) // The value was removed from the local value partition
			continue;
		valueEntry = valueTable->value + vxEntry->globalId;
		if(stringEqual(valueEntry->valueStr, valueStr))
		{
			*vxEntryId = i;
			return TRUE;
		}
	}
	return FALSE;
}
#endif

boolean lookupValue(ValueTable* valueTable, String valueStr, Index* valueEntryId)
{
	Index i;
	ValueEntry* valueEntry;

	assert(valueTable);

#if HASH_TABLE_USE
	if(valueTable->hashTbl != NULL)
	{
		// Use hash table search
		i = hashtable_search(valueTable->hashTbl, valueStr);
		if(i != INDEX_MAX)
		{
			*valueEntryId = i;
			return TRUE;
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
				return TRUE;
			}
		}
	}

	return FALSE;
}
