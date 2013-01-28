/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file EXISerializer.h
 * @brief Interface for serializing an EXI stream
 * Application will use this interface to work with the EXIP serializer
 *
 * @date Sep 30, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#ifndef EXISERIALIZER_H_
#define EXISERIALIZER_H_

#include "procTypes.h"

struct EXISerializer
{
	// For handling the meta-data (document structure)
	errorCode (*startDocument)(EXIStream* strm);
	errorCode (*endDocument)(EXIStream* strm);
	errorCode (*startElement)(EXIStream* strm, QName qname);
	errorCode (*endElement)(EXIStream* strm);
	errorCode (*attribute)(EXIStream* strm, QName qname, EXITypeClass valueType);

	// For handling the data
	errorCode (*intData)(EXIStream* strm, Integer int_val);
	errorCode (*booleanData)(EXIStream* strm, boolean bool_val);
	errorCode (*stringData)(EXIStream* strm, const String str_val);
	errorCode (*floatData)(EXIStream* strm, Float float_val);
	errorCode (*binaryData)(EXIStream* strm, const char* binary_val, Index nbytes);
	errorCode (*dateTimeData)(EXIStream* strm, EXIPDateTime dt_val);
	errorCode (*decimalData)(EXIStream* strm, Decimal dec_val);
	errorCode (*listData)(EXIStream* strm, unsigned int itemCount);

	// Miscellaneous
	errorCode (*processingInstruction)(EXIStream* strm); // TODO: define the parameters!
	errorCode (*namespaceDeclaration)(EXIStream* strm, const String ns, const String prefix, boolean isLocalElementNS);

	// EXI specific
	errorCode (*exiHeader)(EXIStream* strm);
	errorCode (*selfContained)(EXIStream* strm);  // Used for indexing independent elements for random access

	// EXIP specific
	void (*initHeader)(EXIStream* strm);
	errorCode (*initStream)(EXIStream* strm, BinaryBuffer buffer, EXIPSchema* schema, SchemaIdMode schemaIdMode, String* schemaID);
	errorCode (*closeEXIStream)(EXIStream* strm);
};

typedef struct EXISerializer EXISerializer;

/**
 * Used during serialization for easy access to the
 * EXIP serialization API */
extern const EXISerializer serialize;

/**** START: Serializer API implementation  ****/

// For handling the meta-data (document structure)
errorCode startDocument(EXIStream* strm);
errorCode endDocument(EXIStream* strm);
errorCode startElement(EXIStream* strm, QName qname);
errorCode endElement(EXIStream* strm);
/** @todo Consider handling the lexicographical sorting of attributes by exip encoding utilities?*/
errorCode attribute(EXIStream* strm, QName qname, EXITypeClass exiType);

// For handling the data
errorCode intData(EXIStream* strm, Integer int_val);
errorCode booleanData(EXIStream* strm, boolean bool_val);
errorCode stringData(EXIStream* strm, const String str_val);
errorCode floatData(EXIStream* strm, Float float_val);
errorCode binaryData(EXIStream* strm, const char* binary_val, Index nbytes);
errorCode dateTimeData(EXIStream* strm, EXIPDateTime dt_val);
errorCode decimalData(EXIStream* strm, Decimal dec_val);
errorCode listData(EXIStream* strm, unsigned int itemCount);

// Miscellaneous
errorCode processingInstruction(EXIStream* strm); // TODO: define the parameters!
errorCode namespaceDeclaration(EXIStream* strm, const String ns, const String prefix, boolean isLocalElementNS);

// EXI specific
errorCode selfContained(EXIStream* strm);  // Used for indexing independent elements for random access

// EXIP specific
void initHeader(EXIStream* strm);

/**
 * @brief Encodes String value into EXI stream
 *
 * @param[in, out] strm EXI stream
 * @param[in, out] buffer output buffer for storing the encoded EXI stream
 * @param[in] schema a compiled schema information to be used for schema enabled processing, NULL if no schema is available
 * @param[in] schemaIdMode one of SCHEMA_ID_ABSENT, SCHEMA_ID_SET, SCHEMA_ID_NIL or SCHEMA_ID_EMPTY
 * @param[in] schemaID if in SCHEMA_ID_SET a valid string representing the schemaID, NULL otherwise
 * @return Error handling code
 */
errorCode initStream(EXIStream* strm, BinaryBuffer buffer, EXIPSchema *schema,
						SchemaIdMode schemaIdMode, String* schemaID);


errorCode closeEXIStream(EXIStream* strm);

/****  END: Serializer API implementation  ****/


/**** START: Fast, low level API for schema encoding only ****/

/**
 * To be used by code generation tools such as static XML bindings
 * and when efficiency is of high importance
 *
 * @param[in, out] strm EXI stream
 * @param[in] EventCode EXI event code of the production
 * @param[in] qname used only for SE(*), AT(*), SE(uri:*), AT(uri:*) and when
 * a new prefix should be serialized in SE(QName) and AT(QName); NULL otherwise
 * @return Error handling code
 */
errorCode serializeEvent(EXIStream* strm, EventCode ec, QName* qname);

/****  END: Fast, low level API for schema encoding only ****/

#endif /* EXISERIALIZER_H_ */
