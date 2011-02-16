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
 * @file genUtils.h
 * @brief Definition and utility functions for generating Schema-informed Grammar definitions
 * @date Nov 23, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#ifndef GENUTILS_H_
#define GENUTILS_H_

#include "errorHandle.h"
#include "procTypes.h"
#include "dynamicArray.h"
#include "sTables.h"
#include "schema.h"

/**
 * This structure holds pointers to
 * ruleArray[i].prodArray[j].lnRowID
 * ruleArray[i].prodArray[j].uriRowID
 * and their old values before sorting.
 * Based on the old index, the new values of lnRowID and uriRowID
 * after sorting are updated using the pointers
 */
struct productionQname {
	unsigned int* p_uriRowID;
	unsigned int* p_lnRowID;
	unsigned int uriRowID_old;
	unsigned int lnRowID_old;
};

/**
 * @brief Grammar Concatenation Operator
 * The grammar concatenation operator ⊕ is a binary, associative
 * operator that creates a new grammar from its left and right
 * grammar operands. The new grammar accepts any set of symbols
 * accepted by its left operand followed by any set of symbols
 * accepted by its right operand.
 * The left and right grammars are not garbage collected!
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] left left operand - grammar
 * @param[in] right right operand - grammar
 * @param[out] result the result grammar of Concatenation Operator
 * @return Error handling code
 */
errorCode concatenateGrammars(AllocList* memList, struct EXIGrammar* left, struct EXIGrammar* right, struct EXIGrammar** result);

/**
 * @brief Creates new Element Proto-Grammar from XML Schema element declaration
 * Let T-j be the {type definition} of E-i  and Type-j  be the type grammar created from T-j.
 * The grammar Element-i  describing the content model of E-i  is created as follows:
 * Element-i,0 : Type-j,0
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] name element local name
 * @param[in] target_ns element target namespace
 * @param[in] typeDef EXIGrammar of the element type definition
 * @param[in] scope element scope - if NULL then the scope is global otherwise the QName of the complex type which is the scope
 * @param[in] nillable 0 - false, otherwise true
 * @param[out] result the resulted proto-grammar
 * @return Error handling code
 */
errorCode createElementProtoGrammar(AllocList* memList, StringType name, StringType target_ns,
									struct EXIGrammar* typeDef, QName scope, unsigned char nillable,
									struct EXIGrammar** result);

/**
 * @brief Creates Simple Type Grammar from XML Schema simple type definition
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] simpleType qname of the simple type
 * @param[out] result the resulted proto-grammar
 * @return Error handling code
 */
errorCode createSimpleTypeGrammar(AllocList* memList, QName simpleType, struct EXIGrammar** result);

/**
 * @brief Creates Simple EmptyType Grammar from XML Schema simple type definition
 * It doesn't depend on what is the actual Schema simple type
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[out] result the resulted proto-grammar
 * @return Error handling code
 */
errorCode createSimpleEmptyTypeGrammar(AllocList* memList, struct EXIGrammar** result);

/**
 * @brief Creates Complex Type Proto-Grammar from XML Schema complex type definition
 * Sort the attribute use grammars first by qname local-name, then by qname uri.
 * If {content type} is type definition T-j , generate a grammar Content-i as grammar Type-j
 * Then create a copy H-i  of each attribute use grammar
 * Result: Type-i = H-0 ⊕ H-1 ⊕ … ⊕ H-n−1 ⊕ Content-i
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] name complex type local name
 * @param[in] target_ns complex type namespace
 * @param[in] attrUsesArray array of attribute uses grammars included in this complex type.
 *            It should be lexicographically sorted
 * @param[in] attrUsesArraySize the size of the attribute uses array
 * @param[in] wildcardArray array of strings. Possible values: "any" or a set of namespace names and "absent"
 * or 'not' and a namespace name or "not" and "absent"
 * @param[in] wildcardArraySize the size of the wildcard array
 * @param[in] contentTypeGrammar the proto-grammar of the complex type content: either Simple Type Grammar,
 * or Particle grammar or empty
 * @param[out] result the resulted proto-grammar
 * @return Error handling code
 */
errorCode createComplexTypeGrammar(AllocList* memList, StringType name, StringType target_ns,
		                           struct EXIGrammar* attrUsesArray, unsigned int attrUsesArraySize,
		                           StringType* wildcardArray, unsigned int wildcardArraySize,
		                           struct EXIGrammar* contentTypeGrammar,
								   struct EXIGrammar** result);

/**
 * @brief Creates Complex EmptyType Proto-Grammar from XML Schema complex type definition
 * Sort the attribute use grammars first by qname local-name, then by qname uri.
 * TypeEmpty-i = G-0 ⊕ G-1 ⊕ … ⊕ G-n−1 ⊕ Content-i
 * where the grammar Content-i is created as follows:
 * Content-i,0 : EE
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] name complex type local name
 * @param[in] target_ns complex type namespace
 * @param[in] attrUsesArray array of attribute uses grammars included in this complex type
 * @param[in] attrUsesArraySize the size of the attribute uses array
 * @param[in] wildcardArray array of strings. Possible values: "any" or a set of namespace names and "absent"
 * or 'not' and a namespace name or "not" and "absent"
 * @param[in] wildcardArraySize the size of the wildcard array
 * @param[out] result the resulted proto-grammar
 * @return Error handling code
 */
errorCode createComplexEmptyTypeGrammar(AllocList* memList, StringType name, StringType target_ns,
		                           struct EXIGrammar* attrUsesArray, unsigned int attrUsesArraySize,
		                           StringType* wildcardArray, unsigned int wildcardArraySize,
								   struct EXIGrammar** result);

/**
 * @brief Creates Complex Ur-Type Grammar from XML Schema complex ur-type
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[out] result the resulted proto-grammar
 * @return Error handling code
 */
errorCode createComplexUrTypeGrammar(AllocList* memList, struct EXIGrammar** result);

/**
 * @brief Creates Complex Ur-EmptyType Grammar from XML Schema complex ur-type
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[out] result the resulted proto-grammar
 * @return Error handling code
 */
errorCode createComplexUrEmptyTypeGrammar(AllocList* memList, struct EXIGrammar** result);

/**
 * @brief Creates Attribute Use Grammar from XML Schema Attribute Use
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] required 0 - false; otherwise true
 * @param[in] name attribute local name
 * @param[in] target_ns attribute namespace
 * @param[in] simpleType qname of the simple type of the attribute type definition
 * @param[in] scope attribute scope - if NULL then the scope is global otherwise the QName of the complex type which is the scope
 * @param[out] result the resulted proto-grammar
 * @param[in, out] metaURITable temporary String table for holding the strings before sorting.
 * @param[in, out] regProdQname dynamic array of struct productionQname elements
 * @return Error handling code
 */
errorCode createAttributeUseGrammar(AllocList* memList, unsigned char required, StringType name, StringType target_ns,
										  QName simpleType, QName scope, struct EXIGrammar** result, URITable* metaURITable, DynArray* regProdQname);

/**
 * @brief Creates Particle Proto-Grammar from XML Schema particle
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] minOccurs particle's {min Occurs}
 * @param[in] maxOccurs particle's {max Occurs}. If less than 0 then the value is {unbounded}
 * @param[in] termGrammar the grammar created from the particle's term: Element Term, Wildcard Term or Model Group Term
 * @param[out] result the resulted proto-grammar
 * @return Error handling code
 */
errorCode createParticleGrammar(AllocList* memList, unsigned int minOccurs, int32_t maxOccurs,
								struct EXIGrammar* termGrammar, struct EXIGrammar** result);

/**
 * @brief Creates Element Term Proto-Grammar from Particle term that is XML Schema element declaration
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] name element local name
 * @param[in] target_ns element target namespace
 * @param[out] result the resulted proto-grammar
 * @param[in, out] metaURITable temporary String table for holding the strings before sorting.
 * @param[in, out] regProdQname dynamic array of struct productionQname elements
 * @return Error handling code
 */
errorCode createElementTermGrammar(AllocList* memList, StringType name, StringType target_ns,
								   struct EXIGrammar** result, URITable* metaURITable, DynArray* regProdQname);

/**
 * @brief Creates Wildcard Term Proto-Grammar from Particle term that is XML Schema wildcard
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] wildcardArray array of strings. Possible values: "any" or a set of namespace names and "absent"
 * or 'not' and a namespace name or "not" and "absent"
 * @param[in] wildcardArraySize the size of the wildcard array
 * @param[out] result the resulted proto-grammar
 * @return Error handling code
 */
errorCode createWildcardTermGrammar(AllocList* memList, StringType* wildcardArray, unsigned int wildcardArraySize,
								   struct EXIGrammar** result);

/**
 * @brief Creates Sequence Model Group Proto-Grammar from Particle term that is XML Schema Model Group with {compositor} equal to "sequence"
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] pTermArray an array of ParticleTerm grammars included in the sequence Model Group
 * @param[in] pTermArraySize the size of the ParticleTerm grammar array
 * @param[out] result the resulted proto-grammar
 * @return Error handling code
 */
errorCode createSequenceModelGroupsGrammar(AllocList* memList, struct EXIGrammar* pTermArray, unsigned int pTermArraySize,
											struct EXIGrammar** result);

/**
 * @brief Creates Choice Model Group Proto-Grammar from Particle term that is XML Schema Model Group with {compositor} equal to "choice"
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] pTermArray an array of ParticleTerm grammars included in the Choice Model Group
 * @param[in] pTermArraySize the size of the ParticleTerm grammar array
 * @param[out] result the resulted proto-grammar
 * @return Error handling code
 */
errorCode createChoiceModelGroupsGrammar(AllocList* memList, struct EXIGrammar* pTermArray, unsigned int pTermArraySize,
											struct EXIGrammar** result);

/**
 * @brief Creates All Model Group Proto-Grammar from Particle term that is XML Schema Model Group with {compositor} equal to "all"
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] pTermArray an array of ParticleTerm grammars included in the All Model Group
 * @param[in] pTermArraySize the size of the ParticleTerm grammar array
 * @param[out] result the resulted proto-grammar
 * @return Error handling code
 */
errorCode createAllModelGroupsGrammar(AllocList* memList, struct EXIGrammar* pTermArray, unsigned int pTermArraySize,
											struct EXIGrammar** result);

/**
 * @brief Maps a simple XSD type to its EXI datatype representation
 *
 * @param[in] simpleXSDType simple XSD type
 * @param[out] exiType corresponding EXI type
 * @return Error handling code
 */
errorCode getEXIDataType(QName simpleXSDType, ValueType* exiType);

// TODO: consider removing registerQname
/**
 * @brief Register a URI of target namespace or QName of attribute, element or type declared for storing in the string tables.
 * A meta-String tables are used for holding the strings before sorting.
 *
 * @param[in] strm EXIStream used to attach the memory allocations to it.
 * @param[in] name local name for registering
 * @param[in] target_ns uri for registering
 * @param[in, out] metaURITable temporary String table for holding the strings before sorting.
 * @param[in, out] regProdQname dynamic array of struct productionQname elements
 * @param[in] p_uriRowID pointers to ruleArray[i].prodArray[j].uriRowID
 * @param[in] p_lnRowID pointers to ruleArray[i].prodArray[j].lnRowID
 * @return Error handling code
 */
errorCode registerQname(EXIStream* strm, StringType name, StringType target_ns, URITable* metaURITable,
		                DynArray* regProdQname, unsigned int* p_uriRowID, unsigned int* p_lnRowID);


/**
 * @brief Sorts the information in the MetaString tables
 *
 * @param[in, out] memList A list storing the memory allocations
 * @param[in] metaURITable temporary String table for holding the strings before sorting
 * @param[out] schema holds the allocations for the sorted string tables stored in schema->initialStringTables
 * @param[in] regProdQname used to update the uri and ln string tables row IDs
 * @return Error handling code
 */
errorCode stringTablesSorting(AllocList* memList, URITable* metaURITable, ExipSchema* schema, DynArray* regProdQname);

#endif /* GENUTILS_H_ */
