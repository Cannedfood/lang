#include "lang_allocator.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

// TODO: implement something better (an arena allocator for example)

#include "lang_buffer.h"

lang_allocator* lang_new_allocator() {
	lang_buffer* allocations = malloc(sizeof(lang_buffer));
	*allocations = lang_buffer_new();
	return (lang_allocator*)allocations;
}
void lang_free_allocator(lang_allocator* alloc) {
	lang_buffer* allocations = (lang_buffer*) alloc;

	int    n       = allocations->length / sizeof(void*);
	void** pallocs = (void**)allocations->data;
	for(int i = 0; i < n; i++) {
		free(pallocs[i]);
	}
	lang_buffer_destroy(allocations);
	free(allocations);
}

void* lang_alloc(lang_allocator* alloc, int size) {
	lang_buffer* allocations = (lang_buffer*) alloc;
	void* data = malloc(size);
	lang_buffer_pushp(allocations, data);
	return data;
}
void lang_free(lang_allocator* alloc, void* ptr) {
	if(!ptr) return;

	lang_buffer* allocations = (lang_buffer*) alloc;

	int    n       = allocations->length / sizeof(void*);
	void** pallocs = (void**)allocations->data;
	for(int i = 0; i < n; i++) {
		if(pallocs[i] == ptr) {
			pallocs[i] = NULL;
			free(ptr);
			return;
		}
	}
	assert(!"Pointer not found. Double free or invalid pointer?");
}

static void* _lang_alloc_callback(void* user, int size) { return lang_alloc((lang_allocator*) user, size); }
static void  _lang_free_callback (void* user, void* pointer) { return lang_free((lang_allocator*) user, pointer); }

lang_alloc_callbacks lang_alloc_callbacks_for(lang_allocator* alloc) {
	lang_alloc_callbacks result;
	result.userdata = alloc;
	result.alloc = _lang_alloc_callback;
	result.free  = _lang_free_callback;
	return result;
}
