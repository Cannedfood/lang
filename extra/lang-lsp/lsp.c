#include "lsp.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/ip.h>

struct lsp_connection {

};

void write_ip(struct sockaddr_in const* addr, char* out) {
	uint32_t ip = ntohl(addr->sin_addr.s_addr);
	uint16_t port = ntohs(addr->sin_port);
	sprintf(out,
		"%X.%X.%X.%X:%u",
		((ip >>  0) & 0xFF),
		((ip >>  8) & 0xFF),
		((ip >> 16) & 0xFF),
		((ip >> 24) & 0xFF),
		port
	);
}

void httpStartResponse(int connection, const char* code) {
	dprintf(connection, "HTTP/1.1 %s\r\n", code);
}

void httpHeader(int connection, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vdprintf(connection, fmt, args);
	write(connection, "\r\n", 2);
}

void httpEndResponse(int connection, const char* content) {
	int length = content == NULL? 0 : strlen(content);
	if(length == 0) {
		write(connection, "\r\n", 2);
	}
	else {
		httpHeader(connection, "Content-Length: %u", length);
		write(connection, "\r\n", 2);
		write(connection, content, length);
	}
}

struct lsp_server {
	int socket;
	struct sockaddr_in address;
};

lsp_server* lsp_server_open(uint16_t port) {
	lsp_server* server = malloc(sizeof(lsp_server));

	server->socket = socket(AF_INET, SOCK_STREAM, 0);

	server->address.sin_family = AF_INET;
	server->address.sin_addr.s_addr = htonl(INADDR_ANY);
	server->address.sin_port = htons(port? port:1234);

	if(bind(server->socket, (struct sockaddr*) &server->address, sizeof(server->address)) != 0) {
		char ip[64];
		write_ip(&server->address, ip);
		fprintf(stderr, "Failed bind(%s): %s\n", ip, strerror(errno));
		exit(-1);
	}

	return server;
}
void lsp_server_close(lsp_server* server) {
	close(server->socket);
	free(server);
}
int lsp_server_handle_requests(lsp_server* server, int block) {
	if(listen(server->socket, 10) < 0) {
		char ip[64];
		write_ip(&server->address, ip);
		fprintf(stderr, "Failed bind(%s): %s\n", ip, strerror(errno));
		return -1;
	}

	int client = accept(server->socket, NULL, NULL);
	httpStartResponse(client, "200 OK");
	httpEndResponse(client, "Hello world");
	close(client);

	return 0;
}


