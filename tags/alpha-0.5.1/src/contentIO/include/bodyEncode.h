/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file bodyEncode.h
 * @brief API for encoding EXI stream body
 * @date Sep 7, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#ifndef BODYENCODE_H_
#define BODYENCODE_H_

#include "errorHandle.h"
#include "procTypes.h"

/**
 * @brief Finds the grammar production based on the event (and eventually the qname in case of SE and AT)
 * @param[in, out] strm EXI stream
 * @param[in] eventClass event class type to be looked up
 * @param[in] isSchemaType determine if the data type should be encoded as non-schema type
 * @param[in] qname element or attribute QName in case of SE or AT events; NULL otherwise
 * @param[out] prodHit the matched grammar production
 * @return Error handling code
 */
errorCode encodeProduction(EXIStream* strm, EventTypeClass eventClass, boolean isSchemaType, QName* qname, Production* prodHit);

/**
 * @brief Encodes String value into EXI stream
 * @param[in, out] strm EXI stream
 * @param[in] strng string to be written
 * @param[in] qnameID The uri/ln ids in the URI string table
 * @param[in] typeId constraints and restrictions on the String type according to the grammar production
 * @return Error handling code
 */
errorCode encodeStringData(EXIStream* strm, String strng, QNameID qnameID, Index typeId);

/**
 * @brief Encodes QName into EXI stream
 * @param[in, out] strm EXI stream
 * @param[in] qname qname to be written
 * @param[in] eventT (EVENT_SE_ALL or EVENT_AT_ALL) used for error checking purposes:
 * If the given prefix does not exist in the associated partition, the QName MUST be part of an SE event and
 * the prefix MUST be resolved by one of the NS events immediately following the SE event (see resolution rules below).
 * @param[out] qnameID the QName ID
 * @return Error handling code
 */
errorCode encodeQName(EXIStream* strm, QName qname, EventType eventT, QNameID* qnameID);

/**
 * @brief Encodes URI into EXI stream
 * @param[in, out] strm EXI stream
 * @param[in] uri uri to be written
 * @param[out] uriID id of the uri row written in the string table
 * @return Error handling code
 */
errorCode encodeUri(EXIStream* strm, String* uri, SmallIndex* uriID);

/**
 * @brief Encodes Local Name into EXI stream
 * @param[in, out] strm EXI stream
 * @param[in] ln local name to be written
 * @param[out] qnameID id of the qname in the URI and LN string tables
 * @return Error handling code
 */
errorCode encodeLn(EXIStream* strm, String* ln, QNameID* qnameID);

/**
 * @brief Encodes the prefix part of the QName into the EXI stream in case the Preserve.prefixes == TRUE
 * @param[in, out] strm EXI stream
 * @param[in] qname qname to be written
 * @param[in] eventT (EVENT_SE_ALL or EVENT_AT_ALL) used for error checking purposes:
 * If the given prefix does not exist in the associated partition, the QName MUST be part of an SE event and
 * the prefix MUST be resolved by one of the NS events immediately following the SE event (see resolution rules below).
 * @param[in] uriID the QName uriID
 * @return Error handling code
 */
errorCode encodePfxQName(EXIStream* strm, QName* qname, EventType eventT, SmallIndex uriID);

/**
 * @brief Encodes the prefix part of the NS event into the EXI stream
 * @param[in, out] strm EXI stream
 * @param[in] uriID id of the uri row of the URI the string table
 * @param[in] prefix prefix to be written
 * @return Error handling code
 */
errorCode encodePfx(EXIStream* strm, SmallIndex uriID, String* prefix);

/**
 * @brief Encodes Integer value into EXI stream
 * @param[in, out] strm EXI stream
 * @param[in] int_val integer to be written
 * @param[in] typeId index in the type table. It is used to determine the EXI int type and additional restrictions
 * @return Error handling code
 */
errorCode encodeIntData(EXIStream* strm, Integer int_val, Index typeId);

#endif /* BODYENCODE_H_ */
