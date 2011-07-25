/** AUTO-GENERATED: Mon Jul 25 12:38:12 2011
  * Copyright (c) 2010 - 2011, Rumen Kyusakov, EISLAB, LTU
  * $Id$ */

#include "schema.h"

struct PrefixRow ops_prows_0[1] = {{{"",0}}};
PrefixTable ops_pTable_0 = {ops_prows_0, 1, 1, {NULL, 0}};

struct LocalNamesRow ops_LNrows_0[0] = {};
LocalNamesTable ops_lTable_0 = { ops_LNrows_0, 0, 0, {NULL, 0}};

struct PrefixRow ops_prows_1[1] = {{{"xml",3}}};
PrefixTable ops_pTable_1 = {ops_prows_1, 1, 1, {NULL, 0}};

struct LocalNamesRow ops_LNrows_1[4] = {{NULL, {"base", 4}, NULL},{NULL, {"id", 2}, NULL},{NULL, {"lang", 4}, NULL},{NULL, {"space", 5}, NULL}};
LocalNamesTable ops_lTable_1 = { ops_LNrows_1, 4, 4, {NULL, 0}};

struct PrefixRow ops_prows_2[1] = {{{"xsi",3}}};
PrefixTable ops_pTable_2 = {ops_prows_2, 1, 1, {NULL, 0}};

struct LocalNamesRow ops_LNrows_2[2] = {{NULL, {"nil", 3}, NULL},{NULL, {"type", 4}, NULL}};
LocalNamesTable ops_lTable_2 = { ops_LNrows_2, 2, 2, {NULL, 0}};

struct LocalNamesRow ops_LNrows_3[46] = {{NULL, {"ENTITIES", 8}, NULL},{NULL, {"ENTITY", 6}, NULL},{NULL, {"ID", 2}, NULL},{NULL, {"IDREF", 5}, NULL},{NULL, {"IDREFS", 6}, NULL},{NULL, {"NCName", 6}, NULL},{NULL, {"NMTOKEN", 7}, NULL},{NULL, {"NMTOKENS", 8}, NULL},{NULL, {"NOTATION", 8}, NULL},{NULL, {"Name", 4}, NULL},{NULL, {"QName", 5}, NULL},{NULL, {"anySimpleType", 13}, NULL},{NULL, {"anyType", 7}, NULL},{NULL, {"anyURI", 6}, NULL},{NULL, {"base64Binary", 12}, NULL},{NULL, {"boolean", 7}, NULL},{NULL, {"byte", 4}, NULL},{NULL, {"date", 4}, NULL},{NULL, {"dateTime", 8}, NULL},{NULL, {"decimal", 7}, NULL},{NULL, {"double", 6}, NULL},{NULL, {"duration", 8}, NULL},{NULL, {"float", 5}, NULL},{NULL, {"gDay", 4}, NULL},{NULL, {"gMonth", 6}, NULL},{NULL, {"gMonthDay", 9}, NULL},{NULL, {"gYear", 5}, NULL},{NULL, {"gYearMonth", 10}, NULL},{NULL, {"hexBinary", 9}, NULL},{NULL, {"int", 3}, NULL},{NULL, {"integer", 7}, NULL},{NULL, {"language", 8}, NULL},{NULL, {"long", 4}, NULL},{NULL, {"negativeInteger", 15}, NULL},{NULL, {"nonNegativeInteger", 18}, NULL},{NULL, {"nonPositiveInteger", 18}, NULL},{NULL, {"normalizedString", 16}, NULL},{NULL, {"positiveInteger", 15}, NULL},{NULL, {"short", 5}, NULL},{NULL, {"string", 6}, NULL},{NULL, {"time", 4}, NULL},{NULL, {"token", 5}, NULL},{NULL, {"unsignedByte", 12}, NULL},{NULL, {"unsignedInt", 11}, NULL},{NULL, {"unsignedLong", 12}, NULL},{NULL, {"unsignedShort", 13}, NULL}};
LocalNamesTable ops_lTable_3 = { ops_LNrows_3, 46, 46, {NULL, 0}};

Production ops_prodArray_4_0_0_0[2] = {{{5,1}, 4, 28, 1},{{5,1}, 4, 4, 2}};
Production ops_prodArray_4_0_0_1[0] = {};
Production ops_prodArray_4_0_0_2[0] = {};
Production ops_prodArray_4_0_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_0_1_1[0] = {};
Production ops_prodArray_4_0_1_2[0] = {};
Production ops_prodArray_4_0_2_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_0_2_1[0] = {};
Production ops_prodArray_4_0_2_2[0] = {};

GrammarRule ops_ruleArray_4_0[3] = {{{ops_prodArray_4_0_0_0, ops_prodArray_4_0_0_1, ops_prodArray_4_0_0_2}, {2, 0, 0}, {1, 0, 0}},{{ops_prodArray_4_0_1_0, ops_prodArray_4_0_1_1, ops_prodArray_4_0_1_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_0_2_0, ops_prodArray_4_0_2_1, ops_prodArray_4_0_2_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_0 = {ops_ruleArray_4_0, 3, 14, 0};

Production ops_prodArray_4_1_0_0[1] = {{{9,7}, 65535, -1, 1}};
Production ops_prodArray_4_1_0_1[0] = {};
Production ops_prodArray_4_1_0_2[0] = {};
Production ops_prodArray_4_1_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_1_1_1[0] = {};
Production ops_prodArray_4_1_1_2[0] = {};

GrammarRule ops_ruleArray_4_1[2] = {{{ops_prodArray_4_1_0_0, ops_prodArray_4_1_0_1, ops_prodArray_4_1_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_1_1_0, ops_prodArray_4_1_1_1, ops_prodArray_4_1_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_1 = {ops_ruleArray_4_1, 2, 14, 0};

Production ops_prodArray_4_2_0_0[1] = {{{9,22}, 65535, -1, 1}};
Production ops_prodArray_4_2_0_1[0] = {};
Production ops_prodArray_4_2_0_2[0] = {};
Production ops_prodArray_4_2_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_2_1_1[0] = {};
Production ops_prodArray_4_2_1_2[0] = {};

GrammarRule ops_ruleArray_4_2[2] = {{{ops_prodArray_4_2_0_0, ops_prodArray_4_2_0_1, ops_prodArray_4_2_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_2_1_0, ops_prodArray_4_2_1_1, ops_prodArray_4_2_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_2 = {ops_ruleArray_4_2, 2, 14, 0};

Production ops_prodArray_4_3_0_0[1] = {{{9,6}, 65535, -1, 1}};
Production ops_prodArray_4_3_0_1[0] = {};
Production ops_prodArray_4_3_0_2[0] = {};
Production ops_prodArray_4_3_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_3_1_1[0] = {};
Production ops_prodArray_4_3_1_2[0] = {};

GrammarRule ops_ruleArray_4_3[2] = {{{ops_prodArray_4_3_0_0, ops_prodArray_4_3_0_1, ops_prodArray_4_3_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_3_1_0, ops_prodArray_4_3_1_1, ops_prodArray_4_3_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_3 = {ops_ruleArray_4_3, 2, 14, 0};

Production ops_prodArray_4_4_0_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_4_0_1[0] = {};
Production ops_prodArray_4_4_0_2[0] = {};

GrammarRule ops_ruleArray_4_4[1] = {{{ops_prodArray_4_4_0_0, ops_prodArray_4_4_0_1, ops_prodArray_4_4_0_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_empty_grammar = {ops_ruleArray_4_4, 1, 13, 0};

Production ops_prodArray_4_6_0_0[4] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 31, 3},{{5,1}, 4, 14, 2},{{5,1}, 4, 7, 1}};
Production ops_prodArray_4_6_0_1[0] = {};
Production ops_prodArray_4_6_0_2[0] = {};
Production ops_prodArray_4_6_1_0[3] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 31, 3},{{5,1}, 4, 14, 2}};
Production ops_prodArray_4_6_1_1[0] = {};
Production ops_prodArray_4_6_1_2[0] = {};
Production ops_prodArray_4_6_2_0[2] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 31, 3}};
Production ops_prodArray_4_6_2_1[0] = {};
Production ops_prodArray_4_6_2_2[0] = {};
Production ops_prodArray_4_6_3_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_6_3_1[0] = {};
Production ops_prodArray_4_6_3_2[0] = {};

GrammarRule ops_ruleArray_4_6[4] = {{{ops_prodArray_4_6_0_0, ops_prodArray_4_6_0_1, ops_prodArray_4_6_0_2}, {4, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_6_1_0, ops_prodArray_4_6_1_1, ops_prodArray_4_6_1_2}, {3, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_6_2_0, ops_prodArray_4_6_2_1, ops_prodArray_4_6_2_2}, {2, 0, 0}, {1, 0, 0}},{{ops_prodArray_4_6_3_0, ops_prodArray_4_6_3_1, ops_prodArray_4_6_3_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_6 = {ops_ruleArray_4_6, 4, 14, 0};

Production ops_prodArray_4_8_0_0[1] = {{{7,1}, 65535, -1, 1}};
Production ops_prodArray_4_8_0_1[0] = {};
Production ops_prodArray_4_8_0_2[0] = {};

GrammarRule ops_ruleArray_4_8[1] = {{{ops_prodArray_4_8_0_0, ops_prodArray_4_8_0_1, ops_prodArray_4_8_0_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_8 = {ops_ruleArray_4_8, 1, 14, 0};

Production ops_prodArray_4_9_0_0[1] = {{{9,5}, 65535, -1, 1}};
Production ops_prodArray_4_9_0_1[0] = {};
Production ops_prodArray_4_9_0_2[0] = {};
Production ops_prodArray_4_9_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_9_1_1[0] = {};
Production ops_prodArray_4_9_1_2[0] = {};

GrammarRule ops_ruleArray_4_9[2] = {{{ops_prodArray_4_9_0_0, ops_prodArray_4_9_0_1, ops_prodArray_4_9_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_9_1_0, ops_prodArray_4_9_1_1, ops_prodArray_4_9_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_9 = {ops_ruleArray_4_9, 2, 14, 0};

Production ops_prodArray_4_10_0_0[1] = {{{9,5}, 65535, -1, 1}};
Production ops_prodArray_4_10_0_1[0] = {};
Production ops_prodArray_4_10_0_2[0] = {};
Production ops_prodArray_4_10_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_10_1_1[0] = {};
Production ops_prodArray_4_10_1_2[0] = {};

GrammarRule ops_ruleArray_4_10[2] = {{{ops_prodArray_4_10_0_0, ops_prodArray_4_10_0_1, ops_prodArray_4_10_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_10_1_0, ops_prodArray_4_10_1_1, ops_prodArray_4_10_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_10 = {ops_ruleArray_4_10, 2, 14, 0};

Production ops_prodArray_4_11_0_0[1] = {{{9,4}, 65535, -1, 1}};
Production ops_prodArray_4_11_0_1[0] = {};
Production ops_prodArray_4_11_0_2[0] = {};
Production ops_prodArray_4_11_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_11_1_1[0] = {};
Production ops_prodArray_4_11_1_2[0] = {};

GrammarRule ops_ruleArray_4_11[2] = {{{ops_prodArray_4_11_0_0, ops_prodArray_4_11_0_1, ops_prodArray_4_11_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_11_1_0, ops_prodArray_4_11_1_1, ops_prodArray_4_11_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_11 = {ops_ruleArray_4_11, 2, 14, 0};

Production ops_prodArray_4_12_0_0[1] = {{{9,3}, 65535, -1, 1}};
Production ops_prodArray_4_12_0_1[0] = {};
Production ops_prodArray_4_12_0_2[0] = {};
Production ops_prodArray_4_12_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_12_1_1[0] = {};
Production ops_prodArray_4_12_1_2[0] = {};

GrammarRule ops_ruleArray_4_12[2] = {{{ops_prodArray_4_12_0_0, ops_prodArray_4_12_0_1, ops_prodArray_4_12_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_12_1_0, ops_prodArray_4_12_1_1, ops_prodArray_4_12_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_12 = {ops_ruleArray_4_12, 2, 14, 0};

Production ops_prodArray_4_15_0_0[1] = {{{9,5}, 65535, -1, 1}};
Production ops_prodArray_4_15_0_1[0] = {};
Production ops_prodArray_4_15_0_2[0] = {};
Production ops_prodArray_4_15_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_15_1_1[0] = {};
Production ops_prodArray_4_15_1_2[0] = {};

GrammarRule ops_ruleArray_4_15[2] = {{{ops_prodArray_4_15_0_0, ops_prodArray_4_15_0_1, ops_prodArray_4_15_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_15_1_0, ops_prodArray_4_15_1_1, ops_prodArray_4_15_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_15 = {ops_ruleArray_4_15, 2, 14, 0};

Production ops_prodArray_4_16_0_0[1] = {{{9,5}, 65535, -1, 1}};
Production ops_prodArray_4_16_0_1[0] = {};
Production ops_prodArray_4_16_0_2[0] = {};
Production ops_prodArray_4_16_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_16_1_1[0] = {};
Production ops_prodArray_4_16_1_2[0] = {};

GrammarRule ops_ruleArray_4_16[2] = {{{ops_prodArray_4_16_0_0, ops_prodArray_4_16_0_1, ops_prodArray_4_16_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_16_1_0, ops_prodArray_4_16_1_1, ops_prodArray_4_16_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_16 = {ops_ruleArray_4_16, 2, 14, 0};

Production ops_prodArray_4_17_0_0[1] = {{{9,5}, 65535, -1, 1}};
Production ops_prodArray_4_17_0_1[0] = {};
Production ops_prodArray_4_17_0_2[0] = {};
Production ops_prodArray_4_17_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_17_1_1[0] = {};
Production ops_prodArray_4_17_1_2[0] = {};

GrammarRule ops_ruleArray_4_17[2] = {{{ops_prodArray_4_17_0_0, ops_prodArray_4_17_0_1, ops_prodArray_4_17_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_17_1_0, ops_prodArray_4_17_1_1, ops_prodArray_4_17_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_17 = {ops_ruleArray_4_17, 2, 14, 0};

Production ops_prodArray_4_18_0_0[1] = {{{9,5}, 65535, -1, 1}};
Production ops_prodArray_4_18_0_1[0] = {};
Production ops_prodArray_4_18_0_2[0] = {};
Production ops_prodArray_4_18_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_18_1_1[0] = {};
Production ops_prodArray_4_18_1_2[0] = {};

GrammarRule ops_ruleArray_4_18[2] = {{{ops_prodArray_4_18_0_0, ops_prodArray_4_18_0_1, ops_prodArray_4_18_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_18_1_0, ops_prodArray_4_18_1_1, ops_prodArray_4_18_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_18 = {ops_ruleArray_4_18, 2, 14, 0};

Production ops_prodArray_4_19_0_0[1] = {{{9,5}, 65535, -1, 1}};
Production ops_prodArray_4_19_0_1[0] = {};
Production ops_prodArray_4_19_0_2[0] = {};
Production ops_prodArray_4_19_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_19_1_1[0] = {};
Production ops_prodArray_4_19_1_2[0] = {};

GrammarRule ops_ruleArray_4_19[2] = {{{ops_prodArray_4_19_0_0, ops_prodArray_4_19_0_1, ops_prodArray_4_19_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_19_1_0, ops_prodArray_4_19_1_1, ops_prodArray_4_19_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_19 = {ops_ruleArray_4_19, 2, 14, 0};

Production ops_prodArray_4_20_0_0[4] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 33, 3},{{5,1}, 4, 6, 2},{{5,1}, 4, 25, 1}};
Production ops_prodArray_4_20_0_1[0] = {};
Production ops_prodArray_4_20_0_2[0] = {};
Production ops_prodArray_4_20_1_0[3] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 33, 3},{{5,1}, 4, 6, 2}};
Production ops_prodArray_4_20_1_1[0] = {};
Production ops_prodArray_4_20_1_2[0] = {};
Production ops_prodArray_4_20_2_0[2] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 33, 3}};
Production ops_prodArray_4_20_2_1[0] = {};
Production ops_prodArray_4_20_2_2[0] = {};
Production ops_prodArray_4_20_3_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_20_3_1[0] = {};
Production ops_prodArray_4_20_3_2[0] = {};

GrammarRule ops_ruleArray_4_20[4] = {{{ops_prodArray_4_20_0_0, ops_prodArray_4_20_0_1, ops_prodArray_4_20_0_2}, {4, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_20_1_0, ops_prodArray_4_20_1_1, ops_prodArray_4_20_1_2}, {3, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_20_2_0, ops_prodArray_4_20_2_1, ops_prodArray_4_20_2_2}, {2, 0, 0}, {1, 0, 0}},{{ops_prodArray_4_20_3_0, ops_prodArray_4_20_3_1, ops_prodArray_4_20_3_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_20 = {ops_ruleArray_4_20, 4, 14, 0};

Production ops_prodArray_4_21_0_0[1] = {{{9,7}, 65535, -1, 1}};
Production ops_prodArray_4_21_0_1[0] = {};
Production ops_prodArray_4_21_0_2[0] = {};
Production ops_prodArray_4_21_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_21_1_1[0] = {};
Production ops_prodArray_4_21_1_2[0] = {};

GrammarRule ops_ruleArray_4_21[2] = {{{ops_prodArray_4_21_0_0, ops_prodArray_4_21_0_1, ops_prodArray_4_21_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_21_1_0, ops_prodArray_4_21_1_1, ops_prodArray_4_21_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_21 = {ops_ruleArray_4_21, 2, 14, 0};

Production ops_prodArray_4_22_0_0[1] = {{{9,3}, 65535, -1, 1}};
Production ops_prodArray_4_22_0_1[0] = {};
Production ops_prodArray_4_22_0_2[0] = {};
Production ops_prodArray_4_22_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_22_1_1[0] = {};
Production ops_prodArray_4_22_1_2[0] = {};

GrammarRule ops_ruleArray_4_22[2] = {{{ops_prodArray_4_22_0_0, ops_prodArray_4_22_0_1, ops_prodArray_4_22_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_22_1_0, ops_prodArray_4_22_1_1, ops_prodArray_4_22_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_22 = {ops_ruleArray_4_22, 2, 14, 0};

Production ops_prodArray_4_23_0_0[1] = {{{9,3}, 65535, -1, 1}};
Production ops_prodArray_4_23_0_1[0] = {};
Production ops_prodArray_4_23_0_2[0] = {};
Production ops_prodArray_4_23_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_23_1_1[0] = {};
Production ops_prodArray_4_23_1_2[0] = {};

GrammarRule ops_ruleArray_4_23[2] = {{{ops_prodArray_4_23_0_0, ops_prodArray_4_23_0_1, ops_prodArray_4_23_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_23_1_0, ops_prodArray_4_23_1_1, ops_prodArray_4_23_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_23 = {ops_ruleArray_4_23, 2, 14, 0};

Production ops_prodArray_4_24_0_0[1] = {{{9,20}, 65535, -1, 1}};
Production ops_prodArray_4_24_0_1[0] = {};
Production ops_prodArray_4_24_0_2[0] = {};
Production ops_prodArray_4_24_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_24_1_1[0] = {};
Production ops_prodArray_4_24_1_2[0] = {};

GrammarRule ops_ruleArray_4_24[2] = {{{ops_prodArray_4_24_0_0, ops_prodArray_4_24_0_1, ops_prodArray_4_24_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_24_1_0, ops_prodArray_4_24_1_1, ops_prodArray_4_24_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_24 = {ops_ruleArray_4_24, 2, 14, 0};

Production ops_prodArray_4_25_0_0[4] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 2, 3},{{5,1}, 4, 30, 2},{{5,1}, 4, 36, 1}};
Production ops_prodArray_4_25_0_1[0] = {};
Production ops_prodArray_4_25_0_2[0] = {};
Production ops_prodArray_4_25_1_0[3] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 2, 3},{{5,1}, 4, 30, 2}};
Production ops_prodArray_4_25_1_1[0] = {};
Production ops_prodArray_4_25_1_2[0] = {};
Production ops_prodArray_4_25_2_0[2] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 2, 3}};
Production ops_prodArray_4_25_2_1[0] = {};
Production ops_prodArray_4_25_2_2[0] = {};
Production ops_prodArray_4_25_3_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_25_3_1[0] = {};
Production ops_prodArray_4_25_3_2[0] = {};

GrammarRule ops_ruleArray_4_25[4] = {{{ops_prodArray_4_25_0_0, ops_prodArray_4_25_0_1, ops_prodArray_4_25_0_2}, {4, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_25_1_0, ops_prodArray_4_25_1_1, ops_prodArray_4_25_1_2}, {3, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_25_2_0, ops_prodArray_4_25_2_1, ops_prodArray_4_25_2_2}, {2, 0, 0}, {1, 0, 0}},{{ops_prodArray_4_25_3_0, ops_prodArray_4_25_3_1, ops_prodArray_4_25_3_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_25 = {ops_ruleArray_4_25, 4, 14, 0};

Production ops_prodArray_4_30_0_0[6] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 27, 5},{{5,1}, 4, 5, 4},{{5,1}, 4, 26, 3},{{5,1}, 4, 29, 2},{{5,1}, 4, 13, 1}};
Production ops_prodArray_4_30_0_1[0] = {};
Production ops_prodArray_4_30_0_2[0] = {};
Production ops_prodArray_4_30_1_0[5] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 27, 5},{{5,1}, 4, 5, 4},{{5,1}, 4, 26, 3},{{5,1}, 4, 29, 2}};
Production ops_prodArray_4_30_1_1[0] = {};
Production ops_prodArray_4_30_1_2[0] = {};
Production ops_prodArray_4_30_2_0[4] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 27, 5},{{5,1}, 4, 5, 4},{{5,1}, 4, 26, 3}};
Production ops_prodArray_4_30_2_1[0] = {};
Production ops_prodArray_4_30_2_2[0] = {};
Production ops_prodArray_4_30_3_0[3] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 27, 5},{{5,1}, 4, 5, 4}};
Production ops_prodArray_4_30_3_1[0] = {};
Production ops_prodArray_4_30_3_2[0] = {};
Production ops_prodArray_4_30_4_0[2] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 27, 5}};
Production ops_prodArray_4_30_4_1[0] = {};
Production ops_prodArray_4_30_4_2[0] = {};
Production ops_prodArray_4_30_5_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_30_5_1[0] = {};
Production ops_prodArray_4_30_5_2[0] = {};

GrammarRule ops_ruleArray_4_30[6] = {{{ops_prodArray_4_30_0_0, ops_prodArray_4_30_0_1, ops_prodArray_4_30_0_2}, {6, 0, 0}, {3, 0, 0}},{{ops_prodArray_4_30_1_0, ops_prodArray_4_30_1_1, ops_prodArray_4_30_1_2}, {5, 0, 0}, {3, 0, 0}},{{ops_prodArray_4_30_2_0, ops_prodArray_4_30_2_1, ops_prodArray_4_30_2_2}, {4, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_30_3_0, ops_prodArray_4_30_3_1, ops_prodArray_4_30_3_2}, {3, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_30_4_0, ops_prodArray_4_30_4_1, ops_prodArray_4_30_4_2}, {2, 0, 0}, {1, 0, 0}},{{ops_prodArray_4_30_5_0, ops_prodArray_4_30_5_1, ops_prodArray_4_30_5_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_30 = {ops_ruleArray_4_30, 6, 14, 0};

Production ops_prodArray_4_31_0_0[1] = {{{9,2}, 65535, -1, 1}};
Production ops_prodArray_4_31_0_1[1] = {{{2,1}, 2, 0, 0}};
Production ops_prodArray_4_31_0_2[0] = {};
Production ops_prodArray_4_31_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_31_1_1[0] = {};
Production ops_prodArray_4_31_1_2[0] = {};

GrammarRule ops_ruleArray_4_31[2] = {{{ops_prodArray_4_31_0_0, ops_prodArray_4_31_0_1, ops_prodArray_4_31_0_2}, {1, 1, 0}, {1, 0, 0}},{{ops_prodArray_4_31_1_0, ops_prodArray_4_31_1_1, ops_prodArray_4_31_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_31 = {ops_ruleArray_4_31, 2, 14, 0};

Production ops_prodArray_4_34_0_0[1] = {{{9,2}, 65535, -1, 1}};
Production ops_prodArray_4_34_0_1[0] = {};
Production ops_prodArray_4_34_0_2[0] = {};
Production ops_prodArray_4_34_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_34_1_1[0] = {};
Production ops_prodArray_4_34_1_2[0] = {};

GrammarRule ops_ruleArray_4_34[2] = {{{ops_prodArray_4_34_0_0, ops_prodArray_4_34_0_1, ops_prodArray_4_34_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_34_1_0, ops_prodArray_4_34_1_1, ops_prodArray_4_34_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_34 = {ops_ruleArray_4_34, 2, 14, 0};

Production ops_prodArray_4_35_0_0[1] = {{{9,5}, 65535, -1, 1}};
Production ops_prodArray_4_35_0_1[0] = {};
Production ops_prodArray_4_35_0_2[0] = {};
Production ops_prodArray_4_35_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_35_1_1[0] = {};
Production ops_prodArray_4_35_1_2[0] = {};

GrammarRule ops_ruleArray_4_35[2] = {{{ops_prodArray_4_35_0_0, ops_prodArray_4_35_0_1, ops_prodArray_4_35_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_35_1_0, ops_prodArray_4_35_1_1, ops_prodArray_4_35_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_35 = {ops_ruleArray_4_35, 2, 14, 0};

Production ops_prodArray_4_36_0_0[7] = {{{8,1}, 65535, -1, -1},{{7,1}, 65535, -1, 1},{{5,1}, 4, 8, 5},{{5,1}, 4, 38, 4},{{5,1}, 4, 37, 3},{{5,1}, 4, 32, 2},{{5,1}, 4, 0, 1}};
Production ops_prodArray_4_36_0_1[0] = {};
Production ops_prodArray_4_36_0_2[0] = {};
Production ops_prodArray_4_36_1_0[5] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 8, 5},{{5,1}, 4, 38, 4},{{5,1}, 4, 37, 3},{{5,1}, 4, 32, 2}};
Production ops_prodArray_4_36_1_1[0] = {};
Production ops_prodArray_4_36_1_2[0] = {};
Production ops_prodArray_4_36_2_0[4] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 8, 5},{{5,1}, 4, 38, 4},{{5,1}, 4, 37, 3}};
Production ops_prodArray_4_36_2_1[0] = {};
Production ops_prodArray_4_36_2_2[0] = {};
Production ops_prodArray_4_36_3_0[3] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 8, 5},{{5,1}, 4, 38, 4}};
Production ops_prodArray_4_36_3_1[0] = {};
Production ops_prodArray_4_36_3_2[0] = {};
Production ops_prodArray_4_36_4_0[2] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 8, 5}};
Production ops_prodArray_4_36_4_1[0] = {};
Production ops_prodArray_4_36_4_2[0] = {};
Production ops_prodArray_4_36_5_0[2] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 8, 5}};
Production ops_prodArray_4_36_5_1[0] = {};
Production ops_prodArray_4_36_5_2[0] = {};

GrammarRule ops_ruleArray_4_36[6] = {{{ops_prodArray_4_36_0_0, ops_prodArray_4_36_0_1, ops_prodArray_4_36_0_2}, {7, 0, 0}, {3, 0, 0}},{{ops_prodArray_4_36_1_0, ops_prodArray_4_36_1_1, ops_prodArray_4_36_1_2}, {5, 0, 0}, {3, 0, 0}},{{ops_prodArray_4_36_2_0, ops_prodArray_4_36_2_1, ops_prodArray_4_36_2_2}, {4, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_36_3_0, ops_prodArray_4_36_3_1, ops_prodArray_4_36_3_2}, {3, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_36_4_0, ops_prodArray_4_36_4_1, ops_prodArray_4_36_4_2}, {2, 0, 0}, {1, 0, 0}},{{ops_prodArray_4_36_5_0, ops_prodArray_4_36_5_1, ops_prodArray_4_36_5_2}, {2, 0, 0}, {1, 0, 0}}};

EXIGrammar ops_grammar_4_36 = {ops_ruleArray_4_36, 6, 14, 0};

Production ops_prodArray_4_37_0_0[1] = {{{9,22}, 65535, -1, 1}};
Production ops_prodArray_4_37_0_1[0] = {};
Production ops_prodArray_4_37_0_2[0] = {};
Production ops_prodArray_4_37_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_37_1_1[0] = {};
Production ops_prodArray_4_37_1_2[0] = {};

GrammarRule ops_ruleArray_4_37[2] = {{{ops_prodArray_4_37_0_0, ops_prodArray_4_37_0_1, ops_prodArray_4_37_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_37_1_0, ops_prodArray_4_37_1_1, ops_prodArray_4_37_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_37 = {ops_ruleArray_4_37, 2, 14, 0};

Production ops_prodArray_4_38_0_0[1] = {{{9,22}, 65535, -1, 1}};
Production ops_prodArray_4_38_0_1[0] = {};
Production ops_prodArray_4_38_0_2[0] = {};
Production ops_prodArray_4_38_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_38_1_1[0] = {};
Production ops_prodArray_4_38_1_2[0] = {};

GrammarRule ops_ruleArray_4_38[2] = {{{ops_prodArray_4_38_0_0, ops_prodArray_4_38_0_1, ops_prodArray_4_38_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_38_1_0, ops_prodArray_4_38_1_1, ops_prodArray_4_38_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_38 = {ops_ruleArray_4_38, 2, 14, 0};

struct LocalNamesRow ops_LNrows_4[39] = {{NULL, {"alignment", 9}, &ops_grammar_4_0},{NULL, {"base64Binary", 12}, &ops_grammar_4_1},{NULL, {"blockSize", 9}, &ops_grammar_4_2},{NULL, {"boolean", 7}, &ops_grammar_4_3},{NULL, {"byte", 4}, &ops_empty_grammar},{NULL, {"comments", 8}, &ops_empty_grammar},{NULL, {"common", 6}, &ops_grammar_4_6},{NULL, {"compression", 11}, &ops_empty_grammar},{NULL, {"datatypeRepresentationMap", 25}, &ops_grammar_4_8},{NULL, {"dateTime", 8}, &ops_grammar_4_9},{NULL, {"date", 4}, &ops_grammar_4_10},{NULL, {"decimal", 7}, &ops_grammar_4_11},{NULL, {"double", 6}, &ops_grammar_4_12},{NULL, {"dtd", 3}, &ops_empty_grammar},{NULL, {"fragment", 8}, &ops_empty_grammar},{NULL, {"gDay", 4}, &ops_grammar_4_15},{NULL, {"gMonthDay", 9}, &ops_grammar_4_16},{NULL, {"gMonth", 6}, &ops_grammar_4_17},{NULL, {"gYearMonth", 10}, &ops_grammar_4_18},{NULL, {"gYear", 5}, &ops_grammar_4_19},{NULL, {"header", 6}, &ops_grammar_4_20},{NULL, {"hexBinary", 9}, &ops_grammar_4_21},{NULL, {"ieeeBinary32", 12}, &ops_grammar_4_22},{NULL, {"ieeeBinary64", 12}, &ops_grammar_4_23},{NULL, {"integer", 7}, &ops_grammar_4_24},{NULL, {"lesscommon", 10}, &ops_grammar_4_25},{NULL, {"lexicalValues", 13}, &ops_empty_grammar},{NULL, {"pis", 3}, &ops_empty_grammar},{NULL, {"pre-compress", 12}, &ops_empty_grammar},{NULL, {"prefixes", 8}, &ops_empty_grammar},{NULL, {"preserve", 8}, &ops_grammar_4_30},{NULL, {"schemaId", 8}, &ops_grammar_4_31},{NULL, {"selfContained", 13}, &ops_empty_grammar},{NULL, {"strict", 6}, &ops_empty_grammar},{NULL, {"string", 6}, &ops_grammar_4_34},{NULL, {"time", 4}, &ops_grammar_4_35},{NULL, {"uncommon", 8}, &ops_grammar_4_36},{NULL, {"valueMaxLength", 14}, &ops_grammar_4_37},{NULL, {"valuePartitionCapacity", 22}, &ops_grammar_4_38}};
LocalNamesTable ops_lTable_4 = { ops_LNrows_4, 39, 39, {NULL, 0}};

struct URIRow ops_uriRows[5] = {{&ops_pTable_0, &ops_lTable_0, {"", 0}},{&ops_pTable_1, &ops_lTable_1, {"http://www.w3.org/XML/1998/namespace", 36}},{&ops_pTable_2, &ops_lTable_2, {"http://www.w3.org/2001/XMLSchema-instance", 41}},{NULL, &ops_lTable_3, {"http://www.w3.org/2001/XMLSchema", 32}},{NULL, &ops_lTable_4, {"http://www.w3.org/2009/exi", 26}}};
URITable ops_uriTbl = {ops_uriRows, 5, 5, {NULL, 0}};

QNameID ops_qnames[1] = {{4, 20}};

const ExipSchema ops_schema = {&ops_uriTbl, ops_qnames, 1, {NULL, NULL}};
