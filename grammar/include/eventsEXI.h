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
 * @file eventTypesEXI.h
 * @brief Definition and functions for EXI Event Types
 * @date Sep 7, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */
#ifndef EVENTTYPESEXI_H_
#define EVENTTYPESEXI_H_

#include "errorHandle.h"

#define EVENT_SD       0
#define EVENT_ED       1
#define EVENT_SE_QNAME 2
#define EVENT_SE_URI   3
#define EVENT_SE_ALL   4
#define EVENT_EE       5
#define EVENT_AT_QNAME 6
#define EVENT_AT_URI   7
#define EVENT_AT_ALL   8
#define EVENT_CH       9
#define EVENT_NS      10
#define EVENT_CM      11
#define EVENT_PI      12
#define EVENT_DT      13
#define EVENT_ER      14
#define EVENT_SC      15


/****************************************
 * Name           |   Notation   | Value
 * -------------------------------------
 * Start Document |      SD      |  0
 * End Document   |      ED      |  1
 * Start Element  |  SE( qname ) |  2
 * Start Element  |  SE( uri:* ) |  3
 * Start Element  |  SE( * )	 |  4
 * End Element	  |      EE      |  5
 * Attribute	  |  AT( qname ) |  6
 * Attribute      |  AT( uri:* ) |  7
 * Attribute      |  AT( * )     |  8
 * Characters	  |      CH      |  9
 * Nm-space Decl  |	     NS	     | 10
 * Comment	      |      CM      | 11
 * Proc. Instr.   |      PI      | 12
 * DOCTYPE	      |      DT      | 13
 * Entity Ref.    |      ER      | 14
 * Self Contained |      SC      | 15
 ****************************************/
typedef unsigned char EventType;


struct EventCode
{
	unsigned int code[3];
	unsigned char size; // The number of integers constituting the EventCode
};

typedef struct EventCode EventCode;

/**
 * @brief Creates an EventCode instance giving its 3 integer parts
 * @param[in] first first part
 * @param[in] second second part
 * @param[in] third third part
 * @return EventCode the newly created Event Code
 */
EventCode getEventCode3(unsigned int first, unsigned int second, unsigned int third);

/**
 * @brief Creates an EventCode instance giving its 2 integer parts
 * @param[in] first first part
 * @param[in] second second part
 * @return EventCode the newly created Event Code
 */
EventCode getEventCode2(unsigned int first, unsigned int second);

/**
 * @brief Creates an EventCode instance giving its 1 integer parts
 * @param[in] first first part
 * @return EventCode the newly created Event Code
 */
EventCode getEventCode1(unsigned int first);

#endif /* EVENTTYPESEXI_H_ */
