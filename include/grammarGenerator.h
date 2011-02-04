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
 * @file grammarGenerator.h
 * @brief Definition and functions for generating Schema-informed Grammar definitions
 * @date Nov 22, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#ifndef GRAMMARGENERATOR_H_
#define GRAMMARGENERATOR_H_

#include "errorHandle.h"
#include "procTypes.h"
#include "schema.h"

/** Supported schema formats like XML-XSD, EXI-XSD, DTD or any other schema representation supported */
#define SCHEMA_FORMAT_XSD_EXI           0
#define SCHEMA_FORMAT_XSD_XML           1
#define SCHEMA_FORMAT_DTD               2
#define SCHEMA_FORMAT_RELAX_NG          3

/** Form Choice values */
#define FORM_CHOICE_UNQUALIFIED           0
#define FORM_CHOICE_QUALIFIED             1
#define FORM_CHOICE_ABSENT                2

/** Codes for the elements found in the schema */
#define ELEMENT_ELEMENT          0
#define ELEMENT_ATTRIBUTE        1
#define ELEMENT_CHOICE           2
#define ELEMENT_COMPLEX_TYPE     3
#define ELEMENT_COMPLEX_CONTENT  4
#define ELEMENT_GROUP            5
#define ELEMENT_IMPORT           6
#define ELEMENT_SEQUENCE         7
#define ELEMENT_ALL              8
#define ELEMENT_EXTENSION        9
#define ELEMENT_RESTRICTION     10
#define ELEMENT_SIMPLE_CONTENT  11

#define ELEMENT_VOID           255


/** Codes for the attributes found in the schema */
#define ATTRIBUTE_ABSENT     0
#define ATTRIBUTE_NAME       1
#define ATTRIBUTE_TYPE       2
#define ATTRIBUTE_REF        3
#define ATTRIBUTE_MIN_OCCURS 4
#define ATTRIBUTE_MAX_OCCURS 5
#define ATTRIBUTE_FORM       6
#define ATTRIBUTE_BASE       7
#define ATTRIBUTE_USE        8

#define ATTRIBUTE_VOID     255

#define ATTRIBUTE_CONTEXT_ARRAY_SIZE 20

/**
 * Global schema properties (found as an attributes of the schema root element in XSD)
 * They should not change over time of processing
 */
struct globalSchemaProps {
	unsigned char propsStat; // 0 - initial state, 1 - <schema> element is parsed expect attributes, 2 - the properties are all set (<schema> attr. parsed)
	unsigned char expectAttributeData;
	StringType* charDataPointer; // Pointer to the expected character data
//	StringType tmpCharData; // Store temporary string before its processing further
	StringType targetNamespace;
	unsigned char attributeFormDefault; // 0 unqualified, 1 qualified, 2 expecting value, 3 initial state
	unsigned char elementFormDefault;  // 0 unqualified, 1 qualified, 2 expecting value, 3 initial state
};

/**
 * An entry of nested proto-grammar descriptions. It is used to store the
 * current context when passing through schema document
 */
struct elementDescr {
	unsigned char element;  // represented with codes defined above
	StringType attributePointers[ATTRIBUTE_CONTEXT_ARRAY_SIZE]; // the index is the code of the attribute
	struct elementDescr* nextInStack;
};

typedef struct elementDescr ContextStack;

typedef struct EXIGrammar ProtoGrammarsStack;

/**
 * Represents an element declaration with attribute type and the
 * value of the type cannot be found in the TypeGrammar pool.
 * That is, the definition of the type is still not reached.
 * This elements are put in a dynamic array
 * */
struct elementNotResolved {
	QName element;
	QName type;
};

/**
 * @brief Generate a Schema-informed Document Grammar and all Schema-informed Element and Type Grammars
 * Initial implementation is targeted at XML Schema definitions encoded with EXI with default options.
 * The grammar of the schema can be further optimized in the future.
 *
 * @param[in] binaryStream the binary representation of XML schema
 * @param[in] bufLen size of binaryStream - number of bytes
 * @param[in] schemaFormat EXI, XSD, DTD or any other schema representation supported
 * @param[out] strm An empty EXI stream used for memory allocations.
 * @param[out] exipSchema the resulted schema information used for processing EXI streams
 * @return Error handling code
 */
errorCode generateSchemaInformedGrammars(char* binaryStream, uint32_t bufLen, unsigned char schemaFormat,
										EXIStream* strm, ExipSchema* exipSchema);

#endif /* GRAMMARGENERATOR_H_ */
