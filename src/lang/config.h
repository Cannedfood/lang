#ifdef __cplusplus
	#define LANG_API extern "C"
	#define LANG_API_DECL extern "C"
	#define LANG_DEFAULT(VALUE) = VALUE
#else
	#define LANG_API
	#define LANG_API_DECL extern
	#define LANG_DEFAULT(VALUE)
#endif

#define LANG_PARSER_API         LANG_API
#define LANG_TOKENIZER_API      LANG_API
#define LANG_TOKENIZER_API_DECL LANG_API_DECL
#define LANG_BYTECODE_API       LANG_API
#define LANG_UTIL_BUFFER_API    LANG_API
#define LANG_VM_API             LANG_API
#define LANG_AST_API            LANG_API
#define LANG_AST_API_DECL       LANG_API_DECL
