/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file emptyTypeGrammarGen.h
 * @brief API for generating emptyTypeGrammars from standard grammars
 * @date May 3, 2012
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#ifndef EMPTYTYPEGRAMMARGEN_H_
#define EMPTYTYPEGRAMMARGEN_H_

#include "procTypes.h"

/**
 * @brief Generate an emptyTypeGrammar that corresponds to a full EXI grammar
 * This function is used in the case of nil="true" attribute
 *
 * @param[in, out] strm EXI stream representation
 * @param[in] src the full EXI grammar
 * @param[out] dest the resulting emptyTypeGrammar
 * @return Error handling code
 */
errorCode getEmptyTypeGrammar(EXIStream* strm, EXIGrammar* src, EXIGrammar** dest);

#endif /* EMPTYTYPEGRAMMARGEN_H_ */
