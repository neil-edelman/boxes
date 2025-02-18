#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static void private_graph_sanitize(FILE *const fp, const char *str) {
	const char *lazy = str, *escape;
	size_t escape_size;
	assert(str && fp);
	int keep_going = 1;
	do {
		unsigned char ch = (unsigned char)*str;
		if(ch == '\0') { escape = "", escape_size = 0, keep_going = 0; goto force; }
		switch(ch) {
		case '&': escape = "&amp;", escape_size = 5; goto force;
		case '<': escape = "&lt;", escape_size = 4; goto force;
		case '>': escape = "&gt;", escape_size = 4; goto force;
		case '\"': escape = "&quot;", escape_size = 6; goto force;
		case '\'': escape = "&#39;", escape_size = 5; goto force;
		default: continue;
		}
force:
		fwrite(lazy, 1, (size_t)(str - lazy), fp);
		fwrite(escape, 1, escape_size, fp);
		lazy = str + 1;
	} while(str++, keep_going);
}

int main(void) {
	private_graph_sanitize(stdout, "");
	private_graph_sanitize(stdout, "a");
	private_graph_sanitize(stdout, "&");
	private_graph_sanitize(stdout, "<");
	private_graph_sanitize(stdout, ">");
	private_graph_sanitize(stdout, "\"");
	private_graph_sanitize(stdout, "\'");
	private_graph_sanitize(stdout, "\n");
	private_graph_sanitize(stdout, "ipsum & lorem\n");
	return EXIT_SUCCESS;
}
