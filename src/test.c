#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "./util/lang_buffer.h"
#include "./runtime/lang_statemachine.h"
#include "./compiler/lang_bytecode.h"

int main(int argc, const char** argv) {
	lang_statemachine* state = lang_newstate((lang_options*)NULL);

		lang_buffer bytecode = lang_buffer_new();

		if(!lang_bytecode_parse_file("./test.bytecode", &bytecode, 0))
			return -1;
		lang_state_interpret(state, bytecode.data, bytecode.length);

		lang_buffer_free(&bytecode);

	lang_freestate(state);
}
