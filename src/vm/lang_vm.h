#pragma once

#include <stdint.h>

#include "lang_instructions.h"
#include "lang_heap.h"

typedef struct lang_vm lang_vm;
typedef struct lang_options lang_options;

typedef enum lang_instruction lang_instruction;

struct lang_options {
	void* userdata;
	void(*onError)(void* userdata, lang_vm* state, const char* message);
	void(*onPrint)(void* userdata, lang_vm* state, int level, const char* message);
};

struct lang_vm {
	uint64_t stack[512];
	int      top;

	lang_heap heap;

	lang_options options;
};

void   lang_state_pop(lang_vm* state);
double lang_state_popnum(lang_vm* state);
void   lang_state_pushnum(lang_vm* state, double num);
void   lang_state_prepare_call(lang_vm* state, int kargs);
void   lang_state_prepare_return(lang_vm* state, int kresults);

void lang_state_interpret(lang_vm* state, char const* instructions, int num_instructions);

lang_vm* lang_newstate(lang_options const* options);
void lang_freestate(lang_vm* state);
