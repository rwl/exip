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
 * @file buildInGrammars.h
 * @brief Defines a function for generation of build-in Schema type grammars.
 * This functionality is put in this module (and not grammarGen) because of:
 * When the value of the "schemaId" element is empty, no user defined schema information is
 * used for processing the EXI body; however, the built-in XML schema types are available for use in the EXI body
 * This requires that some grammar generation is done even if there is no dynamic
 * grammar generation.
 *
 * @date Nov 28, 2011
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#ifndef BUILDINGRAMMARS_H_
#define BUILDINGRAMMARS_H_

#include "errorHandle.h"
#include "procTypes.h"
#include "dynamicArray.h"
#include "memManagement.h"

/**
 * @brief Generate all build in Schema-informed Element and Type Grammars
 * It is used when the value of the "schemaId" element is empty, no user defined schema information
 * is used for processing the EXI body; however, the built-in XML schema types are available for use in the EXI body
 *
 * @param[out] schema the resulted schema information used for processing EXI streams
 * @return Error handling code
 */
errorCode generateSchemaBuildInGrammars(EXIPSchema* schema);

/**
 * @brief Generate a Schema-informed Type and TypeEmpty Grammars for all build-in XML Schema Types
 * It is used by generateSchemaInformedGrammars() and when the value of the "schemaId" element is empty,
 * no user defined schema information is used for processing the EXI body; however, the built-in XML schema
 * types are available for use in the EXI body.
 *
 * @param[in, out] schema schema for which the grammars are generated
 * @return Error handling code
 */
errorCode generateBuildInTypesGrammars(EXIPSchema* schema);


/**
 * @brief Populate initial simple type array with the build-in simple types
 *
 * @param[in, out] sTypeArr Dynamic array storing the simple types definitions
 * @param[in, out] memList memory allocations
 * @return Error handling code
 */
errorCode createBuildInTypesDefinitions(DynArray* sTypeArr, AllocList* memList);

/**
 * @brief Maps a simple XSD type to its EXI datatype representation
 *
 * @param[in] simpleXSDType simple XSD type QName given as string table ids
 * @param[out] vType corresponding EXI type with constraining facets
 * @return Error handling code
 */
errorCode getEXIDataTypeFromSimpleType(QNameID simpleXSDType, ValueType* vType);

#endif /* BUILDINGRAMMARS_H_ */
