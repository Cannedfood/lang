#pragma once

#include <stdint.h>

struct lang_heap {
	
};
typedef struct lang_heap lang_heap;

enum lang_heap_object_type {
	HeapClass,
	HeapUserdata,
	HeapFunction,
	HeapMap,
	HeapArray
};
typedef enum lang_heap_object_type lang_heap_object_type;

struct lang_heap_object {
	lang_heap_object_type type;
	int marked;
};
