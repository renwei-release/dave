/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __NLP_PARAM_H__
#define __NLP_PARAM_H__

/*
 * http://ictclas.nlpir.org/nlpir/html/readme.htm
 * https://ltp.readthedocs.io/zh_CN/latest/appendix.html#id3
 * https://blog.csdn.net/suibianshen2012/article/details/53487157
 */

typedef enum {
	NLPPartOfSpeech_none,

	NLPPartOfSpeech_adjective,
	NLPPartOfSpeech_other_noun_modifier,
	NLPPartOfSpeech_conjunction,
	NLPPartOfSpeech_adverb,
	NLPPartOfSpeech_exclamation,
	NLPPartOfSpeech_morpheme,
	NLPPartOfSpeech_prefix,
	NLPPartOfSpeech_idiom,
	NLPPartOfSpeech_abbreviation,
	NLPPartOfSpeech_suffix,
	NLPPartOfSpeech_number,
	NLPPartOfSpeech_general_noun,
	NLPPartOfSpeech_direction_noun,
	NLPPartOfSpeech_person_name,
	NLPPartOfSpeech_organization_name,
	NLPPartOfSpeech_location_noun,
	NLPPartOfSpeech_geographical_name,
	NLPPartOfSpeech_temporal_noun,
	NLPPartOfSpeech_other_proper_noun,
	NLPPartOfSpeech_onomatopoeia,
	NLPPartOfSpeech_preposition,
	NLPPartOfSpeech_quantity,
	NLPPartOfSpeech_pronoun,
	NLPPartOfSpeech_auxiliary,
	NLPPartOfSpeech_verb,
	NLPPartOfSpeech_punctuation,
	NLPPartOfSpeech_foreign_words,
	NLPPartOfSpeech_non_lexeme,
	NLPPartOfSpeech_descriptive_words,
	NLPPartOfSpeech_Customary_word,			// https://blog.csdn.net/suibianshen2012/article/details/53487157
	NLPPartOfSpeech_time,
	NLPPartOfSpeech_verb_noun,
	NLPPartOfSpeech_Modal,
	NLPPartOfSpeech_verb_shi,				// https://www.cnblogs.com/flippedkiki/p/7123549.html
	NLPPartOfSpeech_verb_you,
	NLPPartOfSpeech_auxiliary_de,
	NLPPartOfSpeech_person_unkonwn,

	NLPPartOfSpeech_total,

	NLPPartOfSpeech_MAX = 0xffffffffffffffff
} NLPPartOfSpeech;

typedef enum {
	NLPIRLabel_none = 0,

	NLPIRLabel_ROOT,

	NLPIRLabel_Greetings,
	NLPIRLabel_Purposeless_communication,

	NLPIRLabel_DataService,
	NLPIRLabel_DataService_buy,
	NLPIRLabel_DataService_unsubscribe,
	NLPIRLabel_DataService_advisory,
	NLPIRLabel_DataService_complaint,
	NLPIRLabel_DataService_inquire,
	NLPIRLabel_DataService_inquire_traffic,
	NLPIRLabel_DataService_inquire_traffic_limit,
	NLPIRLabel_DataService_inquire_traffic_expired,
	NLPIRLabel_DataService_inquire_sharing_hot_spots,

	NLPIRLabel_VoiceService,
	NLPIRLabel_VoiceService_buy,
	NLPIRLabel_VoiceService_unsubscribe,
	NLPIRLabel_VoiceService_advisory,
	NLPIRLabel_VoiceService_complaint,

	NLPIRLabel_Process_date,
	NLPIRLabel_Process_location,
	NLPIRLabel_Process_person,
	NLPIRLabel_Process_business,

	NLPIRLabel_UnrecognizedIntent,

	NLPIRLabel_MAX,

	NLPIRLabel_max = 0xffffffffffffffff
} NLPIRLabel;

typedef enum {
	NLPNEREntity_none,
	NLPNEREntity_location,
	NLPNEREntity_time,
	NLPNEREntity_company,
	NLPNEREntity_person,
	NLPNEREntity_business_word,
	NLPNEREntity_Net,
	NLPNEREntity_Day,
	NLPNEREntity_Destination,
	NLPNEREntity_max
} NLPNEREntity;

typedef enum {
	NLPDependencySyntax_none,
	NLPDependencySyntax_SBV,
	NLPDependencySyntax_VOB,
	NLPDependencySyntax_IOB,
	NLPDependencySyntax_FOB,
	NLPDependencySyntax_DBL,
	NLPDependencySyntax_ATT,
	NLPDependencySyntax_ADV,
	NLPDependencySyntax_CMP,
	NLPDependencySyntax_COO,
	NLPDependencySyntax_POB,
	NLPDependencySyntax_LAD,
	NLPDependencySyntax_RAD,
	NLPDependencySyntax_IS,
	NLPDependencySyntax_HED,
	NLPDependencySyntax_WP,
	NLPDependencySyntax_max
} NLPDependencySyntax;

typedef enum {
	NLPSemanticRole_none,
	NLPSemanticRole_ADV,
	NLPSemanticRole_BNE,
	NLPSemanticRole_CND,
	NLPSemanticRole_DIR,
	NLPSemanticRole_DGR,
	NLPSemanticRole_EXT,
	NLPSemanticRole_FRQ,
	NLPSemanticRole_LOC,
	NLPSemanticRole_MNR,
	NLPSemanticRole_PRP,
	NLPSemanticRole_TMP,
	NLPSemanticRole_TPC,
	NLPSemanticRole_CRD,
	NLPSemanticRole_PRD,
	NLPSemanticRole_PSR,
	NLPSemanticRole_PSE,
	NLPSemanticRole_max
} NLPSemanticRole;

typedef struct {
	NLPNEREntity entity_label;
	ub ner_len;
	s8 *ner;

	void *next;
} NLPNER;

typedef struct {
	ub seg_index;
	NLPDependencySyntax syntax;
} NLPDS;

typedef struct {
	MCardIdentityType id;
	MCardLocation location;
	MCardTime time;
	MCardContentType type;
	ub content_length;
	s8 *pContent;
} NLPPrimitive;

typedef struct {
	ub seg_number;
	ub seg_total_len;
	ub seg_len;
	s8 *pSegmentor;
	NLPPartOfSpeech pos;
	NLPDS ds;

	void *next;
} NLPSegmentor;

typedef struct {
	NLPIRLabel label;
	float score;

	ub bert_id;
	s8 bert_label[DAVE_LABEL_STR_MAX];
	float bert_score;
	ub qa_id;
	s8 qa_label[DAVE_LABEL_STR_MAX];
	float qa_score;	
} NLPIntentionRecongnition;

typedef struct {
	NLPPrimitive primitive;
	NLPIntentionRecongnition IR;
	NLPSegmentor *pSegmentor;
	NLPNER *pNer;
	MBUF *generate_answer;
	MBUF *engine_answer;
} NLPTalk;

typedef struct {
	MCardContentType type;
	MBUF *answer;
} AIAnswer;

typedef struct {
	NLPTalk talk;

	AIAnswer answer;

	void *up;
	void *next;
} NLPTalks;

#endif

