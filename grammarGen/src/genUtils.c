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
 * @file genUtils.c
 * @brief Implementation of utility functions for generating Schema-informed Grammar definitions
 * @date Nov 23, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "genUtils.h"

errorCode concatenateGrammars(struct EXIGrammar* left, struct EXIGrammar* right, struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createElementProtoGrammar(StringType name, StringType target_ns,
									QName typeDef, QName scope, unsigned char nillable,
									struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createSimpleTypeGrammar(QName simpleType, struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createSimpleEmptyTypeGrammar(struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createComplexTypeGrammar(StringType name, StringType target_ns,
		                           struct EXIGrammar* attrUsesArray, unsigned int attrUsesArraySize,
		                           StringType wildcardArray, unsigned int wildcardArraySize,
		                           struct EXIGrammar* contentTypeGrammar,
								   struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createComplexEmptyTypeGrammar(StringType name, StringType target_ns,
		                           struct EXIGrammar* attrUsesArray, unsigned int attrUsesArraySize,
		                           StringType wildcardArray, unsigned int wildcardArraySize,
								   struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createComplexUrTypeGrammar(struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createComplexUrEmptyTypeGrammar(struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createAttributeUseGrammar(unsigned char required, StringType name, StringType target_ns,
										  QName simpleType, QName scope, struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createParticleGrammar(unsigned int minOccurs, unsigned int maxOccurs,
								struct EXIGrammar* termGrammar, struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createElementTermGrammar(StringType name, StringType target_ns,
								   struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createWildcardTermGrammar(StringType wildcardArray, unsigned int wildcardArraySize,
								   struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createSequenceModelGroupsGrammar(struct EXIGrammar* pTermArray, unsigned int pTermArraySize,
											struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createChoiceModelGroupsGrammar(struct EXIGrammar* pTermArray, unsigned int pTermArraySize,
											struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}

errorCode createAllModelGroupsGrammar(struct EXIGrammar* pTermArray, unsigned int pTermArraySize,
											struct EXIGrammar** result)
{
	return NOT_IMPLEMENTED_YET;
}
