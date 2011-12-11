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
 * @file eventsEXI.h
 * @brief Definition and functions for EXI Event Types
 * @date Sep 7, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */
#ifndef EVENTTYPESEXI_H_
#define EVENTTYPESEXI_H_

#include "errorHandle.h"
#include "procTypes.h"

/**
 * @brief Serialize an event code to an EXI stream
 * @param[in, out] strm EXI bit stream
 * @param[in] currentRule the current grammar rule
 * @param[in] codeLength the number of parts in the event code to be written
 * @param[in] codeLastPart the last part of the event code
 * @return EventCode the newly created Event Code
 */
errorCode writeEventCode(EXIStream* strm, GrammarRule* currentRule, unsigned char codeLength, size_t codeLastPart);

/**
 * @brief Creates EXI event given only EventType.
 * The value content type is set to VALUE_TYPE_NONE
 *
 * @param[in] eType EXI EventType
 * @return EXIEvent with value content type VALUE_TYPE_NONE
 */
EXIEvent getEventDefType(EventType eType);

/**
 * @brief Checks if two EXI events are equal
 *
 * @param[in] e1 first EXIEvent
 * @param[in] e2 second EXIEvent
 * @return 0 if not equal, 1 otherwise
 */
unsigned char eventsIdentical(EXIEvent e1, EXIEvent e2);

/**
 * @brief Checks if two value type classes are equal
 *
 * @param[in] t1 first EXIType
 * @param[in] t2 second EXIType
 * @return 0 if not equal, 1 otherwise
 */
unsigned char valueTypeClassesEqual(EXIType t1, EXIType t2);

#endif /* EVENTTYPESEXI_H_ */
