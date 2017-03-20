/** 2015 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 This is a test of Relates that got out-of-hand; redirect a {.c} file that has
 JavaDoc-style / * *, * /. 

 @author	Neil
 @version	3.0; 2016-08
 @since		3.0; 2016-08 */

#include <stdlib.h>	/* rand, EXIT_* */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp strdup */
#include <ctype.h>	/* isspace */
#include "../src/Relates.h"

/* https://www.programiz.com/c-programming/library-function/ctype.h/isspace */
const char *const white_space = " \n\t\v\f\r";
const char *const separates_param_value = ":\n\t\v\f\r";

#ifdef DEBUG
/* my debugger doesn't like calling with args or piping, hard code */
/*const char *const fn = "/Users/neil/Movies/Common/Text/src/Text.c";*/
const char *const fn = "/Users/neil/Movies/Common/List/src/List.h";
#else
const char *const fn = "stdin";
#endif

/*************************
 * Command line options. */

static void xml(struct Relate *const this);
static void plain(struct Relate *const this);
static void html(struct Relate *const this);

const struct {
	const char *const str;
	const RelateAction fun;
	const char *const open, *const close;
} fmt[] = {
	{ "xml",  &xml,   "<", ">" },
	{ "text", &plain, "<", ">" },
	{ "html", &html,  "&lt;", "&gt;" }
}, *fmt_chosen = fmt + 2;
const size_t fmt_size = sizeof fmt / sizeof *fmt;

/* global root */
static struct Relates *root;

/********************************************************
 * This goes under \see{new_docs} for {TextStrip('@')}. */

typedef void (*RelatesField)(struct Relate *const parent,
	struct Text *const key, struct Text *const value);

/** @implements	RelatesField */
static void new_child(struct Relate *const parent, struct Text *const key,
	struct Text *const value) {
	struct Relate *child;
	/*fprintf(stderr, "here: %s -> %s\n", TextToString(key),
		TextToString(value));*/
	child = RelateNewChild(parent);
	TextCat(RelateGetKey(child), TextToString(key));
	TextCat(RelateGetValue(child), TextToString(value));
}
/** @implements	RelatesField */
static void new_arg_child(struct Relate *const parent, struct Text *const key,
	struct Text *const value) {
	struct Relate *child, *grandc;
	struct Text *arg;
	arg = TextSep(value, separates_param_value, 0);
	TextTrim(value), TextTrim(arg);
	child = RelateNewChild(parent);
	TextCat(RelateGetKey(child),   TextToString(key));
	TextCat(RelateGetValue(child), TextToString(value));
	if(!arg) return;
	grandc = RelateNewChild(child);
	TextCat(RelateGetKey(grandc),   "_arg");
	TextCat(RelateGetValue(grandc), TextToString(arg));
	if(TextIsError(arg))
		fprintf(stderr, "new_arg_child arg: %s.\n", TextGetError(arg));
	Text_(&arg);
}
/** @implements	RelatesField */
static void top_key(struct Relate *const parent, struct Text *const key,
	struct Text *const value) {
	do { break; } while(key);
	/*fprintf(stderr, "HERE!!! %s\n", TextToString(value));*/
	TextCat(RelateGetKey(parent), TextToString(value));
}

static const struct EachMatch {
	const char *word;
	RelatesField what;
} each_head[] = {
	{ "file",    &top_key },
	{ "param",   &new_arg_child },
	{ "author",  &new_child },
	{ "std",     &new_child },
	{ "version", &new_child },
	{ "since",   &new_child },
	{ "fixme",   &new_child }
}, each_fn[] = {
	{ "param",   &new_arg_child },
	{ "return",  &new_child },
	{ "throws",  &new_arg_child },
	{ "implements", &new_child },
	{ "fixme",   &new_child },
	{ "author",  &new_child },
	{ "since",   &new_child }
	/* fixme: deprecated */
};
static const size_t each_head_size = sizeof each_head / sizeof *each_head,
	each_fn_size = sizeof each_fn / sizeof *each_fn;

/** Called from \see{new_docs}. */
static void parse_each(struct Text *const this, struct Relate *const parent,
	const struct EachMatch *const ems, const size_t ems_size) {
	const struct EachMatch *em;
	size_t e, key_sl;
	struct Text *key;
	const char *key_s;

	/*printf("parse_@: '%s'\n", TextToString(this));*/
	if(!(key = TextSep(this, white_space, 0))) {
		if(TextIsError(this)) fprintf(stderr,"Error: %s.\n",TextGetError(this));
		return;
	}
	TextTrim(this);
	/* linear search */
	key_s = TextToString(key);
	key_sl = strlen(key_s);
	for(e = 0; e < ems_size && ((em = ems + e, key_sl != strlen(em->word))
		|| strncmp(em->word, key_s, key_sl)); e++);
	if(e >= ems_size) {
		fprintf(stderr, "Warning: unrecognised @-symbol, '%.*s.'\n",
			(int)key_sl, key_s);
		Text_(&key);
		return;
	}
	em->what(parent, key, this);
}

/***********************
 * General text-things */

/** @return		Is the character pointed to by {s} in the string {str} the
				first on the line?
 @implements	TextPredicate */
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

/***************************
 * Parsing a function name */

/** Function-like chars? */
static int isfunction(int c) {
	/* generics or escape; "<>&;" also have meaning, but we hopefully will be
	 far from that, and C++, I'm sorry */
	return isalnum(c) || c == '_' || c == '<' || c == '>' || c == ';'
		|| c == '&';
}

/** Returns a matching closing parethesis for the {left} or null if it couldn't
 find one. */
static const char *match_parenthesis(const char *const left) {
	unsigned stack = 0;
	char l, r;
	const char *s = left;

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
static int prev_function_part(const char **const pa, const char **const pb) {
	const char *a = *pa;
	const int is_peren = *a == ')' ? -1 : 0;

	a--;
	while(isspace(*a)) a--;
	if(!is_peren) {
		if(*a != ',') return 0; /* the only thing we expect: , */
		a--;
		while(isspace(*a)) a--;
	}
	*pb = a;
	while(isfunction(*a)) a--;
	*pa = a + 1;
	while(isspace(*a)) a--; /* look behind to see if it's really a generic */

	return (*a != '(' && *a != ',') ? 0 : -1;
}

/** Starting from the end of a function, this retrieves the previous generic
 part. Called by \see{parse_generics}. */
static int prev_generic_part(const char *const str, const char **const pa,
	const char **const pb) {
	int is_one = 0;
	const char *a = *pa;

	if(a <= str) return 0;
	if(*a == '_') a++; /* starting */
	if(*--a != '_') return 0;
	a--;
	*pb = a;
	while(isalnum(*a)) {
		if(a <= str) return *pa = str, is_one ? -1 : 0;
		a--, is_one = -1;
	}
	*pa = ++a;

	return is_one ? -1 : 0;
}

/** This is a hack to go from, "struct T_(Foo) *T_I_(Foo, Create)," to
 "struct <T>Foo *<T>Foo<I>Create"; which is entirely more readable! */
static int parse_generics(struct Text *const this) {
	struct Text *temp = 0;
	struct Generic { const char *type, *tend, *name, *nend; }
		generics[16], *generic;
	const size_t generics_size = sizeof generics / sizeof *generics;
	unsigned types_size, names_size, i;
	const char *s0, *s1, *s1end, *s2, *s2end, *s3;
	enum { E_NO, E_A, E_GAVE_UP } e = E_NO;

	if(!this) return 0;
	do {
		if(!(temp = Text())) { e = E_A; break; }
		/* {<s0>bla bla T_I<s1>_(Destroy, World<s2>)<s3>};
		 assume it won't be nested; work backwards */
		s0 = TextToString(this);
		while((s1 = strstr(s0, "_(")) && (s2 = strchr(s1, ')'))) {
			s3 = s2 + 1;
			/* search types "_T_I_" backwards */
			types_size = 0;
			while((prev_generic_part(s0, &s1, &s1end))) {
				if(types_size >= generics_size) { e = E_GAVE_UP; break; }
				generic = generics + types_size++;
				generic->type = s1, generic->tend = s1end;
			} if(e) break;
			/* search "(foo, create)" backwards */
			names_size = 0;
			while((prev_function_part(&s2, &s2end))) {
				if(names_size >= generics_size) { e = E_GAVE_UP; break; }
				generic = generics + names_size++;
				generic->name = s2, generic->nend = s2end;
			} if(e) break;
			/*for(i = types_size; i; i--) {
				generic = generics + i - 1;
				fprintf(stderr, "type: '%.*s'\n",
					(int)(generic->tend - generic->type + 1), generic->type);
			}
			for(i = names_size; i; i--) {
				generic = generics + i - 1;
				fprintf(stderr, "name: '%.*s'\n",
					(int)(generic->nend - generic->name + 1), generic->name);
			}*/
			/* doesn't look like a generic, just cat, continue to the next */
			if(!types_size || types_size != names_size) {
				TextBetweenCat(temp, s0, s3 - 1);
				s0 = s3;
				continue;
			}
			/* all the text up to the generic is unchanged */
			TextBetweenCat(temp, s0, generics[types_size - 1].type - 1);
			/* reverse the reversal */
			for(i = types_size; i; i--) {
				generic = generics + i - 1;
				TextCat(temp, fmt_chosen->open);
				TextBetweenCat(temp, generic->type, generic->tend);
				TextCat(temp, fmt_chosen->close);
				TextBetweenCat(temp, generic->name, generic->nend);
			} if(e) break;
			/* advance */
			s0 = s3;
		}
		if(e) break;
		/* copy the rest (probably nothing) */
		if(!TextCat(temp, s0)) { e = E_A; break; }
		/* copy the temporary back to {this} */
		TextClear(this);
		if(!TextCat(this, TextToString(temp))) { e = E_A; break; }
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
	fprintf(stderr, "parse_generics: '%s'\n", TextToString(this));

	return e ? 0 : -1;
}

/***********************************************************
 * These go in a Pattern array for calling in {TextMatch}. */

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
static void see(struct Text *const this) {
	TextTrim(this), TextTransform(this, "<a href = \"#%s\">%s</a>");
}
/** @implements	TextAction */
static void em(struct Text *const this) { TextTransform(this, "<em>%s</em>"); }
/** @implements	TextAction */
static void amp(struct Text *const this) { TextCopy(this, "&amp;"); }
/** @implements	TextAction */
static void lt(struct Text *const this) { TextCopy(this, "&lt;"); }
/** @implements	TextAction */
static void gt(struct Text *const this) { TextCopy(this, "&gt;"); }
/** @implements	TextAction */
static void paragraph(struct Text *const this){TextCopy(this, "\n</p>\n<p>\n");}

static const struct TextPattern html_escape_pat[] = {
	{ "&",       0,   &amp },
	{ "<",       0,   &lt },
	{ ">",       0,   &gt }
}, html_text_pat[] = {
	{ "\\url{",  "}", &url },
	{ "\\cite{", "}", &cite },
	{ "\\see{",  "}", &see },
	{ "{",       "}", &em }
}, html_paragraphise[] = {
	{ "\n\n",    0,   &paragraph },
	{ "\n \n",   0,   &paragraph }
};
static const size_t
	html_escape_pat_size = sizeof html_escape_pat / sizeof *html_escape_pat,
	html_text_pat_size = sizeof html_text_pat / sizeof *html_text_pat,
	html_paragraphise_size = sizeof html_paragraphise/sizeof *html_paragraphise;

/*****************************************************
 * Also in a second pattern at the root of the file. */

/** Put it into html paragraphs. */
static void paragraphise(struct Text *const this) {
	/* well, that was easy */
	TextTransform(this, "<p>\n%s\n</p>");
	TextMatch(this, html_paragraphise, html_paragraphise_size);
}

/** Matches documents, / * *   * /, and places them in the global {relates}.
 @implements	TextAction */
static void new_docs(struct Text *const this) {
	struct Relate *docs = 0;
	struct Text *signature = 0, *subsig = 0;
	enum { EF_NO, EF_DIRECT, EF_RELATES } ef = EF_NO;
	enum { ES_NO, ES_SPLIT } es = ES_NO;
	enum { TOP_LEVEL, FUNCTION } where = TOP_LEVEL;

	/* find something that looks like a function declaration after {this}? */
	do { /* try */
		struct Text *parent; /* of the doc parsing tree */
		size_t parent_end;
		struct Relate *child;
		const char *function, *function_end;
		const char *ret0, *ret1, *fn0, *fn1, *p0, *p1;

		/* {relates} is a global {Relates} pointer, the children are functions
		 supplied by this function, but it assumes it's not a function for
		 starting, and just cats to the file description */
		docs = RelatesGetRoot(root);
		/* more info on the parent for string searching for a function */
		if(!TextGetMatchInfo(&parent, 0, &parent_end))
			{ ef = EF_DIRECT; break; }
		function = TextToString(parent) + parent_end;
		/* search for function signature immediately below {this};
		 fixme: actually parse; this is sufficient for most cases, I guess */
		/* try to find the end; the minimum "A a(){" */
		if(!(function_end = strpbrk(function, ";{/#")) || *function_end != '{'
			|| function_end - function < 6) break;
		/* new {Text} for the signature */
		signature = Text(), TextBetweenCat(signature, function, function_end-1);
		TextTrim(signature);
		if(!parse_generics(signature)) break;
		/* split it into, eg, {fn_sig} = {fn_ret}{fn_name}{p0}{fn_args}{p1}
		 = "{ int * }{fn} {(}{ void }{)} other {" */
		/*printf("newdocs: %.40s\n", TextToString(signature));*/
		if(!(ret0 = TextToString(signature)) ||
			!(p0 = strchr(ret0, '(')) ||
			!(p1 = match_parenthesis(p0))) break;
		/*printf("ret0 %.200s\np0 %.30s\np1 %.30s\n", ret0, p0, p1);*/
		for(fn1 = p0 - 1; fn1 > ret0 && !isfunction(*fn1); fn1--);
		for(fn0 = fn1; fn0 > ret0 && isfunction(*fn0); fn0--);
		if(isfunction(*fn0)) break;
		ret1 = fn0++;
		/* docs should go in it's own function child instead of in the root */
		where = FUNCTION;
		if(!(docs = RelateNewChild(docs))) { ef = EF_RELATES; break; }
		/* the function name is the key of the {docs} */
		TextBetweenCat(RelateGetKey(docs), fn0, fn1);
		/* others go in sub-parts of docs; return value */
		subsig = Text();
		TextBetweenCat(subsig, ret0, ret1);
		TextTrim(subsig);
		child = RelateNewChild(docs);
		TextCat(RelateGetKey(child), "_return");
		TextCat(RelateGetValue(child), TextToString(subsig));
		/* and argument list */
		TextClear(subsig);
		TextBetweenCat(subsig, p0 + 1, p1 - 1);
		TextTrim(subsig);
		child = RelateNewChild(docs);
		TextCat(RelateGetKey(child), "_args");
		TextCat(RelateGetValue(child), TextToString(subsig));

	} while(0); switch(ef) {
		case EF_NO: break;
		case EF_DIRECT: fprintf(stderr, "new_docs: was directly called and not "
			"part of TextMatch.\n"); break;
		case EF_RELATES: fprintf(stderr, "new_docs relates: %s.\n",
			RelatesGetError(root)); break;
	} { /* finally */
		Text_(&subsig), Text_(&signature);
	}

	/* escape {this} while for "&<>" while we have it all together;
	 fixme: have different ones for each global fmt */
	TextMatch(this, html_escape_pat, html_escape_pat_size);
	/* then parse for additional tokens -- we could do this together, but text
	 inside the tokens would not be escaped; "&<>" is fairly orthogonal */
	TextMatch(this, html_text_pat, html_text_pat_size);

	/* split the doc into '@'; place the first in 'desc' and all the others in
	 their respective @<place> */
	do { /* try */
		struct Text *each, *desc = 0;
		int is_first = -1, is_last = 0, is_first_last = 0;
		do { /* split @ */
			if(!(each = TextSep(this, "@", &is_first_on_line)))
				each = this, is_last = -1;
			TextTrim(each);
			if(is_first) {
				desc = each, is_first = 0; if(is_last) is_first_last = -1;
				continue;
			}
			/* now call @-handler */
			parse_each(each, docs,
				where == TOP_LEVEL ? each_head      : each_fn,
				where == TOP_LEVEL ? each_head_size : each_fn_size);
			if(!is_last) Text_(&each); /* remember each = this on is_last */
		} while(!is_last /*&& (Text_(&each), -1) <- wtf */);
		if(es) break;
		TextTrim(desc);
		paragraphise(desc);
		TextCat(RelateGetValue(docs), TextToString(desc));
		if(!is_first_last) Text_(&desc);
	} while(0);

	switch(es) { /* catch */
		case ES_NO: break;
		case ES_SPLIT: fprintf(stderr, "new_docs split: %s.\n",
			TextGetError(this));
	}

}

static const struct TextPattern root_pat[] = {
	{ "/""** ", "*""/", &new_docs }
	/* fixme: more robust, ie \* { / * * /, 0 }?{\/} */
};
static const size_t root_pat_size = sizeof root_pat / sizeof *root_pat;



/*******
 * XML */

/** Write a bunch of XML CDATA. */
static void cdata(const char *const str) {
	const char *a = str, *b;
	printf("<![CDATA[");
	while((b = strstr(a, "]]>"))) {
		printf("%.*s]]]]><![CDATA[>", (int)(b - a), a);
		a = b + 3 /* "]]>".length */;
	}
	printf("%s]]>", a);
}

/** People use this shit?
 @implements	TextAction */
static void xml_recursive(struct Relate *const this) {
	printf("<key>"), cdata(RelateKey(this)), printf("</key>\n");
	printf("<dict>\n<key>"), cdata(RelateKey(this)), printf("</key>\n");
	printf("<string>"), cdata(RelateValue(this)), printf("</string>\n");
	RelateForEachChildIf(this, 0, &xml_recursive);
	printf("</dict>\n");
}

static void xml(struct Relate *const this) {
	printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	printf("<!DOCTYPE plist>\n");
	printf("<plist version=\"1.0\">\n");
	xml_recursive(this);
	printf("</plist>\n");
}

/********
 * Text */

/** @implements	RelateAction */
static void plain(struct Relate *const this) {
	printf("<key>%s</key>\n", RelateKey(this));
	printf("<value>%s</value>\n", RelateValue(this));
	printf("{\n");
	RelateForEachChildIf(this, 0, &plain);
	printf("}\n\n");
}

/**************************************************
 * HTML (okay, this would be easier with lambdas) */

/** Selects functions by looking for _args.
 @fixme			Have an option "--static", or you could detect .h not _fn.
 @implements	RelatePredicate */
static int select_functions(const struct Relate *const this) {
	const char *const t = RelateValue(RelateGetChild(this, "_return"));
	return t && strncmp("static", t, 6lu) /* fixme: almost */ ? -1 : 0;
}
/** A generic print <dl>.
 @implements	RelateAction */
static void print_dl(struct Relate *const this) {
	const char *const arg = RelateValue(RelateGetChild(this, "_arg"));
	const char *key = RelateKey(this);
	if(!strcmp("param", key)) {
		key = "parameter";
	} if(!strcmp("std", key)) {
		key = "minimum standard";
	}
	printf("\t<dt>%s:%s%s</dt>\n\t<dd>%s</dd>\n", key,
		arg ? " " : "", arg ? arg : "", RelateValue(this));
}
/** @implements	RelateAction */
static void print_function_table(struct Relate *const this) {
	printf("<tr>\n"
		"\t<td>%s</td>\n"
		"\t<td><a href = \"#%s\">%s</a></td>\n"
		"\t<td>%s</td>\n"
		"</tr>\n", RelateGetChildValue(this, "_return"), RelateKey(this),
		RelateKey(this), RelateGetChildValue(this, "_args"));
}
/** @implements	RelateAction */
static void print_function_detail(struct Relate *const this) {
	printf("<div><a name = \"%s\"><!-- --></a>\n"
		"<h3>%s</h3>\n"
		"<pre>%s <b>%s</b> (%s)</pre>\n"
		"%s\n"
		"<dl>\n", RelateKey(this), RelateKey(this),
		RelateGetChildValue(this, "_return"), RelateKey(this),
		RelateGetChildValue(this, "_args"), RelateValue(this));
	RelateForEachChildKey(this, "param", &print_dl);
	RelateForEachChildKey(this, "return", &print_dl);
	RelateForEachChildKey(this, "implements", &print_dl);
	RelateForEachChildKey(this, "throws", &print_dl);
	RelateForEachChildKey(this, "author", &print_dl);
	RelateForEachChildKey(this, "since", &print_dl);
	RelateForEachChildKey(this, "fixme", &print_dl);
	printf("</dl></div>\n\n");
}

/** @implements	RelateAction */
static void html(struct Relate *const this) {
	printf("<!doctype html public \"-//W3C//DTD HTML 4.01//EN\" "
		   "\"http://www.w3.org/TR/html4/strict.dtd\">\n\n");
	printf("<html>\n\n"
		"<head>\n");
	printf("<!-- steal these colour values from JavaDocs -->\n");
	printf("<style type = \"text/css\">\n"
		"\ta:link,  a:visited { color: #4a6782; }\n"
		"\ta:hover, a:focus   { color: #bb7a2a; }\n"
		"\ta:active           { color: #4A6782; }\n"
		"\ttr:nth-child(even) { background: #dee3e9; }\n"
		"\tdiv {\n"
		"\t\tmargin:  4px 0;\n"
		"\t\tpadding: 0 4px 4px 4px;\n"
		"\t}\n"
		"\ttable      { width: 100%%; }\n"
		"\ttd         { padding: 4px; }\n"
		"\th3, h1 {\n"
		"\t\tcolor: #2c4557;\n"
		"\t\tbackground-color: #dee3e9;\n"
		"\t\tpadding:          4px;\n"
		"\t}\n"
		"\th3 {\n"
		"\t\tmargin:           0 -4px;\n"
		"\t\tpadding:          4px;\n"
		"\t}\n"
		"</style>\n");
	printf("<title>%s</title>\n"
		"</head>\n\n\n"
		"<body>\n\n"
		"<h1>%s</h1>\n\n"
		"%s\n\n<dl>\n", RelateKey(this), RelateKey(this), RelateValue(this));
	RelateForEachChildKey(this, "std", &print_dl);
	RelateForEachChildKey(this, "author", &print_dl);
	RelateForEachChildKey(this, "version", &print_dl);
	RelateForEachChildKey(this, "since", &print_dl);
	RelateForEachChildKey(this, "fixme", &print_dl);
	RelateForEachChildKey(this, "param", &print_dl);
	printf("</dl>\n\n\n"
		"<h2>Function Summary</h2>\n\n"
		"<table>\n"
		"<tr><th>Return Type</th><th>Function Name</th>"
		"<th>Argument List</th></tr>\n");
	RelateForEachChildIf(this, &select_functions, &print_function_table);
	printf("</table>\n\n\n"
		"<h2>Function Detail</h2>\n\n");
	RelateForEachChildIf(this, &select_functions, &print_function_detail);
	printf("\n"
		"</body>\n"
		"</html>\n");
}



/******************
 * Main programme */

/** The is a test of Table.
 @param argc	Count
 @param argv	Vector. */
int main(int argc, char *argv[]) {
	struct Text *text = 0;
	FILE *fp = 0;
	enum { E_NO_ERR, E_ERRNO, E_TEXT, E_RELATES } error = E_NO_ERR;

	if(argc > 1) {
		int arg_err = 0;
		if(argc != 2) {
			arg_err = -1;
		} else {
			size_t f;
			for(f = 0; f < fmt_size && strcmp(fmt[f].str, argv[1]); f++);
			if(f < fmt_size) fmt_chosen = fmt + f; else arg_err = -1;
		}
		if(arg_err) {
			fprintf(stderr,"Needs a C file to be input; produces documentation."
				"\nThe default is to produce HTML, but you can specify [html|"
				"text|xml].\n\n");
			return EXIT_FAILURE;
		}
	}
	fprintf(stderr, "Format %s.\n\n", fmt_chosen->str);

	do {

		/* read file */
#ifdef DEBUG
		if(!(fp = fopen(fn, "r"))) { error = E_ERRNO; break; }
#else
		fp = stdin;
#endif
		if(!(text = Text()) || !TextFileCat(text, fp)){ error = E_TEXT; break; }
#ifdef DEBUG
		if(fclose(fp)) { error = E_ERRNO; break; }
		fp = 0;
#endif

		/* create nested associative array in global {root} */
		if(!(root = Relates())) { error = E_RELATES; break; }

		/* parse for " / * * "; it recursively calls things as appropriate */
		if(!TextMatch(text, root_pat, root_pat_size)) { error = E_TEXT; break; }

		/* print out */
		fmt_chosen->fun(RelatesGetRoot(root));

	} while(0);

	switch(error) {
		case E_NO_ERR: break;
		case E_ERRNO:
			perror(fn);
			break;
		case E_TEXT:
			fprintf(stderr, "%s: %s.\n", fn, TextGetError(text)); break;
		case E_RELATES:
			fprintf(stderr, "%s: %s.\n", fn, RelatesGetError(root)); break;
	}

	{
		Relates_(&root); /* global */
		Text_(&text);
#ifdef DEBUG
		fclose(fp);
#endif
	}

	/*fprintf(stderr, "Done all tests; %s.\n", error ? "FAILED" : "SUCCEDED");*/

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
