/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file EXIParser.h
 * @brief Interface for parsing an EXI stream
 * Application will use this interface to work with the EXIP parser
 *
 * @date Sep 30, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#ifndef EXIPARSER_H_
#define EXIPARSER_H_

#include "contentHandler.h"

/**
 * Parses an EXI document.
 */
struct Parser
{
	/** Stream containing the EXI document for parsing. */
	EXIStream strm;
	/** Function pointers for document events. */
	ContentHandler handler;
	void* app_data;
};

typedef struct Parser Parser;

/**
 * @brief Initialize a parser object
 * @param[out] parser the parser object
 * @param[in] buffer an input buffer holding (part of) the representation of EXI stream
 * @param[in] schema schema information when in schema-decoding mode. NULL when in schema-less mode
 * @param[in] app_data Application data to be passed to the content handler callbacks
 * @return Error handling code
 */
errorCode initParser(Parser* parser, BinaryBuffer buffer, EXIPSchema* schema, void* app_data);


/**
 * @brief Parse the header on the EXI stream contained in the parser object
 * @param[in] parser the parser object
 * @param[in] outOfBandOpts TRUE if there are out-of-band options set in parser->strm.header
 * FALSE otherwise
 * @return Error handling code
 */
errorCode parseHeader(Parser* parser, boolean outOfBandOpts);

/**
 * @brief Parse the next content item from the EXI stream contained in the parser object
 * @param[in] parser the parser object
 * @return Error handling code
 */
errorCode parseNext(Parser* parser);

/**
 * @brief Free any memroy allocated by parser object
 * @param[in] parser the parser object
 */
void destroyParser(Parser* parser);

#endif /* EXIPARSER_H_ */
