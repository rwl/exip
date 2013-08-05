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
#include "memManagement.h"
#include "initSchemaInstance.h"
#include "grammars.h"

#define DEFAULT_GLOBAL_QNAME_COUNT 200

// TODO: check if this empty grammar is needed?
//       Also this is platform dependent and must be fixed!
static Production static_grammar_prod_empty[1] = {{0x28FFFFFF, INDEX_MAX, {URI_MAX, LN_MAX}}};
static GrammarRule static_grammar_rule_empty[1] = {{static_grammar_prod_empty, 1, 0x01}};
static EXIGrammar static_grammar_empty = {static_grammar_rule_empty, 0x42000000, 1};

/** It is used by compareAttrUse(), compareProductions() and compareGlobalElemQName() functions */
static EXIPSchema* globalSchemaPtr;

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

/** Used for the attribute grammar generation. */
struct localAttrNames
{
	DynArray dynArray;
	String* attrNames;
	Index count;
};

// Functions for handling of schema elements (defined in the global scope)
// START
/**
 * Given a global or local element definition (isGlobal == TRUE/FALSE),
 * this function builds (if not already built) its associated type grammar
 * and return the index of that type grammar in the grIndex parameter.
 * The type grammar is then used for processing of the corresponding
 * SE(QName) productions.
 */
static errorCode handleElementEl(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* entry, boolean isGlobal, Index* grIndex);

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
static errorCode getAttributeUseProtoGrammars(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* entry, ProtoGrammarArray* attrUseArray, String** attrWildcardNS, struct localAttrNames* aNamesTbl);

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
static errorCode getAttributeProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* attrEntry, boolean isGlobal, ProtoGrammar** attr);

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

/**
 * Given a List entry this function builds the corresponding
 * simple type proto grammar.
 */
static errorCode getListProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* listEntry, ProtoGrammar** list);

// END - converting schema definitions to protogrammars

// Helper functions

/** Descending order comparison.
 * The productions are ordered with the largest event code first. */
static int compareProductions(const void* prod1, const void* prod2);

/**
 * @brief Event Code Assignment to normalized grammar
 *
 * @param[in, out] grammar the normalized grammar for assigning the event codes
 * @return Error handling code
 */
static void assignCodes(ProtoGrammar* grammar);

/** Sorts an array of attribute use proto-grammars */
static void sortAttributeUseGrammars(ProtoGrammarArray* attrUseArray);

/** Used by sortAttributeUseGrammars() */
static int compareAttrUse(const void* attrPG1, const void* attrPG2);

/** Used by sortGlobalElemQnameTable() */
static int compareGlobalElemQName(const void* QNameId1, const void* QNameId2);

/** Given a string value of an Occurance Attribute this function
 * converts it to a int representation (outInt).
 * the "unbounded" value is encoded as -1
 * */
static errorCode parseOccuranceAttribute(const String occurance, int* outInt);

/** Given a simple type QName and TreeTable entry determine the
 * corresponding typeId. The simple type must be a global type.
 * For build-in types the typeEntry can be NULL
 * */
static errorCode getTypeId(BuildContext* ctx, const QNameID typeQnameId, TreeTableEntry* typeEntry, TreeTable* treeT, Index* typeId);

/** Given a simple type TreeTable entry determine the
 * corresponding typeId. It does not create a simple type grammar.
 * This function is used for anonymous simple types for attributes,
 * lists etc.
 * For build-in types the typeEntry can be NULL
 * */
static errorCode getAnonymousTypeId(BuildContext* ctx, TreeTableEntry* typeEntry, TreeTable* treeT, Index* typeId);

/**
 * Given a ProtoGrammar this function assigns the event codes and
 * converts it to an EXIGrammar. The EXIGrammar is then stored in
 * the SchemaGrammarTable of the EXIPSchema object.
 * The index to the grammar is returned in grIndex parameter
 */
static errorCode storeGrammar(BuildContext* ctx, QNameID qnameID, ProtoGrammar* pGrammar, boolean isNillable, Index* grIndex);

static void sortGlobalElemQnameTable(GlobalElemQNameTable *gElTbl);

static void sortEnumTable(EXIPSchema *schema);

/** Check if an attribute with a string name aName is in the table lAttrTbl
 * that contains all the attributes already included for certain complex type.
 * TRUE: present, FASLE not present */
static char isAttrAlreadyPresent(String aName, struct localAttrNames* lAttrTbl);

errorCode convertTreeTablesToExipSchema(TreeTable* treeT, unsigned int count, EXIPSchema* schema)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	unsigned int i = 0;
	unsigned int j = 0;
	BuildContext ctx;
	TreeTableEntry* entry;
	Index grIndex; // Index of the grammar in the schemaGrammarTable

	ctx.schema = schema;
	globalSchemaPtr = schema;
	ctx.emptyGrIndex = INDEX_MAX;
	getEmptyString(&ctx.emptyString);

	TRY(initAllocList(&ctx.tmpMemList));
	TRY(createDynArray(&ctx.gElTbl.dynArray, sizeof(QNameID), DEFAULT_GLOBAL_QNAME_COUNT));

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
					tmp_err_code = EXIP_OK;
					break;
				case ELEMENT_ATTRIBUTE_GROUP:
					// The attribute groups are only needing when referenced within a complex type definition
					tmp_err_code = EXIP_OK;
					break;
				case ELEMENT_ATTRIBUTE:
					// AT (*) in schema-informed grammars bears an untyped value unless there is a
					// global attribute definition available for the qname of the attribute.
					// When a global attribute definition is available the attribute value is
					// represented according to the datatype of the global attribute.
					// TODO: There is a need for array of global attributes in the EXIPSchema object.
					//       This array must be sorted according to qname
					tmp_err_code = EXIP_OK;
					break;
				case ELEMENT_IMPORT:
					//TODO: implement validation checks
					//		1) the namespace of an <import> element must be a target namespace of some tree table
					tmp_err_code = EXIP_OK;
					break;
				case ELEMENT_INCLUDE:
					tmp_err_code = EXIP_OK;
					break;
				case ELEMENT_REDEFINE:
					tmp_err_code = EXIP_OK;
					break;
				case ELEMENT_NOTATION:
					tmp_err_code = EXIP_OK;
					break;
				default:
					tmp_err_code = EXIP_UNEXPECTED_ERROR;
					break;
			}

			if(tmp_err_code != EXIP_OK)
			{
				freeAllocList(&ctx.tmpMemList);
				return tmp_err_code;
			}
		}
	}

	sortGlobalElemQnameTable(&ctx.gElTbl);

	TRY(createDocGrammar(schema, ctx.gElTbl.qname, ctx.gElTbl.count));

	destroyDynArray(&ctx.gElTbl.dynArray);
	sortEnumTable(schema);
	schema->staticGrCount = schema->grammarTable.count;
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

	return EXIP_OK;
}

static int compareAttrUse(const void* attrPG1, const void* attrPG2)
{
	ProtoGrammar** a1 = (ProtoGrammar**) attrPG1;
	ProtoGrammar** a2 = (ProtoGrammar**) attrPG2;

	return compareQNameID(&((*a1)->rule[0].prod[0].qnameId), &((*a2)->rule[0].prod[0].qnameId), &globalSchemaPtr->uriTable);
}

static void sortAttributeUseGrammars(ProtoGrammarArray* attrUseArray)
{
	qsort(attrUseArray->pg, attrUseArray->count, sizeof(ProtoGrammar*), compareAttrUse);
}

static errorCode getElementTermProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* elementEntry, Index grIndex, ProtoGrammar** elTerm)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
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
			TRY(getTypeQName(ctx->schema, treeT, elementEntry->attributePointers[ATTRIBUTE_REF], &qNameID));
		}
		else
			return EXIP_UNEXPECTED_ERROR;
	}
	else
	{
		if(treeT->globalDefs.elemFormDefault == QUALIFIED || stringEqualToAscii(elementEntry->attributePointers[ATTRIBUTE_FORM], "qualified"))
			qNameID.uriId = treeT->globalDefs.targetNsId;
		else
			qNameID.uriId = 0;

		if(!lookupLn(&ctx->schema->uriTable.uri[qNameID.uriId].lnTable, elementEntry->attributePointers[ATTRIBUTE_NAME], &qNameID.lnId))
			return EXIP_UNEXPECTED_ERROR;
	}

	TRY(parseOccuranceAttribute(elementEntry->attributePointers[ATTRIBUTE_MIN_OCCURS], &minOccurs));
	TRY(parseOccuranceAttribute(elementEntry->attributePointers[ATTRIBUTE_MAX_OCCURS], &maxOccurs));

	if(minOccurs < 0 || maxOccurs < -1)
		return EXIP_UNEXPECTED_ERROR;

	TRY(createElementTermGrammar(&elTermGrammar, qNameID, grIndex));

	elParticleGrammar = (ProtoGrammar*) memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
	if(elParticleGrammar == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;

	TRY(createParticleGrammar(minOccurs, maxOccurs, &elTermGrammar, elParticleGrammar));

	destroyProtoGrammar(&elTermGrammar);

	*elTerm = elParticleGrammar;

	return EXIP_OK;
}

static errorCode handleElementEl(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* entry, boolean isGlobal, Index* grIndex)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	String type;
	QNameID elQNameID;
	QNameID typeQNameID;
	boolean isNillable = FALSE;

#if DEBUG_GRAMMAR_GEN == ON && EXIP_DEBUG_LEVEL == INFO
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
			return EXIP_UNEXPECTED_ERROR;
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
			return EXIP_UNEXPECTED_ERROR;
	}

	if(isGlobal)
	{
		// Check if the element grammar is not already created
		if(GET_LN_URI_QNAME(ctx->schema->uriTable, elQNameID).elemGrammar != INDEX_MAX)
		{
			*grIndex = GET_LN_URI_QNAME(ctx->schema->uriTable, elQNameID).elemGrammar;
			return EXIP_OK;
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
		 * attribute or an anonymous type definition. If both are missing then it is
		 * an empty element declaration (<xs:element name="unconstrained"/>) that has ·xs:anyType· by default */
		if(entry->child.entry == NULL)
		{
			*grIndex = GET_LN_URI_IDS(ctx->schema->uriTable, XML_SCHEMA_NAMESPACE_ID, SIMPLE_TYPE_ANY_TYPE).typeGrammar;
			GET_LN_URI_QNAME(ctx->schema->uriTable, elQNameID).elemGrammar = *grIndex;
			return EXIP_OK;
		}
		else if(entry->child.entry->element == ELEMENT_SIMPLE_TYPE)
		{
			TRY(getSimpleTypeProtoGrammar(ctx, entry->child.treeT, entry->child.entry, &pg));
		}
		else if(entry->child.entry->element == ELEMENT_COMPLEX_TYPE)
		{
			TRY(getComplexTypeProtoGrammar(ctx, entry->child.treeT, entry->child.entry, &pg));
		}
		else if(entry->child.entry->element == ELEMENT_ELEMENT)
		{
			// In case of ref="..." attribute
			
			return handleElementEl(ctx, entry->child.treeT, entry->child.entry, TRUE, grIndex);
		}
		else
			return EXIP_UNEXPECTED_ERROR;

		if(entry->child.entry->loopDetection != 0 && entry->child.entry->loopDetection != INDEX_MAX)
		{
			// This should be the case only for entry->child.entry->element == ELEMENT_COMPLEX_TYPE
			*grIndex = entry->child.entry->loopDetection;

			if(pg != NULL)
			{
				EXIGrammar exiGr;

				assignCodes(pg);
				TRY(convertProtoGrammar(&ctx->schema->memList, pg, &exiGr));

				// The grammar has a content2 grammar if and only if there are AT
				// productions that point to the content grammar rule OR the content index is 0.
				if(GET_CONTENT_INDEX(exiGr.props) == 0)
					SET_HAS_CONTENT2(exiGr.props);
				else
				{
					Index r, p;
					boolean prodFound = FALSE;
					for(r = 0; r < GET_CONTENT_INDEX(exiGr.props); r++)
					{
						for(p = 0; p < RULE_GET_AT_COUNT(exiGr.rule[r].meta); p++)
						{
							if(GET_PROD_NON_TERM(exiGr.rule[r].production[exiGr.rule[r].pCount-1-p].content) == GET_CONTENT_INDEX(exiGr.props))
							{
								SET_HAS_CONTENT2(exiGr.props);
								prodFound = TRUE;
								break;
							}
						}
						if(prodFound)
							break;
					}
				}

				ctx->schema->grammarTable.grammar[*grIndex].count = exiGr.count;
				ctx->schema->grammarTable.grammar[*grIndex].props = exiGr.props;
				ctx->schema->grammarTable.grammar[*grIndex].rule = exiGr.rule;

				destroyProtoGrammar(pg);
			}
		}
		else
		{
			TRY(storeGrammar(ctx, elQNameID, pg, isNillable, grIndex));
		}
		/* If the element is globally defined -> store the index of its grammar in the
		 * LnEntry in the string tables */
		if(isGlobal == TRUE)
			GET_LN_URI_QNAME(ctx->schema->uriTable, elQNameID).elemGrammar = *grIndex;

	}
	else // The element has a particular named type
	{
		/* Find the QNameID of the type of this element */
		TRY(getTypeQName(ctx->schema, treeT, type, &typeQNameID));

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
				return EXIP_UNEXPECTED_ERROR;
			else if(entry->child.entry->element == ELEMENT_SIMPLE_TYPE)
			{
				TRY(getSimpleTypeProtoGrammar(ctx, entry->child.treeT, entry->child.entry, &pg));
			}
			else if(entry->child.entry->element == ELEMENT_COMPLEX_TYPE)
			{
				TRY(getComplexTypeProtoGrammar(ctx, entry->child.treeT, entry->child.entry, &pg));
			}
			else
				return EXIP_UNEXPECTED_ERROR;

			if(entry->child.entry->loopDetection != 0 && entry->child.entry->loopDetection != INDEX_MAX)
			{
				// This should be the case only for entry->child.entry->element == ELEMENT_COMPLEX_TYPE
				*grIndex = entry->child.entry->loopDetection;

				if(pg != NULL)
				{
					EXIGrammar exiGr;

					assignCodes(pg);
					TRY(convertProtoGrammar(&ctx->schema->memList, pg, &exiGr));

					// The grammar has a content2 grammar if and only if there are AT
					// productions that point to the content grammar rule OR the content index is 0.
					if(GET_CONTENT_INDEX(exiGr.props) == 0)
						SET_HAS_CONTENT2(exiGr.props);
					else
					{
						Index r, p;
						boolean prodFound = FALSE;
						for(r = 0; r < GET_CONTENT_INDEX(exiGr.props); r++)
						{
							for(p = 0; p < RULE_GET_AT_COUNT(exiGr.rule[r].meta); p++)
							{
								if(GET_PROD_NON_TERM(exiGr.rule[r].production[exiGr.rule[r].pCount-1-p].content) == GET_CONTENT_INDEX(exiGr.props))
								{
									SET_HAS_CONTENT2(exiGr.props);
									prodFound = TRUE;
									break;
								}
							}
							if(prodFound)
								break;
						}
					}

					ctx->schema->grammarTable.grammar[*grIndex].count = exiGr.count;
					ctx->schema->grammarTable.grammar[*grIndex].props = exiGr.props;
					ctx->schema->grammarTable.grammar[*grIndex].rule = exiGr.rule;

					destroyProtoGrammar(pg);
				}
			}
			else
			{
				TRY(storeGrammar(ctx, typeQNameID, pg, isNillable, grIndex));
			}

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
	 * If the element is global and it is not recursively added already,
	 * add it to the GlobalElemQNameTable.
	 * This table is used to generate the schema-informed document grammar.
	 */
	if(isGlobal && entry->loopDetection == 0)
	{
		Index dynElID;

		TRY(addDynEntry(&ctx->gElTbl.dynArray, &elQNameID, &dynElID));
		entry->loopDetection = INDEX_MAX;
	}

	return EXIP_OK;
}

static errorCode getAttributeProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* attrEntry, boolean isGlobal, ProtoGrammar** attr)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	boolean required = FALSE;
	Index typeId;
	QNameID atQnameID;

	if (!isStringEmpty(&(attrEntry->attributePointers[ATTRIBUTE_USE])) &&
			stringEqualToAscii(attrEntry->attributePointers[ATTRIBUTE_USE], "required"))
	{
		required = TRUE;
	}

	// Validation checks
	if(isStringEmpty(&attrEntry->attributePointers[ATTRIBUTE_NAME]))
	{
		// The attribute does not have a name.
		// Then it should be local and have a ref="..." attribute.
		// It must not have a type attribute as well.
		if(isGlobal || isStringEmpty(&attrEntry->attributePointers[ATTRIBUTE_REF]) || !isStringEmpty(&attrEntry->attributePointers[ATTRIBUTE_TYPE]))
			return EXIP_UNEXPECTED_ERROR;
	}
	else
	{
		/* If the attribute form "qualified" then the element attribute is the target namespace */
		if(isGlobal || treeT->globalDefs.attrFormDefault == QUALIFIED || stringEqualToAscii(attrEntry->attributePointers[ATTRIBUTE_FORM], "qualified"))
			atQnameID.uriId = treeT->globalDefs.targetNsId;
		else
			atQnameID.uriId = 0; // URI	0	"" [empty string]

		/* The attribute qname must be already in the string tables */
		if(!lookupLn(&ctx->schema->uriTable.uri[atQnameID.uriId].lnTable, attrEntry->attributePointers[ATTRIBUTE_NAME], &atQnameID.lnId))
			return EXIP_UNEXPECTED_ERROR;
	}

	if(!isStringEmpty(&attrEntry->attributePointers[ATTRIBUTE_TYPE]))
	{
		// The attribute has defined type
		QNameID stQNameID;
		// global type for the attribute
		TRY(getTypeQName(ctx->schema, treeT, attrEntry->attributePointers[ATTRIBUTE_TYPE], &stQNameID));
		TRY(getTypeId(ctx, stQNameID, attrEntry->child.entry, attrEntry->child.treeT, &typeId));
	}
	else
	{
		if(attrEntry->child.entry == NULL)
		{
			// The attribute does not have defined type and anonymous simple type definition:
			// hence ·xs:anySimpleType·

			typeId = SIMPLE_TYPE_ANY_SIMPLE_TYPE;
		}
		else if(attrEntry->child.entry->element == ELEMENT_ATTRIBUTE)
		{
			// A reference to a global attribute
			return getAttributeProtoGrammar(ctx, attrEntry->child.treeT, attrEntry->child.entry, TRUE, attr);
		}
		else
		{
			// an anonymous type for the attribute
			TRY(getAnonymousTypeId(ctx, attrEntry->child.entry, attrEntry->child.treeT, &typeId));
		}
	}

	*attr = (ProtoGrammar*) memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
	if(*attr == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;
	
	TRY(createAttributeUseGrammar(required, typeId, *attr, atQnameID));

	return EXIP_OK;
}

static errorCode getSimpleTypeProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* simpleEntry, ProtoGrammar** simplType)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;

	if(simpleEntry->child.entry == NULL)
	{
		/** Empty simple type*/
		*simplType = NULL;
	}
	else if(simpleEntry->child.entry->element == ELEMENT_RESTRICTION)
	{
		TRY(getRestrictionSimpleProtoGrammar(ctx, treeT, simpleEntry->child.entry, simplType));
	}
	else if(simpleEntry->child.entry->element == ELEMENT_LIST)
	{
		TRY(getListProtoGrammar(ctx, treeT, simpleEntry->child.entry, simplType));
	}
	else if(simpleEntry->child.entry->element == ELEMENT_UNION)
	{
		*simplType = (ProtoGrammar*) memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
		if(*simplType == NULL)
			return EXIP_MEMORY_ALLOCATION_ERROR;

		TRY(createSimpleTypeGrammar(SIMPLE_TYPE_STRING, *simplType));
	}
	else
		return EXIP_UNEXPECTED_ERROR;

	return EXIP_OK;
}

static errorCode handleSimpleTypeEl(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* entry)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;

#if DEBUG_GRAMMAR_GEN == ON && EXIP_DEBUG_LEVEL == INFO
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
			return EXIP_UNEXPECTED_ERROR;

		if(GET_LN_URI_QNAME(ctx->schema->uriTable, stQNameID).typeGrammar == INDEX_MAX)
		{
			// The EXIP grammars corresponding to this simple type are not yet created
			Index grIndex;

			TRY(getSimpleTypeProtoGrammar(ctx, treeT, entry, &simpleProtoGrammar));
			TRY(storeGrammar(ctx, stQNameID, simpleProtoGrammar, FALSE, &grIndex));

			GET_LN_URI_QNAME(ctx->schema->uriTable, stQNameID).typeGrammar = grIndex;

			// When Strict is True: If Tk either has named sub-types or is a simple type definition of which {variety} is union...
			if(entry->child.entry->element == ELEMENT_RESTRICTION)
			{
				QNameID baseTypeQnameId;

				TRY(getTypeQName(ctx->schema, entry->child.treeT, entry->child.entry->attributePointers[ATTRIBUTE_BASE], &baseTypeQnameId));

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
		return EXIP_UNEXPECTED_ERROR;
	}

	return EXIP_OK;
}

static errorCode getSimpleContentProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* sContEntry, ProtoGrammar** sCont)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;

	if(sContEntry->child.entry == NULL)
		return EXIP_UNEXPECTED_ERROR;
	else if(sContEntry->child.entry->element == ELEMENT_RESTRICTION)
	{
		tmp_err_code = getRestrictionSimpleProtoGrammar(ctx, treeT, sContEntry->child.entry, sCont);
	}
	else if(sContEntry->child.entry->element == ELEMENT_EXTENSION)
	{
		tmp_err_code = getExtensionSimpleProtoGrammar(ctx, sContEntry->child.treeT, sContEntry->child.entry, sCont);
	}
	else
		tmp_err_code = EXIP_UNEXPECTED_ERROR;

	return tmp_err_code;
}

/* entry should be a complex_type or extension element */
static errorCode getContentTypeProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* entry, ProtoGrammar** content)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;

	*content = NULL;

	if(entry->child.entry == NULL)
	{
		// empty complex_type or extension element
		tmp_err_code = EXIP_OK;
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
		tmp_err_code = EXIP_OK;
	}
	else
		return EXIP_UNEXPECTED_ERROR;

	return tmp_err_code;
}

/* entry should be a complex_type or extension or restriction or attributeGroup element */
static errorCode getAttributeUseProtoGrammars(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* entry, ProtoGrammarArray* attrUseArray, String** attrWildcardNS, struct localAttrNames* aNamesTbl)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	TreeTableEntry* attrUse = NULL;

	if(entry->child.entry != NULL)
	{
		if(entry->child.entry->element == ELEMENT_ATTRIBUTE ||
			entry->child.entry->element == ELEMENT_ATTRIBUTE_GROUP ||
			entry->child.entry->element == ELEMENT_ANY_ATTRIBUTE)
		{	
			attrUse = entry->child.entry;
		}
		else if(entry->child.entry->next != NULL &&
				(entry->child.entry->next->element == ELEMENT_ATTRIBUTE ||
				 entry->child.entry->next->element == ELEMENT_ATTRIBUTE_GROUP ||
				 entry->child.entry->next->element == ELEMENT_ANY_ATTRIBUTE)
				)
		{
			attrUse = entry->child.entry->next;
		}
		else if(entry->child.entry->element == ELEMENT_SIMPLE_CONTENT && entry->child.entry->child.entry != NULL && entry->child.entry->child.entry->element == ELEMENT_EXTENSION)
		{
			 if(entry->child.entry->child.entry->child.entry != NULL)
			 {
				 if(entry->child.entry->child.entry->child.entry->element == ELEMENT_ATTRIBUTE ||
					entry->child.entry->child.entry->child.entry->element == ELEMENT_ATTRIBUTE_GROUP ||
					entry->child.entry->child.entry->child.entry->element == ELEMENT_ANY_ATTRIBUTE)
				 {
					 attrUse = entry->child.entry->child.entry->child.entry;
				 }
				 else if(entry->child.entry->child.entry->child.entry->next != NULL &&
						(entry->child.entry->child.entry->child.entry->next->element == ELEMENT_ATTRIBUTE ||
						 entry->child.entry->child.entry->child.entry->next->element == ELEMENT_ATTRIBUTE_GROUP ||
						 entry->child.entry->child.entry->child.entry->next->element == ELEMENT_ANY_ATTRIBUTE)
				 	 	)
				 {
					 attrUse = entry->child.entry->child.entry->child.entry->next;
				 }
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
					String aName;
					if(!isStringEmpty(&attrUse->attributePointers[ATTRIBUTE_REF]))
						aName = attrUse->attributePointers[ATTRIBUTE_REF];
					else
						aName = attrUse->attributePointers[ATTRIBUTE_NAME];

					if(!isAttrAlreadyPresent(aName, aNamesTbl))
					{
						TRY(addDynEntry(&aNamesTbl->dynArray, &aName, &entryId));

						if(!stringEqualToAscii(attrUse->attributePointers[ATTRIBUTE_USE], "prohibited"))
						{
							TRY(getAttributeProtoGrammar(ctx, treeT, attrUse, FALSE, &attrPG));
							TRY(addDynEntry(&attrUseArray->dynArray, &attrPG, &entryId));
						}
					}
				}
				else if(attrUse->element == ELEMENT_ATTRIBUTE_GROUP)
				{
					if(attrUse->child.entry != NULL)
					{
						TRY(getAttributeUseProtoGrammars(ctx, attrUse->child.treeT,  attrUse->child.entry, attrUseArray, attrWildcardNS, aNamesTbl));
					}
					else
						return EXIP_UNEXPECTED_ERROR;
				}
				else if(attrUse->element == ELEMENT_ANY_ATTRIBUTE)
				{
					*attrWildcardNS = &attrUse->attributePointers[ATTRIBUTE_NAMESPACE];
				}
				else
					return EXIP_UNEXPECTED_ERROR;

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
				TRY(getAttributeUseProtoGrammars(ctx, entry->child.entry->child.treeT, entry->child.entry->child.entry, attrUseArray, attrWildcardNS, aNamesTbl));
			}
		}
	}

	if(entry->element == ELEMENT_EXTENSION || entry->element == ELEMENT_RESTRICTION)
	{
		if(entry->supertype.entry != NULL && entry->supertype.entry->element == ELEMENT_COMPLEX_TYPE)
		{
			TRY(getAttributeUseProtoGrammars(ctx, entry->supertype.treeT, entry->supertype.entry, attrUseArray, attrWildcardNS, aNamesTbl));
		}
	}

	/* NOTE ON:
	 * === Deriving complex types through restriction ===
	 * When deriving complex types through restriction it is not required by
	 * the XSD spec that you repeat all the inherited attributes.
	 * This is yet another quirk in the XSD spec - all content elements should be repeated
	 * during restriction but not the attribute uses!?!?
	 * In any way, the schema code is much more comprehensible when the attribute uses are
	 * repeated but still this messy code here is needed to keep the compliance with the
	 * XSD specification.
	 */

	return EXIP_OK;
}

static errorCode getComplexTypeProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* complEntry, ProtoGrammar** complType)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	ProtoGrammarArray attrUseArray;
	ProtoGrammar* contentTypeGrammar = NULL;
	String* attrWildcardNS = NULL;
	boolean isMixedContent = FALSE;
	Index i;

	if(complEntry->loopDetection == 0)
	{
		// The complexType entry has not been processed before
		complEntry->loopDetection = INDEX_MAX;
	}
	else if(complEntry->loopDetection == INDEX_MAX)
	{
		// The complexType has already been processed once.
		// Add dummy grammar to the grammarTable that will be replaced by the real one later
		TRY(addDynEntry(&ctx->schema->grammarTable.dynArray, &static_grammar_empty, &complEntry->loopDetection));
		*complType = NULL;
		return EXIP_OK;
	}
	else
	{
		// The complexType entry has already been processed at least twice
		*complType = NULL;
		return EXIP_OK;
	}

	if(!isStringEmpty(&complEntry->attributePointers[ATTRIBUTE_MIXED])
			&& stringEqualToAscii(complEntry->attributePointers[ATTRIBUTE_MIXED], "true"))
	{
		isMixedContent = TRUE;
	}

	TRY(getContentTypeProtoGrammar(ctx, treeT, complEntry, &contentTypeGrammar));
	TRY(createDynArray(&attrUseArray.dynArray, sizeof(ProtoGrammar*), 10));

	{ // get all the attribute uses
		struct localAttrNames aNamesTbl;
		TRY(createDynArray(&aNamesTbl.dynArray, sizeof(String), 20));
		TRY(getAttributeUseProtoGrammars(ctx, treeT, complEntry, &attrUseArray, &attrWildcardNS, &aNamesTbl));

		destroyDynArray(&aNamesTbl.dynArray);
	}

	sortAttributeUseGrammars(&attrUseArray);

	if(attrWildcardNS != NULL)
	{
		// There is an {attribute wildcard} ANY_ATTRIBUTE
		NsTable nsTable;
		Index dummyEntryId;
		QNameID qnameID;
		ProtoRuleEntry* pRuleEntry;
		ProtoGrammar* pAttrWildGrammar;

		TRY(createDynArray(&nsTable.dynArray, sizeof(String), 5));
		TRY(getNsList(treeT, *attrWildcardNS, &nsTable));

		pAttrWildGrammar = memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
		if(pAttrWildGrammar == NULL)
			return EXIP_MEMORY_ALLOCATION_ERROR;

		TRY(createProtoGrammar(2, pAttrWildGrammar));
		TRY(addProtoRule(pAttrWildGrammar, 5, &pRuleEntry));
		TRY(addEEProduction(pRuleEntry));
		TRY(addDynEntry(&attrUseArray.dynArray, &pAttrWildGrammar, &dummyEntryId));

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
				TRY(addProduction(&attrUseArray.pg[i]->rule[0], EVENT_AT_ALL, INDEX_MAX, qnameID, 0));
			}
		}
		else if(nsTable.count >= 1)
		{
			Index j;

			qnameID.lnId = LN_MAX;

			for(j = 0; j < nsTable.count; j++)
			{
				if(!lookupUri(&ctx->schema->uriTable, nsTable.base[j], &qnameID.uriId))
				 	return EXIP_UNEXPECTED_ERROR;

				for(i = 0; i < attrUseArray.count; i++)
				{
					TRY(addProduction(&attrUseArray.pg[i]->rule[0], EVENT_AT_URI, INDEX_MAX, qnameID, 0));
				}
			}
		}
		else
			return EXIP_UNEXPECTED_ERROR;

		destroyDynArray(&nsTable.dynArray);
	}

	if(contentTypeGrammar == NULL && attrUseArray.count == 0) // An empty element: <xsd:complexType />
	{
		*complType = NULL;
	}
	else
	{
		*complType = (ProtoGrammar*) memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
		if(*complType == NULL)
			return EXIP_MEMORY_ALLOCATION_ERROR;

		TRY(createComplexTypeGrammar(&attrUseArray, contentTypeGrammar, isMixedContent, *complType));

		if(contentTypeGrammar != NULL)
			destroyProtoGrammar(contentTypeGrammar);
	}

	for(i = 0; i < attrUseArray.count; i++)
	{
		destroyProtoGrammar(attrUseArray.pg[i]);
	}

	destroyDynArray(&attrUseArray.dynArray);

	return EXIP_OK;
}

static errorCode handleComplexTypeEl(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* entry)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;

#if DEBUG_GRAMMAR_GEN == ON && EXIP_DEBUG_LEVEL == INFO
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>Handle ComplexType: "));
	printString(&entry->attributePointers[ATTRIBUTE_NAME]);
#endif

	if(!isStringEmpty(&entry->attributePointers[ATTRIBUTE_NAME]))
	{
		QNameID ctQNameID;

		ctQNameID.uriId = treeT->globalDefs.targetNsId;

		/** The type qname must be in the string tables */
		if(!lookupLn(&ctx->schema->uriTable.uri[ctQNameID.uriId].lnTable, entry->attributePointers[ATTRIBUTE_NAME], &ctQNameID.lnId))
			return EXIP_UNEXPECTED_ERROR;

		if(GET_LN_URI_QNAME(ctx->schema->uriTable, ctQNameID).typeGrammar == INDEX_MAX)
		{
			// The EXIP grammars are not yet created
			ProtoGrammar* complType;
			Index grIndex;

			TRY(getComplexTypeProtoGrammar(ctx, treeT, entry, &complType));

			if(entry->loopDetection != 0 && entry->loopDetection != INDEX_MAX)
			{
				// This should be the case only for entry->element == ELEMENT_COMPLEX_TYPE
				grIndex = entry->loopDetection;
				if(complType != NULL)
				{
					EXIGrammar exiGr;

					assignCodes(complType);
					TRY(convertProtoGrammar(&ctx->schema->memList, complType, &exiGr));

					// The grammar has a content2 grammar if and only if there are AT
					// productions that point to the content grammar rule OR the content index is 0.
					if(GET_CONTENT_INDEX(exiGr.props) == 0)
						SET_HAS_CONTENT2(exiGr.props);
					else
					{
						Index r, p;
						boolean prodFound = FALSE;
						for(r = 0; r < GET_CONTENT_INDEX(exiGr.props); r++)
						{
							for(p = 0; p < RULE_GET_AT_COUNT(exiGr.rule[r].meta); p++)
							{
								if(GET_PROD_NON_TERM(exiGr.rule[r].production[exiGr.rule[r].pCount-1-p].content) == GET_CONTENT_INDEX(exiGr.props))
								{
									SET_HAS_CONTENT2(exiGr.props);
									prodFound = TRUE;
									break;
								}
							}
							if(prodFound)
								break;
						}
					}

					ctx->schema->grammarTable.grammar[grIndex].count = exiGr.count;
					ctx->schema->grammarTable.grammar[grIndex].props = exiGr.props;
					ctx->schema->grammarTable.grammar[grIndex].rule = exiGr.rule;

					destroyProtoGrammar(complType);
				}
			}
			else
			{
				TRY(storeGrammar(ctx, ctQNameID, complType, FALSE, &grIndex));
			}

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

						TRY(getTypeQName(ctx->schema, entry->child.entry->child.treeT, entry->child.entry->child.entry->attributePointers[ATTRIBUTE_BASE], &baseTypeQnameId));

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
		return EXIP_UNEXPECTED_ERROR;
	}

	return EXIP_OK;
}

static errorCode getComplexContentProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* cConEntry, ProtoGrammar** cCont)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;

	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>Handle Complex Content Proto Grammar"));

	if(cConEntry->child.entry == NULL)
		return EXIP_UNEXPECTED_ERROR;
	else if(cConEntry->child.entry->element == ELEMENT_RESTRICTION)
	{
		tmp_err_code = getRestrictionComplexProtoGrammar(ctx, treeT, cConEntry->child.entry, cCont);
	}
	else if(cConEntry->child.entry->element == ELEMENT_EXTENSION)
	{
		tmp_err_code = getExtensionComplexProtoGrammar(ctx, treeT, cConEntry->child.entry, cCont);
	}
	else
		tmp_err_code = EXIP_UNEXPECTED_ERROR;

	return tmp_err_code;
}

static errorCode getSequenceProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* seqEntry, ProtoGrammar** seq)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	ProtoGrammar* particleGrammar = NULL;
	ProtoGrammar seqGrammar;
	ProtoGrammar* seqPartGrammar;
	int minOccurs = 1;
	int maxOccurs = 1;
	TreeTableEntry* nextIterator;
	ProtoGrammarArray partGrammarTbl;
	Index dummyTblIndx;
	Index i;

	TRY(createDynArray(&partGrammarTbl.dynArray, sizeof(ProtoGrammar*), 30));

	TRY(parseOccuranceAttribute(seqEntry->attributePointers[ATTRIBUTE_MIN_OCCURS], &minOccurs));
	TRY(parseOccuranceAttribute(seqEntry->attributePointers[ATTRIBUTE_MAX_OCCURS], &maxOccurs));

	if(minOccurs < 0 || maxOccurs < -1)
		return EXIP_UNEXPECTED_ERROR;

	nextIterator = seqEntry->child.entry;
	while(nextIterator != NULL)
	{
		if(nextIterator->element == ELEMENT_ELEMENT)
		{
			Index grIndex;

			TRY(handleElementEl(ctx, treeT, nextIterator, FALSE, &grIndex));
			TRY(getElementTermProtoGrammar(ctx, treeT, nextIterator, grIndex, &particleGrammar));
		}
		else if(nextIterator->element == ELEMENT_GROUP)
		{
			TRY(getGroupProtoGrammar(ctx, treeT, nextIterator, &particleGrammar));
		}
		else if(nextIterator->element == ELEMENT_CHOICE)
		{
			TRY(getChoiceProtoGrammar(ctx, treeT, nextIterator, &particleGrammar));
		}
		else if(nextIterator->element == ELEMENT_SEQUENCE)
		{
			TRY(getSequenceProtoGrammar(ctx, treeT, nextIterator, &particleGrammar));
		}
		else if(nextIterator->element == ELEMENT_ANY)
		{
			TRY(getAnyProtoGrammar(ctx, treeT, nextIterator, &particleGrammar));
		}
		else
			return EXIP_UNEXPECTED_ERROR;

		TRY(addDynEntry(&partGrammarTbl.dynArray, &particleGrammar, &dummyTblIndx));
		nextIterator = nextIterator->next;
	}

	TRY(createSequenceModelGroupsGrammar(partGrammarTbl.pg, partGrammarTbl.count, &seqGrammar));

	for(i = 0; i < partGrammarTbl.count; i++)
	{
		destroyProtoGrammar(partGrammarTbl.pg[i]);
	}

	destroyDynArray(&partGrammarTbl.dynArray);

	seqPartGrammar = (ProtoGrammar*) memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
	if(seqPartGrammar == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;

	TRY(createParticleGrammar(minOccurs, maxOccurs, &seqGrammar, seqPartGrammar));

	destroyProtoGrammar(&seqGrammar);
	*seq = seqPartGrammar;

	return EXIP_OK;
}

static errorCode getAnyProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* anyEntry, ProtoGrammar** any)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	ProtoGrammar wildTermGrammar;
	ProtoGrammar* wildParticleGrammar;
	int minOccurs = 1;
	int maxOccurs = 1;
	NsTable nsTable;

	TRY(createDynArray(&nsTable.dynArray, sizeof(String), 5));

	TRY(getNsList(treeT, anyEntry->attributePointers[ATTRIBUTE_NAMESPACE], &nsTable));

	TRY(parseOccuranceAttribute(anyEntry->attributePointers[ATTRIBUTE_MIN_OCCURS], &minOccurs));
	TRY(parseOccuranceAttribute(anyEntry->attributePointers[ATTRIBUTE_MAX_OCCURS], &maxOccurs));

	if(minOccurs < 0 || maxOccurs < -1)
		return EXIP_UNEXPECTED_ERROR;

	TRY(createWildcardTermGrammar(nsTable.base, nsTable.count, &ctx->schema->uriTable, &wildTermGrammar));

	wildParticleGrammar = (ProtoGrammar*)memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
	if(wildParticleGrammar == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;

	TRY(createParticleGrammar(minOccurs, maxOccurs, &wildTermGrammar, wildParticleGrammar));

	destroyProtoGrammar(&wildTermGrammar);
	destroyDynArray(&nsTable.dynArray);

	*any = wildParticleGrammar;

	return EXIP_OK;
}

static errorCode getChoiceProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* chEntry, ProtoGrammar** choice)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	ProtoGrammar choiceGrammar;
	ProtoGrammar* choicePartGrammar;
	TreeTableEntry* nextIterator;
	ProtoGrammarArray particleProtoGrammarArray;
	ProtoGrammar* particleGrammar = NULL;
	Index entryId, i;
	int minOccurs = 1;
	int maxOccurs = 1;

	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>Handle Choice "));

	TRY(parseOccuranceAttribute(chEntry->attributePointers[ATTRIBUTE_MIN_OCCURS], &minOccurs));
	TRY(parseOccuranceAttribute(chEntry->attributePointers[ATTRIBUTE_MAX_OCCURS], &maxOccurs));

	if(minOccurs < 0 || maxOccurs < -1)
		return EXIP_UNEXPECTED_ERROR;

	TRY(createDynArray(&particleProtoGrammarArray.dynArray, sizeof(ProtoGrammar*), 15));

	nextIterator = chEntry->child.entry;
	while(nextIterator != NULL)
	{
		if(nextIterator->element == ELEMENT_ELEMENT)
		{
			Index grIndex;
			TRY(handleElementEl(ctx, treeT, nextIterator, FALSE, &grIndex));
			TRY(getElementTermProtoGrammar(ctx, treeT, nextIterator, grIndex, &particleGrammar));
		}
		else if(nextIterator->element == ELEMENT_GROUP)
		{
			TRY(getGroupProtoGrammar(ctx, treeT, nextIterator, &particleGrammar));
		}
		else if(nextIterator->element == ELEMENT_CHOICE)
		{
			TRY(getChoiceProtoGrammar(ctx, treeT, nextIterator, &particleGrammar));
		}
		else if(nextIterator->element == ELEMENT_SEQUENCE)
		{
			TRY(getSequenceProtoGrammar(ctx, treeT, nextIterator, &particleGrammar));
		}
		else if(nextIterator->element == ELEMENT_ANY)
		{
			TRY(getAnyProtoGrammar(ctx, treeT, nextIterator, &particleGrammar));
		}
		else
			return EXIP_UNEXPECTED_ERROR;

		TRY(addDynEntry(&particleProtoGrammarArray.dynArray, &particleGrammar, &entryId));
		nextIterator = nextIterator->next;
	}

	TRY(createChoiceModelGroupsGrammar(&particleProtoGrammarArray, &choiceGrammar));

	for(i = 0; i < particleProtoGrammarArray.count; i++)
	{
		destroyProtoGrammar(particleProtoGrammarArray.pg[i]);
	}

	destroyDynArray(&particleProtoGrammarArray.dynArray);

	choicePartGrammar = (ProtoGrammar*)memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
	if(choicePartGrammar == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;

	TRY(createParticleGrammar(minOccurs, maxOccurs, &choiceGrammar, choicePartGrammar));
	destroyProtoGrammar(&choiceGrammar);
	*choice = choicePartGrammar;

	return EXIP_OK;
}

static errorCode getAllProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* allEntry, ProtoGrammar** all)
{
	return EXIP_NOT_IMPLEMENTED_YET;
}

static errorCode getGroupProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* grEntry, ProtoGrammar** group)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	ProtoGrammar* particleGrammar = NULL;
	ProtoGrammar* grPartGrammar;
	int minOccurs = 1;
	int maxOccurs = 1;

#if DEBUG_GRAMMAR_GEN == ON && EXIP_DEBUG_LEVEL == INFO
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>Handle Group: "));
	printString(&grEntry->attributePointers[ATTRIBUTE_REF]);
#endif

	TRY(parseOccuranceAttribute(grEntry->attributePointers[ATTRIBUTE_MIN_OCCURS], &minOccurs));
	TRY(parseOccuranceAttribute(grEntry->attributePointers[ATTRIBUTE_MAX_OCCURS], &maxOccurs));

	if(minOccurs < 0 || maxOccurs < -1)
		return EXIP_UNEXPECTED_ERROR;

	// There should be a global group definition referenced through ref attribute
	if(grEntry->child.entry == NULL)
		return EXIP_UNEXPECTED_ERROR;

	if(grEntry->child.entry->child.entry == NULL)
	{
		// empty group.
		// The content of 'group (global)' must match (annotation?, (all | choice | sequence)). Not enough
		// elements were found.
		return EXIP_UNEXPECTED_ERROR;
	}
	else if(grEntry->child.entry->child.entry->element == ELEMENT_SEQUENCE)
	{
		TRY(getSequenceProtoGrammar(ctx, grEntry->child.entry->child.treeT, grEntry->child.entry->child.entry, &particleGrammar));
	}
	else if(grEntry->child.entry->child.entry->element == ELEMENT_CHOICE)
	{
		TRY(getChoiceProtoGrammar(ctx, grEntry->child.entry->child.treeT, grEntry->child.entry->child.entry, &particleGrammar));
	}
	else if(grEntry->child.entry->child.entry->element == ELEMENT_ALL)
	{
		TRY(getAllProtoGrammar(ctx, grEntry->child.entry->child.treeT, grEntry->child.entry->child.entry, &particleGrammar));
	}
	else
		return EXIP_UNEXPECTED_ERROR;

	grPartGrammar = (ProtoGrammar*)memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
	if(grPartGrammar == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;

	TRY(createParticleGrammar(minOccurs, maxOccurs, particleGrammar, grPartGrammar));

	destroyProtoGrammar(particleGrammar);
	*group = grPartGrammar;

	return EXIP_OK;
}

static errorCode getExtensionSimpleProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* extEntry, ProtoGrammar** ext)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	QNameID baseTypeId;
	ProtoGrammar* resultProtoGrammar = NULL;
	Index typeId;

#if DEBUG_GRAMMAR_GEN == ON && EXIP_DEBUG_LEVEL == INFO
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>Handle SimpleContent Extension: "));
	printString(&extEntry->attributePointers[ATTRIBUTE_BASE]);
#endif

	TRY(getTypeQName(ctx->schema, treeT, extEntry->attributePointers[ATTRIBUTE_BASE], &baseTypeId));

	resultProtoGrammar = (ProtoGrammar*) memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
	if(resultProtoGrammar == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;

	TRY(getTypeId(ctx, baseTypeId, extEntry->supertype.entry, extEntry->supertype.treeT, &typeId));

	// Extension from a simple type only
	TRY(createSimpleTypeGrammar(typeId, resultProtoGrammar));
	*ext = resultProtoGrammar;

	return EXIP_OK;
}

static errorCode getExtensionComplexProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* extEntry, ProtoGrammar** ext)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	QNameID baseTypeId;
	ProtoGrammar* contentTypeGrammarBase;
	ProtoGrammar* contentTypeGrammarExt;
	ProtoGrammar* resultProtoGrammar = NULL;
	ProtoGrammar* contGrArr[2];

#if DEBUG_GRAMMAR_GEN == ON && EXIP_DEBUG_LEVEL == INFO
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>Handle ComplexContent Extension: "));
	printString(&extEntry->attributePointers[ATTRIBUTE_BASE]);
#endif

	TRY(getTypeQName(ctx->schema, treeT, extEntry->attributePointers[ATTRIBUTE_BASE], &baseTypeId));

	resultProtoGrammar = (ProtoGrammar*) memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
	if(resultProtoGrammar == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;

	// Extension from a complex type only

	TreeTableEntry* base = extEntry->supertype.entry;
	if(base == NULL)
		return EXIP_UNEXPECTED_ERROR;
	else if(base->element == ELEMENT_COMPLEX_TYPE)
	{
		TRY(handleComplexTypeEl(ctx, extEntry->supertype.treeT, base));
		TRY(getContentTypeProtoGrammar(ctx, extEntry->supertype.treeT, base, &contentTypeGrammarBase));
	}
	else if(base->element == ELEMENT_SIMPLE_TYPE)
	{
		// When <complexContent> is used, the base type must be a complexType. Base simpleType is an error.
		return EXIP_UNEXPECTED_ERROR;
	}
	else
	{
		return EXIP_UNEXPECTED_ERROR;
	}

	TRY(getContentTypeProtoGrammar(ctx, treeT, extEntry, &contentTypeGrammarExt));

	contGrArr[0] = contentTypeGrammarBase;
	contGrArr[1] = contentTypeGrammarExt;

	TRY(createSequenceModelGroupsGrammar(contGrArr, 2, resultProtoGrammar));
	if(contentTypeGrammarBase != NULL)
		destroyProtoGrammar(contentTypeGrammarBase);
	if(contentTypeGrammarExt != NULL)
		destroyProtoGrammar(contentTypeGrammarExt);

	*ext = resultProtoGrammar;

	return EXIP_OK;
}

static errorCode getRestrictionSimpleProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* resEntry, ProtoGrammar** restr)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	ProtoGrammar* simpleRestrictedGrammar;
	QNameID baseTypeID;
	SimpleType newSimpleType;
	Index typeId;
	Index simpleTypeId;
	TreeTableEntry* tmpEntry;
	unsigned int enumCount = 0; // the number of <xs:enumeration in the restriction>

	if(isStringEmpty(&resEntry->attributePointers[ATTRIBUTE_BASE]))
	{
		// No base type defined. There should be an anonymous simple type
		if(resEntry->child.entry != NULL && resEntry->child.entry->element == ELEMENT_SIMPLE_TYPE)
		{
			// Very weird use case of the XSD spec. Does not bring any useful features. Ignored for now.
			return EXIP_NOT_IMPLEMENTED_YET;
		}
		else
			return EXIP_UNEXPECTED_ERROR;
	}
	else
	{
		TRY(getTypeQName(ctx->schema, treeT, resEntry->attributePointers[ATTRIBUTE_BASE], &baseTypeID));
		TRY(getTypeId(ctx, baseTypeID, resEntry->supertype.entry, resEntry->supertype.treeT, &typeId));
	}

	// TODO: check if there are cases when the EXI type changes after restriction
	newSimpleType.content  = ctx->schema->simpleTypeTable.sType[typeId].content;
	// remove the presence of named subtype
	REMOVE_TYPE_FACET(newSimpleType.content, TYPE_FACET_NAMED_SUBTYPE_UNION);
	newSimpleType.max = ctx->schema->simpleTypeTable.sType[typeId].max;
	newSimpleType.min = ctx->schema->simpleTypeTable.sType[typeId].min;
	newSimpleType.length = ctx->schema->simpleTypeTable.sType[typeId].length;

	tmpEntry = resEntry->child.entry;

	while(tmpEntry != NULL)
	{
		if(tmpEntry->element == ELEMENT_MIN_INCLUSIVE)
		{
			SET_TYPE_FACET(newSimpleType.content, TYPE_FACET_MIN_INCLUSIVE);
			TRY(stringToInt64(&resEntry->child.entry->attributePointers[ATTRIBUTE_VALUE], &newSimpleType.min));
		}
		else if(tmpEntry->element == ELEMENT_MIN_EXCLUSIVE)
		{
			SET_TYPE_FACET(newSimpleType.content, TYPE_FACET_MIN_EXCLUSIVE);
			TRY(stringToInt64(&resEntry->child.entry->attributePointers[ATTRIBUTE_VALUE], &newSimpleType.min));
		}
		else if(tmpEntry->element == ELEMENT_MIN_LENGTH)
		{
			SET_TYPE_FACET(newSimpleType.content, TYPE_FACET_MIN_LENGTH);
			TRY(stringToInt64(&resEntry->child.entry->attributePointers[ATTRIBUTE_VALUE], &newSimpleType.min));
		}
		else if(tmpEntry->element == ELEMENT_MAX_INCLUSIVE)
		{
			SET_TYPE_FACET(newSimpleType.content, TYPE_FACET_MAX_INCLUSIVE);
			TRY(stringToInt64(&resEntry->child.entry->attributePointers[ATTRIBUTE_VALUE], &newSimpleType.max));
		}
		else if(tmpEntry->element == ELEMENT_MAX_EXCLUSIVE)
		{
			SET_TYPE_FACET(newSimpleType.content, TYPE_FACET_MAX_EXCLUSIVE);
			TRY(stringToInt64(&resEntry->child.entry->attributePointers[ATTRIBUTE_VALUE], &newSimpleType.max));
		}
		else if(tmpEntry->element == ELEMENT_MAX_LENGTH)
		{
			SET_TYPE_FACET(newSimpleType.content, TYPE_FACET_MAX_LENGTH);
			TRY(stringToInt64(&resEntry->child.entry->attributePointers[ATTRIBUTE_VALUE], &newSimpleType.max));
		}
		else if(tmpEntry->element == ELEMENT_LENGTH)
		{
			int ml = 0;
			SET_TYPE_FACET(newSimpleType.content, TYPE_FACET_LENGTH);
			TRY(stringToInteger(&resEntry->child.entry->attributePointers[ATTRIBUTE_VALUE], &ml));
			newSimpleType.length = (unsigned int) ml;
		}
		else if(tmpEntry->element == ELEMENT_TOTAL_DIGITS)
		{
			SET_TYPE_FACET(newSimpleType.content, TYPE_FACET_TOTAL_DIGITS);
			return EXIP_NOT_IMPLEMENTED_YET;
		}
		else if(tmpEntry->element == ELEMENT_FRACTION_DIGITS)
		{
			SET_TYPE_FACET(newSimpleType.content, TYPE_FACET_FRACTION_DIGITS);
			return EXIP_NOT_IMPLEMENTED_YET;
		}
		else if(tmpEntry->element == ELEMENT_PATTERN)
		{
			SET_TYPE_FACET(newSimpleType.content, TYPE_FACET_PATTERN);
			// TODO: needs to be implemented. It is also needed for the XML Schema grammars
			// COMMENT #SCHEMA#: ignore for now
//			return EXIP_NOT_IMPLEMENTED_YET;
		}
		else if(tmpEntry->element == ELEMENT_WHITE_SPACE)
		{
			SET_TYPE_FACET(newSimpleType.content, TYPE_FACET_WHITE_SPACE);
			return EXIP_NOT_IMPLEMENTED_YET;
		}
		else if(tmpEntry->element == ELEMENT_ENUMERATION)
		{
			enumCount += 1;
		}
		else
			return EXIP_UNEXPECTED_ERROR;

		tmpEntry = tmpEntry->next;
	}

	// Handling of enumerations
	if(enumCount > 0) // There are enumerations defined
	{
		struct TreeTableEntry* enumEntry;
		unsigned int enumIter = 0;
		size_t valSize = 0;
		EnumDefinition eDef;
		Index elId;

		eDef.count = enumCount;
		eDef.values = NULL;
		/* The next index in the simpleTypeTable will be assigned to the newly created simple type
		 * containing the enumeration */
		eDef.typeId = ctx->schema->simpleTypeTable.count;

		SET_TYPE_FACET(newSimpleType.content, TYPE_FACET_ENUMERATION);

		switch(GET_EXI_TYPE(ctx->schema->simpleTypeTable.sType[typeId].content))
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
			case VALUE_TYPE_NON_NEGATIVE_INT:
				valSize = sizeof(UnsignedInteger);
				break;
		}

		eDef.values = memManagedAllocate(&ctx->schema->memList, valSize*(eDef.count));
		if(eDef.values == NULL)
			return EXIP_MEMORY_ALLOCATION_ERROR;

		enumEntry = resEntry->child.entry;
		while(enumEntry != NULL)
		{
			if(enumEntry->element == ELEMENT_ENUMERATION)
			{
				switch(GET_EXI_TYPE(ctx->schema->simpleTypeTable.sType[typeId].content))
				{
					case VALUE_TYPE_STRING:
					{
						String tmpStr;

						TRY(cloneStringManaged(&enumEntry->attributePointers[ATTRIBUTE_VALUE], &tmpStr, &ctx->schema->memList));

						((String*) eDef.values)[enumIter].length = tmpStr.length;
						((String*) eDef.values)[enumIter].str = tmpStr.str;
					}
						break;
					case VALUE_TYPE_BOOLEAN:
						return EXIP_NOT_IMPLEMENTED_YET;
						break;
					case VALUE_TYPE_DATE_TIME:
						return EXIP_NOT_IMPLEMENTED_YET;
						break;
					case VALUE_TYPE_DECIMAL:
						return EXIP_NOT_IMPLEMENTED_YET;
						break;
					case VALUE_TYPE_FLOAT:
						return EXIP_NOT_IMPLEMENTED_YET;
						break;
					case VALUE_TYPE_INTEGER:
						return EXIP_NOT_IMPLEMENTED_YET;
						break;
					case VALUE_TYPE_SMALL_INTEGER:
						return EXIP_NOT_IMPLEMENTED_YET;
						break;
					case VALUE_TYPE_NON_NEGATIVE_INT:
					{
						 int64_t tmpInt;

						TRY(stringToInt64(&enumEntry->attributePointers[ATTRIBUTE_VALUE], &tmpInt));
						((UnsignedInteger*) eDef.values)[enumIter] = (UnsignedInteger) tmpInt;
					}
						break;
					default:
						return EXIP_NOT_IMPLEMENTED_YET;
				}
			}
			enumEntry = enumEntry->next;
			enumIter++;
		}

		TRY(addDynEntry(&ctx->schema->enumTable.dynArray, &eDef, &elId));
	}

	TRY(addDynEntry(&ctx->schema->simpleTypeTable.dynArray, &newSimpleType, &simpleTypeId));

	simpleRestrictedGrammar = (ProtoGrammar*) memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
	if(simpleRestrictedGrammar == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;

	TRY(createSimpleTypeGrammar(simpleTypeId, simpleRestrictedGrammar));
	*restr = simpleRestrictedGrammar;

	return EXIP_OK;
}

static errorCode getRestrictionComplexProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* resEntry, ProtoGrammar** restr)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	QNameID baseTypeId;

#if DEBUG_GRAMMAR_GEN == ON && EXIP_DEBUG_LEVEL == INFO
	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>Handle ComplexContent Restriction: "));
	printString(&resEntry->attributePointers[ATTRIBUTE_BASE]);
#endif

	TRY(getTypeQName(ctx->schema, treeT, resEntry->attributePointers[ATTRIBUTE_BASE], &baseTypeId));

	// Restriction from a complex type only

	if((baseTypeId.uriId == XML_SCHEMA_NAMESPACE_ID && baseTypeId.lnId == SIMPLE_TYPE_ANY_TYPE) || // "xs:anyType"
			(resEntry->supertype.entry != NULL && resEntry->supertype.entry->element == ELEMENT_COMPLEX_TYPE))
	{
		TRY(getContentTypeProtoGrammar(ctx, treeT, resEntry, restr));
	}
	else
	{
		/* When <complexContent> is used, the base type must
		 be a complexType. Base simpleType is an error.*/
		return EXIP_UNEXPECTED_ERROR;
	}

	return EXIP_OK;
}

static errorCode getTypeId(BuildContext* ctx, const QNameID typeQnameId, TreeTableEntry* typeEntry, TreeTable* treeT, Index* typeId)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;

	if(typeQnameId.uriId == XML_SCHEMA_NAMESPACE_ID &&
			typeQnameId.lnId < SIMPLE_TYPE_COUNT) // == http://www.w3.org/2001/XMLSchema i.e. build-in type
	{
		*typeId = typeQnameId.lnId;
	}
	else
	{
		if(GET_LN_URI_QNAME(ctx->schema->uriTable, typeQnameId).typeGrammar == INDEX_MAX)
		{
			// The EXIP grammars are not yet created for that simple type
			if(typeEntry == NULL)
				return EXIP_UNEXPECTED_ERROR;
			else if(typeEntry->element == ELEMENT_SIMPLE_TYPE)
			{
				TRY(handleSimpleTypeEl(ctx, treeT, typeEntry));
			}
			else if(typeEntry->element == ELEMENT_COMPLEX_TYPE)
			{
				// Only simple types should be passed and have a valid typeId
				return EXIP_UNEXPECTED_ERROR;
			}
			else
			{
				return EXIP_UNEXPECTED_ERROR;
			}

		}

		*typeId = (GET_TYPE_GRAMMAR_QNAMEID(ctx->schema, typeQnameId))->rule[0].production[0].typeId;
		if(*typeId == INDEX_MAX)
			return EXIP_UNEXPECTED_ERROR;
	}

	return EXIP_OK;
}

static errorCode getAnonymousTypeId(BuildContext* ctx, TreeTableEntry* typeEntry, TreeTable* treeT, Index* typeId)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	ProtoGrammar* tmpGr = NULL;

	if(typeEntry == NULL)
		return EXIP_UNEXPECTED_ERROR;
	else if(typeEntry->element == ELEMENT_SIMPLE_TYPE)
	{
		TRY(getSimpleTypeProtoGrammar(ctx, treeT, typeEntry, &tmpGr));
	}
	else
	{
		return EXIP_UNEXPECTED_ERROR;
	}

	if(tmpGr == NULL)
		return EXIP_UNEXPECTED_ERROR;

	*typeId = tmpGr->rule[0].prod[0].typeId;

	destroyProtoGrammar(tmpGr);

	return EXIP_OK;
}

static errorCode getListProtoGrammar(BuildContext* ctx, TreeTable* treeT, TreeTableEntry* listEntry, ProtoGrammar** list)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	Index itemTypeId = INDEX_MAX;
	SimpleType listSimpleType;
	Index listEntrySimplID;

	DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>Handle list"));

	*list = (ProtoGrammar*) memManagedAllocate(&ctx->tmpMemList, sizeof(ProtoGrammar));
	if(*list == NULL)
		return EXIP_MEMORY_ALLOCATION_ERROR;

	listSimpleType.content = 0;
	SET_EXI_TYPE(listSimpleType.content, VALUE_TYPE_LIST);
	listSimpleType.max = 0;
	listSimpleType.min = 0;
	listSimpleType.length = 0;

	if(!isStringEmpty(&listEntry->attributePointers[ATTRIBUTE_ITEM_TYPE]))
	{
		QNameID itemTypeQnameId;

		// The list has item type a global simple type. It should not have child entries
		if(listEntry->child.entry != NULL)
			return EXIP_UNEXPECTED_ERROR;

		TRY(getTypeQName(ctx->schema, treeT, listEntry->attributePointers[ATTRIBUTE_ITEM_TYPE], &itemTypeQnameId));
		TRY(getTypeId(ctx, itemTypeQnameId, listEntry->supertype.entry, listEntry->supertype.treeT, &itemTypeId));
	}
	else
	{
		TRY(getAnonymousTypeId(ctx, listEntry->child.entry, listEntry->child.treeT, &itemTypeId));
	}

	listSimpleType.length = itemTypeId;

	TRY(addDynEntry(&ctx->schema->simpleTypeTable.dynArray, &listSimpleType, &listEntrySimplID));
	TRY(createSimpleTypeGrammar(listEntrySimplID, *list));

	return EXIP_OK;
}

static errorCode storeGrammar(BuildContext* ctx, QNameID qnameID, ProtoGrammar* pGrammar, boolean isNillable, Index* grIndex)
{
	errorCode tmp_err_code = EXIP_UNEXPECTED_ERROR;
	EXIGrammar exiGr;

	if(pGrammar == NULL)
	{
		exiGr = static_grammar_empty;
		if(ctx->emptyGrIndex == INDEX_MAX)
		{
			TRY(addDynEntry(&ctx->schema->grammarTable.dynArray, &exiGr, &ctx->emptyGrIndex));
		}

		*grIndex = ctx->emptyGrIndex;
	}
	else
	{
		assignCodes(pGrammar);

		TRY(convertProtoGrammar(&ctx->schema->memList, pGrammar, &exiGr));

		if(isNillable)
			SET_NILLABLE_GR(exiGr.props);

		// The grammar has a content2 grammar if and only if there are AT
		// productions that point to the content grammar rule OR the content index is 0.
		if(GET_CONTENT_INDEX(exiGr.props) == 0)
			SET_HAS_CONTENT2(exiGr.props);
		else
		{
			Index r, p;
			boolean prodFound = FALSE;
			for(r = 0; r < GET_CONTENT_INDEX(exiGr.props); r++)
			{
				for(p = 0; p < RULE_GET_AT_COUNT(exiGr.rule[r].meta); p++)
				{
					if(GET_PROD_NON_TERM(exiGr.rule[r].production[exiGr.rule[r].pCount-1-p].content) == GET_CONTENT_INDEX(exiGr.props))
					{
						SET_HAS_CONTENT2(exiGr.props);
						prodFound = TRUE;
						break;
					}
				}
				if(prodFound)
					break;
			}
		}

		TRY(addDynEntry(&ctx->schema->grammarTable.dynArray, &exiGr, grIndex));
		destroyProtoGrammar(pGrammar);
	}

#if DEBUG_GRAMMAR_GEN == ON && EXIP_DEBUG_LEVEL == INFO
	{
		SmallIndex t = 0;

		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("\n>Stored grammar ["));
		printString(&ctx->schema->uriTable.uri[qnameID.uriId].uriStr);
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, (":"));
		printString(&GET_LN_URI_QNAME(ctx->schema->uriTable, qnameID).lnStr);
		DEBUG_MSG(INFO, DEBUG_GRAMMAR_GEN, ("]:"));
		for(t = 0; t < exiGr.count; t++)
		{
			TRY(printGrammarRule(t, &(exiGr.rule[t]), ctx->schema));
		}
	}
#endif

	return EXIP_OK;
}

static int compareGlobalElemQName(const void* QNameId1, const void* QNameId2)
{
	QNameID* q1 = (QNameID*) QNameId1;
	QNameID* q2 = (QNameID*) QNameId2;

	return compareQNameID(q1, q2, &globalSchemaPtr->uriTable);
}

static void sortGlobalElemQnameTable(GlobalElemQNameTable *gElTbl)
{
	qsort(gElTbl->qname, gElTbl->count, sizeof(QNameID), compareGlobalElemQName);
}

static void sortEnumTable(EXIPSchema *schema)
{
	qsort(schema->enumTable.enumDef, schema->enumTable.count, sizeof(EnumDefinition), compareEnumDefs);
}

static char isAttrAlreadyPresent(String aName, struct localAttrNames* lAttrTbl)
{
	Index i;

	for(i = 0; i < lAttrTbl->count; i++)
	{
		if(stringEqual(aName, lAttrTbl->attrNames[i]))
			return TRUE;
	}

	return FALSE;
}

void assignCodes(ProtoGrammar* grammar)
{
	Index i = 0;

	for (i = 0; i < grammar->count; i++)
	{
		qsort(grammar->rule[i].prod, grammar->rule[i].count, sizeof(Production), compareProductions);
	}
}

static int compareProductions(const void* prod1, const void* prod2)
{
	Production* p1 = (Production*) prod1;
	Production* p2 = (Production*) prod2;

	if(GET_PROD_EXI_EVENT(p1->content) < GET_PROD_EXI_EVENT(p2->content))
		return 1;
	else if(GET_PROD_EXI_EVENT(p1->content) > GET_PROD_EXI_EVENT(p2->content))
		return -1;
	else // the same event Type
	{
		if(GET_PROD_EXI_EVENT(p1->content) == EVENT_AT_QNAME)
		{
			return -compareQNameID(&(p1->qnameId), &(p2->qnameId), &globalSchemaPtr->uriTable);
		}
		else if(GET_PROD_EXI_EVENT(p1->content) == EVENT_AT_URI)
		{
			if(p1->qnameId.uriId < p2->qnameId.uriId)
			{
				return 1;
			}
			else if(p1->qnameId.uriId > p2->qnameId.uriId)
			{
				return -1;
			}
			else
				return 0;
		}
		else if(GET_PROD_EXI_EVENT(p1->content) == EVENT_SE_QNAME)
		{
			// TODO: figure out how it works??? if this really works for all cases. Seems very unlikely that it does!
			if(GET_PROD_NON_TERM(p1->content) < GET_PROD_NON_TERM(p2->content))
				return 1;
			else
				return -1;
		}
		else if(GET_PROD_EXI_EVENT(p1->content) == EVENT_SE_URI)
		{
			// TODO: figure out how it should be done
		}
		return 0;
	}
}

