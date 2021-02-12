#include "catch.hpp"


#include <string_view>
#include <lang/compiler/lang_tokens.h>

TEST_CASE("tokenizer", "[compiler][parser][tokens]") {
	lang_tokenizer tokenizer;

	SECTION("line comments") {
		lang_tokenizer_init(&tokenizer,
			"// This is a comment\n"
			"# As is this\n"
		);

		tokenizer.next();
		CHECK(tokenizer.token.type == lang_token_line_comment);
		CHECK(tokenizer.token.string_view() == "// This is a comment");
		tokenizer.next();
		CHECK(tokenizer.token.type == lang_token_line_comment);
		CHECK(tokenizer.token.string_view() == "# As is this");
		tokenizer.next();
		CHECK(tokenizer.token.type == lang_token_end_of_file);
	}

	SECTION("block comments") {
		lang_tokenizer_init(&tokenizer,
			"/* Block comment 1 */\n"
			"/* Block \n"
			" comment 2 */\n"
			"/* Block comment 3 \n"
			"*/"
		);

		tokenizer.next();
		CHECK(tokenizer.token.type == lang_token_block_comment);
		CHECK(tokenizer.token.string_view() == "/* Block comment 1 */");
		CHECK(tokenizer.token.line == 1);
		CHECK(tokenizer.token.character == 0);
		CHECK(tokenizer.token.character == 0);
		tokenizer.next();
		CHECK(tokenizer.token.type == lang_token_block_comment);
		CHECK(tokenizer.token.string_view() == "/* Block \n comment 2 */");
		CHECK(tokenizer.token.line == 2);
		CHECK(tokenizer.token.character == 0);
		tokenizer.next();
		CHECK(tokenizer.token.type == lang_token_block_comment);
		CHECK(tokenizer.token.string_view() == "/* Block comment 3 \n*/");
		CHECK(tokenizer.token.line == 4);
		CHECK(tokenizer.token.character == 0);
		tokenizer.next();
		CHECK(tokenizer.token.type == lang_token_end_of_file);
		CHECK(tokenizer.token.character == 2);
	}

	SECTION("string literals") {
		lang_tokenizer_init(&tokenizer,
			R"("Hello" "\"")"
		);

		tokenizer.next();
		CHECK(tokenizer.token.type == lang_token_string_literal);
		CHECK(tokenizer.token.string_view() == R"("Hello")");
		tokenizer.next();
		CHECK(tokenizer.token.type == lang_token_string_literal);
		CHECK(tokenizer.token.string_view() == R"("\"")");
		tokenizer.next();
		CHECK(tokenizer.token.type == lang_token_end_of_file);

		// TODO: test multiline strings whenever they are added
	}
}

