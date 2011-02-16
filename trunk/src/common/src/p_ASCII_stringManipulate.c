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
 * @file p_ASCII_stringManipulate.c
 * @brief String manipulation functions used for UCS <-> ASCII transformations
 * Note! This file is platform dependent.
 * @date Sep 3, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "stringManipulate.h"
#include "memManagement.h"
#include <string.h>
#include <stdio.h>

errorCode allocateStringMemory(CharType** str, uint32_t UCSchars, AllocList* memList)
{
	(*str) = (CharType*) memManagedAllocate(memList, sizeof(CharType)*UCSchars);
	if((*str) == NULL)
		return MEMORY_ALLOCATION_ERROR;
	return ERR_OK;
}

/**
 * Simple translation working only for ASCII characters
 */
errorCode UCSToChar(uint32_t code_point, CharType* ch)
{
	*ch = (CharType) code_point;
	return ERR_OK;
}

/**
 * Simple translation working only for ASCII characters
 */
errorCode writeCharToString(StringType* str, uint32_t code_point, uint32_t UCSposition)
{
	str->str[UCSposition] = (CharType) code_point;
	return ERR_OK;
}

errorCode getEmptyString(StringType* emptyStr)
{
	emptyStr->length = 0;
	emptyStr->str = NULL;

	return ERR_OK;
}

char isStrEmpty(const StringType* str)
{
	if(str == NULL || str->length == 0)
		return 1;
	return 0;
}

errorCode asciiToString(const char* inStr, StringType* outStr, AllocList* memList, unsigned char clone)
{
	outStr->length = strlen(inStr);
	if(outStr->length > 0)  // If == 0 -> empty string
	{
		if(clone == FALSE)
		{
			outStr->str = (CharType*) inStr;
			return ERR_OK;
		}
		else
		{
			outStr->str = (CharType*) memManagedAllocate(memList, sizeof(CharType)*(outStr->length));
			if(outStr->str == NULL)
				return MEMORY_ALLOCATION_ERROR;
			memcpy(outStr->str, inStr, outStr->length);
			return ERR_OK;
		}
	}
	else
		outStr->str = NULL;
	return ERR_OK;
}

char str_equal(const StringType str1, const StringType str2)
{
	if(str1.length != str2.length)
		return 0;
	else
	{
		if(str1.length == 0)
		{
			if(str1.str == NULL && str2.str == NULL)
				return 1;
			else
				return 0;
		}
		else
		{
			int i = 0;
			for(i = 0; i < str1.length; i++)
			{
				if(str1.str[i] != str2.str[i])
					return 0;
			}
			return 1;
		}
	}
}

char strEqualToAscii(const StringType str1, char* str2)
{
	if(str1.length != strlen(str2))
		return 0;
	else
	{
		int i = 0;
		for(i = 0; i < str1.length; i++)
		{
			if(str1.str[i] != str2[i])
				return 0;
		}
		return 1;
	}
}

int str_compare(const StringType str1, const StringType str2)
{
	if((char*) str1.str == NULL)
	{
		if((char*) str2.str == NULL)
			return 0;
		else
			return -1;
	}
	else if((char*) str2.str == NULL)
		return 1;

	return strcmp((char*) str1.str, (char*) str2.str);

}

errorCode getUCSCodePoint(const StringType* str, uint32_t charIndex, uint32_t* UCScp)
{
	if(str->length <= charIndex)
		return OUT_OF_BOUND_BUFFER;
	*UCScp = (unsigned int) str->str[charIndex];
	return ERR_OK;
}

void printString(const StringType* inStr)
{
	int i = 0;
	if(inStr->length == 0)
		return;
	for(i = 0; i < inStr->length; i++)
	{
		DEBUG_CHAR_OUTPUT(inStr->str[i]);
	}
}
