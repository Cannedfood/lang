#pragma once

#include <stdint.h>

enum lang_heap_object_type {
	HeapClass,
	HeapInstance,
	HeapUserdata,
	HeapFunction,
	HeapMap,
	HeapArray
};
typedef enum lang_heap_object_type lang_heap_object_type;

typedef struct lang_heap_object lang_heap_object;
struct lang_heap_object {
	lang_heap_object_type type;
	int marked;
};

typedef struct lang_heap_class lang_heap_class;
struct lang_heap_class {
	lang_heap_object_type type;
	int marked;

	struct {
		const char* name;
	} *memberVariables;
	int numMembers;

	struct {
		const char* name;
		uint64_t func;
	} *memberFuncs;
	int numMemberFuncs;
};

typedef struct lang_heap_instance lang_heap_instance;
struct lang_heap_instance {
	lang_heap_object_type type;
	int marked;


};

typedef struct lang_heap lang_heap;
struct lang_heap {
	lang_heap_object** objects;
	int                numObjects;
};

uint64_t lang_heap_alloc();
void     lang_heap_free(uint64_t);
