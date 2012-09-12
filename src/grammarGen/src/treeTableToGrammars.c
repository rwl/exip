/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file treeTableToGrammars.c
 * @brief Implementation of functions for converting a TreeTable structure containing the schema definitions to EXIPGrammar object
 * @date Mar 13, 2012
 * @author Rumen Kyusakov
 * @author Robert Cragie
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "treeTableSchema.h"
#include "stringManipulate.h"
#include "genUtils.h"
#include "grammarRules.h"
#include "memManagement.h"
#include "initSchemaInstance.h"
#include "grammars.h"

#define DEFAULT_GLOBAL_QNAME_COUNT 200

extern EXIGrammar static_grammar_empty;

struct GlobalElemQNameTable
{
	DynArray dynArray;
	QNameID* qname;
	Index count;
};

typedef struct GlobalElemQNameTable GlobalElemQNameTable;

/**
 * Context/State data used to generate EXIPSchema grammars from a source TreeTable 
 * (schema tree).
 */
struct buildContext
{
	EXIPSchema* schema;
	GlobalElemQNameTable gElTbl;
	Index emptyGrIndex;			// The empty grammar index in the grammar table
	String emptyString;
	AllocList tmpMemList; 		// Temporary allocations during the schema creation
};

typedef struct buildContext BuildContext;

// Functions for handling of schema elements (defined in the global scope)
// START
/**
 * Given a global or local element definition (isGlobal == TRUE/FALSE),
 * this function builds (if not already built) its associated type grammar
 * and return the index of that type grammar in the grIndex parameter.
 * The type grammar is then used for processing of the corresponding
 * SE(QName) productions.
 */
static errorCode handleElementEl(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* entry, unsigned char isGlobal, Index* grIndex);

/**
 * Builds a simple type grammar if it is not already built.
 * The grammar is stored in the SchemaGrammarTable and linked in the
 * string tables under the qname corresponding to the qname of the simple type
 */
static errorCode handleSimpleTypeEl(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* entry);

/**
 * Builds a complex type grammar if it is not already built.
 * The grammar is stored in the SchemaGrammarTable and linked in the
 * string tables under the qname corresponding to the qname of the complex type
 */
static errorCode handleComplexTypeEl(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* entry);

// END - handling of schema elements

// Functions for converting schema definitions to protogrammars
// START

/**
 * Builds all the attribute uses of a complex type definition or extension.
 * The complex types define:
 * -# a contentType that includes
 *    local elements within model groups (sequences, choices, groups etc.)
 * -# attribute uses
 */
static errorCode getContentTypeProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* entry, ProtoGrammar** content);

/**
 * Builds all the attribute uses of a complex type definition or extension.
 * The complex types define:
 * -# a contentType that includes
 *    local elements within model groups (sequences, choices, groups etc.)
 * -# attribute uses
 */
static errorCode getAttributeUseProtoGrammars(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* entry, ProtoGrammarArray* attrUseArray, String** attrWildcardNS);

/**
 * For local scope elements, this function builds the Particle Grammar of the element definition
 * that is then used for creating the grammars for
 * model groups (sequences, choices, groups etc.)
 */
static errorCode getElementTermProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* elementEntry, Index grIndex, ProtoGrammar** elTerm);

/**
 * Given an attribute use entry this function builds the corresponding
 * attribute use proto grammar.
 */
static errorCode getAttributeProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* attrEntry, ProtoGrammar** attr);

/**
 * Given a Simple Type entry this function builds the corresponding
 * simple type proto grammar.
 */
static errorCode getSimpleTypeProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* simpleEntry, ProtoGrammar** simplType);

/**
 * Given a Simple Content entry this function builds the corresponding
 * simple Content proto grammar.
 */
static errorCode getSimpleContentProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* sContEntry, ProtoGrammar** sCont);

/**
 * Given a Complex Type entry this function builds the corresponding
 * Complex Type proto grammar.
 */
static errorCode getComplexTypeProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* complEntry, ProtoGrammar** complType);

/**
 * Given a Complex Content entry this function builds the corresponding
 * Complex Content proto grammar.
 */
static errorCode getComplexContentProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* cConEntry, ProtoGrammar** cCont);

/**
 * Given a Sequence entry this function builds the corresponding
 * Sequence proto grammar.
 */
static errorCode getSequenceProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* seqEntry, ProtoGrammar** seq);

/**
 * Given an Any entry this function builds the corresponding
 * Any proto grammar.
 */
static errorCode getAnyProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* anyEntry, ProtoGrammar** any);

/**
 * Given a Choice entry this function builds the corresponding
 * Choice proto grammar.
 */
static errorCode getChoiceProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* chEntry, ProtoGrammar** choice);

/**
 * Given a All entry this function builds the corresponding
 * All proto grammar.
 */
static errorCode getAllProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* allEntry, ProtoGrammar** all);

/**
 * Given a group entry this function builds the corresponding
 * group particle proto grammar.
 */
static errorCode getGroupProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* grEntry, ProtoGrammar** group);

/**
 * Given a ComplexContent Extension entry this function builds the corresponding
 * Extension proto grammar.
 */
static errorCode getExtensionComplexProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* extEntry, ProtoGrammar** ext);

/**
 * Given a SimpleContent Extension entry this function builds the corresponding
 * Extension proto grammar.
 */
static errorCode getExtensionSimpleProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* extEntry, ProtoGrammar** ext);

/**
 * Given a SimpleContent Restriction entry this function builds the corresponding
 * Restriction proto grammar.
 */
static errorCode getRestrictionSimpleProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* resEntry, ProtoGrammar** restr);

/**
 * Given a ComplexContent Restriction entry this function builds the corresponding
 * Restriction proto grammar.
 */
static errorCode getRestrictionComplexProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* resEntry, ProtoGrammar** restr);

// END - converting schema definitions to protogrammars

// Helper functions

/** Sorts an array of attribute use proto-grammars */
static void sortAttributeUseGrammars(ProtoGrammarArray* attrUseArray);

/** Used by sortAttributeUseGrammars() */
static int compareAttrUse(const void* attrPG1, const void* attrPG2);

/** Given a string value of an Occurance Attribute this function
 * converts it to a int representation (outInt).
 * the "unbounded" value is encoded as -1
 * */
static errorCode parseOccuranceAttribute(const String occurance, int* outInt);

/** Given a simple type QName and TreeTable entry determine the
 * corresponding typeId.
 * For build-in types the typeEntry can be NULL
 * */
static errorCode getTypeId(BuildContext* ctx, const QNameID typeQnameId, TreeTableEntry* typeEntry, TreeTable* treeT, Index* typeId);

/**
 * Given a ProtoGrammar this function assigns the event codes and
 * converts it to an EXIGrammar. The EXIGrammar is then stored in
 * the SchemaGrammarTable of the EXIPSchema object.
 * The index to the grammar is returned in grIndex parameter
 */
static errorCode storeGrammar(BuildContext* ctx, QNameID qnameID, ProtoGrammar* pGrammar, unsigned char isNillable, Index* grIndex);

static void sortGlobalElemQnameTable(GlobalElemQNameTable *gElTbl);

static void sortEnumTable(EXIPSchema *schema);

errorCode convertTreeTablesToExipSchema(TreeTable* treeT, unsigned int count, EXIPSchema* schema)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned int i = 0;
	unsigned int j = 0;
	BuildContext ctx;
	TreeTableEntry* entry;
	Index grIndex; // Index of the grammar in the schemaGrammarTable

	ctx.schema = schema;
	ctx.emptyGrIndex = INDEX_MAX;
	getEmptyString(&ctx.emptyString);

	tmp_err_code = initAllocList(&ctx.tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createDynArray(&ctx.gElTbl.dynArray, sizeof(QNameID), DEFAULT_GLOBAL_QNAME_COUNT, &ctx.tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	/** For every tree table */
	for(i = 0; i < count; i++)
	{
		/** For every global entry in the tree table */
		for(j = 0; j < treeT[i].count; j++)
		{
			entry = &treeT[i].tree[j];
			switch(entry->element)
			{
				case ELEMENT_ELEMENT:
					tmp_err_code = handleElementEl(&ctx, &treeT[i], entry, TRUE, &grIndex);
					break;
				case ELEMENT_SIMPLE_TYPE:
					tmp_err_code = handleSimpleTypeEl(&ctx, &treeT[i], entry);
					break;
				case ELEMENT_COMPLEX_TYPE:
					tmp_err_code = handleComplexTypeEl(&ctx, &treeT[i], entry);
					break;
				case ELEMENT_GROUP:
					// The model groups are only needing when referenced within a complex type definition
					tmp_err_code = ERR_OK;
					break;
				case ELEMENT_ATTRIBUTE_GROUP:
					// The attribute groups are only needing when referenced within a complex type definition
					tmp_err_code = ERR_OK;
					break;
				case ELEMENT_ATTRIBUTE:
					// AT (*) in schema-informed grammars bears an untyped value unless there is a
					// global attribute definition available for the qname of the attribute.
					// When a global attribute definition is available the attribute value is
					// represented according to the datatype of the global attribute.
					// TODO: There is a need for array of global attributes in the EXIPSchema object.
					//       This array must be sorted according to qname
					tmp_err_code = ERR_OK;
					break;
				case ELEMENT_IMPORT:
					//TODO: implement validation checks
					//		1) the namespace of an <import> element must be a target namespace of some tree table
					tmp_err_code = ERR_OK;
					break;
				case ELEMENT_INCLUDE:
					tmp_err_code = ERR_OK;
					break;
				case ELEMENT_REDEFINE:
					tmp_err_code = ERR_OK;
					break;
				case ELEMENT_NOTATION:
					tmp_err_code = ERR_OK;
					break;
				default:
					tmp_err_code = UNEXPECTED_ERROR;
					break;
			}

			if(tmp_err_code != ERR_OK)
				break;
		}
	}

	sortGlobalElemQnameTable(&ctx.gElTbl);

	tmp_err_code = createDocGrammar(schema, ctx.gElTbl.qname, ctx.gElTbl.count);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	sortEnumTable(schema);

	freeAllocList(&ctx.tmpMemList);

	return tmp_err_code;
}

static errorCode parseOccuranceAttribute(const String occurance, int* outInt)
{
	if(isStringEmpty(&occurance))
		*outInt = 1; // The default value
	else if(stringEqualToAscii(occurance, "unbounded"))
		*outInt = -1;
	else
	{
		return stringToInteger(&occurance, outInt);
	}

	return ERR_OK;
}

static int compareAttrUse(const void* attrPG1, const void* attrPG2)
{
	ProtoGrammar* a1 = (ProtoGrammar*) attrPG1;
	ProtoGrammar* a2 = (ProtoGrammar*) attrPG2;

	return compareQNameID(&(a1->rule[0].prod[0].qnameId), &(a2->rule[0].prod[0].qnameId));
}

static void sortAttributeUseGrammars(ProtoGrammarArray* attrUseArray)
{
	qsort(attrUseArray->pg, attrUseArray->count, sizeof(ProtoGrammar), compareAttrUse);
}

static errorCode getElementTermProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* elementEntry, Index grIndex, ProtoGrammar** elTerm)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ProtoGrammar elTermGrammar;
	ProtoGrammar* elParticleGrammar;
	int minOccurs = 1;
	int maxOccurs = 1;
	QNameID qNameID;

	*elTerm = NULL;

	if(isStringEmpty(&elementEntry->attributePointers[ATTRIBUTE_NAME]))
	{
		// The element does not have a name attribute.
		// Then it should have a ref="..." attribute.
		// It must not have a type attribute as well.
		if(!isStringEmpty(&elementEntry->attributePointers[ATTRIBUTE_REF]))
		{
			tmp_err_code = getTypeQName(ctx->schema, treeT, elementEntry->attributePointers[ATTRIBUTE_REF], &qNameID);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
			return UNEXPECTED_ERROR;
	}
	else
	{
		if(treeT->globalDefs.elemFormDefault == QUALIFIED || stringEqualToAscii(elementEntry->attributePointers[ATTRIBUTE_FORM], "qualified"))
			qNameID.uriId = treeT->globalDefs.targetNsId;
		else
			qNameID.uriId = 0;

		if(!lookupLn(&ctx->schema->uriTable.uri[qNameID.uriId].lnTable, elementEntry->attributePointers[ATTRIBUTE_NAME], &qNameID.lnId))
			return UNEXPECTED_ERROR;
	}

	tmp_err_code = parseOccuranceAttribute(elementEntry->attributePointers[ATTRIBUTE_MIN_OCCURS], &minOccurs);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	tmp_err_code = parseOccuranceAttribute(elementEntry->attributePointers[ATTRIBUTE_MAX_OCCURS], &maxOccurs);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(minOccurs < 0 || maxOccurs < -1)
		return UNEXPECTED_ERROR;

	tmp_err_code = createElementTermGrammar(&ctx->tmpMemList, &elTermGrammar, qNameID, grIndex);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	elParticleGrammar = (ProtoGrammar*) memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
	if(elParticleGrammar == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_err_code = createParticleGrammar(&ctx->tmpMemList, minOccurs, maxOccurs,
										 &elTermGrammar, elParticleGrammar);

	*elTerm = elParticleGrammar;

	return ERR_OK;
}

static errorCode handleElementEl(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* entry, unsigned char isGlobal, Index* grIndex)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	String type;
	QNameID elQNameID;
	QNameID typeQNameID;
	unsigned char isNillable = FALSE;

#if DEBUG_GRAMMAR_GEN == ON
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>Handle Element: "));
	printString(&entry->attributePointers[ATTRIBUTE_NAME]);
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (" of type: "));
	printString(&entry->attributePointers[ATTRIBUTE_TYPE]);
#endif

	type = entry->attributePointers[ATTRIBUTE_TYPE];

	// Validation checks
	if(isStringEmpty(&entry->attributePointers[ATTRIBUTE_NAME]))
	{
		// The element does not have a name attribute.
		// Then it should be local and have a ref="..." attribute.
		// It must not have a type attribute as well.
		if(isGlobal || isStringEmpty(&entry->attributePointers[ATTRIBUTE_REF]) || !isStringEmpty(&type))
			return UNEXPECTED_ERROR;
	}
	else
	{
		/**
		 * If the element in global or has form "qualified" then the element namespace is the target namespace
		 */
		if(isGlobal || treeT->globalDefs.elemFormDefault == QUALIFIED || stringEqualToAscii(entry->attributePointers[ATTRIBUTE_FORM], "qualified"))
			elQNameID.uriId = treeT->globalDefs.targetNsId;
		else
			elQNameID.uriId = 0;

		/** The element qname must be already in the string tables */
		if(!lookupLn(&ctx->schema->uriTable.uri[elQNameID.uriId].lnTable, entry->attributePointers[ATTRIBUTE_NAME], &elQNameID.lnId))
			return UNEXPECTED_ERROR;
	}

	if(isGlobal)
	{
		// Check if the element grammar is not already created
		if(GET_LN_URI_QNAME(ctx->schema->uriTable, elQNameID).elemGrammar != INDEX_MAX)
		{
			*grIndex = GET_LN_URI_QNAME(ctx->schema->uriTable, elQNameID).elemGrammar;
			return ERR_OK;
		}
	}

	if(!isStringEmpty(&entry->attributePointers[ATTRIBUTE_NILLABLE]) &&
		stringEqualToAscii(entry->attributePointers[ATTRIBUTE_NILLABLE], "true"))
	{
		isNillable = TRUE;
	}

	/* Check if the element declaration has a type="..." attribute */
	if(isStringEmpty(&type))
	{
		/*
		 * The element does not have a named type at this point
		 * i.e. there must be some complex/simple type as a child.
		 * Create the grammars for this QNameID
		 */
		ProtoGrammar* pg = NULL;

		/* If the element does not have a type then it should have either ref="..."
		 * attribute or an anonymous type definition */
		if(entry->child.entry == NULL)
			return UNEXPECTED_ERROR;
		else if(entry->child.entry->element == ELEMENT_SIMPLE_TYPE)
		{
			tmp_err_code = getSimpleTypeProtoGrammar(ctx, entry->child.treeT, entry->child.entry, &pg);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else if(entry->child.entry->element == ELEMENT_COMPLEX_TYPE)
		{
			tmp_err_code = getComplexTypeProtoGrammar(ctx, entry->child.treeT, entry->child.entry, &pg);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else if(entry->child.entry->element == ELEMENT_ELEMENT)
		{
			// In case of ref="..." attribute
			
			return handleElementEl(ctx, entry->child.treeT, entry->child.entry, TRUE, grIndex);
		}
		else
			return UNEXPECTED_ERROR;

		tmp_err_code = storeGrammar(ctx, elQNameID, pg, isNillable, grIndex);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/* If the element is globally defined -> store the index of its grammar in the
		 * LnEntry in the string tables */
		if(isGlobal == TRUE)
			GET_LN_URI_QNAME(ctx->schema->uriTable, elQNameID).elemGrammar = *grIndex;

	}
	else // The element has a particular named type
	{
		/* Find the QNameID of the type of this element */
		tmp_err_code = getTypeQName(ctx->schema, treeT, type, &typeQNameID);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		/*
		 * If the grammars for the type are already created, simply assign them to this QNameID,
		 * otherwise, create the type grammars first and then assign them to this QNameID.
		 */

		if(GET_LN_URI_QNAME(ctx->schema->uriTable, typeQNameID).typeGrammar == INDEX_MAX)
		{
			// the type definition is still not reached.
			// The type definition should be linked to the child of the element description in the tree table
			ProtoGrammar* pg = NULL;

			if(entry->child.entry == NULL)
				return UNEXPECTED_ERROR;
			else if(entry->child.entry->element == ELEMENT_SIMPLE_TYPE)
			{
				tmp_err_code = getSimpleTypeProtoGrammar(ctx, entry->child.treeT, entry->child.entry, &pg);
			}
			else if(entry->child.entry->element == ELEMENT_COMPLEX_TYPE)
			{
				tmp_err_code = getComplexTypeProtoGrammar(ctx, entry->child.treeT, entry->child.entry, &pg);
			}
			else
				return UNEXPECTED_ERROR;

			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			tmp_err_code = storeGrammar(ctx, typeQNameID, pg, isNillable, grIndex);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			/* Store the index of the type grammar in the
			 * LnEntry in the string tables */
			GET_LN_URI_QNAME(ctx->schema->uriTable, typeQNameID).typeGrammar = *grIndex;
		}

		/* If the element is globally defined -> store the index of its type grammar in the
		 * LnEntry in the string tables. Otherwise simply assigned the returned grammar index to
		 * the index of its type grammar */
		if(isGlobal == TRUE)
			GET_LN_URI_QNAME(ctx->schema->uriTable, elQNameID).elemGrammar = GET_LN_URI_QNAME(ctx->schema->uriTable, typeQNameID).typeGrammar;

		*grIndex = GET_LN_URI_QNAME(ctx->schema->uriTable, typeQNameID).typeGrammar;
	}

	/*
	 * If the element is global and it is part of the main tree table,
	 * add it to the GlobalElemQNameTable.
	 * This table is used to generate the schema-informed document grammar.
	 */
	if(isGlobal && treeT->globalDefs.isMain == TRUE)
	{
		Index dynElID;

		tmp_err_code = addDynEntry(&ctx->gElTbl.dynArray, &elQNameID, &dynElID, &ctx->tmpMemList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

static errorCode getAttributeProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* attrEntry, ProtoGrammar** attr)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	unsigned char required = 0;
	QNameID stQNameID;
	Index typeId;
	QNameID atQnameID;

	if (!isStringEmpty(&(attrEntry->attributePointers[ATTRIBUTE_USE])) &&
			stringEqualToAscii(attrEntry->attributePointers[ATTRIBUTE_USE], "required"))
	{
		required = TRUE;
	}
	if(treeT->globalDefs.attrFormDefault == QUALIFIED || stringEqualToAscii(attrEntry->attributePointers[ATTRIBUTE_FORM], "qualified"))
		atQnameID.uriId = treeT->globalDefs.targetNsId;
	else
		atQnameID.uriId = 0; // URI	0	"" [empty string]

	if(!lookupLn(&ctx->schema->uriTable.uri[atQnameID.uriId].lnTable, attrEntry->attributePointers[ATTRIBUTE_NAME], &atQnameID.lnId))
		return UNEXPECTED_ERROR;

	tmp_err_code = getTypeQName(ctx->schema, treeT, attrEntry->attributePointers[ATTRIBUTE_TYPE], &stQNameID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	*attr = (ProtoGrammar*) memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
	if(*attr == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_err_code = getTypeId(ctx, stQNameID, attrEntry->child.entry, attrEntry->child.treeT, &typeId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createAttributeUseGrammar(&ctx->tmpMemList, required, typeId, *attr, atQnameID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	return ERR_OK;
}

static errorCode getSimpleTypeProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* simpleEntry, ProtoGrammar** simplType)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	if(simpleEntry->child.entry == NULL)
	{
		/** Empty simple type*/
		*simplType = NULL;
	}
	else if(simpleEntry->child.entry->element == ELEMENT_RESTRICTION)
	{
		tmp_err_code = getRestrictionSimpleProtoGrammar(ctx, treeT, simpleEntry->child.entry, simplType);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else if(simpleEntry->child.entry->element == ELEMENT_LIST)
	{
		return NOT_IMPLEMENTED_YET;
	}
	else if(simpleEntry->child.entry->element == ELEMENT_UNION)
	{
		*simplType = (ProtoGrammar*) memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
		if(*simplType == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_err_code = createSimpleTypeGrammar(&ctx->tmpMemList, SIMPLE_TYPE_STRING, *simplType);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
		return UNEXPECTED_ERROR;

	return ERR_OK;
}

static errorCode handleSimpleTypeEl(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* entry)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

#if DEBUG_GRAMMAR_GEN == ON
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>Handle SimpleType: "));
	printString(&entry->attributePointers[ATTRIBUTE_NAME]);
#endif

	if(!isStringEmpty(&entry->attributePointers[ATTRIBUTE_NAME]))
	{
		// Named simple type
		QNameID stQNameID;
		ProtoGrammar* simpleProtoGrammar;

		stQNameID.uriId = treeT->globalDefs.targetNsId;

		/** The type qname must be in the string tables */
		if(!lookupLn(&ctx->schema->uriTable.uri[stQNameID.uriId].lnTable, entry->attributePointers[ATTRIBUTE_NAME], &stQNameID.lnId))
			return UNEXPECTED_ERROR;

		if(GET_LN_URI_QNAME(ctx->schema->uriTable, stQNameID).typeGrammar == INDEX_MAX)
		{
			// The EXIP grammars corresponding to this simple type are not yet created
			Index grIndex;

			tmp_err_code = getSimpleTypeProtoGrammar(ctx, treeT, entry, &simpleProtoGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			tmp_err_code = storeGrammar(ctx, stQNameID, simpleProtoGrammar, FALSE, &grIndex);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			GET_LN_URI_QNAME(ctx->schema->uriTable, stQNameID).typeGrammar = grIndex;

			// When Strict is True: If Tk either has named sub-types or is a simple type definition of which {variety} is union...
			if(entry->child.entry->element == ELEMENT_RESTRICTION)
			{
				QNameID baseTypeQnameId;

				tmp_err_code = getTypeQName(ctx->schema, entry->child.treeT, entry->child.entry->attributePointers[ATTRIBUTE_BASE], &baseTypeQnameId);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;

				SET_NAMED_SUB_TYPE_OR_UNION((GET_TYPE_GRAMMAR_QNAMEID(ctx->schema, baseTypeQnameId))->props);
			}
			else if(entry->child.entry->element == ELEMENT_UNION)
			{
				SET_NAMED_SUB_TYPE_OR_UNION((GET_TYPE_GRAMMAR_QNAMEID(ctx->schema, stQNameID))->props);
			}
		}
	}
	else
	{
		// The global simple types must have names
		return UNEXPECTED_ERROR;
	}

	return ERR_OK;
}

static errorCode getSimpleContentProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* sContEntry, ProtoGrammar** sCont)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	if(sContEntry->child.entry == NULL)
		return UNEXPECTED_ERROR;
	else if(sContEntry->child.entry->element == ELEMENT_RESTRICTION)
	{
		tmp_err_code = getRestrictionSimpleProtoGrammar(ctx, treeT, sContEntry->child.entry, sCont);
	}
	else if(sContEntry->child.entry->element == ELEMENT_EXTENSION)
	{
		tmp_err_code = getExtensionSimpleProtoGrammar(ctx, sContEntry->child.treeT, sContEntry->child.entry, sCont);
	}
	else
		tmp_err_code = UNEXPECTED_ERROR;

	return tmp_err_code;
}

/* entry should be a complex_type or extension element */
static errorCode getContentTypeProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* entry, ProtoGrammar** content)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	*content = NULL;

	if(entry->child.entry == NULL)
	{
		// empty complex_type or extension element
		tmp_err_code = ERR_OK;
	}
	else if(entry->element == ELEMENT_COMPLEX_TYPE && entry->child.entry->element == ELEMENT_SIMPLE_CONTENT)
	{
		tmp_err_code = getSimpleContentProtoGrammar(ctx, entry->child.treeT, entry->child.entry, content);
	}
	else if(entry->element == ELEMENT_COMPLEX_TYPE && entry->child.entry->element == ELEMENT_COMPLEX_CONTENT)
	{
		tmp_err_code = getComplexContentProtoGrammar(ctx, entry->child.treeT, entry->child.entry, content);
	}
	else if(entry->child.entry->element == ELEMENT_SEQUENCE)
	{
		tmp_err_code = getSequenceProtoGrammar(ctx, entry->child.treeT, entry->child.entry, content);
	}
	else if(entry->child.entry->element == ELEMENT_GROUP)
	{
		tmp_err_code = getGroupProtoGrammar(ctx, entry->child.treeT, entry->child.entry, content);
	}
	else if(entry->child.entry->element == ELEMENT_ALL)
	{
		tmp_err_code = getAllProtoGrammar(ctx, entry->child.treeT, entry->child.entry, content);
	}
	else if(entry->child.entry->element == ELEMENT_CHOICE)
	{
		tmp_err_code = getChoiceProtoGrammar(ctx, entry->child.treeT, entry->child.entry, content);
	}
	else if(entry->child.entry->element == ELEMENT_ATTRIBUTE ||
			entry->child.entry->element == ELEMENT_ATTRIBUTE_GROUP ||
			entry->child.entry->element == ELEMENT_ANY_ATTRIBUTE)
	{
		// Ignored -> attributes are handles by getAttributeUseProtoGrammars()
		tmp_err_code = ERR_OK;
	}
	else
		return UNEXPECTED_ERROR;

	return tmp_err_code;
}

/* entry should be a complex_type or extension or restriction element */
static errorCode getAttributeUseProtoGrammars(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* entry, ProtoGrammarArray* attrUseArray, String** attrWildcardNS)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	TreeTableEntry* attrUse = NULL;

	if(entry->child.entry != NULL)
	{
		if(entry->child.entry->element == ELEMENT_ATTRIBUTE)
			attrUse = entry->child.entry;
		else if(entry->child.entry->next != NULL && entry->child.entry->next->element == ELEMENT_ATTRIBUTE)
			attrUse = entry->child.entry->next;
		else if(entry->child.entry->element == ELEMENT_SIMPLE_CONTENT && entry->child.entry->child.entry != NULL && entry->child.entry->child.entry->element == ELEMENT_EXTENSION)
		{
			 if(entry->child.entry->child.entry->child.entry != NULL)
			 {
				 if(entry->child.entry->child.entry->child.entry->element == ELEMENT_ATTRIBUTE)
					 attrUse = entry->child.entry->child.entry->child.entry;
				 else if(entry->child.entry->child.entry->child.entry->next != NULL && entry->child.entry->child.entry->child.entry->next->element == ELEMENT_ATTRIBUTE)
					 attrUse = entry->child.entry->child.entry->child.entry->next;
			 }
		}

		if(attrUse != NULL)
		{
			ProtoGrammar* attrPG;
			Index entryId;
			do
			{
				if(attrUse->element == ELEMENT_ATTRIBUTE)
				{
					tmp_err_code = getAttributeProtoGrammar(ctx, treeT, attrUse, &attrPG);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;

					tmp_err_code = addDynEntry(&attrUseArray->dynArray, attrPG, &entryId, &ctx->tmpMemList);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				}
				else if(attrUse->element == ELEMENT_ATTRIBUTE_GROUP)
				{
					return NOT_IMPLEMENTED_YET;
				}
				else if(attrUse->element == ELEMENT_ANY_ATTRIBUTE)
				{
					*attrWildcardNS = &attrUse->attributePointers[ATTRIBUTE_NAMESPACE];
				}
				else
					return UNEXPECTED_ERROR;

				attrUse = attrUse->next;
			}
			while(attrUse != NULL);
		}

		if(entry->element == ELEMENT_COMPLEX_TYPE)
		{
			if((entry->child.entry->element == ELEMENT_COMPLEX_CONTENT) &&
			   (entry->child.entry->child.entry != NULL))
			{
				// Follow the extensions and restrictions.
				// entry->child.entry->child.entry->element is either ELEMENT_EXTENSION or ELEMENT_RESTRICTION
				tmp_err_code = getAttributeUseProtoGrammars(ctx, entry->child.entry->child.treeT, entry->child.entry->child.entry, attrUseArray, attrWildcardNS);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
	}

	if(entry->element == ELEMENT_EXTENSION)
	{
		/**
		 * @page deriving_complex_restriction Deriving complex types through restriction
		 * When deriving complex types through restriction you should repeat all the
		 * inherited attributes, although the XSD spec does not require that for some reason.
		 * This is yet another quirk in the XSD spec - all content elements should be repeated
		 * during restriction but not the attribute uses!?!?
		 * In any way, the schema code is much more comprehensible when the attribute uses are
		 * repeated but still this deviation might be revised in the future releases of the library.
		 */

		if(entry->supertype.entry->element == ELEMENT_COMPLEX_TYPE)
		{
			tmp_err_code = getAttributeUseProtoGrammars(ctx, entry->supertype.treeT, entry->supertype.entry, attrUseArray, attrWildcardNS);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}

	return ERR_OK;
}

static errorCode getComplexTypeProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* complEntry, ProtoGrammar** complType)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ProtoGrammarArray attrUseArray;
	ProtoGrammar* contentTypeGrammar = NULL;
	String* attrWildcardNS = NULL;

	tmp_err_code = getContentTypeProtoGrammar(ctx, treeT, complEntry, &contentTypeGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = createDynArray(&attrUseArray.dynArray, sizeof(ProtoGrammar), 10, &ctx->tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = getAttributeUseProtoGrammars(ctx, treeT, complEntry, &attrUseArray, &attrWildcardNS);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	sortAttributeUseGrammars(&attrUseArray);

	if(attrWildcardNS != NULL)
	{
		// There is an {attribute wildcard} ANY_ATTRIBUTE
		NsTable nsTable;
		ProtoGrammar attrWildGrammar;
		Index dummyEntryId;
		Index i;
		QNameID qnameID;

		tmp_err_code = createDynArray(&nsTable.dynArray, sizeof(String), 5, &ctx->tmpMemList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = getNsList(&ctx->tmpMemList, treeT, *attrWildcardNS, &nsTable);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = createProtoGrammar(&ctx->tmpMemList, 2, 5, &attrWildGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = addEEProduction(&ctx->tmpMemList, &attrWildGrammar.rule[0]);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		attrWildGrammar.count = 1;

		tmp_err_code = addDynEntry(&attrUseArray.dynArray, &attrWildGrammar, &dummyEntryId, &ctx->tmpMemList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(nsTable.count == 0 ||		// default is "##any"
			(nsTable.count == 1 &&
					(stringEqualToAscii(nsTable.base[0], "##any") || stringEqualToAscii(nsTable.base[0], "##other"))
			)
		  )
		{
			for(i = 0; i < attrUseArray.count; i++)
			{
				qnameID.uriId = URI_MAX;
				qnameID.lnId = LN_MAX;
				tmp_err_code = addProduction(&ctx->tmpMemList, &attrUseArray.pg[i].rule[0], EVENT_AT_ALL, INDEX_MAX, qnameID, 0);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
		}
		else if(nsTable.count >= 1)
		{
			Index j;

			qnameID.lnId = LN_MAX;

			for(j = 0; j < nsTable.count; j++)
			{
				if(!lookupUri(&ctx->schema->uriTable, nsTable.base[j], &qnameID.uriId))
				 	return UNEXPECTED_ERROR;

				for(i = 0; i < attrUseArray.count; i++)
				{
					tmp_err_code = addProduction(&ctx->tmpMemList, &attrUseArray.pg[i].rule[0], EVENT_AT_URI, INDEX_MAX, qnameID, 0);
					if(tmp_err_code != ERR_OK)
						return tmp_err_code;
				}
			}
		}
		else
			return UNEXPECTED_ERROR;
	}

	if(contentTypeGrammar == NULL && attrUseArray.count == 0) // An empty element: <xsd:complexType />
	{
		*complType = NULL;
	}
	else
	{
		*complType = (ProtoGrammar*) memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
		if(*complType == NULL)
			return MEMORY_ALLOCATION_ERROR;

		tmp_err_code = createComplexTypeGrammar(&ctx->tmpMemList, &attrUseArray,
										        contentTypeGrammar, *complType);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

	return ERR_OK;
}

static errorCode handleComplexTypeEl(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* entry)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

#if DEBUG_GRAMMAR_GEN == ON
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>Handle ComplexType: "));
	printString(&entry->attributePointers[ATTRIBUTE_NAME]);
#endif

	if(!isStringEmpty(&entry->attributePointers[ATTRIBUTE_NAME]))
	{
		QNameID ctQNameID;

		ctQNameID.uriId = treeT->globalDefs.targetNsId;

		/** The type qname must be in the string tables */
		if(!lookupLn(&ctx->schema->uriTable.uri[ctQNameID.uriId].lnTable, entry->attributePointers[ATTRIBUTE_NAME], &ctQNameID.lnId))
			return UNEXPECTED_ERROR;

		if(GET_LN_URI_QNAME(ctx->schema->uriTable, ctQNameID).typeGrammar == INDEX_MAX)
		{
			// The EXIP grammars are not yet created
			ProtoGrammar* complType;
			Index grIndex;

			tmp_err_code = getComplexTypeProtoGrammar(ctx, treeT, entry, &complType);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			tmp_err_code = storeGrammar(ctx, ctQNameID, complType, FALSE, &grIndex);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			GET_LN_URI_QNAME(ctx->schema->uriTable, ctQNameID).typeGrammar = grIndex;

			// When Strict is True: If Tk either has named sub-types or is a simple type definition of which {variety} is union...
			if(entry->child.entry != NULL && entry->child.entry->child.entry != NULL)
			{
				if(entry->child.entry->element == ELEMENT_SIMPLE_CONTENT ||
						entry->child.entry->element == ELEMENT_COMPLEX_CONTENT)
				{
					if(entry->child.entry->child.entry->element == ELEMENT_RESTRICTION)
					{
						QNameID baseTypeQnameId;

						tmp_err_code = getTypeQName(ctx->schema, entry->child.entry->child.treeT, entry->child.entry->child.entry->attributePointers[ATTRIBUTE_BASE], &baseTypeQnameId);
						if(tmp_err_code != ERR_OK)
							return tmp_err_code;

						SET_NAMED_SUB_TYPE_OR_UNION((GET_TYPE_GRAMMAR_QNAMEID(ctx->schema, baseTypeQnameId))->props);
					}
					else if(entry->child.entry->child.entry->element == ELEMENT_EXTENSION)
					{
						SET_NAMED_SUB_TYPE_OR_UNION((GET_TYPE_GRAMMAR_QNAMEID(ctx->schema, ctQNameID))->props);
					}
				}
			}
		}

	}
	else
	{
		// The global complex types must have names
		return UNEXPECTED_ERROR;
	}

	return ERR_OK;
}

static errorCode getComplexContentProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* cConEntry, ProtoGrammar** cCont)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	if(cConEntry->child.entry == NULL)
		return UNEXPECTED_ERROR;
	else if(cConEntry->child.entry->element == ELEMENT_RESTRICTION)
	{
		tmp_err_code = getRestrictionComplexProtoGrammar(ctx, treeT, cConEntry->child.entry, cCont);
	}
	else if(cConEntry->child.entry->element == ELEMENT_EXTENSION)
	{
		tmp_err_code = getExtensionComplexProtoGrammar(ctx, treeT, cConEntry->child.entry, cCont);
	}
	else
		tmp_err_code = UNEXPECTED_ERROR;

	return tmp_err_code;
}

static errorCode getSequenceProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* seqEntry, ProtoGrammar** seq)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ProtoGrammar* particleGrammar = NULL;
	ProtoGrammar seqGrammar;
	ProtoGrammar* seqPartGrammar;
	int minOccurs = 1;
	int maxOccurs = 1;
	TreeTableEntry* nextIterator;
	GenericStack* pStack = NULL;

	tmp_err_code = parseOccuranceAttribute(seqEntry->attributePointers[ATTRIBUTE_MIN_OCCURS], &minOccurs);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	tmp_err_code = parseOccuranceAttribute(seqEntry->attributePointers[ATTRIBUTE_MAX_OCCURS], &maxOccurs);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(minOccurs < 0 || maxOccurs < -1)
		return UNEXPECTED_ERROR;

	nextIterator = seqEntry->child.entry;
	while(nextIterator != NULL)
	{
		if(nextIterator->element == ELEMENT_ELEMENT)
		{
			Index grIndex;

			tmp_err_code = handleElementEl(ctx, treeT, nextIterator, FALSE, &grIndex);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			
			tmp_err_code = getElementTermProtoGrammar(ctx, treeT, nextIterator, grIndex, &particleGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else if(nextIterator->element == ELEMENT_GROUP)
		{
			tmp_err_code = getGroupProtoGrammar(ctx, treeT, nextIterator, &particleGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else if(nextIterator->element == ELEMENT_CHOICE)
		{
			tmp_err_code = getChoiceProtoGrammar(ctx, treeT, nextIterator, &particleGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else if(nextIterator->element == ELEMENT_SEQUENCE)
		{
			tmp_err_code = getSequenceProtoGrammar(ctx, treeT, nextIterator, &particleGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else if(nextIterator->element == ELEMENT_ANY)
		{
			tmp_err_code = getAnyProtoGrammar(ctx, treeT, nextIterator, &particleGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
			return UNEXPECTED_ERROR;

		pushOnStack(&pStack, particleGrammar);
		nextIterator = nextIterator->next;
	}

	tmp_err_code = createSequenceModelGroupsGrammar(&ctx->tmpMemList, &pStack, &seqGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	seqPartGrammar = (ProtoGrammar*)memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
	if(seqPartGrammar == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_err_code = createParticleGrammar(&ctx->tmpMemList, minOccurs, maxOccurs, &seqGrammar, seqPartGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	*seq = seqPartGrammar;

	return ERR_OK;
}

static errorCode getAnyProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* anyEntry, ProtoGrammar** any)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ProtoGrammar wildTermGrammar;
	ProtoGrammar* wildParticleGrammar;
	int minOccurs = 1;
	int maxOccurs = 1;
	NsTable nsTable;

	tmp_err_code = createDynArray(&nsTable.dynArray, sizeof(String), 5, &ctx->tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = getNsList(&ctx->tmpMemList, treeT, anyEntry->attributePointers[ATTRIBUTE_NAMESPACE], &nsTable);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = parseOccuranceAttribute(anyEntry->attributePointers[ATTRIBUTE_MIN_OCCURS], &minOccurs);
	tmp_err_code += parseOccuranceAttribute(anyEntry->attributePointers[ATTRIBUTE_MAX_OCCURS], &maxOccurs);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(minOccurs < 0 || maxOccurs < -1)
		return UNEXPECTED_ERROR;

	tmp_err_code = createWildcardTermGrammar(&ctx->tmpMemList, nsTable.base, nsTable.count, &ctx->schema->uriTable, &wildTermGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	wildParticleGrammar = (ProtoGrammar*)memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
	if(wildParticleGrammar == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_err_code = createParticleGrammar(&ctx->tmpMemList, minOccurs, maxOccurs,
			&wildTermGrammar, wildParticleGrammar);

	*any = wildParticleGrammar;

	return ERR_OK;
}

static errorCode getChoiceProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* chEntry, ProtoGrammar** choice)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ProtoGrammar choiceGrammar;
	ProtoGrammar* choicePartGrammar;
	TreeTableEntry* nextIterator;
	ProtoGrammarArray particleProtoGrammarArray;
	ProtoGrammar* particleGrammar = NULL;
	Index entryId;
	int minOccurs = 1;
	int maxOccurs = 1;

	tmp_err_code = parseOccuranceAttribute(chEntry->attributePointers[ATTRIBUTE_MIN_OCCURS], &minOccurs);
	tmp_err_code += parseOccuranceAttribute(chEntry->attributePointers[ATTRIBUTE_MAX_OCCURS], &maxOccurs);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(minOccurs < 0 || maxOccurs < -1)
		return UNEXPECTED_ERROR;

	tmp_err_code = createDynArray(&particleProtoGrammarArray.dynArray, sizeof(ProtoGrammar), 15, &ctx->tmpMemList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	nextIterator = chEntry->child.entry;
	while(nextIterator != NULL)
	{
		if(nextIterator->element == ELEMENT_ELEMENT)
		{
			Index grIndex;
			tmp_err_code = handleElementEl(ctx, treeT, nextIterator, FALSE, &grIndex);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;

			tmp_err_code = getElementTermProtoGrammar(ctx, treeT, nextIterator, grIndex, &particleGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else if(nextIterator->element == ELEMENT_GROUP)
		{
			tmp_err_code = getGroupProtoGrammar(ctx, treeT, nextIterator, &particleGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else if(nextIterator->element == ELEMENT_CHOICE)
		{
			tmp_err_code = getChoiceProtoGrammar(ctx, treeT, nextIterator, &particleGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else if(nextIterator->element == ELEMENT_SEQUENCE)
		{
			tmp_err_code = getSequenceProtoGrammar(ctx, treeT, nextIterator, &particleGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else if(nextIterator->element == ELEMENT_ANY)
		{
			tmp_err_code = getAnyProtoGrammar(ctx, treeT, nextIterator, &particleGrammar);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
			return UNEXPECTED_ERROR;

		tmp_err_code = addDynEntry(&particleProtoGrammarArray.dynArray, particleGrammar, &entryId, &ctx->tmpMemList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
		nextIterator = nextIterator->next;
	}

	tmp_err_code = createChoiceModelGroupsGrammar(&ctx->tmpMemList, &particleProtoGrammarArray, &choiceGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	choicePartGrammar = (ProtoGrammar*)memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
	if(choicePartGrammar == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_err_code = createParticleGrammar(&ctx->tmpMemList, minOccurs, maxOccurs, &choiceGrammar, choicePartGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	*choice = choicePartGrammar;

	return ERR_OK;
}

static errorCode getAllProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* allEntry, ProtoGrammar** all)
{
	return NOT_IMPLEMENTED_YET;
}

static errorCode getGroupProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* grEntry, ProtoGrammar** group)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ProtoGrammar* particleGrammar = NULL;
	ProtoGrammar* grPartGrammar;
	int minOccurs = 1;
	int maxOccurs = 1;

	tmp_err_code = parseOccuranceAttribute(grEntry->attributePointers[ATTRIBUTE_MIN_OCCURS], &minOccurs);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;
	tmp_err_code = parseOccuranceAttribute(grEntry->attributePointers[ATTRIBUTE_MAX_OCCURS], &maxOccurs);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	if(minOccurs < 0 || maxOccurs < -1)
		return UNEXPECTED_ERROR;

	// There should be a global group definition referenced through ref attribute
	if(grEntry->child.entry == NULL)
		return UNEXPECTED_ERROR;

	if(grEntry->child.entry->child.entry == NULL)
	{
		// empty group.
		// The content of 'group (global)' must match (annotation?, (all | choice | sequence)). Not enough
		// elements were found.
		return UNEXPECTED_ERROR;
	}
	else if(grEntry->child.entry->child.entry->element == ELEMENT_SEQUENCE)
	{
		tmp_err_code = getSequenceProtoGrammar(ctx, grEntry->child.entry->child.treeT, grEntry->child.entry->child.entry, &particleGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else if(grEntry->child.entry->child.entry->element == ELEMENT_CHOICE)
	{
		tmp_err_code = getChoiceProtoGrammar(ctx, grEntry->child.entry->child.treeT, grEntry->child.entry->child.entry, &particleGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else if(grEntry->child.entry->child.entry->element == ELEMENT_ALL)
	{
		tmp_err_code = getAllProtoGrammar(ctx, grEntry->child.entry->child.treeT, grEntry->child.entry->child.entry, &particleGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
		return UNEXPECTED_ERROR;

	grPartGrammar = (ProtoGrammar*)memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
	if(grPartGrammar == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_err_code = createParticleGrammar(&ctx->tmpMemList, minOccurs, maxOccurs, particleGrammar, grPartGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	*group = grPartGrammar;

	return ERR_OK;
}

static errorCode getExtensionSimpleProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* extEntry, ProtoGrammar** ext)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	QNameID baseTypeId;
	ProtoGrammar* resultProtoGrammar = NULL;
	Index typeId;

#if DEBUG_GRAMMAR_GEN == ON
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>Handle SimpleContent Extension: "));
	printString(&extEntry->attributePointers[ATTRIBUTE_BASE]);
#endif

	tmp_err_code = getTypeQName(ctx->schema, treeT, extEntry->attributePointers[ATTRIBUTE_BASE], &baseTypeId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	resultProtoGrammar = (ProtoGrammar*) memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
	if(resultProtoGrammar == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_err_code = getTypeId(ctx, baseTypeId, extEntry->supertype.entry, extEntry->supertype.treeT, &typeId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Extension from a simple type only
	tmp_err_code = createSimpleTypeGrammar(&ctx->tmpMemList, typeId, resultProtoGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	*ext = resultProtoGrammar;

	return ERR_OK;
}

static errorCode getExtensionComplexProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* extEntry, ProtoGrammar** ext)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	QNameID baseTypeId;
	ProtoGrammar* contentTypeGrammarBase;
	ProtoGrammar* contentTypeGrammarExt;
	ProtoGrammar* resultProtoGrammar = NULL;
	GenericStack* pStack = NULL;

#if DEBUG_GRAMMAR_GEN == ON
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>Handle ComplexContent Extension: "));
	printString(&extEntry->attributePointers[ATTRIBUTE_BASE]);
#endif

	tmp_err_code = getTypeQName(ctx->schema, treeT, extEntry->attributePointers[ATTRIBUTE_BASE], &baseTypeId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	resultProtoGrammar = (ProtoGrammar*) memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
	if(resultProtoGrammar == NULL)
		return MEMORY_ALLOCATION_ERROR;

	// Extension from a complex type only

	TreeTableEntry* base = extEntry->supertype.entry;
	if(base == NULL)
		return UNEXPECTED_ERROR;
	else if(base->element == ELEMENT_COMPLEX_TYPE)
	{
		tmp_err_code = handleComplexTypeEl(ctx, extEntry->supertype.treeT, base);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = getContentTypeProtoGrammar(ctx, extEntry->supertype.treeT, base, &contentTypeGrammarBase);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else if(base->element == ELEMENT_SIMPLE_TYPE)
	{
		// When <complexContent> is used, the base type must be a complexType. Base simpleType is an error.
		return UNEXPECTED_ERROR;
	}
	else
	{
		return UNEXPECTED_ERROR;
	}

	tmp_err_code = getContentTypeProtoGrammar(ctx, treeT, extEntry, &contentTypeGrammarExt);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	pushOnStack(&pStack, contentTypeGrammarBase);
	pushOnStack(&pStack, contentTypeGrammarExt);

	tmp_err_code = createSequenceModelGroupsGrammar(&ctx->tmpMemList, &pStack, resultProtoGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	*ext = resultProtoGrammar;

	return ERR_OK;
}

static errorCode getRestrictionSimpleProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* resEntry, ProtoGrammar** restr)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	ProtoGrammar* simpleRestrictedGrammar;
	QNameID baseTypeID;
	SimpleType newSimpleType;
	Index typeId;
	Index simpleTypeId;

	tmp_err_code = getTypeQName(ctx->schema, treeT, resEntry->attributePointers[ATTRIBUTE_BASE], &baseTypeID);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	tmp_err_code = getTypeId(ctx, baseTypeID, resEntry->supertype.entry, resEntry->supertype.treeT, &typeId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// TODO: check if there are cases when the EXI type changes after restriction
	newSimpleType.exiType = ctx->schema->simpleTypeTable.sType[typeId].exiType;
	newSimpleType.facetPresenceMask = ctx->schema->simpleTypeTable.sType[typeId].facetPresenceMask;
	// remove the presence of named subtype
	newSimpleType.facetPresenceMask = newSimpleType.facetPresenceMask & (~TYPE_FACET_NAMED_SUBTYPE_UNION);
	newSimpleType.maxInclusive = ctx->schema->simpleTypeTable.sType[typeId].maxInclusive;
	newSimpleType.minInclusive = ctx->schema->simpleTypeTable.sType[typeId].minInclusive;
	newSimpleType.maxLength = ctx->schema->simpleTypeTable.sType[typeId].maxLength;

	if(resEntry->child.entry != NULL)
	{
		if(resEntry->child.entry->element == ELEMENT_MIN_INCLUSIVE)
		{
			newSimpleType.facetPresenceMask = newSimpleType.facetPresenceMask | TYPE_FACET_MIN_INCLUSIVE;
			tmp_err_code = stringToInteger(&resEntry->child.entry->attributePointers[ATTRIBUTE_VALUE], &newSimpleType.minInclusive);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else if(resEntry->child.entry->element == ELEMENT_MAX_INCLUSIVE)
		{
			newSimpleType.facetPresenceMask = newSimpleType.facetPresenceMask | TYPE_FACET_MAX_INCLUSIVE;
			tmp_err_code = stringToInt64(&resEntry->child.entry->attributePointers[ATTRIBUTE_VALUE], &newSimpleType.maxInclusive);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else if(resEntry->child.entry->element == ELEMENT_MAX_LENGTH)
		{
			int ml = 0;
			newSimpleType.facetPresenceMask = newSimpleType.facetPresenceMask | TYPE_FACET_MAX_LENGTH;
			tmp_err_code = stringToInteger(&resEntry->child.entry->attributePointers[ATTRIBUTE_VALUE], &ml);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
			newSimpleType.maxLength = (unsigned int) ml;
		}
		else if(resEntry->child.entry->element == ELEMENT_ENUMERATION)
		{
			struct TreeTableEntry* enumEntry;
			unsigned int enumIter = 0;
			size_t valSize = 0;
			EnumDefinition eDef;
			Index elId;

			eDef.count = 0;
			eDef.values = NULL;
			/* The next index in the simpleTypeTable will be assigned to the newly created simple type
			 * containing the enumeration */
			eDef.typeId = ctx->schema->simpleTypeTable.count;

			newSimpleType.facetPresenceMask = newSimpleType.facetPresenceMask | TYPE_FACET_ENUMERATION;

			switch(ctx->schema->simpleTypeTable.sType[typeId].exiType)
			{
				case VALUE_TYPE_STRING:
					valSize = sizeof(String);
					break;
				case VALUE_TYPE_BOOLEAN:
					valSize = sizeof(char);
					break;
				case VALUE_TYPE_DATE_TIME:
					valSize = sizeof(EXIPDateTime);
					break;
				case VALUE_TYPE_DECIMAL:
					valSize = sizeof(Decimal);
					break;
				case VALUE_TYPE_FLOAT:
					valSize = sizeof(Float);
					break;
				case VALUE_TYPE_INTEGER:
					valSize = sizeof(Integer);
					break;
				case VALUE_TYPE_SMALL_INTEGER:
					valSize = sizeof(uint16_t);
					break;
			}

			enumEntry = resEntry->child.entry;
			while(enumEntry != NULL)
			{
				if(enumEntry->element == ELEMENT_ENUMERATION)
					eDef.count++;
				enumEntry = enumEntry->next;
			}

			eDef.values = memManagedAllocate(&ctx->schema->memList, valSize*(eDef.count));
			if(eDef.values == NULL)
				return MEMORY_ALLOCATION_ERROR;

			enumEntry = resEntry->child.entry;
			while(enumEntry != NULL)
			{
				if(enumEntry->element == ELEMENT_ENUMERATION)
				{
					switch(ctx->schema->simpleTypeTable.sType[typeId].exiType)
					{
						case VALUE_TYPE_STRING:
						{
							String tmpStr;

							tmp_err_code = cloneString(&enumEntry->attributePointers[ATTRIBUTE_VALUE], &tmpStr, &ctx->schema->memList);
							if(tmp_err_code != ERR_OK)
								return tmp_err_code;

							((String*) eDef.values)[enumIter].length = tmpStr.length;
							((String*) eDef.values)[enumIter].str = tmpStr.str;
						}
							break;
						case VALUE_TYPE_BOOLEAN:
							return NOT_IMPLEMENTED_YET;
							break;
						case VALUE_TYPE_DATE_TIME:
							return NOT_IMPLEMENTED_YET;
							break;
						case VALUE_TYPE_DECIMAL:
							return NOT_IMPLEMENTED_YET;
							break;
						case VALUE_TYPE_FLOAT:
							return NOT_IMPLEMENTED_YET;
							break;
						case VALUE_TYPE_INTEGER:
							return NOT_IMPLEMENTED_YET;
							break;
						case VALUE_TYPE_SMALL_INTEGER:
							return NOT_IMPLEMENTED_YET;
							break;
						default:
							return NOT_IMPLEMENTED_YET;
					}
				}
				enumEntry = enumEntry->next;
				enumIter++;
			}

			tmp_err_code = addDynEntry(&ctx->schema->enumTable.dynArray, &eDef, &elId, &ctx->schema->memList);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
		else
		{
			return NOT_IMPLEMENTED_YET;
		}
	}

	tmp_err_code = addDynEntry(&ctx->schema->simpleTypeTable.dynArray, &newSimpleType, &simpleTypeId, &ctx->schema->memList);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	simpleRestrictedGrammar = (ProtoGrammar*) memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
	if(simpleRestrictedGrammar == NULL)
		return MEMORY_ALLOCATION_ERROR;

	tmp_err_code = createSimpleTypeGrammar(&ctx->tmpMemList, simpleTypeId, simpleRestrictedGrammar);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	*restr = simpleRestrictedGrammar;

	return ERR_OK;
}

static errorCode getRestrictionComplexProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* resEntry, ProtoGrammar** restr)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	QNameID baseTypeId;

#if DEBUG_GRAMMAR_GEN == ON
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>Handle ComplexContent Restriction: "));
	printString(&resEntry->attributePointers[ATTRIBUTE_BASE]);
#endif

	tmp_err_code = getTypeQName(ctx->schema, treeT, resEntry->attributePointers[ATTRIBUTE_BASE], &baseTypeId);
	if(tmp_err_code != ERR_OK)
		return tmp_err_code;

	// Restriction from a complex type only

	if((baseTypeId.uriId == 3 && baseTypeId.lnId == 12) || // "xs:anyType"
			(resEntry->supertype.entry != NULL && resEntry->supertype.entry->element == ELEMENT_COMPLEX_TYPE))
	{
		tmp_err_code = getContentTypeProtoGrammar(ctx, treeT, resEntry, restr);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
	{
		/* When <complexContent> is used, the base type must
		 be a complexType. Base simpleType is an error.*/
		return UNEXPECTED_ERROR;
	}

	return ERR_OK;
}

static errorCode getTypeId(BuildContext* ctx, const QNameID typeQnameId, TreeTableEntry* typeEntry, TreeTable* treeT, Index* typeId)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;

	if(typeQnameId.uriId == 3) // == http://www.w3.org/2001/XMLSchema i.e. build-in type
	{
		tmp_err_code = getEXIDataTypeFromSimpleType(typeQnameId, typeId);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}
	else
	{
		if(GET_LN_URI_QNAME(ctx->schema->uriTable, typeQnameId).typeGrammar == INDEX_MAX)
		{
			// The EXIP grammars are not yet created for that simple type
			if(typeEntry == NULL)
				return UNEXPECTED_ERROR;
			else if(typeEntry->element == ELEMENT_SIMPLE_TYPE)
			{
				tmp_err_code = handleSimpleTypeEl(ctx, treeT, typeEntry);
				if(tmp_err_code != ERR_OK)
					return tmp_err_code;
			}
			else if(typeEntry->element == ELEMENT_COMPLEX_TYPE)
			{
				// Only simple types should be passed and have a valid typeId
				return UNEXPECTED_ERROR;
			}
			else
			{
				return UNEXPECTED_ERROR;
			}

		}

		*typeId = (GET_TYPE_GRAMMAR_QNAMEID(ctx->schema, typeQnameId))->rule[0].part[0].prod[0].typeId;
		if(*typeId == INDEX_MAX)
			return UNEXPECTED_ERROR;
	}

	return ERR_OK;
}

static errorCode storeGrammar(BuildContext* ctx, QNameID qnameID, ProtoGrammar* pGrammar, unsigned char isNillable, Index* grIndex)
{
	errorCode tmp_err_code = UNEXPECTED_ERROR;
	EXIGrammar exiGr;

	if(pGrammar == NULL)
	{
		exiGr = static_grammar_empty;
		if(ctx->emptyGrIndex == INDEX_MAX)
		{
			tmp_err_code = addDynEntry(&ctx->schema->grammarTable.dynArray, &exiGr, &ctx->emptyGrIndex, &ctx->schema->memList);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}

		*grIndex = ctx->emptyGrIndex;
	}
	else
	{
		tmp_err_code = assignCodes(pGrammar);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		tmp_err_code = convertProtoGrammar(&ctx->schema->memList, pGrammar, &exiGr);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;

		if(isNillable)
			SET_NILLABLE(exiGr.props);

		tmp_err_code = addDynEntry(&ctx->schema->grammarTable.dynArray, &exiGr, grIndex, &ctx->schema->memList);
		if(tmp_err_code != ERR_OK)
			return tmp_err_code;
	}

#if DEBUG_GRAMMAR_GEN == ON
	{
		SmallIndex t = 0;

		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>Stored grammar ["));
		printString(&ctx->schema->uriTable.uri[qnameID.uriId].uriStr);
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (":"));
		printString(&GET_LN_URI_QNAME(ctx->schema->uriTable, qnameID).lnStr);
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("]:"));
		for(t = 0; t < exiGr.count; t++)
		{
			tmp_err_code = printGrammarRule(t, &(exiGr.rule[t]), ctx->schema);
			if(tmp_err_code != ERR_OK)
				return tmp_err_code;
		}
	}
#endif

	return ERR_OK;
}

static void sortGlobalElemQnameTable(GlobalElemQNameTable *gElTbl)
{
	qsort(gElTbl->qname, gElTbl->count, sizeof(QNameID), compareQNameID);
}

static void sortEnumTable(EXIPSchema *schema)
{
	qsort(schema->enumTable.enumDef, schema->enumTable.count, sizeof(EnumDefinition), compareEnumDefs);
}
