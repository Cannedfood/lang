#pragma once

#include <stdint.h>

enum lang_heap_object_type {
	HeapClass,
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

typedef struct lang_heap lang_heap;
struct lang_heap {
	lang_heap_object** objects;
	int                numObjects;
};

