/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file decodeTestEXI.h
 * @brief Interface for a function decoding sample EXI messages
 *
 * @date Nov 5, 2012
 * @author Rumen Kyusakov
 * @version 0.4.1
 * @par[Revision] $Id$
 */

#ifndef DECODETESTEXI_H_
#define DECODETESTEXI_H_

#include "procTypes.h"

#define OUT_EXI 0
#define OUT_XML 1

errorCode decode(EXIPSchema* schemaPtr, unsigned char outFlag, FILE *infile, size_t (*inputStream)(void* buf, size_t size, void* stream));

#endif /* DECODETESTEXI_H_ */
