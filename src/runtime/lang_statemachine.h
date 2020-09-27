#pragma once

#include <stdint.h>

#include "lang_instructions.h"
#include "lang_heap.h"

typedef struct lang_statemachine lang_statemachine;
typedef struct lang_options lang_options;

typedef enum lang_instruction lang_instruction;

struct lang_options {
	void* userdata;
	void(*onError)(void* userdata, lang_statemachine* state, const char* message);
	void(*onPrint)(void* userdata, lang_statemachine* state, int level, const char* message);
};

struct lang_statemachine {
	uint64_t stack[512];
	int      top;

	lang_heap heap;

	lang_options options;
};

void   lang_state_pop(lang_statemachine* state);
double lang_state_popnum(lang_statemachine* state);
void   lang_state_pushnum(lang_statemachine* state, double num);
void   lang_state_prepare_call(lang_statemachine* state, int kargs);
void   lang_state_prepare_return(lang_statemachine* state, int kresults);

void lang_state_interpret(lang_statemachine* state, char const* instructions, int num_instructions);

lang_statemachine* lang_newstate(lang_options const* options);
void lang_freestate(lang_statemachine* state);
