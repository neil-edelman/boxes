/** Copyright 2015 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 This is a test of Text; call it with a .c file.
 
 @fixme		Needs to split '@', current sol'n doesn't cut it; new sol'n needed.
			How about "TestSplit(char split, ...)", or just use strchr and have
			new child exposed, NewChildFromBuffer(key, value).

 @author	Neil
 @version	3.0; 2016-08
 @since		3.0; 2016-08 */

#include <stdlib.h>	/* rand, EXIT_* */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp strdup */
#include <ctype.h>	/* isspace */
#include "../src/Text.h"

/* static const char *const things_in_header[] = {
	"author", "version", "since", "fixme"
};	static const char *const things_in_fn[] = {
	"return", "throws", "implements", "fixme", "author"
}; */

/** Write a bunch of XML CDATA. */
static void cdata(char *const str) {
	char *a = str, *b;
	printf("<![CDATA[");
	while((b = strstr(a, "]]>"))) {
		printf("%.*s]]]]><![CDATA[>", (int)(b - a), a);
		a += 3;
	}
	printf("%s]]>", a);
}

static void print_text(struct Text *const);

/** XML is wierd. */
static void xml_recursive(struct Text *const this, const int is_top) {
	if(!is_top) printf("<key><![CDATA[%s]]></key>\n", TextGetKey(this));
	printf("<dict>\n");
	/* fixme: %s has "]]>" it will fail, go through strstr and  */
	printf("<key>");
	cdata(TextGetKey(this));
	printf("</key>\n<string>");
	cdata(TextGetValue(this));
	printf("</string>\n");
	if(!is_top) {
		printf("<key>begin</key><integer>%lu</integer>\n"
			"<key>end</key><integer>%lu</integer>\n",
			TextGetParentStart(this), TextGetParentEnd(this));
	}
	TextForEachPassed(this, 0, &print_text);
	/*for(i = 0; i < this->downs_size; i++) {
		down = this->downs[i];
		xml(down, fp, 0);
	}*/
	printf("</dict>\n");
}

/** @implements	TextAction */
static void print_text(struct Text *const this) { xml_recursive(this, 0); }

static void xml(struct Text *const this) {
	/*size_t i;
	 char *cursor;
	 if(!this) return;
	 cursor = this->buffer;
	 for(i = 0; i < this->downs_size; i++) {
	 down = this->downs[i];
	 fprintf(fp, "[%.*s]\\", (int)(this->buffer+down->up_begin-cursor), cursor);
	 TextXML(down, fp);
	 fprintf(fp, "/");
	 cursor = this->buffer + down->up_end;
	 }
	 fprintf(fp, "[%s].", cursor);*/
	printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	printf("<!DOCTYPE plist>\n");
	printf("<plist version=\"1.0\">\n");
	xml_recursive(this, -1);
	printf("</plist>\n");
}

/***********************
 * General text-things */

/** Moves a copy of white-space trimmed at the beginning and end to {str}. */
static void trim(char *const str) {
	char *e = str + strlen(str) - 1, *s = str;
	while(e > str && isspace(*e)) e--;
	e++, *e = '\0';
	while(isspace(*s)) s++;
	if(s - str) memmove(str, s, (size_t)(e - s + 1));
}

/** @return		Is the character pointed to by {s} in the string {str} the
				first on the line? */
static int is_first_on_line(const char *const str, const char *s) {
	if(str >= s) return -1;
	s--;
	while(s >= str) {
		if(*s == '\0' || *s == '\n') return -1;
		if(!isspace(*s)) return 0;
		s--;
	}
	return -1;
}

/** @return		The number of characters in a word starting at {str}. */
static size_t word_length(char *str) {
	char *s = str;
	while(isalnum(*s)) s++;
	return s - str;
}

/**********************************
 * These go in TextPattern array. */

/** Must be in rfc3986 format; \url{https://www.ietf.org/rfc/rfc3986.txt }.
 @implements	TextAction */
static void url(struct Text *const this) {
	trim(TextGetValue(this));
	TextAdd(this, "<a href = \"%s\">%s</a>");
}
/** Must be in query format; \url{ https://www.ietf.org/rfc/rfc3986.txt }.
 @implements	TextAction */
static void cite(struct Text *const this) {
	trim(TextGetValue(this));
	TextAdd(this, "<a href = \"https://scholar.google.ca/scholar?q=%s\">%s</a>");
}
/** @implements	TextAction */
static void em(struct Text *const this) { TextAdd(this, "<em>%s</em>"); }
/** @implements	TextAction */
static void amp(struct Text *const this) { TextAdd(this, "&amp;"); }
/** @implements	TextAction */
static void lt(struct Text *const this) { TextAdd(this, "&lt;"); }
/** @implements	TextAction */
static void gt(struct Text *const this) { TextAdd(this, "&gt"); }
static void new_docs(struct Text *const); /* prototype: recursive TextPattern */

static const struct TextPattern tp_docs[] = {
	{ "/** ", "*/", &new_docs }
}, tp_inner[] = {
	{ "\\url{", "}", &url },
	{ "\\cite{", "}", &cite },
	{ "{", "}", &em },
	{ "&", 0, &amp },
	{ "<", 0, &lt },
	{ ">", 0, &gt }
};

/** @implements	TextAction */
static void new_docs(struct Text *const this) {
	char *const text_buf = TextGetValue(this);
	struct Text *doc;
	char *s0, *s1;
	int is_first, is_last;
	size_t key_length;
	char *key;
	char desc[10] = "_desc"; /* the first part that has no @ */

	/* match delineated be each */
	for(is_first = -1, is_last = 0, s0 = s1 = text_buf; !is_last; ) {
		/* skip the embedded 'each's */
		while((s1 = strpbrk(s1, "@")) && !is_first_on_line(text_buf, s1)) s1++;
		if(!s1) is_last = -1, s1 = s0 + strlen(s0);
		if(is_first) {
			key = desc, key_length = strlen(desc);
		} else {
			key = s0, key_length = word_length(s0), s0 += key_length;
		}
		fprintf(stderr, "new_docs: \"%.*s\"->\"%.*s\"\n", (int)key_length, key, (int)(s1 - s0), s0);
		if(!(doc = TextNewChild(this, key, key_length, s0, (size_t)(s1 - s0))))
			{ fprintf(stderr, "new_docs: %s.\n", TextGetError(this)); return; }
		TextMatch(doc, tp_inner, sizeof tp_inner / sizeof *tp_inner);

		is_first = 0;
		s0 = s1 = s1 + 1;
	}

	/* **************************here************************************* */
	/* str = strpbrk(TextGetParentBuffer()[TextGetParentEnd()], "{;") if(!{)
	 * *str = '\0', parse_generic() */
	/*printf("new_docs: \"%s\".\n", TextGetBuffer(sub));*/
	/*TextMatch(this, tp_inner, sizeof tp_inner / sizeof *tp_inner);*/
}

/*           *
 *************/

/** The is a test of Text.
 @param argc	Count
 @param argv	Vector. */
int main(int argc, char *argv[]) {
	enum { E_NO_ERR, E_ERRNO, E_TEXT/*, E_LIST*/ } error = E_NO_ERR;
	struct Text *text_buf = 0;
	char *fn;

	if(argc != 2) {
		/*fn = "src/Test.c";*/
		fprintf(stderr, "Needs <filename>.\n");
		return EXIT_FAILURE;
	} else {
		fn = argv[1];
	}

	do {

		if(!(text_buf = TextFile(fn))) { error = E_TEXT; break; }
		/* parse for " / * * " */
		if(!TextMatch(text_buf, tp_docs, sizeof tp_docs / sizeof *tp_docs))
			{ error = E_TEXT; break; }
		/*printf("***%s***\n", TextToString(text));*/
		xml(text_buf);

	} while(0);
	switch(error) {
		case E_NO_ERR: break;
		case E_ERRNO: perror(fn); break;
		case E_TEXT:
			fprintf(stderr, "%s: %s.\n", fn, TextGetError(text_buf)); break;
	}
	{
		Text_(&text_buf);
	}

	fprintf(stderr, "Done all tests; %s.\n", error ? "FAILED" : "SUCCEDED");

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}

#if 0

/** Returns between str and peren or null. */
static char *match_opening_perenthesis(const char *const str, char *peren) {
	unsigned stack = 0;
	while(str < peren) {
		switch(*peren) {
			case '\0': return 0;
			case ')': stack++; break;
			case '(': stack--; break;
			default: break;
		}
		if(!stack) return peren;
		peren--;
	}
	return 0;
}

/** \see{prev_fuction_part}, except generics */
static char *prev_generic_part(const char *const str, char *a) {
	int is_one = 0;
	if(a <= str) return 0;
	if(*a == '_') a++; /* starting */
	if(*--a != '_') return 0;
	a--;
	while(isalnum(*a)) {
		if(a <= str) return 0;
		a--;
		is_one = -1;
	}
	/*if(is_one) printf("prev_g ret: '%s'\n", a + 1);*/
	return is_one ? a + 1 : 0;
}

/** Function-like chars? */
static int isfunction(int c) {
	return isalnum(c) || c == '_' || c == '<' || c == '>';
}

static void cutoff_generic(char *a) {
	while(isalnum(*a)) a++;
	*a = '\0';
}

static void cutoff_name(char *a) {
	while(isfunction(*a)) a++;
	*a = '\0';
}

/** Searches backwards from the previous char to {a}, hits a function, and
 stops when it hits the end of something that looks function-like -- only call
 when you know it has a '('. */
static char *prev_function_part(char *a) {
	const int is_peren = *a == ')' ? -1 : 0;
	/*printf("prev_f '%s'\n", a);*/
	a--;
	/*while(!isfunction(*a)) { if(*a == '(') return 0; a--; }*/
	while(isspace(*a)) a--;
	if(!is_peren && *a != ',') return 0; /* the only thing we expect: , */
	if(is_peren  && *a == ',') return a + 1; /* eg, "(AddIf,)" */
	a--;
	while(isspace(*a)) a--;
	while(isfunction(*a)) a--;
	/*printf("pfp ret: '%s'\n", a + 1);*/
	return a + 1;
}


/******/

static void print_text(struct Hash *const h) {
	print_parsed_paragraph(h->value);
}

static void print_header_text(struct Hash *const h) {
	printf("<h3>");
	print_parsed(h->key);
	printf("</h3>\n");
	print_parsed_paragraph(h->value);
}

static void print_desc_list(struct Hash *const h) {
	printf("\t<dt>%s</dt>\n", h->key);
	printf("\t<dd>%s</dd>\n", h->value);
}

/** This is a hack to go from, "struct T_(Foo) *T_I_(Foo, Create)," to
 "struct <T>Foo *<T>Foo<I>Create"; which is entirely more readable! Could
 create a new static buffer, or just return the argument if it's not
 modified. */
static char *parse_generics(char *const fn) {
	char *start_of_buffer = buffer_pos;
	char temp[2048];
	struct { char *generic, *name; } gen[16], *g; /* one generic */
	unsigned gen_i_g, gen_i_n, i; /* generics get substituted backwards */
	char *a, *b, *c, *generic, *name;
	size_t fn_len;
	
	fn_len = strlen(fn);
	if(fn_len >= sizeof temp
	   || (size_t)(buffer + sizeof buffer / sizeof *buffer - buffer_pos + 1)
	   < sizeof temp / sizeof *temp) {
		fprintf(stderr, "%32s: this was not parsed for generics because the "
				"buffer was full.\n", fn);
		return 0;
	}
	strcpy(temp, fn);
	a = temp;
	/* parse into "gen" -- assume it won't be nested */
	while((b = strstr(a, "_(")) && (c = strchr(b, ')'))) {
		/*printf("interpret is here: (a='%s', b='%s')\n", a, b);*/
		/* search "_T_I_" */
		gen_i_g = 0;
		generic = b;
		while((generic = prev_generic_part(fn, generic))) {
			if(gen_i_g >= sizeof gen / sizeof *gen) return 0;
			gen[gen_i_g++].generic = generic;
		}
		/* search "(foo, create)" */
		gen_i_n = 0;
		name = c;
		while((name = prev_function_part(name))) {
			if(gen_i_n >= sizeof gen / sizeof *gen) return 0;
			gen[gen_i_n++].name = name;
		}
		if(gen_i_g != gen_i_n) return 0; /* that's weird */
		/* terminate strings; print into a permanent buffer */
		/* printf("*** adding \"%.*s\"\n", (int)(gen[gen_i_g - 1].generic - a),
		 a); */
		sprintf(buffer_pos, "%.*s", (int)(gen[gen_i_g - 1].generic - a), a);
		buffer_pos += strlen(buffer_pos);
		for(i = gen_i_g; i; i--) {
			g = gen + i - 1;
			cutoff_generic(g->generic);
			cutoff_name(g->name);
			/* printf("*** adding \"<%s>%s\"\n", g->generic, g->name); */
			sprintf(buffer_pos, "<%s>%s", g->generic, g->name);
			buffer_pos += strlen(buffer_pos);
		}
		/* advance */
		a = c + 1;
	}
	/* just a copy is good, too! */
	/*if(a == temp) return fn;*/ /* no generics */
	/* copy the rest (probably nothing) */
	/* printf("*** adding \"%s\"\n", a); */
	strcpy(buffer_pos, a);
	buffer_pos += strlen(a) + 1;
	
	return start_of_buffer;
}

int main(int argc, char **argv) {
	struct _SuperCat cat;
	static char buffer[0x40000];
	char get[1024];
	unsigned no_c = 0 /* chapter index */, i;
	struct Chapter *ch = 0;
	char *lperen, *rperen;
	enum { E_NO, E_ERRNO, E_MAX } error = E_NO;

	char *cursor, *end, *fn, *args;
	int is_in_comment = 0, is_fn_search  = 0;

	if(argc > 1 || (!argv && argv)) { usage(); return EXIT_SUCCESS; }

	/* read the whole buffer */
	_super_cat_init(&cat, buffer, sizeof buffer / sizeof *buffer);
	while(fgets(get, (int)sizeof get, stdin)) _super_cat(&cat, get);
	if(ferror(stdin)) { perror("stdin"); return EXIT_FAILURE; }

	/* parse */
	cursor = buffer;
	do {
		if(!is_in_comment) {
			char *return_value, *fn_name;
			/* could be \ before, in a string, but it will do for now */
			/* two docs must not be on consecutive lines */
			/* all fn must have docs, or else they are invisible */
			/* the start of a docs comment */

			end = strstr(cursor, "/" "** ");

			/* laughably fragile: before going on, let one check that there's a
			 function, and add it to the previous Chapter;
			 fixme: put in fn! */
			if(is_fn_search
			   && ch->hash_no
			   && (fn = strpbrk(cursor, ";{/"))
			   /* fn is a {, means a fn is near? */
			   && *fn == '{'
			   && (!end || fn < end)
			   /* cursor < fn, start from the end: "(args)" of "void *fn(args)" */
			   && (*fn = '\0', cursor = trim(cursor),
				   rperen = strrchr(cursor, ')'))
			   && (lperen = match_opening_perenthesis(cursor, rperen))
			   /* fn:"void fn", args:"args" of "void *fn0args0" */
			   && (args = parse_generics(trim(lperen)))
			   && (*lperen++ = '\0', *rperen = '\0',
				   fn = parse_generics(trim(cursor)))
			   /* return_value:"void *" and fn:"fn" of copy "args0void *0fn0" */
			   && (return_value = fn, strlen(fn))
			   ) {
				/* fn_name is the last isfunction */
				for(fn_name = fn + strlen(fn) - 1;
					isspace(*fn_name) && fn_name > fn;
					fn_name--);
				for( ; isfunction(*fn_name) && fn_name > fn; fn_name--);
				fn_name++;
				/* break apart */
				if((fn_name = buffer_split(fn_name))) {
					/* fprintf(stderr, "return value: <%s> fn name: <%s>\n", return_value, fn_name); */
					trim(return_value);
					/* place them in hash */
					ch_put(ch, "_return", return_value);
					ch_put(ch, "_fn",     fn_name);
					ch_put(ch, "_args",   args);
				}
			}

			is_fn_search = 0;

			/* get back to the start of the auto-doc */
			if((end)) {
				is_fn_search = 0; /* reset */
				end += 4, is_in_comment = -1; /* for "/" "**" */
				/* move to the next chapter */
				if(no_c >= max_chapters) { error = E_MAX; break; }
				init_chapter(ch = chapters + no_c++);
			}
			cursor = end;
		} else {
			/* could be \ before, in a string, but it will do for now */
			end = strstr(cursor, "*" "/");
			if(end) {
				is_in_comment = 0;
				*end = '\0';
				is_fn_search = -1;
			}
			parse_each(ch, trim(cursor));
			/*printf("<%s>\n", cursor);*/
			cursor = end ? end + 2 : 0;
		}
	} while(cursor);

	if(cursor) {
		switch(error) {
			case E_NO: fprintf(stderr, "No error?\n"); break;
			case E_ERRNO: perror(programme); break;
			case E_MAX: fprintf(stderr, "A hard maximum was exceeded; consider "
								"re-designing your C file?\n"); break;
		}
		return EXIT_FAILURE;
	}
	if(!no_c) {
		fprintf(stderr, "No Doc /" "** comments *" "/ found.\n");
		return EXIT_FAILURE;
	}

	/* print header */
	printf("<!doctype html public \"-//W3C//DTD HTML 4.01//EN\" "
		   "\"http://www.w3.org/TR/html4/strict.dtd\">\n\n");
	printf("<html>\n\n");
	printf("<head>\n");
	printf("<link rel = \"stylesheet\" type = \"text/css\" "
		   "href = \"stylesheet.css\">\n");
	printf("<title>%s</title>\n", ch_get(chapters + 0, "file"));
	printf("</head>\n\n");
	printf("<body>\n");
	printf("<h1>%s</h1>\n\n", ch_get(chapters + 0, "file"));
	ch_for_each_passed(chapters + 0, &match_main,  &print_text);
	ch_for_each_passed(chapters + 0, &match_param, &print_header_text);
	ch_for_each_passed(chapters+0, &match_things_in_header, &print_header_text);
	printf("<hr>\n\n");

	/* print table */
	printf("<!-- function summary -->\n\n");
	printf("<table>\n");
	printf("<tr><th colspan = 2><h2>Function Summary</h2></th></tr>\n");
	for(i = 0; i < no_c; i++) {
		ch = chapters + i;
		if(!ch_get(ch, "_fn")) continue;
		printf("<tr>\n");
		printf("\t<td>%s</td>\n", ch_get(ch, "_return"));
		printf("\t<td><a href = \"#%s\">%s</a>%s</td>\n", ch_get(ch, "_fn"),
			   ch_get(ch, "_fn"), ch_get(ch, "_args"));
		printf("</tr>\n");
	}
	printf("</table>\n");
	printf("<hr>\n\n");

	/* print details */
	printf("<!-- function detail -->\n\n");
	printf("<h2>Function Detail</h2>\n\n");
	for(i = 0; i < no_c; i++) {
		ch = chapters + i;
		if(!ch_get(ch, "_fn")) continue;
		printf("<a name = \"%s\"><!-- --></a>\n", ch_get(ch, "_fn"));
		printf("<h3>%s</h3>\n", ch_get(ch, "_fn"));
		printf("<pre>%s <b>%s</b> %s</pre>\n", ch_get(ch, "_return"),
			   ch_get(ch, "_fn"), ch_get(ch, "_args"));
		printf("<dl>\n");
		ch_for_each_passed(ch, &match_param, &print_desc_list);
		ch_for_each_passed(ch, &match_things_in_fn, &print_desc_list);
		printf("</dl>\n");
		ch_for_each_passed(ch, &match_main, &print_text);
	}
	printf("<hr>\n\n");

	/* print closing */
	printf("<hr>\n");
	printf("</body>\n\n");
	printf("</html>\n");

	/* debug */
	for(i = 0; i < no_c; i++) {
		ch = chapters + i;
		fprintf(stderr, "  __new chapter__\n");
		ch_print(ch);
	}

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
