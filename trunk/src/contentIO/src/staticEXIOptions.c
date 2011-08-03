/** AUTO-GENERATED: Wed Aug  3 13:47:51 2011
  * Copyright (c) 2010 - 2011, Rumen Kyusakov, EISLAB, LTU
  * $Id$ */

#include "procTypes.h"

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

Production ops_prodArray_4_0_0_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_0_0_1[0] = {};
Production ops_prodArray_4_0_0_2[0] = {};

GrammarRule ops_ruleArray_4_0[1] = {{{ops_prodArray_4_0_0_0, ops_prodArray_4_0_0_1, ops_prodArray_4_0_0_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar empty_grammar = {ops_ruleArray_4_0, 1, 13, 0};

Production ops_prodArray_4_1_0_0[1] = {{{9,5}, 65535, -1, 1}};
Production ops_prodArray_4_1_0_1[0] = {};
Production ops_prodArray_4_1_0_2[0] = {};
Production ops_prodArray_4_1_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_1_1_1[0] = {};
Production ops_prodArray_4_1_1_2[0] = {};

GrammarRule ops_ruleArray_4_1[2] = {{{ops_prodArray_4_1_0_0, ops_prodArray_4_1_0_1, ops_prodArray_4_1_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_1_1_0, ops_prodArray_4_1_1_1, ops_prodArray_4_1_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_1 = {ops_ruleArray_4_1, 2, 14, 0};

Production ops_prodArray_4_2_0_0[4] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 38, 3},{{5,1}, 4, 37, 2},{{5,1}, 4, 36, 1}};
Production ops_prodArray_4_2_0_1[0] = {};
Production ops_prodArray_4_2_0_2[0] = {};
Production ops_prodArray_4_2_1_0[3] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 38, 3},{{5,1}, 4, 37, 2}};
Production ops_prodArray_4_2_1_1[0] = {};
Production ops_prodArray_4_2_1_2[0] = {};
Production ops_prodArray_4_2_2_0[2] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 38, 3}};
Production ops_prodArray_4_2_2_1[0] = {};
Production ops_prodArray_4_2_2_2[0] = {};
Production ops_prodArray_4_2_3_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_2_3_1[0] = {};
Production ops_prodArray_4_2_3_2[0] = {};

GrammarRule ops_ruleArray_4_2[4] = {{{ops_prodArray_4_2_0_0, ops_prodArray_4_2_0_1, ops_prodArray_4_2_0_2}, {4, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_2_1_0, ops_prodArray_4_2_1_1, ops_prodArray_4_2_1_2}, {3, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_2_2_0, ops_prodArray_4_2_2_1, ops_prodArray_4_2_2_2}, {2, 0, 0}, {1, 0, 0}},{{ops_prodArray_4_2_3_0, ops_prodArray_4_2_3_1, ops_prodArray_4_2_3_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_2 = {ops_ruleArray_4_2, 4, 14, 0};

Production ops_prodArray_4_4_0_0[4] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 3, 3},{{5,1}, 4, 2, 2},{{5,1}, 4, 35, 1}};
Production ops_prodArray_4_4_0_1[0] = {};
Production ops_prodArray_4_4_0_2[0] = {};
Production ops_prodArray_4_4_1_0[3] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 3, 3},{{5,1}, 4, 2, 2}};
Production ops_prodArray_4_4_1_1[0] = {};
Production ops_prodArray_4_4_1_2[0] = {};
Production ops_prodArray_4_4_2_0[2] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 3, 3}};
Production ops_prodArray_4_4_2_1[0] = {};
Production ops_prodArray_4_4_2_2[0] = {};
Production ops_prodArray_4_4_3_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_4_3_1[0] = {};
Production ops_prodArray_4_4_3_2[0] = {};

GrammarRule ops_ruleArray_4_4[4] = {{{ops_prodArray_4_4_0_0, ops_prodArray_4_4_0_1, ops_prodArray_4_4_0_2}, {4, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_4_1_0, ops_prodArray_4_4_1_1, ops_prodArray_4_4_1_2}, {3, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_4_2_0, ops_prodArray_4_4_2_1, ops_prodArray_4_4_2_2}, {2, 0, 0}, {1, 0, 0}},{{ops_prodArray_4_4_3_0, ops_prodArray_4_4_3_1, ops_prodArray_4_4_3_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_4 = {ops_ruleArray_4_4, 4, 14, 0};

Production ops_prodArray_4_5_0_0[1] = {{{9,6}, 65535, -1, 1}};
Production ops_prodArray_4_5_0_1[0] = {};
Production ops_prodArray_4_5_0_2[0] = {};
Production ops_prodArray_4_5_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_5_1_1[0] = {};
Production ops_prodArray_4_5_1_2[0] = {};

GrammarRule ops_ruleArray_4_5[2] = {{{ops_prodArray_4_5_0_0, ops_prodArray_4_5_0_1, ops_prodArray_4_5_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_5_1_0, ops_prodArray_4_5_1_1, ops_prodArray_4_5_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_5 = {ops_ruleArray_4_5, 2, 14, 0};

Production ops_prodArray_4_6_0_0[1] = {{{9,5}, 65535, -1, 1}};
Production ops_prodArray_4_6_0_1[0] = {};
Production ops_prodArray_4_6_0_2[0] = {};
Production ops_prodArray_4_6_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_6_1_1[0] = {};
Production ops_prodArray_4_6_1_2[0] = {};

GrammarRule ops_ruleArray_4_6[2] = {{{ops_prodArray_4_6_0_0, ops_prodArray_4_6_0_1, ops_prodArray_4_6_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_6_1_0, ops_prodArray_4_6_1_1, ops_prodArray_4_6_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_6 = {ops_ruleArray_4_6, 2, 14, 0};

Production ops_prodArray_4_7_0_0[1] = {{{9,5}, 65535, -1, 1}};
Production ops_prodArray_4_7_0_1[0] = {};
Production ops_prodArray_4_7_0_2[0] = {};
Production ops_prodArray_4_7_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_7_1_1[0] = {};
Production ops_prodArray_4_7_1_2[0] = {};

GrammarRule ops_ruleArray_4_7[2] = {{{ops_prodArray_4_7_0_0, ops_prodArray_4_7_0_1, ops_prodArray_4_7_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_7_1_0, ops_prodArray_4_7_1_1, ops_prodArray_4_7_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_7 = {ops_ruleArray_4_7, 2, 14, 0};

Production ops_prodArray_4_8_0_0[1] = {{{9,5}, 65535, -1, 1}};
Production ops_prodArray_4_8_0_1[0] = {};
Production ops_prodArray_4_8_0_2[0] = {};
Production ops_prodArray_4_8_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_8_1_1[0] = {};
Production ops_prodArray_4_8_1_2[0] = {};

GrammarRule ops_ruleArray_4_8[2] = {{{ops_prodArray_4_8_0_0, ops_prodArray_4_8_0_1, ops_prodArray_4_8_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_8_1_0, ops_prodArray_4_8_1_1, ops_prodArray_4_8_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_8 = {ops_ruleArray_4_8, 2, 14, 0};

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

Production ops_prodArray_4_11_0_0[2] = {{{5,1}, 4, 16, 1},{{5,1}, 4, 0, 2}};
Production ops_prodArray_4_11_0_1[0] = {};
Production ops_prodArray_4_11_0_2[0] = {};
Production ops_prodArray_4_11_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_11_1_1[0] = {};
Production ops_prodArray_4_11_1_2[0] = {};
Production ops_prodArray_4_11_2_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_11_2_1[0] = {};
Production ops_prodArray_4_11_2_2[0] = {};

GrammarRule ops_ruleArray_4_11[3] = {{{ops_prodArray_4_11_0_0, ops_prodArray_4_11_0_1, ops_prodArray_4_11_0_2}, {2, 0, 0}, {1, 0, 0}},{{ops_prodArray_4_11_1_0, ops_prodArray_4_11_1_1, ops_prodArray_4_11_1_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_11_2_0, ops_prodArray_4_11_2_1, ops_prodArray_4_11_2_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_11 = {ops_ruleArray_4_11, 3, 14, 0};

Production ops_prodArray_4_12_0_0[1] = {{{9,7}, 65535, -1, 1}};
Production ops_prodArray_4_12_0_1[0] = {};
Production ops_prodArray_4_12_0_2[0] = {};
Production ops_prodArray_4_12_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_12_1_1[0] = {};
Production ops_prodArray_4_12_1_2[0] = {};

GrammarRule ops_ruleArray_4_12[2] = {{{ops_prodArray_4_12_0_0, ops_prodArray_4_12_0_1, ops_prodArray_4_12_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_12_1_0, ops_prodArray_4_12_1_1, ops_prodArray_4_12_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_12 = {ops_ruleArray_4_12, 2, 14, 0};

Production ops_prodArray_4_13_0_0[1] = {{{9,4}, 65535, -1, 1}};
Production ops_prodArray_4_13_0_1[0] = {};
Production ops_prodArray_4_13_0_2[0] = {};
Production ops_prodArray_4_13_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_13_1_1[0] = {};
Production ops_prodArray_4_13_1_2[0] = {};

GrammarRule ops_ruleArray_4_13[2] = {{{ops_prodArray_4_13_0_0, ops_prodArray_4_13_0_1, ops_prodArray_4_13_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_13_1_0, ops_prodArray_4_13_1_1, ops_prodArray_4_13_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_13 = {ops_ruleArray_4_13, 2, 14, 0};

Production ops_prodArray_4_14_0_0[1] = {{{9,3}, 65535, -1, 1}};
Production ops_prodArray_4_14_0_1[0] = {};
Production ops_prodArray_4_14_0_2[0] = {};
Production ops_prodArray_4_14_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_14_1_1[0] = {};
Production ops_prodArray_4_14_1_2[0] = {};

GrammarRule ops_ruleArray_4_14[2] = {{{ops_prodArray_4_14_0_0, ops_prodArray_4_14_0_1, ops_prodArray_4_14_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_14_1_0, ops_prodArray_4_14_1_1, ops_prodArray_4_14_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_14 = {ops_ruleArray_4_14, 2, 14, 0};

Production ops_prodArray_4_15_0_0[1] = {{{9,2}, 65535, -1, 1}};
Production ops_prodArray_4_15_0_1[0] = {};
Production ops_prodArray_4_15_0_2[0] = {};
Production ops_prodArray_4_15_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_15_1_1[0] = {};
Production ops_prodArray_4_15_1_2[0] = {};

GrammarRule ops_ruleArray_4_15[2] = {{{ops_prodArray_4_15_0_0, ops_prodArray_4_15_0_1, ops_prodArray_4_15_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_15_1_0, ops_prodArray_4_15_1_1, ops_prodArray_4_15_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_15 = {ops_ruleArray_4_15, 2, 14, 0};

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

Production ops_prodArray_4_19_0_0[1] = {{{9,7}, 65535, -1, 1}};
Production ops_prodArray_4_19_0_1[0] = {};
Production ops_prodArray_4_19_0_2[0] = {};
Production ops_prodArray_4_19_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_19_1_1[0] = {};
Production ops_prodArray_4_19_1_2[0] = {};

GrammarRule ops_ruleArray_4_19[2] = {{{ops_prodArray_4_19_0_0, ops_prodArray_4_19_0_1, ops_prodArray_4_19_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_19_1_0, ops_prodArray_4_19_1_1, ops_prodArray_4_19_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_19 = {ops_ruleArray_4_19, 2, 14, 0};

Production ops_prodArray_4_20_0_0[1] = {{{9,20}, 65535, -1, 1}};
Production ops_prodArray_4_20_0_1[0] = {};
Production ops_prodArray_4_20_0_2[0] = {};
Production ops_prodArray_4_20_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_20_1_1[0] = {};
Production ops_prodArray_4_20_1_2[0] = {};

GrammarRule ops_ruleArray_4_20[2] = {{{ops_prodArray_4_20_0_0, ops_prodArray_4_20_0_1, ops_prodArray_4_20_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_20_1_0, ops_prodArray_4_20_1_1, ops_prodArray_4_20_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_20 = {ops_ruleArray_4_20, 2, 14, 0};

Production ops_prodArray_4_24_0_0[1] = {{{9,3}, 65535, -1, 1}};
Production ops_prodArray_4_24_0_1[0] = {};
Production ops_prodArray_4_24_0_2[0] = {};
Production ops_prodArray_4_24_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_24_1_1[0] = {};
Production ops_prodArray_4_24_1_2[0] = {};

GrammarRule ops_ruleArray_4_24[2] = {{{ops_prodArray_4_24_0_0, ops_prodArray_4_24_0_1, ops_prodArray_4_24_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_24_1_0, ops_prodArray_4_24_1_1, ops_prodArray_4_24_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_24 = {ops_ruleArray_4_24, 2, 14, 0};

Production ops_prodArray_4_25_0_0[1] = {{{9,3}, 65535, -1, 1}};
Production ops_prodArray_4_25_0_1[0] = {};
Production ops_prodArray_4_25_0_2[0] = {};
Production ops_prodArray_4_25_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_25_1_1[0] = {};
Production ops_prodArray_4_25_1_2[0] = {};

GrammarRule ops_ruleArray_4_25[2] = {{{ops_prodArray_4_25_0_0, ops_prodArray_4_25_0_1, ops_prodArray_4_25_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_25_1_0, ops_prodArray_4_25_1_1, ops_prodArray_4_25_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_25 = {ops_ruleArray_4_25, 2, 14, 0};

Production ops_prodArray_4_26_0_0[1] = {{{9,22}, 65535, -1, 1}};
Production ops_prodArray_4_26_0_1[0] = {};
Production ops_prodArray_4_26_0_2[0] = {};
Production ops_prodArray_4_26_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_26_1_1[0] = {};
Production ops_prodArray_4_26_1_2[0] = {};

GrammarRule ops_ruleArray_4_26[2] = {{{ops_prodArray_4_26_0_0, ops_prodArray_4_26_0_1, ops_prodArray_4_26_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_26_1_0, ops_prodArray_4_26_1_1, ops_prodArray_4_26_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_26 = {ops_ruleArray_4_26, 2, 14, 0};

Production ops_prodArray_4_27_0_0[1] = {{{9,22}, 65535, -1, 1}};
Production ops_prodArray_4_27_0_1[0] = {};
Production ops_prodArray_4_27_0_2[0] = {};
Production ops_prodArray_4_27_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_27_1_1[0] = {};
Production ops_prodArray_4_27_1_2[0] = {};

GrammarRule ops_ruleArray_4_27[2] = {{{ops_prodArray_4_27_0_0, ops_prodArray_4_27_0_1, ops_prodArray_4_27_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_27_1_0, ops_prodArray_4_27_1_1, ops_prodArray_4_27_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_27 = {ops_ruleArray_4_27, 2, 14, 0};

Production ops_prodArray_4_29_0_0[1] = {{{7,1}, 65535, -1, 1}};
Production ops_prodArray_4_29_0_1[0] = {};
Production ops_prodArray_4_29_0_2[0] = {};
Production ops_prodArray_4_29_1_0[1] = {{{7,1}, 65535, -1, 2}};
Production ops_prodArray_4_29_1_1[0] = {};
Production ops_prodArray_4_29_1_2[0] = {};
Production ops_prodArray_4_29_2_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_29_2_1[0] = {};
Production ops_prodArray_4_29_2_2[0] = {};

GrammarRule ops_ruleArray_4_29[3] = {{{ops_prodArray_4_29_0_0, ops_prodArray_4_29_0_1, ops_prodArray_4_29_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_29_1_0, ops_prodArray_4_29_1_1, ops_prodArray_4_29_1_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_29_2_0, ops_prodArray_4_29_2_1, ops_prodArray_4_29_2_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_29 = {ops_ruleArray_4_29, 3, 14, 0};

Production ops_prodArray_4_31_0_0[7] = {{{8,1}, 65535, -1, -1},{{7,1}, 65535, -1, 1},{{5,1}, 4, 29, 6},{{5,1}, 4, 27, 5},{{5,1}, 4, 26, 4},{{5,1}, 4, 21, 3},{{5,1}, 4, 11, 2}};
Production ops_prodArray_4_31_0_1[0] = {};
Production ops_prodArray_4_31_0_2[0] = {};
Production ops_prodArray_4_31_1_0[7] = {{{8,1}, 65535, -1, -1},{{7,1}, 65535, -1, 1},{{5,1}, 4, 29, 6},{{5,1}, 4, 27, 5},{{5,1}, 4, 26, 4},{{5,1}, 4, 21, 3},{{5,1}, 4, 11, 2}};
Production ops_prodArray_4_31_1_1[0] = {};
Production ops_prodArray_4_31_1_2[0] = {};
Production ops_prodArray_4_31_2_0[5] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 29, 6},{{5,1}, 4, 27, 5},{{5,1}, 4, 26, 4},{{5,1}, 4, 21, 3}};
Production ops_prodArray_4_31_2_1[0] = {};
Production ops_prodArray_4_31_2_2[0] = {};
Production ops_prodArray_4_31_3_0[4] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 29, 6},{{5,1}, 4, 27, 5},{{5,1}, 4, 26, 4}};
Production ops_prodArray_4_31_3_1[0] = {};
Production ops_prodArray_4_31_3_2[0] = {};
Production ops_prodArray_4_31_4_0[3] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 29, 6},{{5,1}, 4, 27, 5}};
Production ops_prodArray_4_31_4_1[0] = {};
Production ops_prodArray_4_31_4_2[0] = {};
Production ops_prodArray_4_31_5_0[2] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 29, 6}};
Production ops_prodArray_4_31_5_1[0] = {};
Production ops_prodArray_4_31_5_2[0] = {};
Production ops_prodArray_4_31_6_0[2] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 29, 6}};
Production ops_prodArray_4_31_6_1[0] = {};
Production ops_prodArray_4_31_6_2[0] = {};

GrammarRule ops_ruleArray_4_31[7] = {{{ops_prodArray_4_31_0_0, ops_prodArray_4_31_0_1, ops_prodArray_4_31_0_2}, {7, 0, 0}, {3, 0, 0}},{{ops_prodArray_4_31_1_0, ops_prodArray_4_31_1_1, ops_prodArray_4_31_1_2}, {7, 0, 0}, {3, 0, 0}},{{ops_prodArray_4_31_2_0, ops_prodArray_4_31_2_1, ops_prodArray_4_31_2_2}, {5, 0, 0}, {3, 0, 0}},{{ops_prodArray_4_31_3_0, ops_prodArray_4_31_3_1, ops_prodArray_4_31_3_2}, {4, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_31_4_0, ops_prodArray_4_31_4_1, ops_prodArray_4_31_4_2}, {3, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_31_5_0, ops_prodArray_4_31_5_1, ops_prodArray_4_31_5_2}, {2, 0, 0}, {1, 0, 0}},{{ops_prodArray_4_31_6_0, ops_prodArray_4_31_6_1, ops_prodArray_4_31_6_2}, {2, 0, 0}, {1, 0, 0}}};

EXIGrammar ops_grammar_4_31 = {ops_ruleArray_4_31, 7, 14, 0};

Production ops_prodArray_4_32_0_0[6] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 23, 5},{{5,1}, 4, 28, 4},{{5,1}, 4, 34, 3},{{5,1}, 4, 22, 2},{{5,1}, 4, 30, 1}};
Production ops_prodArray_4_32_0_1[0] = {};
Production ops_prodArray_4_32_0_2[0] = {};
Production ops_prodArray_4_32_1_0[5] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 23, 5},{{5,1}, 4, 28, 4},{{5,1}, 4, 34, 3},{{5,1}, 4, 22, 2}};
Production ops_prodArray_4_32_1_1[0] = {};
Production ops_prodArray_4_32_1_2[0] = {};
Production ops_prodArray_4_32_2_0[4] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 23, 5},{{5,1}, 4, 28, 4},{{5,1}, 4, 34, 3}};
Production ops_prodArray_4_32_2_1[0] = {};
Production ops_prodArray_4_32_2_2[0] = {};
Production ops_prodArray_4_32_3_0[3] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 23, 5},{{5,1}, 4, 28, 4}};
Production ops_prodArray_4_32_3_1[0] = {};
Production ops_prodArray_4_32_3_2[0] = {};
Production ops_prodArray_4_32_4_0[2] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 23, 5}};
Production ops_prodArray_4_32_4_1[0] = {};
Production ops_prodArray_4_32_4_2[0] = {};
Production ops_prodArray_4_32_5_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_32_5_1[0] = {};
Production ops_prodArray_4_32_5_2[0] = {};

GrammarRule ops_ruleArray_4_32[6] = {{{ops_prodArray_4_32_0_0, ops_prodArray_4_32_0_1, ops_prodArray_4_32_0_2}, {6, 0, 0}, {3, 0, 0}},{{ops_prodArray_4_32_1_0, ops_prodArray_4_32_1_1, ops_prodArray_4_32_1_2}, {5, 0, 0}, {3, 0, 0}},{{ops_prodArray_4_32_2_0, ops_prodArray_4_32_2_1, ops_prodArray_4_32_2_2}, {4, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_32_3_0, ops_prodArray_4_32_3_1, ops_prodArray_4_32_3_2}, {3, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_32_4_0, ops_prodArray_4_32_4_1, ops_prodArray_4_32_4_2}, {2, 0, 0}, {1, 0, 0}},{{ops_prodArray_4_32_5_0, ops_prodArray_4_32_5_1, ops_prodArray_4_32_5_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_32 = {ops_ruleArray_4_32, 6, 14, 0};

Production ops_prodArray_4_33_0_0[1] = {{{9,22}, 65535, -1, 1}};
Production ops_prodArray_4_33_0_1[0] = {};
Production ops_prodArray_4_33_0_2[0] = {};
Production ops_prodArray_4_33_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_33_1_1[0] = {};
Production ops_prodArray_4_33_1_2[0] = {};

GrammarRule ops_ruleArray_4_33[2] = {{{ops_prodArray_4_33_0_0, ops_prodArray_4_33_0_1, ops_prodArray_4_33_0_2}, {1, 0, 0}, {0, 0, 0}},{{ops_prodArray_4_33_1_0, ops_prodArray_4_33_1_1, ops_prodArray_4_33_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_33 = {ops_ruleArray_4_33, 2, 14, 0};

Production ops_prodArray_4_35_0_0[4] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 33, 3},{{5,1}, 4, 32, 2},{{5,1}, 4, 31, 1}};
Production ops_prodArray_4_35_0_1[0] = {};
Production ops_prodArray_4_35_0_2[0] = {};
Production ops_prodArray_4_35_1_0[3] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 33, 3},{{5,1}, 4, 32, 2}};
Production ops_prodArray_4_35_1_1[0] = {};
Production ops_prodArray_4_35_1_2[0] = {};
Production ops_prodArray_4_35_2_0[2] = {{{8,1}, 65535, -1, -1},{{5,1}, 4, 33, 3}};
Production ops_prodArray_4_35_2_1[0] = {};
Production ops_prodArray_4_35_2_2[0] = {};
Production ops_prodArray_4_35_3_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_35_3_1[0] = {};
Production ops_prodArray_4_35_3_2[0] = {};

GrammarRule ops_ruleArray_4_35[4] = {{{ops_prodArray_4_35_0_0, ops_prodArray_4_35_0_1, ops_prodArray_4_35_0_2}, {4, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_35_1_0, ops_prodArray_4_35_1_1, ops_prodArray_4_35_1_2}, {3, 0, 0}, {2, 0, 0}},{{ops_prodArray_4_35_2_0, ops_prodArray_4_35_2_1, ops_prodArray_4_35_2_2}, {2, 0, 0}, {1, 0, 0}},{{ops_prodArray_4_35_3_0, ops_prodArray_4_35_3_1, ops_prodArray_4_35_3_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_35 = {ops_ruleArray_4_35, 4, 14, 0};

Production ops_prodArray_4_38_0_0[1] = {{{9,2}, 65535, -1, 1}};
Production ops_prodArray_4_38_0_1[1] = {{{2,1}, 2, 0, 0}};
Production ops_prodArray_4_38_0_2[0] = {};
Production ops_prodArray_4_38_1_0[1] = {{{8,1}, 65535, -1, -1}};
Production ops_prodArray_4_38_1_1[0] = {};
Production ops_prodArray_4_38_1_2[0] = {};

GrammarRule ops_ruleArray_4_38[2] = {{{ops_prodArray_4_38_0_0, ops_prodArray_4_38_0_1, ops_prodArray_4_38_0_2}, {1, 1, 0}, {1, 0, 0}},{{ops_prodArray_4_38_1_0, ops_prodArray_4_38_1_1, ops_prodArray_4_38_1_2}, {1, 0, 0}, {0, 0, 0}}};

EXIGrammar ops_grammar_4_38 = {ops_ruleArray_4_38, 2, 14, 0};

struct LocalNamesRow ops_LNrows_4[39] = {{NULL, {"byte", 4}, &empty_grammar},{NULL, {"gYear", 5}, &ops_grammar_4_1},{NULL, {"common", 6}, &ops_grammar_4_2},{NULL, {"strict", 6}, &empty_grammar},{NULL, {"header", 6}, &ops_grammar_4_4},{NULL, {"boolean", 7}, &ops_grammar_4_5},{NULL, {"gMonth", 6}, &ops_grammar_4_6},{NULL, {"gDay", 4}, &ops_grammar_4_7},{NULL, {"dateTime", 8}, &ops_grammar_4_8},{NULL, {"date", 4}, &ops_grammar_4_9},{NULL, {"time", 4}, &ops_grammar_4_10},{NULL, {"alignment", 9}, &ops_grammar_4_11},{NULL, {"hexBinary", 9}, &ops_grammar_4_12},{NULL, {"decimal", 7}, &ops_grammar_4_13},{NULL, {"double", 6}, &ops_grammar_4_14},{NULL, {"string", 6}, &ops_grammar_4_15},{NULL, {"pre-compress", 12}, &empty_grammar},{NULL, {"gYearMonth", 10}, &ops_grammar_4_17},{NULL, {"gMonthDay", 9}, &ops_grammar_4_18},{NULL, {"base64Binary", 12}, &ops_grammar_4_19},{NULL, {"integer", 7}, &ops_grammar_4_20},{NULL, {"selfContained", 13}, &empty_grammar},{NULL, {"prefixes", 8}, &empty_grammar},{NULL, {"pis", 3}, &empty_grammar},{NULL, {"ieeeBinary32", 12}, &ops_grammar_4_24},{NULL, {"ieeeBinary64", 12}, &ops_grammar_4_25},{NULL, {"valueMaxLength", 14}, &ops_grammar_4_26},{NULL, {"valuePartitionCapacity", 22}, &ops_grammar_4_27},{NULL, {"comments", 8}, &empty_grammar},{NULL, {"datatypeRepresentationMap", 25}, &ops_grammar_4_29},{NULL, {"dtd", 3}, &empty_grammar},{NULL, {"uncommon", 8}, &ops_grammar_4_31},{NULL, {"preserve", 8}, &ops_grammar_4_32},{NULL, {"blockSize", 9}, &ops_grammar_4_33},{NULL, {"lexicalValues", 13}, &empty_grammar},{NULL, {"lesscommon", 10}, &ops_grammar_4_35},{NULL, {"compression", 11}, &empty_grammar},{NULL, {"fragment", 8}, &empty_grammar},{NULL, {"schemaId", 8}, &ops_grammar_4_38}};
LocalNamesTable ops_lTable_4 = { ops_LNrows_4, 39, 39, {NULL, 0}};

struct URIRow ops_uriRows[5] = {{&ops_pTable_0, &ops_lTable_0, {"", 0}},{&ops_pTable_1, &ops_lTable_1, {"http://www.w3.org/XML/1998/namespace", 36}},{&ops_pTable_2, &ops_lTable_2, {"http://www.w3.org/2001/XMLSchema-instance", 41}},{NULL, &ops_lTable_3, {"http://www.w3.org/2001/XMLSchema", 32}},{NULL, &ops_lTable_4, {"http://www.w3.org/2009/exi", 26}}};
URITable ops_uriTbl = {ops_uriRows, 5, 5, {NULL, 0}};

QNameID ops_qnames[1] = {{4, 4}};

const EXIPSchema ops_schema = {&ops_uriTbl, ops_qnames, 1, {NULL, NULL}};
