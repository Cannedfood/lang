#pragma once

// Allocator
typedef struct lang_allocator lang_allocator;

lang_allocator* lang_new_allocator();
void            lang_free_allocator(lang_allocator*);

void* lang_alloc(lang_allocator* alloc, int size);
void  lang_free (lang_allocator* alloc, void* data);

// Allocation callbacks
typedef struct lang_alloc_callbacks lang_alloc_callbacks;
struct lang_alloc_callbacks {
	void* userdata;
	void* (*alloc)(void* userdata, int size);
	void (*free)(void* userdata, void* pointer);
};

lang_alloc_callbacks lang_alloc_callbacks_for(lang_allocator* alloc);
