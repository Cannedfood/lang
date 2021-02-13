#pragma once

#include <stdint.h>

// Server
typedef struct lsp_server lsp_server;

lsp_server* lsp_server_open(uint16_t port);
void        lsp_server_close(lsp_server* server);

// Handle requests
typedef struct lsp_connection lsp_connection;
typedef struct lsp_handlers {
	void* userpointer;
} lsp_handlers;

int lsp_server_handle_requests(lsp_server* server, int block);



// Protocol data
typedef const char* lsp_uri;

struct lsp_position {
	int line;
	int character;
};

struct lsp_range {
	struct lsp_position start, end;
};

struct lsp_location {
	lsp_uri uri;
	struct lsp_range range;
};
