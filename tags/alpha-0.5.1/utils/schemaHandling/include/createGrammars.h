/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file createGrammars.h
 * @brief Interface to functions to Generate EXI grammars from XML schema definition
 *
 * @date Oct 13, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */
#ifndef CREATEGRAMMARS_H_
#define CREATEGRAMMARS_H_

#include "procTypes.h"

errorCode toText(EXIPSchema* schemaPtr, FILE *outfile);

errorCode toStaticSrc(EXIPSchema* schemaPtr, char* prefix, FILE *outfile);

errorCode toDynSrc(EXIPSchema* schemaPtr, FILE *outfile);

errorCode toEXIP(EXIPSchema* schemaPtr, FILE *outfile);

#endif /* CREATEGRAMMARS_H_ */
