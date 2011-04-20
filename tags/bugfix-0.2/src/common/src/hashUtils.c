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
 * @file hashUtils.c
 * @brief Implementation of the hash function used in the processor
 * @date Nov 23, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "procTypes.h"

uint32_t djbHash(void* str, unsigned int len)
{
	char* tmp = str;
	uint32_t hash = 5381;
	unsigned int i = 0;

	for(i = 0; i < len; tmp++, i++)
	{
		hash = ((hash << 5) + hash) + (*tmp);
	}

	return hash;
}

int keyEqual(char* key1, unsigned int len1, char* key2, unsigned int len2)
{
	if(len1 != len2)
		return 0;
	else
	{
		if(len1 == 0)
		{
			if(key1 == NULL && key2 == NULL)
				return 1;
			else
				return 0;
		}
		else
		{
			int i = 0;
			for(i = 0; i < len1; i++)
			{
				if(key1[i] != key2[i])
					return 0;
			}
			return 1;
		}
	}
}

void createKey64bits(uint32_t first, uint32_t second, char* key)
{
	memcpy(key, &first, sizeof(uint32_t));
	memcpy(key+sizeof(uint32_t), &second, sizeof(uint32_t));
}

// TODO: For the grammar pool hash table use the URI string table ID
//       and Local Name ID directly as a key. That is, the key is a
//       pair of two uint32_t instead of 64 bits chars. This will make the
//       equality check much faster. Use the functions below. createKey64bits()
//       will no longer be needed. Specialization of the hash table code would be
//       required.

// http://www.concentric.net/~ttwang/tech/inthash.htm
uint32_t hash6432shift(uint32_t first, uint32_t second)
{
	uint64_t key = (((uint64_t) first)<<32) & ((uint64_t) second);

	key = (~key) + (key << 18); // key = (key << 18) - key - 1;
	key = key^(key >> 31);
	key = key * 21; // key = (key + (key << 2)) + (key << 4);
	key = key ^ (key >> 11);
	key = key + (key << 6);
	key = key ^ (key >> 22);
	return (uint32_t) key;
}

int pairKeyEqual(uint32_t uri1, uint32_t ln1, uint32_t uri2, uint32_t ln2)
{
	return uri1 == uri2 && ln1 == ln2;
}
