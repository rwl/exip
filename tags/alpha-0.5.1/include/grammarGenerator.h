/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file grammarGenerator.h
 * @brief Definition and functions for generating Schema-informed Grammar definitions
 * @date Nov 22, 2010
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#ifndef GRAMMARGENERATOR_H_
#define GRAMMARGENERATOR_H_

#include "errorHandle.h"
#include "procTypes.h"

/** @name Supported schema formats
 * Such as XML-XSD, EXI-XSD, DTD or any other schema representation
 */
enum SchemaFormat
{
	SCHEMA_FORMAT_XSD_EXI   = 0,
	SCHEMA_FORMAT_XSD_XML   = 1,
	SCHEMA_FORMAT_DTD       = 2,
	SCHEMA_FORMAT_RELAX_NG  = 3
};

typedef enum SchemaFormat SchemaFormat;

/**
 * @brief Given a set of XML schemes, generates all Schema-informed Element and Type Grammars
 * as well as the pre-populated string tables (i.e. EXIPSchema instance)
 * The current version supports only SCHEMA_FORMAT_XSD_EXI format of the XML schema.
 *
 * @param[in] buffers an array of input buffers holding the representation of the schema.
 * 			  Each buffer refers to one schema file.
 * 			  The first buffer (buffers[0]) MUST be the main XSD document and the other buffers
 * 			  should be all the XSDs that are referenced from the main XSD document.
 * @param[in] bufCount the number of buffers in the array
 * @param[in] schemaFormat EXI, XSD, DTD or any other schema representation supported
 * @param[in] opt options used for EXI schemaFormat - otherwise NULL. If options are set then they will be used
 * for processing the EXI XSD stream although no options are specified in the EXI header. If there are
 * options defined in the EXI header of the XSD stream then this parameter must be NULL.
 * @param[out] schema the resulted schema information used for processing EXI streams
 * @return Error handling code
 */
errorCode generateSchemaInformedGrammars(BinaryBuffer* buffers, unsigned int bufCount, SchemaFormat schemaFormat, EXIOptions* opt, EXIPSchema* schema);

/**
 * @brief Frees all the memory allocated by an EXIPSchema object
 * @param[in] schema the schema containing the EXI grammars to be freed
 */
void destroySchema(EXIPSchema* schema);

#endif /* GRAMMARGENERATOR_H_ */
