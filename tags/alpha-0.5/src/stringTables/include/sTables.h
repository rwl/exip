/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file sTables.h
 * @brief Functions describing EXI sting tables operations
 * @date Sep 14, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#ifndef STABLES_H_
#define STABLES_H_

#include "procTypes.h"
#include "errorHandle.h"

#define DEFAULT_VALUE_ENTRIES_NUMBER 50
#define DEFAULT_URI_ENTRIES_NUMBER   4
#define DEFAULT_PFX_ENTRIES_NUMBER   2
#define DEFAULT_LN_ENTRIES_NUMBER    10
#define DEFAULT_VX_ENTRIES_NUMBER    10

// Get local name entry from pointer to URI table using QNameID
#define GET_LN_P_URI_QNAME(uriTable, qnameID) ((uriTable)->uri[(qnameID).uriId].lnTable.ln[(qnameID).lnId])

// Get local name entry from URI table using QNameID
#define GET_LN_URI_QNAME(uriTable, qnameID) ((uriTable).uri[(qnameID).uriId].lnTable.ln[(qnameID).lnId])

// Get local name entry from pointer to URI table using QNameID pointer
#define GET_LN_P_URI_P_QNAME(uriTable, qnameID) ((uriTable)->uri[(qnameID)->uriId].lnTable.ln[(qnameID)->lnId])

// Get local name entry from URI table using QNameID pointer
#define GET_LN_URI_P_QNAME(uriTable, qnameID) ((uriTable).uri[(qnameID)->uriId].lnTable.ln[(qnameID)->lnId])

// Get local name entry from pointer to URI table using separate URI and LN IDs
#define GET_LN_P_URI_IDS(uriTable, uriId, lnId) ((uriTable)->uri[uriId].lnTable.ln[lnId])

// Get local name entry from URI table using separate URI and LN IDs
#define GET_LN_URI_IDS(uriTable, uriId, lnId) ((uriTable).uri[uriId].lnTable.ln[lnId])


/**
 * @brief Creates fresh empty ValueTable (value partition of EXI string table)
 * This operation includes allocation of memory for DEFAULT_VALUE_ENTRIES_NUMBER number of value entries
 * @param[in, out] valueTable ValueTable string table partition
 * @return Error handling code
 */
errorCode createValueTable(ValueTable* valueTable);

/**
 * @brief Creates fresh empty PfxTable (prefix partition of EXI string table)
 * This operation includes allocation of memory for DEFAULT_PFX_ENTRIES_NUMBER number of prefix entries
 * @param[out] pfxTable Prefix string table partition
 * @return Error handling code
 */
errorCode createPfxTable(PfxTable** pfxTable);

/**
 * @brief Add new row into the URI string table
 *
 * @param[in, out] uriTable URI string table partition
 * @param[in] uriStr The string representing this URI. The String can be allocated on the stack.
 * @param[out] uriEntryId The ID of the UriEntry added
 * @return Error handling code
 */
errorCode addUriEntry(UriTable* uriTable, String uriStr, SmallIndex* uriEntryId);

/**
 * @brief Add new row into the Local-Names string table
 *
 * @param[out] lnTable Local Names string table partition
 * @param[in] lnStr The string representing this local name. The String can be allocated on the stack.
 * @param[out] lnEntryId the ID of the LnEntry added
 * @return Error handling code
 */
errorCode addLnEntry(LnTable* lnTable, String lnStr, Index* lnEntryId);

/**
 * @brief Add a new row into the Global ValueTable string table and Local value cross string table
 *
 * @param[in, out] strm EXI stream of bits
 * @param[in] valueStr The string representing this global value. The String can be allocated on the stack.
 * @param[in] qnameID The URI:ln QNameID
 * @return Error handling code
 */
errorCode addValueEntry(EXIStream* strm, String valueStr, QNameID qnameID);

/**
 * @brief Add a new entry into the Prefix string table
 *
 * @param[in, out] pfxTable Prefix string table partition
 * @param[in] pfxStr The string representing this prefix. The String can be allocated on the stack.
 * @param[out] pfxEntryId the ID of the PfxEntry added
 * @return Error handling code
 */
errorCode addPfxEntry(PfxTable* pfxTable, String pfxStr, SmallIndex* pfxEntryId);

/**
 * @brief Create an initial string table entry
 * Create an initial string table entry, including URI, prefix and local name partitions
 *
 * @param[in, out] uriTable URI string table partition
 * @param[in] uri namespace string
 * @param[in] createPfx TRUE if prefix table to be created, FALSE otherwise
 * @param[in] pfx prefix string
 * @param[in] lnBase Character array representing array of local names, or NULL if not present
 * @param[in] lnSize Number of local names in array at lnBase
 * @return Error handling code
 */
errorCode createUriTableEntry(UriTable* uriTable, const String uri, int createPfx,
	const String pfx, const String* lnBase, Index lnSize);

/**
 * @brief Create all string table partitions for a URI table
 * It also inserts the default entries in the table.
 * Because the behavior depends on the EXI options of the stream
 * it is important that the options are initialized before
 * calling this function.
 *
 * @param[in, out] uriTable An empty UriTable; The memory must be already allocated for it
 * @param[in] withSchema TRUE if there is schema for this stream; FALSE otherwise;
 * Can be retrieved from strm->opts->schemaID != NULL
 * @return Error handling code
 */
errorCode createUriTableEntries(UriTable* uriTable, boolean withSchema);

/**
 * @brief Search the URI table for a particular string value
 * Implements full scan
 *
 * @param[in] uriTable URI table to be searched
 * @param[in] uriStr The string searched for
 * @param[out] uriEntryId If found, ID of the UriEntry with that string
 * @return FALSE-not found, TRUE found
 */
boolean lookupUri(UriTable* uriTable, String uriStr, SmallIndex* uriEntryId);

/**
 * @brief Search the local names table for a particular string value
 * Implements full scan
 * RCC 20120201: Try to optimize?
 *
 * @param[in] lnTable Local names table to be searched
 * @param[in] lnStr The local name string searched for
 * @param[out] lnEntryId if found, ID of the LnEntry with that string
 * @return FALSE-not found, TRUE found
 */
boolean lookupLn(LnTable* lnTable, String lnStr, Index* lnEntryId);

/**
 * @brief Search the Prefix table for a particular string value
 * Implements full scan
 *
 * @param[in] pfxTable Prefix table to be searched
 * @param[in] pfxStr The string searched for
 * @param[out] pfxEntryId if found, ID of the Prefix row with that string
 * @return FALSE-not found, TRUE found
 */
boolean lookupPfx(PfxTable* pfxTable, String pfxStr, SmallIndex* pfxEntryId);

/**
 * @brief Search the value cross table for a particular string value
 * Search the value cross table (local partition of the value table) for a particular string value
 * Implements full scan
 * RCC 20120201: Try to optimize?
 *
 * @param[in] valueTable Global Value table - used to check the string values
 * @param[in] vxTable Value cross table - local partition of the Value table to be searched
 * @param[in] valueStr The string searched for
 * @param[out] vxEntryId if found, ID of the VxEntry with that string
 * @return FALSE-not found, TRUE found
 */
boolean lookupVx(ValueTable* valueTable, VxTable* vxTable, String valueStr, Index* vxEntryId);

/**
 * @brief Search the global Value table for a particular string value
 * Implements full scan when opts->valuePartitionCapacity < DEFAULT_VALUE_ROWS_NUMBER
 * Hash search otherwise
 *
 * @param[in] valueTable Global Value table to be searched
 * @param[in] valueStr The string searched for
 * @param[out] valueEntryId if found, ID of the ValueEntry with that string
 * @return FALSE-not found, TRUE found
 */
boolean lookupValue(ValueTable* valueTable, String valueStr, Index* valueEntryId);

#endif /* STABLES_H_ */
