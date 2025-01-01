#include <stdio.h>
#include <string.h>
#include "orcish.h"

...
#	if defined QUOTE || defined QUOTE_
#		error Cannot be defined.
#	endif
#	define QUOTE_(name) #name
#	define QUOTE(name) QUOTE_(name)
#	ifdef static /* Private functions. */
#		undef static
#	endif

static const char *T_(to_string)(const struct t_(trie) *);

/** Works by side-effects, _ie_ fills the type with data. */
typedef void (*pT_(action_fn))(pT_(entry) *);

typedef void (*pT_(tree_file_fn))(struct pT_(tree) *, size_t, FILE *);

#if 0
/** Prints `tree` to `stdout`; useful in debugging. */
static void pT_(print)(const struct pT_(tree) *const tree) {
	const struct trie_branch *branch;
	unsigned b, i;
	assert(tree);
	printf("%s:\n"
		"left ", orcify(tree));
	for(b = 0; b < tree->bsize; b++) branch = tree->branch + b,
		printf("%s%u", b ? ", " : "", branch->left);
	printf("\n"
		"skip ");
	for(b = 0; b < tree->bsize; b++) branch = tree->branch + b,
		printf("%s%u", b ? ", " : "", branch->skip);
	printf("\n"
		"leaves ");
	for(i = 0; i <= tree->bsize; i++)
		printf("%s%s", i ? ", " : "", trie_bmp_test(&tree->bmp, i)
			? orcify(tree->leaf[i].as_link)
			: pT_(key_string)(pT_(entry_key)(tree->leaf[i].as_entry)));
	printf("\n");
}
#endif

/** Make sure `tree` is in a valid state, (and all the children.) */
static void pT_(valid_tree)(/*const*/ struct pT_(tree) *const tree) {
	unsigned i;
	int cmp = 0;
	const char *str1 = 0;
	assert(tree && tree->bsize <= TRIE_ORDER - 1);
	for(i = 0; i < tree->bsize; i++)
		assert(tree->branch[i].left < tree->bsize - i);
	for(i = 0; i <= tree->bsize; i++) {
		if(trie_bmp_test(&tree->bmp, i)) {
			pT_(valid_tree)(tree->leaf[i].as_link);
		} else {
			const char *str2;
			struct pT_(ref) ref;
			ref.tree = tree, ref.lf = i;
			str2 = pT_(ref_to_string)(&ref);
			if(str1) cmp = strcmp(str1, str2), assert(cmp < 0);
			str1 = str2;
		}
	}
}

/** Makes sure the `trie` is in a valid state. */
static void pT_(valid)(const struct t_(trie) *const trie) {
	if(!trie || !trie->root) return;
	pT_(valid_tree)(trie->root);
}

static pT_(key) pT_(entry_key)(const pT_(entry) *entry) {
#ifdef TRIE_ENTRY
	return t_(key)(entry);
#else
	return *entry;
#endif
}

static void pT_(test)(void) {
	struct t_(trie) trie = t_(trie)();
	size_t i, unique, count;
	unsigned letter_counts[UCHAR_MAX];
	const size_t letter_counts_size
		= sizeof letter_counts / sizeof *letter_counts;
	struct { pT_(entry) entry; int is_in; } tests[2000], *test_end, *test;
	const size_t tests_size = sizeof tests / sizeof *tests;
	pT_(remit) e;

	/* Idle. */
	errno = 0;
	pT_(valid)(0);
	pT_(valid)(&trie);
	pT_(graph)(&trie, "graph/trie/" QUOTE(TRIE_NAME) "-idle.gv", 0);
	t_(trie_)(&trie), pT_(valid)(&trie);
#if defined(TREE_ENTRY) || !defined(TRIE_KEY)
	e = T_(match)(&trie, ""), assert(!e);
	e = T_(get)(&trie, ""), assert(!e);
#else
	{
		enum trie_result r;
		r = T_(match)(&trie, "", &e), assert(r == TRIE_ABSENT);
		r = T_(get)(&trie, "", &e), assert(r == TRIE_ABSENT);
	}
#endif

	/* Make random data. */
	for(test = tests, test_end = test + tests_size; test < test_end; test++)
		t_(filler)(&test->entry), test->is_in = 0;

	/* Adding. */
	unique = 0;
	memset(letter_counts, 0, sizeof letter_counts);
	for(i = 0; i < tests_size; i++) {
		int show = !((i + 1) & i) || i + 1 == tests_size;
		pT_(key) k;
		test = tests + i;
		k = pT_(entry_key)(&test->entry);
		if(show) printf("%lu: adding %s.\n", (unsigned long)i,
#ifdef TRIE_ENTRY
			t_(string)(t_(key)(&test->entry))
#else
			t_(string)(test->entry)
#endif
			);
		switch(
#ifndef TRIE_ENTRY /* <!-- key set */
		T_(try)(&trie, k)
#else /* key set --><!-- map */
		T_(try)(&trie, k, &e)
#endif /* map --> */
		) {
		case TRIE_ERROR: perror("trie"); assert(0); return;
		case TRIE_ABSENT: test->is_in = 1; unique++;
			letter_counts[(unsigned char)*t_(string)(k)]++;
#ifdef TRIE_ENTRY
			*e = test->entry;
#endif
			break;
		case TRIE_PRESENT: /*printf("Key %s is in trie already.\n",
			pT_(key_string)(key)); spam */ break;
		}
		if(show) {
			printf("Now: %s.\n", T_(to_string)(&trie));
			pT_(graph)(&trie, "graph/trie/" QUOTE(TRIE_NAME) "-insert.gv", i);
		}
		assert(!errno);
	}
	pT_(valid)(&trie);
	/* Check keys -- there's some key that's there. */
	for(i = 0; i < tests_size; i++) {
		const char *estring, *const tstring
#ifdef TRIE_ENTRY
			= t_(string)(t_(key)(&tests[i].entry));
#else
			= t_(string)(tests[i].entry);
#endif
#if defined(TREE_ENTRY) || !defined(TRIE_KEY)
		e = T_(get)(&trie, tstring), assert(e);
#else
		{
			enum trie_result r;
			r = T_(get)(&trie, tstring, &e);
			assert(r == TRIE_PRESENT);
		}
#endif
#ifdef TRIE_ENTRY
		estring = t_(string)(t_(key)(e));
#else
		estring = t_(string)(e);
#endif
		/*printf("<%s->%s>\n", estring, tstring);*/
		assert(!strcmp(estring, tstring));
	}
	/* Add up all the letters; should be equal to the overall count. */
	for(count = 0, i = 0; i < letter_counts_size; i++) {
		char letter[2];
		unsigned count_letter = 0;
		struct T_(cursor) cur;
		int output = 0;
		letter[0] = (char)i, letter[1] = '\0';
		for(cur = T_(prefix)(&trie, letter); T_(exists)(&cur); T_(next)(&cur)) {
			/*e = T_(element)(&it); haven't made yet */
			/*printf("%s<%s>", output ? "" : letter,
				pT_(key_string)(pT_(entry_key)(e)));*/
			count_letter++, output = 1;
		}
		/*if(output) printf("\n");*/
		if(i) {
			assert(count_letter == letter_counts[i]);
			count += count_letter;
		} else { /* Sentinel; "" gets all the trie. */
			assert(count_letter == unique);
#if defined(TREE_ENTRY) || !defined(TRIE_KEY)
			if(T_(get)(&trie, "")) count++; /* Is "" included? */
#else
			if(T_(get)(&trie, "", 0) == TRIE_PRESENT) count++;
#endif
		}
	}
	printf("Counted by letter %lu elements, checksum %lu.\n",
		(unsigned long)count, (unsigned long)unique);
	assert(count == unique);
	T_(clear)(&trie);
	{
		struct T_(cursor) cur = T_(prefix)(&trie, "");
		assert(!T_(exists)(&cur));
	}
	t_(trie_)(&trie), assert(!trie.root), pT_(valid)(&trie);
	assert(!errno);
}

/* Temporary. Avoid recursion. This must match <box.h>. */
#undef BOX_MINOR
#undef BOX_MAJOR
#define BOX_END
#include "../src/box.h"
#define pTtrie_(n) BOX_CAT(private, BOX_CAT(TRIE_NAME, BOX_CAT(trie, n)))
/* Pointer array for random sampling. */
#define ARRAY_NAME pTtrie_(handle)
#define ARRAY_TYPE pTtrie_(entry) *
#include "../src/array.h"
/* Backing for the trie. */
#define POOL_NAME pTtrie_(entry)
#define POOL_TYPE pTtrie_(entry)
#include "../src/pool.h"
#undef pTtrie_
#define BOX_START
#include "../src/box.h"
#define BOX_MINOR TRIE_NAME
#define BOX_MAJOR trie

static void pT_(test_random)(void) {
	struct pT_(entry_pool) entries = pT_(entry_pool)();
	struct pT_(handle_array) handles = pT_(handle_array)();
	struct t_(trie) trie = t_(trie)();
	const size_t expectation = 1000;
	size_t i, size = 0;
	FILE *const fp = fopen("graph/trie/" QUOTE(TRIE_NAME) "-random.data", "w");
#if !defined(TREE_ENTRY) && defined(TRIE_KEY)
	enum trie_result result;
#endif
	printf("Random test; expectation value of items %lu.\n",
		(unsigned long)expectation);
	if(!fp) goto catch;
	for(i = 0; i < 5 * expectation; i++) {
		size_t j;
		if((unsigned)rand() > size * (RAND_MAX / (2 * expectation))) {
			/* Create item. */
			pT_(entry) *epool, **handle
#ifdef TRIE_ENTRY
				, *e
#endif
				;
			pT_(key) key;
			if(!(epool = pT_(entry_pool_new)(&entries))) goto catch;
			t_(filler)(epool);
			key = pT_(entry_key)(epool);
			/*printf("Creating %s: ", pT_(key_string)(key));*/
			switch(
#ifdef TRIE_ENTRY
				T_(try)(&trie, key, &e)
#else
				T_(try)(&trie, key)
#endif
			) {
			case TRIE_ERROR:
				/*printf("error.\n");*/
				goto catch;
			case TRIE_ABSENT:
				/*printf("unique.\n");*/
				size++;
#ifdef TRIE_ENTRY
				*e = *epool;
#endif
				if(!(handle = pT_(handle_array_new)(&handles))) goto catch;
				*handle = epool;
				break;
			case TRIE_PRESENT:
				/*printf("present.\n");*/
				pT_(entry_pool_remove)(&entries, epool);
				break;
			}
		} else { /* Delete item. */
			unsigned r = (unsigned)rand() / (RAND_MAX / handles.size + 1);
			pT_(entry) *handle = handles.data[r];
#ifdef TRIE_ENTRY
			const char *const string = t_(string)(t_(key)(handle));
#else
			const char *const string = t_(string)(*handle);
#endif
			int success;
			/*printf("Deleting %s.\n", string);*/
			success = T_(remove)(&trie, string), assert(success);
			pT_(handle_array_lazy_remove)(&handles, handles.data + r);
			pT_(entry_pool_remove)(&entries, handle);
			size--;
		}
		if(fp) fprintf(fp, "%lu\n", (unsigned long)size);
		if(i % (5 * expectation / 10) == 5 * expectation / 20)
			pT_(graph)(&trie, "graph/trie/" QUOTE(TRIE_NAME) "-step.gv", i);
		for(j = 0; j < handles.size; j++) {
			pT_(remit) r;
			/*pT_(entry) *e = T_(get)(&trie,
				pT_(key_string)(pT_(entry_key)(handles.data[j]))); */
#ifdef TRIE_ENTRY
			r = T_(get)(&trie, t_(string)(t_(key)(handles.data[j])));
			assert(r);
#elif !defined(TRIE_KEY)
			r = T_(get)(&trie, t_(string)(*handles.data[j]));
			assert(r);
#else
			result = T_(get)(&trie, t_(string)(*handles.data[j]), &r);
			assert(result == TRIE_PRESENT);
#endif
		}
	}
	pT_(graph)(&trie, "graph/trie/" QUOTE(TRIE_NAME) "-step.gv", i);
	/*for(i = 0; i < handles.size; i++) printf("%s\n",
		pT_(key_string)(pT_(entry_key)(handles.data[i])));*/
	goto finally;
catch:
	perror("random test");
	assert(0);
finally:
	if(fp) fclose(fp);
	t_(trie_)(&trie);
	pT_(entry_pool_)(&entries);
	pT_(handle_array_)(&handles);
}

#if 0
	/* This is old code that is a superior test; merge it, maybe? */
	for( ; i; i--) {
		int is;
		show = 1/*!(i & (i - 1))*/;
		if(show) trie_str_no++;
		if(show) printf("\"%s\" remove.\n", str_array[i - 1]);
		is = str_trie_remove(&strs, str_array[i - 1]);
		if(show) trie_str_graph(&strs, "graph/trie/str-deleted.gv");
		for(j = 0; j < sizeof str_array / sizeof *str_array; j++) {
			const char *get = str_trie_get(&strs, str_array[j]);
			const int want_to_get = j < i - 1;
			printf("Test get(%s) = %s, (%swant to get.)\n",
				str_array[j], get ? get : "<didn't find>",
				want_to_get ? "" : "DON'T ");
			assert(!(want_to_get ^ (get == str_array[j])));
		}
	}
#endif

/** Will be tested on stdout. Requires `TRIE_TEST`, and not `NDEBUG` while
 defining `assert`. @allow */
static void T_(test)(void) {
	printf("<" QUOTE(TRIE_NAME) ">trie"
#ifdef TRIE_KEY
		" custom key <" QUOTE(TRIE_KEY) ">"
#endif
#ifdef TRIE_ENTRY
		" entry <" QUOTE(TRIE_ENTRY) ">"
#endif
		" testing using <" QUOTE(TRIE_TEST) ">:\n");
	pT_(test)();
	pT_(test_random)();
	fprintf(stderr, "Done tests of <" QUOTE(TRIE_NAME) ">trie.\n\n");
}

#undef QUOTE
#undef QUOTE_
