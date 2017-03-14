/** Copyright 2015 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 This is a test of Relates; call it with a .c file.

 @author	Neil
 @version	3.0; 2016-08
 @since		3.0; 2016-08 */

#include <stdlib.h>	/* rand, EXIT_* */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp strdup */
#include <ctype.h>	/* isspace */
#include "../src/Relates.h"

/* static const char *const things_in_header[] = {
	"author", "version", "since", "fixme"
};	static const char *const things_in_fn[] = {
	"return", "throws", "implements", "fixme", "author"
}; */

/***************
 * XML testing */

/** Write a bunch of XML CDATA. FIXME */
static void cdata(const char *const str) {
	const char *a = str, *b;
	printf("<![CDATA[");
	while((b = strstr(a, "]]>"))) {
		printf("%.*s]]]]><![CDATA[>", (int)(b - a), a);
		a = b + 3 /* ]]>.length */;
	}
	printf("%s]]>", a);
}

/* prototype -- calls recursively
 @implem */
static void print_text(struct Relate *const);

/** XML is weird. */
static void xml_recursive(struct Relate *const this, const int is_top) {
	const struct RelateParent *rp;
	if(!is_top) printf("<key><![CDATA[%s]]></key>\n", RelateKey(this));
	printf("<dict>\n<key>");
	cdata(RelateKey(this));
	printf("</key>\n<string>");
	cdata(RelateValue(this));
	printf("</string>\n");
	if(!is_top && (rp = RelateGetValueParent(this)) && rp->is_within) {
		printf("<key>begin</key><integer>%lu</integer>\n"
			"<key>end</key><integer>%lu</integer>\n", rp->start, rp->end);
	}
	RelateForEachTrueChild(this, 0, &print_text);
	printf("</dict>\n");
}

/** @implements	TableAction */
static void print_text(struct Relate *const this) { xml_recursive(this, 0); }

static void xml(struct Relate *const this) {
	printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	printf("<!DOCTYPE plist>\n");
	printf("<plist version=\"1.0\">\n");
	xml_recursive(this, -1);
	printf("</plist>\n");
}

/***********************
 * General text-things */

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

/***************************
 * Parsing a function name */

/** Function-like chars? */
static int isfunction(int c) {
	/* generics or escape; "<>&;" also have meaning, but we hopefully will be
	 far from that */
	return isalnum(c) || c == '_' || c == '<' || c == '>' || c == ';'
		|| c == '&';
}

/** Returns a matching closing parethesis for the {left} or null if it couldn't
 find one. */
static char *match_parenthesis(char *const left) {
	unsigned stack = 0;
	char l, r;
	char *s = left;

	switch(l = *left) {
		case '[': r = ']'; break;
		case '{': r = '}'; break;
		case '(': r = ')'; break;
		case '<': r = '>'; break;
		case '`': r = '\'';break;
		default: return 0;
	}
	while(*s) {
		if(*s == l) stack++;
		else if(*s == r && !--stack) return s;
		s++;
	}
	return 0;
}

/** Searches backwards from the previous char to {a}, hits a function, and
 stops when it hits the end of something that looks function-like -- only call
 when you know it has a '('. Called by \see{parse_generics}. */
static const char *prev_function_part(const char *a) {
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

/** Starting from the end of a function, this retrieves the previous generic
 part. Called by \see{parse_generics}. */
static const char *prev_generic_part(const char *const str, const char *a) {
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

/** This is a hack to go from, "struct T_(Foo) *T_I_(Foo, Create)," to
 "struct <T>Foo *<T>Foo<I>Create"; which is entirely more readable! */
static int parse_generics(struct Relate *const this) {
	struct Text *temp = 0;
	struct Generic { const char *type, *name; } generics[16], *generic;
	const size_t generics_size = sizeof generics / sizeof *generics;
	struct Text *original = RelateGetValue(this);
	unsigned types_size, names_size, i;
	const char *start, *type, *name, *end;
	enum { E_NO, E_A, E_GAVE_UP } e = E_NO;

	do {
		if(!(temp = Text("parse_generics"))) { e = E_A; break; }
		/* {<start>bla bla T_I<type>_(Destroy, World<name>)<end>};
		 assume it won't be nested; work backwards */
		start = TextToString(original);
		while((type = strstr(start, "_(")) && (name = strchr(type, ')'))) {
			end = name + 1;
			/* search types "_T_I_" backwards */
			types_size = 0;
			while((type = prev_generic_part(start, type))) {
				if(types_size >= generics_size) { e = E_GAVE_UP; break; }
				generics[types_size++].type = type;
			}
			if(e) break;
			/* search "(foo, create)" backwards */
			names_size = 0;
			while((name = prev_function_part(name))) {
				if(names_size >= generics_size) { e = E_GAVE_UP; break; }
				generics[names_size++].name = name;
			}
			if(e) break;
			if(!types_size || types_size != names_size) { e = E_GAVE_UP; break;}
			/* all the text up to the generic is unchanged */
			if(!TextNCat(temp, start,
				(size_t)(generics[types_size-1].type - start)))
				{ e = E_A; break; }
			/* reverse the reversal */
			for(i = types_size; i; i--) {
				size_t type_len, name_len;
				const char *type_end, *name_end;
				generic = generics + i - 1;
				/* fixme: probably should have assigned these up top */
				for(type_end = generic->type; isalnum(*type_end);   type_end++);
				type_len = type_end - generic->type;
				for(name_end = generic->name; isfunction(*name_end);name_end++);
				name_len = name_end - generic->name;
				/*fprintf(stderr, "parse_generics: <%.*s>%.*s\n", (int)type_len,
					generic->type, (int)name_len, generic->name);*/
				/* fixme: <, >, are verboten in html */
				if(!TextPrintfCat(temp, "<%s>%s", generic->type, generic->name))
					{ e = E_A; break; }
				/*if(!TextCat(temp_val, "<")
					|| !TextNCat(temp, generic->type, type_len)
					|| !TextCat(temp, ">")
					|| !TextNCat(temp, generic->name, name_len))*/
			}
			if(e) break;
			/* advance */
			start = end;
		}
		if(e) break;
		/* copy the rest (probably nothing) */
		if(!TextCat(temp, start)) { e = E_A; break; }
		/* copy the temporary a to this */
		if(!TextCopy(original, TextToString(temp))) { e = E_A; break; }
	} while(0);
	switch(e) {
		case E_NO: break;
		case E_A: fprintf(stderr, "parse_generics: temp buffer, %s.\n",
			TextGetError(temp)); break;
		case E_GAVE_UP: fprintf(stderr, "parse_generics: syntax error.\n");
			break;
	}
	/*fprintf(stderr, "parse_generics: <%s>\n\n", TableGetValue(a));*/
	{ /* finally */
		Text_(&temp);
	}

	return e ? 0 : -1;
}

/*******************************************************
 * These go in a Pattern array for calling in {Match}. */

/** Must be in rfc3986 format; \url{https://www.ietf.org/rfc/rfc3986.txt }.
 @implements	TextAction */
static void url(struct Text *const this) {
	TextTrim(this), TextTransform(this, "<a href = \"%s\">%s</a>");
}
/** Must be in query format; \url{ https://www.ietf.org/rfc/rfc3986.txt }.
 @implements	TextAction */
static void cite(struct Text *const this) {
	TextTrim(this), TextTransform(this,
		"<a href = \"https://scholar.google.ca/scholar?q=%s\">%s</a>");
}
/** @implements	TextAction */
static void em(struct Text *const this) { TextTransform(this, "<em>%s</em>"); }
/** @implements	TextAction */
static void amp(struct Text *const this) { TextCopy(this, "&amp;"); }
/** @implements	TextAction */
static void lt(struct Text *const this) { TextCopy(this, "&lt;"); }
/** @implements	TextAction */
static void gt(struct Text *const this) { TextCopy(this, "&gt;"); }

static const struct TextPattern tpattern[] = {
	{ "\\url{",  "}", &url },
	{ "\\cite{", "}", &cite },
	{ "{",       "}", &em },
	{ "&",       0,   &amp },
	{ "<",       0,   &lt },
	{ ">",       0,   &gt }
};

#if 0

static struct Relate *new_doc_store; /* global for \see{new_docs} */

/** Matches / * *   * / and calls {tpattern} on those matches. Needs
 {new_doc_store} to be set.
 @implements	RelateAction */
static void new_docs(struct Text *const this) {
	struct Table *doc_text, *sig_text;
	char *s0, *s1;
	int is_first, is_last;
	size_t key_length;
	char *key;
	char desc_key[] = "_desc", signature_key[] = "_signature",
	return_key[] = "_return", fn_key[] = "_fn", args_key[] = "_args";
	struct TextCut cut = { 0, 0, 0 };

	/* search for function signature immediately below */
	do {
		struct Text *parent;
		size_t parent_end;
		const char *start, *end, *sig, *opening, *closing;

		TextGetMatchParentInfo(&parent, 0, &parent_end);
		start = TextToString(parent) + parent_end;
		/* fixme: actually parse; this is sufficient for most cases, I guess */
		end = strpbrk(start, ";{/#");
		if(!end || *end != '{') break;
		if(!(sig_text = RelateNewChild(new_doc_store))) { fprintf(stderr,
			"new_docs: parsing signature, %s.\n", TextGetError(this)); break; }
		, signature_key, strlen(signature_key),
		start, strlen(start))))
		TableTrim(sig_text);
		/* parse for (very author-cetric, sorry!) generics */
		if(!(parse_generics(sig_text))) break;
		/* split the signature into to separate parts */
		if(!(sig = TableGetValue(sig_text)) || !(opening = strchr(sig, '('))
		   || opening == sig || !(closing = match_parenthesis(opening))) break;
		/* select what looks like a function name */
		for(s1 = opening - 1; s1 > sig && !isfunction(*s1) && *s1; s1--); s1++;
		for(s0 = s1 - 1; s0 > sig && isfunction(*s0); s0--);
		if(s0 == sig) break;
		s0++;
		/* return type is all the stuff ahead of the function name */
		/*fprintf(stderr, "new_docs: \"%.*s\", \"%.*s\", \"%.*s\"\n",
		 (int)(s0 - sig), sig, (int)(s1 - s0), s0,
		 (int)(closing - opening - 1), opening + 1);*/
		/* { _signature } = { _return, _fn, _args }; sorry fans of the old
		 syntax; the most impotant is fn, goes last */
		if(!TableNewChild(this, return_key, strlen(return_key), sig,
			(size_t)(s0 - sig)) || !TableNewChild(this, args_key,
			strlen(args_key), opening + 1, (size_t)(closing - opening - 1))
			|| !TableNewChild(this, fn_key, strlen(fn_key), s0,
			(size_t)(s1 - s0))) break;
	} while(0);
	{ /* finnally */
		TableUncut(&cut);
	}

	/* match delineated be each, '@' */
	for(is_first = -1, is_last = 0, s0 = s1 = text_buf; !is_last; ) {
		/* skip the embedded 'each's */
		while((s1 = strpbrk(s1, "@")) && !is_first_on_line(text_buf, s1)) s1++;
		if(!s1) is_last = -1, s1 = s0 + strlen(s0);
		if(is_first) {
			key = desc_key, key_length = strlen(desc_key);
		} else {
			key = s0, key_length = word_length(s0), s0 += key_length;
		}
		/* fprintf(stderr, "new_docs: \"%.*s\"->\"%.*s\"\n",
		 (int)key_length, key, (int)(s1 - s0), s0); */
		if(!(doc_text
			 = TableNewChild(this, key, key_length, s0, (size_t)(s1 - s0))))
		{ fprintf(stderr, "new_docs: %s.\n", TableGetError(this)); return; }
		TableTrim(doc_text);

		/* parse it for additional \foo{} */
		TableMatch(doc_text, tp_inner, sizeof tp_inner / sizeof *tp_inner);

		is_first = 0;
		s0 = s1 = s1 + 1;
	}

}

static const struct TextPattern rpattern[] = {
	{ "/""** ", "*""/", &new_docs }
	/* fixme: more robust, ie \* { / * * /, 0 }? */
};
#endif



/******************
 * Main programme */

/** Selects functions by looking for _fn.
 @implements	TablePredicate
 @fixme			No; instead of the key being a child, make it the key; we can
				do that now. */
static int select_functions(struct Relate *const this) {
	return RelateGetChildKey(this, "_fn") ? -1 : 0;
}

/** Does the inverse of \see{select_fuctions}.
 @implements	TablePredicate */
static int select_non_functions(struct Relate *const this) {
	return !select_functions(this);
}

/** Prints header (at the top of the page, supposedly.) */
static void print_header(struct Relate *const this) {
	struct Relate *sub;
	if((sub = RelateGetChildKey(this, "file"))) {
		printf("<h1>%s</h1>\n", RelateValue(sub));
	}
	if((sub = RelateGetChildKey(this, "_desc"))) {
		printf("%s\n", RelateValue(sub));
	}
}

/** The is a test of Table.
 @param argc	Count
 @param argv	Vector. */
int main(int argc, char *argv[]) {
	struct Relates *rs = 0;
	struct Relate *rs_root;
	struct Text *rs_root_value;
	FILE *fp = 0;
	const char *fn;
	enum { E_NO_ERR, E_ERRNO, E_RS, E_VALUE } error = E_NO_ERR;

#if 0
	if(argc != 2) {
		/*fn = "src/Test.c";*/
		fprintf(stderr, "Needs <filename>.\n");
		return EXIT_FAILURE;
	} else {
		fn = argv[1];
	}
#else
	fn = "/Users/neil/Movies/Common/Text/src/Text.c";
#endif

	do {

		/* new Relates; getting values */
		if(!(rs = Relates("_docs_root"))) { error = E_RS; break; }
		rs_root = RelatesGetRoot(rs);
		rs_root_value = RelateGetValue(rs_root);
		/* opening {fn}; reading */
		if(!(fp = fopen(fn, "r"))) { error = E_ERRNO; break; }
		if(!TextFileCat(rs_root_value, fp)) { error = E_VALUE; break; }
		if(fclose(fp)) { error = E_ERRNO; break; }
		/* parse for " / * * "; it recursively calls things as appropriate */
		if(!TextMatch(RelateGetValue(rs_root), tpattern, sizeof tpattern / sizeof *tpattern))
			{ error = E_RS; break; }
		/*printf("***%s***\n", TableToString(text));*/
#if 1
		xml(rs_root);
#else
		/* print the header(s?) */
		RelateForEachTrueChild(r, &select_non_functions, &print_header);
#endif

	} while(0);
	switch(error) {
		case E_NO_ERR: break;
		case E_ERRNO: perror(fn); break;
		case E_RS:
			fprintf(stderr, "%s: %s.\n", fn, RelatesGetError(rs)); break;
		case E_VALUE:
			fprintf(stderr, "%s: %s.\n", fn, TextGetError(rs_root_value));break;
	}
	{
		Relates_(&rs);
		fclose(fp);
	}

	fprintf(stderr, "Done all tests; %s.\n", error ? "FAILED" : "SUCCEDED");

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}

#if 0

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
				"re-design?\n"); break;
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
