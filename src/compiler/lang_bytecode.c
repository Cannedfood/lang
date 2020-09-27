#include "lang_bytecode.h"

#include "../runtime/lang_instructions.h"

#include <stdio.h> // sscanf
#include <string.h> // strlen

int starts_with(const char* with, const char* text) {
	for (int i = 0; with[i] != '\0'; i++) {
		if(with[i] != text[i]) return 0;
	}
	return 1;
}

int lang_bytecode_parse_line(const char* line, byte_buffer* into) {
	if(starts_with("; ", line)) return 1;

	double f;
	int i;

	if(sscanf(line, "push %lf",  &f) == 1) { bb_pushc(into, instr_push_num); bb_pushf64(into, f); return 1; }
	if(sscanf(line, "call %i",   &i) == 1) { bb_pushc(into, instr_call);     bb_pushc  (into, i); return 1; }
	if(sscanf(line, "return %i", &i) == 1) { bb_pushc(into, instr_return);   bb_pushc  (into, i); return 1; }
	if(sscanf(line, "jump %i",   &i) == 1) { bb_pushc(into, instr_jump);     bb_pushi32(into, i); return 1; }
	if(sscanf(line, "jumpc %i",  &i) == 1) { bb_pushc(into, instr_jumpc);    bb_pushi32(into, i); return 1; }

	for(int i = 0; i < lang_instruction_infos_count; i++) {
		if(starts_with(lang_instruction_infos[i].name, line)) {
			bb_pushc(into, i);
			return 1;
		}
	}

	return 0;
}

static void _lang_bytecode_default_errfn(const char* file, int lineNumber, const char* line, const char* message) {
	printf("%s:%i: \"%s\" | %s\n", file, lineNumber, line, message);
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

int lang_bytecode_parse_file(const char* path, byte_buffer* into, lang_bytecode_errfn errorfn_or_null) {
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
			errorfn_or_null(path, lineCounter, line, "Unknown instruction name");
		}
	}

	fclose(file);

	// Fix jump locations
	// puts("Relink");
	int instruction = 0;
	for (
		int position = start;
		position < into->length;
		position += _lang_instruction_size(into->data[position]))
	{
		// printf(" %i %i %s\n", instruction, position, lang_instruction_infos[into->data[position]].name);
		if(
			into->data[position] == instr_jump ||
			into->data[position] == instr_jumpc)
		{
			int* jumpAmount = (int*)(into->data + position + 1);
			if(*jumpAmount < 0)
				*jumpAmount = _lang_instructions_size(into->data + start, instruction + *jumpAmount) - position;
			else
				*jumpAmount = _lang_instructions_size(into->data + position, *jumpAmount);

			// printf("   to %i\n", position + *jumpAmount);
		}
		instruction++;
	}

	return !hasError;
}
