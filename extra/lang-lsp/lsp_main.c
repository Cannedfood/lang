#include "lsp.h"

#include <string.h>

int main(int argc, char const** argv) {
	lsp_server* server = lsp_server_open(1234);

	while (1) {
		lsp_server_handle_requests(server, 1);
	}

	lsp_server_close(server);
}
