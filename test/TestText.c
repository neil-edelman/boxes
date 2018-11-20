/** 2018 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 @title TestText
 @author Neil
 @std C89/90
 @version 2018-10
 @since
 2018-01
 2018-06 */

#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf fopen FILE */
#include <string.h>	/* strerror */
#include <limits.h>	/* INT_MAX, UINT_MAX */
#include <assert.h>	/* assert */
#include <errno.h>	/* errno */
#include <time.h>   /* timing */
#include "../src/Text.h"
#include "split.h"

/* This is not implemented by the C90 standard, but it should have been. In
 case we are using to C90, define this and hope that the compiler is in this
 millenium. It's just a test. */
#if 0
int snprintf(char *s, size_t n, const char *format, /* args */ ...);
#endif

#define WIDTH 78

static const char *const head = "abstract.txt",
	*const body = "lorem.txt";
	/**const para = "para.txt",
	*const longpara = "long.txt";*/

/** Helper for {fclose} is a little more robust about null-values.
 @param pfp: A pointer to the file pointer.
 @return Success.
 @throws {fclose} errors. */
static int pfclose(FILE **const pfp) {
	FILE *fp;
	int is;
	if(!pfp || !(fp = *pfp)) return 1;
	/* Whatever {fclose} returns, the file pointer is now useless. "After the
	 call to fclose(), any use of stream results in undefined behavior." */
	is = (fclose(fp) != EOF);
	*pfp = fp = 0;
	return is;
}

/** Splits all the words on in {src} on different lines on before the cursor in
 {dst}.
 @param pwords: If not null, the number of words split is stored.
 @return Success.
 @throws {realloc} errors. */
static int split(const struct Line *const src, struct Text *const dst,
	size_t *const pwords) {
	const char *cursor, *start, *end;
	size_t words = 0;
	assert(dst && src && pwords);
	for(cursor = LineGet(src); start = trim(cursor), end = next(start);
		cursor = end) {
		assert(start < end);
		if(!LineBetweenCat(LineCopyMeta(src, dst), start, end)) break;
		words++;
	}
	*pwords = words;
	return !end;
}

/** Splits the entire paragraph starting with cusor of {src} into strings
 before the cursor of {dst}. The cursor is updated in {src} to the line after
 or reset if there was no line after. A paragraph is delimited by lines
 composed of only classic white-space; this skips over all the delimiters,
 if the file has words, it outputs them to {dst}, and stops with a delimiter or
 end-of-text, and returns true, otherwise the cursor is reset and it will
 return false.
 @return Whether a paragraph was output.
 @throws realloc */
static int split_para(struct Text *const src, struct Text *const dst,
	size_t *const pwords) {
	const struct Line *line;
	int is_para = 0;
	size_t words = 0, total_words = 0;
	assert(src && dst && pwords);
	*pwords = 0;
	while((line = TextNext(src))) {
		if(!split(line, dst, &words)) return 0;
		if(words) is_para = 1; else if(is_para) break;
		total_words += words;
	}
	*pwords = total_words;
	return is_para;
}

/** \url{ http://xxyxyz.org/line-breaking/ }. */

/** Most things need more space. */
struct Work {
	size_t offset, breaks;
	unsigned minimum;
};

#define POOL_NAME Work
#define POOL_TYPE struct Work
#define POOL_STACK
#include "../src/Pool.h"

static struct WorkPool work;

struct Search {
	size_t i0, j0, i1, j1;
};

#define POOL_NAME Search
#define POOL_TYPE struct Search
#define POOL_STACK
#include "../src/Pool.h"

static struct SearchPool search;

/** Wraps the line at 80.
 @param words: Any words after the cursor is erased.
 @return Success.
 @throws {realloc} errors. */
static int greedy(struct Text *const words, struct Text *const wrap) {
	const struct Line *word;
	struct Line *line = 0;
	const char *const space = " ", *const space_end = space + 1;
	const char *str;
	size_t line_len = 0;
	assert(words && wrap);
	while((word = TextNext(words))) {
		if((!line || ((line_len = LineLength(line))
			&& line_len + 1 + LineLength(word) > WIDTH))
			&& !(line_len = 0, line = LineCopyMeta(word, wrap))) return 0;
		if(line_len && !LineBetweenCat(line, space, space_end)) return 0;
		if(!(str = LineGet(word))
			|| !LineBetweenCat(line, str, str + LineLength(word))) return 0;
		TextRemove(words);
	}
	(void)work;
	return 1;
}

/*******/

static int add_words(const struct Line *const word) {
	const struct Work *const last_w = WorkPoolPeek(&work);
	const size_t offset = last_w ? last_w->offset : 0;
	struct Work *const w = WorkPoolNew(&work);
	w->offset = offset + LineLength(word);
	w->minimum = 1000000000l/*UINT_MAX*/;
	w->breaks = 0;
	return 1;
}

/*static void print_work(void) {
	struct Work *w;
	size_t i;
	printf("Line\tOffset\tBreak\tMinima\n");
	for(i = 0, w = 0; (w = WorkPoolNext(&work, w)); i++)
		printf("%lu\t%lu\t%lu\t%u\n", i, w->offset, w->breaks, w->minimum);
}*/

static unsigned cost(const size_t i, const size_t j) {
	const struct Work *i_work = WorkPoolGet(&work, i),
		*j_work = WorkPoolGet(&work, j);
	const unsigned w = (unsigned)(j_work->offset - i_work->offset + j - i - 1);
	assert(i_work && j_work);
	return (w > WIDTH)
		? 100000 * (w - WIDTH) : i_work->minimum + (WIDTH - w) * (WIDTH - w);
}

/** Called after getting {work} filled. */
static int words_work_to_wrap(struct Text *const words,
	struct Text *const wrap) {
	const char *const space = " ", *const space_end = space + 1;
	const struct Line *word;
	size_t lines_in_para = 0, delta, i, j;
	struct Line *line;
	struct Work *j_work = 0;
	const char *str;
	assert(words && TextHasContent(words) && wrap && WorkPoolSize(&work));
	/*print_work();*/
	/* Read off the work. */
	TextCursorSet(words, 0);
	j = WorkPoolSize(&work) - 1;
	while(j > 0) {
		j_work = WorkPoolGet(&work, j), assert(j_work);
		i = j_work->breaks;
		/* Backtrack. The underlying linked-list is not indexable. */
		assert(i < j);
		for(delta = j - i; delta; delta--)
			if(!(word = TextPrevious(words))) return errno = ERANGE, 0;
		/* Consume all the of the {words} on the same line on the end. */
		if(!(line = LineCopyMeta(word, wrap))) return 0;
		TextPrevious(wrap);
		lines_in_para++;
		delta = j - i;
		do {
			str = LineGet(word), assert(str);
			if(!LineBetweenCat(line, str, str + LineLength(word))) return 0;
			TextRemove(words);
			word = TextNext(words);
			if(!--delta) break;
			if(!LineBetweenCat(line, space, space_end)) return 0;
		} while(1);
		j = i;
	}
	/* This has the effect of reversing order. */
	while(lines_in_para) lines_in_para--, TextNext(wrap);
	assert(!TextHasContent(words));
	return 1;
}

static int dynamic(struct Text *const words, struct Text *const wrap) {
	struct Line *word, *word2;
	struct Work *i_work, *j1_work;
	const size_t count = TextLineCount(words);
	size_t c;
	int *slack = malloc(sizeof *slack * count * count), s;
	size_t i, j;
	int done = 0;
	assert(words && wrap);
	do {
		/* Create the table. */
		if(!slack) break;
		memset(slack, 0, sizeof *slack * count * count); /* Actually need? */
		for(i = 0, TextCursorSet(words, 0); (word = TextNext(words)); i++) {
			assert(i < count);
			slack[i * count + i] = WIDTH - (int)LineLength(word);
			for(j = i + 1; j < count; j++) {
				word2 = TextNext(words), assert(word2);
				slack[i * count + j] = slack[i * count + j - 1]
					- (int)LineLength(word2) - 1;
			}
			TextCursorSet(words, word);
		}
		/* Create scratch space. */
		if(!(i_work = (WorkPoolClear(&work), WorkPoolNew(&work)))) break;
		/*i_work->offset = <--This doesn't use offset.*/
		i_work->breaks = 0, i_work->minimum = 0;
		if(TextAll(words, &add_words)) break;
		/* Optimise. */
		for(j = 0; j < count; j++) {
			i = j;
			do {
				i_work = WorkPoolGet(&work, i), assert(i_work);
				s = slack[i * count + j];
				c = s < 0 ? INT_MAX : i_work->minimum + s * s;
				j1_work = WorkPoolGet(&work, j + 1), assert(j1_work);
				if(j1_work->minimum > c)
					j1_work->minimum = (unsigned)c, j1_work->breaks = i;
			} while(i--);
		}
		done = 1;
	} while(0); if(!done) perror("error");
	free(slack), slack = 0;
	return done && words_work_to_wrap(words, wrap);
}

static void index_to_string(const size_t *n, char (*const a)[12]) {
	snprintf(*a, sizeof *a, "%lu", *n);
}

#define POOL_NAME Index
#define POOL_TYPE size_t
#define POOL_TO_STRING &index_to_string
#define POOL_STACK
#include "../src/Pool.h"

#define POOL_NAME IndexPool
#define POOL_TYPE struct IndexPool
#define POOL_STACK
#include "../src/Pool.h"

struct Smawk {
	struct IndexPoolPool rows_stack, cols_stack;
	size_t no_rows_stack, no_cols_stack;
} smawk; /* Already zeroed. This means no concurrency. */

/** Clears all memory that's been reserved. */
static void Smawk_(void) {
	struct IndexPool *rows, *cols;
	while((cols = IndexPoolPoolPop(&smawk.cols_stack))) IndexPool_(cols);
	IndexPoolPool_(&smawk.cols_stack);
	while((rows = IndexPoolPoolPop(&smawk.rows_stack))) IndexPool_(rows);
	IndexPoolPool_(&smawk.rows_stack);
	smawk.no_rows_stack = smawk.no_cols_stack = 0;
}

/** Restarts Smawk. */
static void SmawkRestart(void) {
	smawk.no_rows_stack = smawk.no_cols_stack = 0;
}

/** Every pointer to any rows is invalidated.
 @return A new empty rows stack.
 @throws Memory. */
static struct IndexPool *SmawkRows(void) {
	struct IndexPool *rows;
	assert(smawk.no_rows_stack <= IndexPoolPoolSize(&smawk.rows_stack));
	if(smawk.no_rows_stack < IndexPoolPoolSize(&smawk.rows_stack)) {
		rows = IndexPoolPoolGet(&smawk.rows_stack, smawk.no_rows_stack);
		assert(rows);
		IndexPoolClear(rows);
	} else {
		if(!(rows = IndexPoolPoolNew(&smawk.rows_stack))) return 0;
		IndexPool(rows);
	}
	smawk.no_rows_stack++;
	return rows;
}

/** Pops from the stack; doesn't have any checks. */
static void SmawkRows_(void) {
	assert(smawk.no_rows_stack);
	smawk.no_rows_stack--;
}

static struct IndexPool *SmawkPeekRows(void) {
	assert(smawk.no_rows_stack <= IndexPoolPoolSize(&smawk.rows_stack));
	if(!smawk.no_rows_stack) return 0;
	return IndexPoolPoolGet(&smawk.rows_stack, smawk.no_rows_stack - 1);
}

static struct IndexPool *SmawkPeekRows2(void) {
	assert(smawk.no_rows_stack <= IndexPoolPoolSize(&smawk.rows_stack));
	if(smawk.no_rows_stack < 2) return 0;
	return IndexPoolPoolGet(&smawk.rows_stack, smawk.no_rows_stack - 2);
}

/** Every pointer to any columns is invalidated.
 @return A new empty columns stack.
 @throws Memory. */
static struct IndexPool *SmawkColumns(void) {
	struct IndexPool *cols;
	assert(smawk.no_cols_stack <= IndexPoolPoolSize(&smawk.cols_stack));
	if(smawk.no_cols_stack < IndexPoolPoolSize(&smawk.cols_stack)) {
		cols = IndexPoolPoolGet(&smawk.cols_stack, smawk.no_cols_stack);
		assert(cols);
		IndexPoolClear(cols);
	} else {
		if(!(cols = IndexPoolPoolNew(&smawk.cols_stack))) return 0;
		IndexPool(cols);
	}
	smawk.no_cols_stack++;
	return cols;
}

static void SmawkColumns_(void) {
	assert(smawk.no_cols_stack);
	smawk.no_cols_stack--;
}

static struct IndexPool *SmawkPeekColumns(void) {
	assert(smawk.no_cols_stack <= IndexPoolPoolSize(&smawk.cols_stack));
	if(!smawk.no_cols_stack) return 0;
	return IndexPoolPoolGet(&smawk.cols_stack, smawk.no_cols_stack - 1);
}

static struct IndexPool *SmawkPeekColumns2(void) {
	assert(smawk.no_cols_stack <= IndexPoolPoolSize(&smawk.cols_stack));
	if(smawk.no_cols_stack < 2) return 0;
	return IndexPoolPoolGet(&smawk.cols_stack, smawk.no_cols_stack - 2);
}

static int Smawk(void) {
	size_t i, *n, *c, *s, *r, *col, *row;
	size_t j, end;
	size_t rows_size, rows2_size, cols_size;
	/* Order matters! */
	struct IndexPool *rows2 = SmawkRows() /* stack = [] */,
		*rows = SmawkPeekRows2();
	const struct IndexPool *cols = SmawkPeekColumns();
	unsigned cst;
	struct Work *w;
	if(!rows || !rows2 || !cols) return 0;
	i = 0;
	rows_size = IndexPoolSize(rows);
	while(i < rows_size) { /* while i < len(rows) */
		r = IndexPoolGet(rows, i), assert(r);
		if((rows2_size = IndexPoolSize(rows2))) { /* if stack */
			unsigned cost_sc, cost_rc;
			c = IndexPoolGet(cols,  rows2_size - 1), assert(c);
			s = IndexPoolGet(rows2, rows2_size - 1), assert(s);
			cost_sc = cost(*s, *c);
			cost_rc = cost(*r, *c);
			/*printf("work %u\n", w);*/
			if(cost_sc/*(*s, *c)*/ < cost_rc/*(*r, *c)*/) {
				if(rows2_size < IndexPoolSize(cols)) {
					if(!(n = IndexPoolNew(rows2))) return 0;
					*n = *r;
				}
				i++;
			} else {
				IndexPoolPop(rows2);
			}
		} else {
			if(!(n = IndexPoolNew(rows2))) return 0;
			*n = *r;
			i++;
		}
	}

	cols_size = IndexPoolSize(cols);
	if(cols_size > 1) {
		struct IndexPool *const cols2 = SmawkColumns();
		size_t *item;
		if(!cols2) return 0;
		/* Now we have to update {cols} because it is in the dynamic buffer. */
		cols = SmawkPeekColumns2(), assert(cols);
		/* Populate columns for recursion. */
		for(i = 1; i < cols_size; i += 2) {
			if(!(item = IndexPoolNew(cols2))) return 0;
			*item = *IndexPoolGet(cols, i);
		}
		Smawk();
		/* Update pointers -- has possibly changed. */
		rows2 = SmawkPeekRows(), rows = SmawkPeekRows2(), assert(rows2 && rows);
	}

	i = j = 0;
	while(j < cols_size) {
		if(j + 1 < cols_size) {
			size_t *elt = IndexPoolGet(cols, j + 1);
			assert(elt);
			w = WorkPoolGet(&work, *elt);
			end = w->breaks;
		} else {
			/* This is now {rows_size}; it has been updated. */
			size_t *item = IndexPoolGet(rows2, IndexPoolSize(rows2) - 1);
			assert(item);
			end = *item;
		}
		col = IndexPoolGet(cols, j), assert(col);
		row = IndexPoolGet(rows2, i), assert(row);
		cst = cost(*row, *col);
		w = WorkPoolGet(&work, *col), assert(w);
		if(cst < w->minimum) {
			w->minimum = (unsigned)cst;
			w->breaks = *row;
		}
		if(*row < end) i += 1;
		else j += 2;
	}
	SmawkColumns_();
	SmawkRows_();
	return 1;
}

static int linear(struct Text *const words, struct Text *const wrap) {
	struct Work *w;
	size_t n, i = 0, offset = 0, r, edge, j, y;
	unsigned x;
	assert(words && wrap);
	/* Set up work. */
	if(!(w = (WorkPoolClear(&work), WorkPoolNew(&work)))) return 0;
	w->offset = w->breaks = 0, w->minimum = 0;
	if(TextAll(words, &add_words)) return 0;
	n = WorkPoolSize(&work); /* n = count + 1 */
	SmawkRestart();
	for( ; ; ) {
		r = 1 << (i + 1); if(r > n) r = n;
		edge = (1 << i) + offset;
		{ /* Put in the {rows}, {cols}. */
			const size_t end = r + offset;
			size_t range, *item;
			struct IndexPool *const rows = SmawkRows(),
				*const cols = SmawkColumns();
			if(!rows || !cols) return 0;
			for(range = offset; range < edge; range++) {
				if(!(item = IndexPoolNew(rows))) return 0;
				*item = range;
			}
			for(range = edge; range < end; range++) {
				if(!(item = IndexPoolNew(cols))) return 0;
				*item = range;
			}
		}
		if(!Smawk()) return smawk.no_rows_stack = smawk.no_cols_stack = 0, 0;
		w = WorkPoolGet(&work, r - 1 + offset), assert(w);
		x = w->minimum;
		for(j = 1 << i; j < r - 1; j++) {
			y = cost(j + offset, r - 1 + offset);
			if(x < y) continue;
			n -= j;
			i = 0;
			offset += j;
			break;
		}
		if(j == r - 1) {
			if(r == n) break;
			i++;
		}
	}
	return words_work_to_wrap(words, wrap);
}

static int divide_search(const size_t i0, const size_t j0,
	const size_t i1, const size_t j1) {
	size_t j, i;
	unsigned c;
	struct Work *j_work;
	struct Search s_copy, *s, *t;
	if(!(s = SearchPoolNew(&search))) return 0;
	s->i0 = i0, s->j0 = j0, s->i1 = i1, s->j1 = j1;/* stack = [(i0,j0,i1,j1)] */
	while((s = SearchPoolPop(&search))) {
		assert(s);
		if(s->j0 >= s->j1) continue;
		j = (s->j0 + s->j1) >> 1;
		j_work = WorkPoolGet(&work, j), assert(j_work);
		for(i = s->i0; i < s->i1; i++) {
			c = cost(i, j);
			if(c > j_work->minimum) continue;
			j_work->minimum = c;
			j_work->breaks = i;
		}
		memcpy(&s_copy, s, sizeof s_copy); /* The PoolNew invalidates. */
		if(!(t = SearchPoolNew(&search))) return 0;
		t->i0 = j_work->breaks, t->j0 = j + 1,
			t->i1 = s_copy.i1, t->j1 = s_copy.j1;
		if(!(t = SearchPoolNew(&search))) return 0;
		t->i0 = s_copy.i0, t->j0 = s_copy.j0,
			t->i1 = j_work->breaks + 1, t->j1 = j;
	}
	return 1;
}

static int divide(struct Text *const words, struct Text *const wrap) {
	struct Work *x_work;
	size_t n, i = 0, offset = 0, r, edge, x, j, y;
	assert(words && wrap);
	if(!(x_work = (WorkPoolClear(&work), WorkPoolNew(&work)))) return 0;
	x_work->offset = x_work->breaks = 0, x_work->minimum = 0;
	if(TextAll(words, &add_words)) return 0;
	n = WorkPoolSize(&work)/* + 1 it's already +1 with the [0] */;
	for( ; ; ) {
		r = 1 << (i + 1); if(r > n) r = n;
		edge = (1 << i) + offset;
		if(!divide_search(0 + offset, edge, edge, r + offset)) return 0;
		x_work = WorkPoolGet(&work, r - 1 + offset);
		assert(x_work);
		x = x_work->minimum;
		for(j = 1 << i; j < r - 1; j++) {
			y = cost(j + offset, r - 1 + offset);
			if(x < y) continue;
			n -= j;
			i = 0;
			offset += j;
			break;
		}
		if(j == r - 1) {
			if(r == n) break;
			i++;
		}
	}
	return words_work_to_wrap(words, wrap);
}

typedef int (*BiTextConsumer)(struct Text *const, struct Text *const);

static const struct {
	const char *name;
	const BiTextConsumer go;
} algorthms[] = {
	{ "Greedy", &greedy },
	{ "Dynamic", &dynamic },
	{ "Divide and Conquer", &divide },
	{ "Smawk", &linear }
}, *algorthm = algorthms + 3;

/** Expects {head} and {body} to be on the same directory as it is called from.
 Word wraps.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	FILE *fp = 0;
	struct Text *text = 0, *words = 0, *wrap = 0;
	const struct Line *newline = 0;
	const char *e = 0;
	do { /* Try. */
		unsigned long time_clock = 0;
		size_t word_no;
		if(!(text = Text()) || !(words = Text()) || !(wrap = Text()))
			{ e = "Text"; break; }
		/* Load all. In reality, would read from stdin, just testing. */
#if 1
		if(!(fp = fopen(head, "r"))
			|| !TextFile(text, fp, head)
			|| !pfclose(&fp)) { e = head; break; }
		if(!TextNew(text)) { e = "edit"; break; }
		if(!(fp = fopen(body, "r"))
			|| !TextFile(text, fp, body)
			|| !pfclose(&fp)) { e = body; break; }
		fprintf(stderr, "Loaded files <%s> and <%s>.\n", head, body);
#else
		/*if(!(fp = fopen(para, "r"))
		   || !TextFile(text, fp, para)
		   || !pfclose(&fp)) { e = para; break; }
		fprintf(stderr, "Loaded file <%s>.\n", para);*/
		if(!(fp = fopen(longpara, "r"))
		   || !TextFile(text, fp, longpara)
		   || !pfclose(&fp)) { e = longpara; break; }
		fprintf(stderr, "Loaded file <%s>.\n", longpara);
#endif
		/* Split the text into words and then wraps them. */
		do {
			/* Insert a double-break between paragraphs. */
			if(newline) LineCopyMeta(newline, wrap);
			/* Splits the paragraph into words.
			 If false, newlines at EOF or error (fixme: handle error.) */
			if(!split_para(text, words, &word_no)) break;
			/* Apply word-wrapping; the words are consumed. */
			time_clock -= clock();
			if(!algorthm->go(words, wrap)) { e = "wrap"; break; };
			time_clock += clock();
		} while((newline = TextCursor(text)));
		if(e) break;
		if(!TextPrint(wrap, stdout, "%a: <%s>\n")) { e = "stdout"; break; }
		fprintf(stderr, "This was wrapped using %s in %fms.\n", algorthm->name,
			time_clock * 1000.0 / CLOCKS_PER_SEC);
	} while(0); if(e) perror(e); /* Catch. */
	if(!pfclose(&fp)) perror("shutdown"); /* Finally. */
	Text_(&wrap), Text_(&words), Text_(&text);
	/* Clear any temp values. */
	WorkPool_(&work), SearchPool_(&search);
	Smawk_();
	return e ? EXIT_FAILURE : EXIT_SUCCESS;
}
