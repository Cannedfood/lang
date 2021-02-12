#pragma once

#include "../config.h"

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

LANG_VM_API void   lang_state_pop(lang_vm* state);
LANG_VM_API double lang_state_popnum(lang_vm* state);
LANG_VM_API void   lang_state_pushnum(lang_vm* state, double num);
LANG_VM_API void   lang_state_prepare_call(lang_vm* state, int kargs);
LANG_VM_API void   lang_state_prepare_return(lang_vm* state, int kresults);

LANG_VM_API void lang_state_interpret(lang_vm* state, char const* instructions, int num_instructions);

LANG_VM_API lang_vm* lang_newstate(lang_options const* options);
LANG_VM_API void     lang_freestate(lang_vm* state);
