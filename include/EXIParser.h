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
 * @version 0.5
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

struct EXIParser
{
	errorCode (*initParser)(Parser* parser, BinaryBuffer buffer, void* app_data);
	errorCode (*parseHeader)(Parser* parser, boolean outOfBandOpts);
	errorCode (*setSchema)(Parser* parser, EXIPSchema* schema);
	errorCode (*parseNext)(Parser* parser);
	errorCode (*pushEXIData)(char* inBuf, unsigned int bufSize, Parser* parser);
	void (*destroyParser)(Parser* parser);
};

typedef struct EXIParser EXIParser;

/**
 * Used during parsing for easy access to the
 * EXIP parsing API */
extern const EXIParser parse;

/**
 * @brief Initialize a parser object
 * @param[out] parser the parser object
 * @param[in] buffer an input buffer holding (part of) the representation of EXI stream
 * @param[in] app_data Application data to be passed to the content handler callbacks
 * @return Error handling code
 */
errorCode initParser(Parser* parser, BinaryBuffer buffer, void* app_data);


/**
 * @brief Parse the header on the EXI stream contained in the parser object
 * @param[in] parser the parser object
 * @param[in] outOfBandOpts TRUE if there are out-of-band options set in parser->strm.header
 * FALSE otherwise
 * @return Error handling code
 */
errorCode parseHeader(Parser* parser, boolean outOfBandOpts);

/**
 * @brief Initialize the schema information to be used for parsing
 * @param[in] parser the parser object
 * @param[in] schema object; if parser.strm.header.opts.schemaIDMode == SCHEMA_ID_NIL or
 * parser.strm.header.opts.schemaIDMode == SCHEMA_ID_EMPTY the schema object is ignored;
 * if parser.strm.header.opts.schemaIDMode == SCHEMA_ID_ABSENT and schema == NULL then
 * schema-less mode, schema != NULL schema enabled;
 * if parser.strm.header.opts.schemaIDMode == SCHEMA_ID_SET and schema == NULL is an error
 * parser.strm.header.opts.schemaID contains the identifier of the schema if any
 *
 * @return Error handling code
 */
errorCode setSchema(Parser* parser, EXIPSchema* schema);

/**
 * @brief Parse the next content item from the EXI stream contained in the parser object
 *
 * @param[in] parser the parser object
 *
 * @return Error handling code; EXIP_PARSING_COMPLETE when the parsing is completed succesfully,
 * EXIP_BUFFER_END_REACHED when the whole content of the buffer is parsed but there is more
 * events in the EXI stream. In case of EXIP_BUFFER_END_REACHED, pushEXIData() must be used
 * to fill the buffer with the next chunk of the EXI stream before calling parseNext() again.
 */
errorCode parseNext(Parser* parser);

/**
 * @brief Pushes more data to the parsing buffer to parse
 *
 * This function is used to implement push-parsing interface.
 * After parseNext() returns EXIP_BUFFER_END_REACHED use this
 * function to add more data to the parsing buffer.
 *
 * @warning Padding bits to fill a byte when in bit-packed mode
 * should not be used as they will be interpreted as if being part
 * of the EXI stream. This can be a concern when streaming EXI
 * data which is not byte-aligned.
 *
 * @param[in] inBuf the next EXI stream chunk to be parsed
 * @param[in] bufSize the size in bytes of the inBuf
 * @param[in] parser the parser object
 *
 * @return Error handling code
 */
errorCode pushEXIData(char* inBuf, unsigned int bufSize, Parser* parser);

/**
 * @brief Free any memroy allocated by parser object
 * @param[in] parser the parser object
 */
void destroyParser(Parser* parser);

#endif /* EXIPARSER_H_ */
