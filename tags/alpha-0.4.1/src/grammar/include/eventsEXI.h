/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file eventsEXI.h
 * @brief Definition and functions for EXI Event Types
 * @date Sep 7, 2010
 * @author Rumen Kyusakov
 * @version 0.4
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
errorCode writeEventCode(EXIStream* strm, GrammarRule* currentRule, unsigned char codeLength, Index codeLastPart);

/**
 * @brief Checks if two value type classes are equal
 *
 * @param[in] t1 first EXIType
 * @param[in] t2 second EXIType
 * @return 0 if not equal, 1 otherwise
 */
unsigned char valueTypeClassesEqual(EXIType t1, EXIType t2);

#endif /* EVENTTYPESEXI_H_ */
