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
 * @file sTables.h
 * @brief Types and functions describing EXI sting tables
 * @date Sep 14, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#ifndef STABLES_H_
#define STABLES_H_

#include "procTypes.h"

struct ValueRow {
	unsigned int globalId;
	StringType* string_val;
};

struct ValueTable {
	struct ValueRow* rows; // Dynamic array
	unsigned int rowCount; // The number of rows
	unsigned int arrayDimension; // The size of the Dynamic array
};

typedef struct ValueTable ValueTable;

struct ValueLocalCrossTable {
	unsigned int* valueRowIds; // Dynamic array
	unsigned int rowCount; // The number of rows
	unsigned int arrayDimension; // The size of the Dynamic array
};

typedef struct ValueLocalCrossTable ValueLocalCrossTable;

struct PrefixRow {
	unsigned int id;
	StringType* string_val;
};

struct PrefixTable {
	struct PrefixRow* rows; // Dynamic array
	unsigned int rowCount; // The number of rows
	unsigned int arrayDimension; // The size of the Dynamic array
};

typedef struct PrefixTable PrefixTable;

struct LocalNamesRow {
	unsigned int id;
	ValueLocalCrossTable* vCrossTable;
	StringType* string_val;
};

struct LocalNamesTable {
	struct LocalNamesRow* rows; // Dynamic array
	unsigned int rowCount; // The number of rows
	unsigned int arrayDimension; // The size of the Dynamic array
};

typedef struct LocalNamesTable LocalNamesTable;

struct URIRow {
	unsigned int id;
	PrefixTable* pTable;
	LocalNamesTable* lTable;
	StringType* string_val;
};

struct URITable {
	struct URIRow* rows; // Dynamic array
	unsigned int rowCount; // The number of rows
	unsigned int arrayDimension; // The size of the Dynamic array
};

typedef struct URITable URITable;

#endif /* STABLES_H_ */
