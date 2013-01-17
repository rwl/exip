/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file contentHandler.h
 * @brief SAX-like interface for parsing the content of an EXI stream
 * The applications should register to this handlers with callback functions
 * invoked when the processor pass through the stream. This interface is lower level than SAX.
 * If you want to use SAX API you should wrap this interface.
 * @date Sep 7, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#ifndef CONTENTHANDLER_H_
#define CONTENTHANDLER_H_

#include "procTypes.h"

/** Macros for the returning code from the ContentHandler callbacks */
#define EXIP_HANDLER_OK   0
#define EXIP_HANDLER_STOP 1

/**
 * Simple container for function pointers for document events.
 */
struct ContentHandler
{
	// For handling the meta-data (document structure)
	char (*startDocument)(void* app_data);
	char (*endDocument)(void* app_data);
	char (*startElement)(QName qname, void* app_data);
	char (*endElement)(void* app_data);
	char (*attribute)(QName qname, void* app_data);

	// For handling the data
	char (*intData)(Integer int_val, void* app_data);
	char (*booleanData)(unsigned char bool_val, void* app_data);
	char (*stringData)(const String str_val, void* app_data);
	char (*floatData)(Float float_val, void* app_data);
	char (*binaryData)(const char* binary_val, Index nbytes, void* app_data);
	char (*dateTimeData)(EXIPDateTime dt_val, void* app_data);
	char (*decimalData)(Decimal dec_val, void* app_data);
	char (*listData)(EXIType exiType, unsigned int itemCount, void* app_data);

	// Miscellaneous
	char (*processingInstruction)(void* app_data); // TODO: define the parameters!
	char (*namespaceDeclaration)(const String ns, const String prefix, unsigned char isLocalElementNS, void* app_data);

	// For error handling
	char (*warning)(const char code, const char* msg, void* app_data);
	char (*error)(const char code, const char* msg, void* app_data);
	char (*fatalError)(const char code, const char* msg, void* app_data);

	// EXI specific
	char (*selfContained)(void* app_data);  // Used for indexing independent elements for random access
};

typedef struct ContentHandler ContentHandler;

/**
 * @brief Initialize the content handler before use
 * @param[in] handler fresh ContentHandler
 *
 */
void initContentHandler(ContentHandler* handler);

#endif /* CONTENTHANDLER_H_ */
