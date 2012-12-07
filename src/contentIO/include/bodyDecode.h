/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file bodyDecode.h
 * @brief API for decoding EXI stream body
 * @date Sep 7, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#ifndef BODYDECODE_H_
#define BODYDECODE_H_

#include "contentHandler.h"

/**
 * @brief Process the next grammar production in the Current Grammar
 *
 * Returns the terminal symbol of the production i.e. the EXI Event Type;
 * @param[in] strm EXI stream of bits
 * @param[out] nonTermID_out unique identifier of right-hand side Non-terminal
 * @param[in] handler content handler callbacks
 * @param[in] app_data Application data to be passed to the content handler callbacks
 * @return Error handling code
 */
errorCode processNextProduction(EXIStream* strm, SmallIndex* nonTermID_out, ContentHandler* handler, void* app_data);

/**
 * @brief Decodes a QName from the EXI stream
 *
 * @param[in, out] strm EXI stream representation
 * @param[out] qname the QName decoded
 * @param[out] qnameID the QNameID in the string tables
 * @return Error handling code
 */
errorCode decodeQName(EXIStream* strm, QName* qname, QNameID* qnameID);

/**
 * @brief Decodes the URI part of a QName from the EXI stream
 *
 * @param[in, out] strm EXI stream representation
 * @param[out] uriId The URI id in the URI string table
 * @return Error handling code
 */
errorCode decodeUri(EXIStream* strm, SmallIndex* uriId);

/**
 * @brief Decodes the Local name part of a QName from the EXI stream
 *
 * @param[in, out] strm EXI stream representation
 * @param[in] uriId The URI id in the URI string table
 * @param[out] lnId The LN id in the Local name string table
 * @return Error handling code
 */
errorCode decodeLn(EXIStream* strm, Index uriId, Index* lnId);

/**
 * @brief Decodes the prefix component of the QName from the EXI stream
 * In case of Preserve.prefixes the QName type contains a prefix also
 *
 * @param[in, out] strm EXI stream representation
 * @param[out] qname qname that holds the prefix
 * @param[in] uriId The URI id in the URI string table
 * @return Error handling code
 */
errorCode decodePfxQname(EXIStream* strm, QName* qname, SmallIndex uriId);

/**
 * @brief Decodes the Prefix part of a NS event from the EXI stream
 *
 * @param[in, out] strm EXI stream representation
 * @param[in] uriID The URI id in the URI string table
 * @param[out] pfxId The prefix id in the Prefix string table
 * @return Error handling code
 */
errorCode decodePfx(EXIStream* strm, SmallIndex uriID, SmallIndex* pfxId);

/**
 * @brief Decodes a string value from the EXI stream
 * @param[in, out] strm EXI stream representation
 * @param[in] qnameID The uri/ln ids in the URI string table
 * @param[out] value the string decoded
 * @return Error handling code
 */
errorCode decodeStringValue(EXIStream* strm, QNameID qnameID, String* value);

/**
 * @brief Decodes the content of EXI event
 * @param[in, out] strm EXI stream representation
 * @param[in] prodHit the current grammar production
 * @param[in] handler application content handler; stores the callback functions
 * @param[out] nonTermID_out nonTerminal ID after the content decoding
 * @param[in] app_data Application data to be passed to the content handler callbacks
 * @return Error handling code
 */
errorCode decodeEventContent(EXIStream* strm, Production* prodHit, ContentHandler* handler, SmallIndex* nonTermID_out, void* app_data);

/**
 * @brief Decodes the value content item of an EXI event
 *
 * @param[in, out] strm EXI stream representation
 * @param[in] typeId index of the type in the SimpleTypeTable
 * @param[in] handler application content handler; stores the callback functions
 * @param[out] nonTermID_out nonTerminal ID after the content decoding
 * @param[in] localQNameID the local uri/ln IDs
 * @param[in] app_data Application data to be passed to the content handler callbacks
 * @return Error handling code
 */
errorCode decodeValueItem(EXIStream* strm, Index typeId, ContentHandler* handler, SmallIndex* nonTermID_out, QNameID localQNameID, void* app_data);

/**
 * @brief Decodes NS event
 * @param[in, out] strm EXI stream representation
 * @param[in] handler application content handler; stores the callback functions
 * @param[out] nonTermID_out nonTerminal ID after the content decoding
 * @param[in] app_data Application data to be passed to the content handler callbacks
 * @return Error handling code
 */
errorCode decodeNSEvent(EXIStream* strm, ContentHandler* handler, SmallIndex* nonTermID_out, void* app_data);

/**
 * @brief Decodes SE(*) event
 * @param[in, out] strm EXI stream representation
 * @param[in] handler application content handler; stores the callback functions
 * @param[out] nonTermID_out nonTerminal ID after the content decoding
 * @param[in] app_data Application data to be passed to the content handler callbacks
 * @return Error handling code
 */
errorCode decodeSEWildcardEvent(EXIStream* strm, ContentHandler* handler, SmallIndex* nonTermID_out, void* app_data);

/**
 * @brief Decodes AT(*) event
 * @param[in, out] strm EXI stream representation
 * @param[in] handler application content handler; stores the callback functions
 * @param[out] nonTermID_out nonTerminal ID after the content decoding
 * @param[in] app_data Application data to be passed to the content handler callbacks
 * @return Error handling code
 */
errorCode decodeATWildcardEvent(EXIStream* strm, ContentHandler* handler, SmallIndex* nonTermID_out, void* app_data);


#endif /* BODYDECODE_H_ */
