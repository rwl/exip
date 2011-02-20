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
 * @file sortingAlgorithms.c
 * @brief Implementation for some sorting algorithms used for lexicographical sorting
 *
 * @date Jan 26, 2011
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

#include "sortingAlgorithms.h"

void insertionSort(unsigned int array[], size_t length, comapreFunc cmpFunc, void* args)
{
	size_t indx;
	unsigned int cur_val;
	unsigned int prev_val;

	if(length <= 1)
		return;

	prev_val = array[0];

	for (indx = 1; indx < length; indx++)
	{
		cur_val = array[indx];
		if((*cmpFunc)(prev_val, cur_val, args) > 0)
		{
			/* out of order: array[indx-1] > array[indx] */
			size_t indx2;
			array[indx] = prev_val; /* move up the larger item first */

			/* find the insertion point for the smaller item */
			for (indx2 = indx - 1; indx2 > 0;)
			{
				unsigned int temp_val = array[indx2 - 1];
				if((*cmpFunc)(temp_val, cur_val, args) > 0)
				{
					array[indx2--] = temp_val;
					/* still out of order, move up 1 slot to make room */
				}
				else
					break;
			}
			array[indx2] = cur_val; /* insert the smaller item right here */
		}
		else
		{
			/* in order, advance to next element */
			prev_val = cur_val;
		}
	}
}
