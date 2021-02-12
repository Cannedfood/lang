#include "lang_instructions.h"

#include <stdint.h>

lang_instruction_info lang_instruction_infos[] = {
	#define LANG_INSTRUCTION(name, push, pop, extra_argtype, extra_argtype_size) { #name, push, pop, #extra_argtype, extra_argtype_size }
	#include "lang_instructions.txt"
	#undef LANG_INSTRUCTION
};
