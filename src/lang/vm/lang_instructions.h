#pragma once

enum lang_instruction {
	#define LANG_INSTRUCTION(name, push, pop, extra_argtype, extra_argtype_size) instr_##name
	#include "lang_instructions.txt"
	#undef LANG_INSTRUCTION

	lang_num_instructions
};

struct lang_instruction_info {
	const char* name;
	int push, pop;
	const char* extra_argtype_name;
	int extra_argtype_size;
};
typedef struct lang_instruction_info lang_instruction_info;

extern lang_instruction_info lang_instruction_infos[];
