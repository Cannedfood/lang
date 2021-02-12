#include "lang_vm.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define LANG_BITCAST(to_type, value) (*(to_type*)&value)

LANG_VM_API inline
void lang_state_pop(lang_vm* state) {
	if(state->top < 1) {
		state->options.onError(state->options.userdata, state, "Stack Underflow");
		return;
	}

	state->top--;
}
LANG_VM_API inline
double lang_state_popnum(lang_vm* state) {
	lang_state_pop(state);
	return LANG_BITCAST(double, state->stack[state->top + 1]);
}
LANG_VM_API
void lang_state_pushnum(lang_vm* state, double num) {
	if(state->top > (sizeof(state->stack)/sizeof(state->stack[0])) - 1) {
		state->options.onError(state->options.userdata, state, "Stack Overflow");
		return;
	}

	state->top++;
	state->stack[state->top] = LANG_BITCAST(uint64_t, num);
}
LANG_VM_API
void lang_state_prepare_call(lang_vm* state, int kargs) {
	// TODO
	exit(-2);
}
LANG_VM_API
void lang_state_prepare_return(lang_vm* state, int kresults) {
	// TODO
	exit(-2);
}

LANG_VM_API
void lang_state_interpret(lang_vm* state, char const* instructions, int num_instructions) {
	char const* end = instructions + num_instructions;

	#define instr(name) case instr_##name:
	// #define instr(name) case instr_##name: puts("instr_"#name);

	while(instructions < end) {
		switch(instructions[0]) {
			instr(push_num) {
				lang_state_pushnum(state, *(double const*)(instructions + 1));
				instructions += 1 + sizeof(double);
			} break;
			instr(popn) { lang_state_pop(state); ++instructions; } break;
			instr(print_debug) {
				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				sprintf(buffer, "instr_print_debug: %f", lang_state_popnum(state));
				state->options.onPrint(state->options.userdata, state, 3, buffer);
				instructions++;
			} break;
			instr(add)  { double a = lang_state_popnum(state), b = lang_state_popnum(state); lang_state_pushnum(state, b + a); instructions++; } break;
			instr(sub)  { double a = lang_state_popnum(state), b = lang_state_popnum(state); lang_state_pushnum(state, b - a); instructions++; } break;
			instr(mul)  { double a = lang_state_popnum(state), b = lang_state_popnum(state); lang_state_pushnum(state, b * a); instructions++; } break;
			instr(div)  { double a = lang_state_popnum(state), b = lang_state_popnum(state); lang_state_pushnum(state, b / a); instructions++; } break;
			instr(jump) {
				int n = (*(int*) (instructions + 1));
				// printf("\t%i\n", n);
				instructions += n;
			} break;
			instr(call) {
				int nArgs = instructions[1];

			} break;
			instr(return) {
				int nResults = instructions[1];
			} break;
			instr(halt) return;
			default: printf("Unknown instruction %i\n", instructions[0]); return;
		}
	}
}


static void _lang_default_onError(void* userdata, lang_vm* state, const char* message) {
	// TODO: stack trace
	state->options.onPrint(state->options.userdata, state, 0, message);
	exit(-1);
}

static void _lang_default_onPrint(void* userdata, lang_vm* state, int level, const char* message) {
	printf("[lang](%i) %s\n", level, message);
}

LANG_VM_API
lang_vm* lang_newstate(lang_options const* options) {
	lang_vm* state = calloc(1, sizeof(lang_vm));

	if(options) state->options = *options;

	if(!state->options.onError) state->options.onError = _lang_default_onError;
	if(!state->options.onPrint) state->options.onPrint = _lang_default_onPrint;

	return state;
}

LANG_VM_API
void lang_freestate(lang_vm* state) {
	free(state);
}
