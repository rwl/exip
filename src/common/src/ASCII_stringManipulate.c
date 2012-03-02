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
 * @file ASCII_stringManipulate.c
 * @brief String manipulation functions used for UCS <-> ASCII transformations
 * Note! This file is platform dependent.
 * @date Sep 3, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "stringManipulate.h"
#include "memManagement.h"

#define PARSING_STRING_MAX_LENGTH 100

errorCode allocateStringMemory(CharType** str, size_t UCSchars, AllocList* memList)
{
	(*str) = (CharType*) memManagedAllocate(memList, sizeof(CharType)*UCSchars);
	if((*str) == NULL)
		return MEMORY_ALLOCATION_ERROR;
	return ERR_OK;
}

/**
 * Simple translation working only for ASCII characters
 */
errorCode writeCharToString(String* str, uint32_t code_point, size_t* writerPosition)
{
	if(*writerPosition >= str->length)
		return OUT_OF_BOUND_BUFFER;
	str->str[*writerPosition] = (CharType) code_point;
	*writerPosition += 1;
	return ERR_OK;
}

void getEmptyString(String* emptyStr)
{
	emptyStr->length = 0;
	emptyStr->str = NULL;
}

char isStringEmpty(const String* str)
{
	if(str == NULL || str->length == 0)
		return 1;
	return 0;
}

errorCode asciiToString(const char* inStr, String* outStr, AllocList* memList, unsigned char clone)
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

char stringEqual(const String str1, const String str2)
{
	if(str1.length != str2.length)
		return 0;
	else // The strings have the same length
	{
		size_t i;
		for(i = 0; i < str1.length; i++)
		{
			if(str1.str[i] != str2.str[i])
				return 0;
		}
		return 1;
	}
}

char stringEqualToAscii(const String str1, const char* str2)
{
	if(str1.length != strlen(str2))
		return 0;
	else // The strings have the same length
	{
		size_t i;
		for(i = 0; i < str1.length; i++)
		{
			if(str1.str[i] != str2[i])
				return 0;
		}
		return 1;
	}
}

int stringCompare(const String str1, const String str2)
{
	/* Check for NULL string pointers */
	if(str1.str == NULL)
	{
		if(str2.str == NULL)
			return 0;
		return -1;
	}
	else if(str2.str == NULL)
		return 1;
	else // None of the strings is NULL
	{
		int diff;
		size_t i;
		for(i = 0; i < str1.length && i < str2.length; i++)
		{
			diff = str1.str[i] - str2.str[i];
			if(diff)
				return diff; // There is a difference in the characters at index i
		}
		/* Up to index i the strings have the same characters and might differ only in length*/
		return str1.length - str2.length;
	}
}

// TODO: consider removing the OUT_OF_BOUND_BUFFER check: the length of the string must be valid
// => return void

errorCode readCharFromString(const String* str, size_t* readerPosition, uint32_t* UCScp)
{
	if(*readerPosition >= str->length)
		return OUT_OF_BOUND_BUFFER;
	*UCScp = (uint32_t) str->str[*readerPosition];
	*readerPosition += 1;
	return ERR_OK;
}

errorCode cloneString(const String* src, String* newStr, AllocList* memList)
{
	if(newStr == NULL)
		return NULL_POINTER_REF;
	newStr->str = memManagedAllocate(memList, sizeof(CharType)*src->length);
	if(newStr->str == NULL)
		return MEMORY_ALLOCATION_ERROR;
	newStr->length = src->length;
	memcpy(newStr->str, src->str, src->length);
	return ERR_OK;
}

size_t getIndexOfChar(const String* src, CharType sCh)
{
	size_t i;

	for(i = 0; i < src->length; i++)
	{
		if(src->str[i] == sCh)
			return i;
	}
	return SIZE_MAX;
}

errorCode stringToInteger(const String* src, int* number)
{
	char buff[PARSING_STRING_MAX_LENGTH];
	long result;
	char *endPointer;

	if(src->length == 0 || src->length >= PARSING_STRING_MAX_LENGTH)
		return INVALID_STRING_OPERATION;

	memcpy(buff, src->str, src->length);
	buff[src->length] = '\0';

	result = strtol(buff, &endPointer, 10);

	if(result == LONG_MAX || result == LONG_MIN || *src->str == *endPointer)
		return INVALID_STRING_OPERATION;

	if(result >= INT_MAX || result <= INT_MIN)
		return OUT_OF_BOUND_BUFFER;

	*number = (int) result;

	return ERR_OK;
}

#if EXIP_DEBUG == ON

void printString(const String* inStr)
{
	size_t i = 0;
	if(inStr->length == 0)
		return;
	for(i = 0; i < inStr->length; i++)
	{
		DEBUG_CHAR_OUTPUT(inStr->str[i]);
	}
}

#endif /* EXIP_DEBUG */
