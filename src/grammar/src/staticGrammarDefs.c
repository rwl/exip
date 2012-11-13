/*==================================================================*\
|                EXIP - Embeddable EXI Processor in C                |
|--------------------------------------------------------------------|
|          This work is licensed under BSD 3-Clause License          |
|  The full license terms and conditions are located in LICENSE.txt  |
\===================================================================*/

/**
 * @file staticGrammarDefs.c
 * @brief Defines common static grammar definitions
 * @date Jul 10, 2012
 * @author Rumen Kyusakov
 * @version 0.4
 * @par[Revision] $Id$
 */

#include "procTypes.h"

/* START: EMPTY GRAMMAR DEFS */
/*
 * Single empty production:
 *  : EE */
Production static_prod_empty_part0[1] =
{
    {
		EVENT_EE, INDEX_MAX,
        {SMALL_INDEX_MAX, INDEX_MAX},
        SMALL_INDEX_MAX
    }
};

/*
 * Empty rule (single EE production with event code 0):
 * NonTerm : EE */
GrammarRule static_rule_empty[1] =
{
    {static_prod_empty_part0, NULL, 1, 0, 0, 0}
};

/*
 * Empty grammar (single EE production in 1 rule):
 * NonTerm : EE */
EXIGrammar static_grammar_empty = {static_rule_empty, 1, 0x0a, 0};

/* END: EMPTY GRAMMAR DEFS */

/* START: DOCUMENT GRAMMAR DEFS */

/*
 * The start document/fragment production:
 *  : SD DocContent */
Production static_prod_start_doc_part0[1] =
{
    {
		EVENT_SD, INDEX_MAX,
        {SMALL_INDEX_MAX, INDEX_MAX},
        GR_DOC_CONTENT
    }
};

/*
 * The DocEnd ED production:
 *  : ED */
Production static_prod_doc_end_part0[1] =
{
    {
		EVENT_ED, INDEX_MAX,
        {SMALL_INDEX_MAX, INDEX_MAX},
        SMALL_INDEX_MAX
    }
};

/* END: DOCUMENT GRAMMAR DEFS */
