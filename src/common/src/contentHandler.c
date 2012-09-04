/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file contentHandler.c
 * @brief Implementation of help functions for contentHandler interface
 * @date Oct 22, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "contentHandler.h"

void initContentHandler(ContentHandler* handler)
{
	handler->attribute = NULL;
	handler->binaryData = NULL;
	handler->booleanData = NULL;
	handler->dateTimeData = NULL;
	handler->decimalData = NULL;
	handler->endDocument = NULL;
	handler->endElement = NULL;
	handler->error = NULL;
	handler->fatalError = NULL;
	handler->floatData = NULL;
	handler->intData = NULL;
	handler->processingInstruction = NULL;
	handler->namespaceDeclaration = NULL;
	handler->selfContained = NULL;
	handler->startDocument = NULL;
	handler->startElement = NULL;
	handler->stringData = NULL;
	handler->warning = NULL;
}
