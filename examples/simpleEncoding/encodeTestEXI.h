/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file encodeTestEXI.h
 * @brief Interface for a function encoding a sample EXI message
 *
 * @date Nov 5, 2012
 * @author Rumen Kyusakov
 * @version 0.5
 * @par[Revision] $Id$
 */

#ifndef ENCODETESTEXI_H_
#define ENCODETESTEXI_H_

#include "procTypes.h"

errorCode encode(EXIPSchema* schemaPtr, FILE *outfile, size_t (*outputStream)(void* buf, size_t size, void* stream));

#endif /* ENCODETESTEXI_H_ */
