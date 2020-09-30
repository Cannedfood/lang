#include "lang_bytecode.h"

#include "../runtime/lang_instructions.h"

#include "./lang_parse_util.h"

#include <stdio.h> // sscanf
#include <string.h> // strlen

int lang_bytecode_parse_line(const char* line, lang_buffer* into) {
	if(lang_starts_with("; ", line)) return 1;

	double f;
	int i;

	if(sscanf(line, "push %lf",  &f) == 1) { lang_buffer_pushc(into, instr_push_num); lang_buffer_pushf64(into, f); return 1; }
	if(sscanf(line, "call %i",   &i) == 1) { lang_buffer_pushc(into, instr_call);     lang_buffer_pushc  (into, i); return 1; }
	if(sscanf(line, "return %i", &i) == 1) { lang_buffer_pushc(into, instr_return);   lang_buffer_pushc  (into, i); return 1; }
	if(sscanf(line, "jump %i",   &i) == 1) { lang_buffer_pushc(into, instr_jump);     lang_buffer_pushi32(into, i); return 1; }
	if(sscanf(line, "jumpc %i",  &i) == 1) { lang_buffer_pushc(into, instr_jumpc);    lang_buffer_pushi32(into, i); return 1; }

	for(int i = 0; i < lang_instruction_infos_count; i++) {
		if(lang_starts_with(lang_instruction_infos[i].name, line)) {
			lang_buffer_pushc(into, i);
			return 1;
		}
	}

	return 0;
}

static int _lang_instruction_size(const char instruction) {
	return 1 + lang_instruction_infos[instruction].extra_argtype_size;
}

static int _lang_instructions_size(const char* instructions, int n) {
	int position = 0;
	for (int i = 0; i < n; i++) {
		position += _lang_instruction_size(instructions[position]);
	}
	return position;
}

int lang_bytecode_convert_jumps_to_bytes(const char* bytecode, int length) {
	// Fix jump locations
	// puts("Relink");
	int instruction = 0;
	for (
		int position = 0;
		position < length;
		position += _lang_instruction_size(bytecode[position]))
	{
		// printf(" %i %i %s\n", instruction, position, lang_instruction_infos[into->data[position]].name);
		if(
			bytecode[position] == instr_jump ||
			bytecode[position] == instr_jumpc)
		{
			int* jumpAmount = (int*)(bytecode + position + 1);
			if(*jumpAmount < 0)
				*jumpAmount = _lang_instructions_size(bytecode, instruction + *jumpAmount) - position;
			else
				*jumpAmount = _lang_instructions_size(bytecode + position, *jumpAmount);

			// printf("   to %i\n", position + *jumpAmount);
		}
		instruction++;
	}

	return 1;
}

static void _lang_bytecode_default_errfn(void* userptr, const char* file, int lineNumber, const char* line, const char* message) {
	printf("%s:%i: \"%s\" | %s\n", file, lineNumber, line, message);
}

int lang_bytecode_parse_file(const char* path, lang_buffer* into, lang_bytecode_errfn errorfn_or_null, void* errfn_userptr) {
	if(!errorfn_or_null) errorfn_or_null = _lang_bytecode_default_errfn;

	int start = into->length;

	int hasError = 0;

	FILE* file = fopen(path, "r");
	if(!file) return 0;

	int lineCounter = 0;

	char line[256];
	while(fgets(line, sizeof(line), file)) {
		lineCounter++;

		if(line[0] == '\0' || line[0] == '\n') continue;
		// printf("%zu\n", into->length);

		if(!lang_bytecode_parse_line(line, into)) {
			hasError = 1;
			line[strlen(line) - 1] = '\0'; // Remove '\n'
			errorfn_or_null(errfn_userptr, path, lineCounter, line, "Unknown instruction name");
		}
	}

	fclose(file);

	lang_bytecode_convert_jumps_to_bytes(into->data + start, into->length);

	return !hasError;
}
