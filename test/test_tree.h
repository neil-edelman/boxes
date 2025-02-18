#ifdef TREE_NON_STATIC
void T_(test)(void);
#endif
#ifndef TREE_DECLARE_ONLY

/*#	include "../src/orcish.h" */
#	include <stdio.h>
#	include <string.h>

#	if defined QUOTE || defined QUOTE_
#		error Cannot be defined.
#	endif
#	define QUOTE_(name) #name
#	define QUOTE(name) QUOTE_(name)
#	ifdef static /* Private functions. */
#		undef static
#	endif

/** This makes the key-value in the same place; will have to copy. */
struct pT_(tree_test) {
	int in;
	pT_(key) key;
#	ifdef TREE_VALUE
	pT_(value) value;
#	endif
};

#	ifdef TREE_VALUE
typedef void (*pT_(action_fn))(pT_(key) *, pT_(value) *);
#	else
typedef void (*pT_(action_fn))(pT_(key) *);
#	endif

/** Makes sure the `tree` is in a valid state. */
static void pT_(valid)(const struct t_(tree) *const tree) {
	if(!tree) return; /* Null. */
	if(!tree->trunk.bough)
		{ assert(!tree->trunk.height); return; } /* Idle. */
	if(!tree->trunk.height) { return; } /* Empty. */
	/*...*/
}

/** Ca'n't use `qsort` with `size` because we don't have a comparison;
 <data:<PB>compare> only has to separate it into two, not three. (One can use
 `qsort` compare in this compare, but generally not the other way around.) */
static void pT_(sort)(struct pT_(tree_test) *a, const size_t size) {
	struct pT_(tree_test) temp;
	size_t i;
	for(i = 1; i < size; i++) {
		size_t j;
		for(j = i; j && t_(less)(a[j - 1].key, a[i].key) > 0; j--);
		if(j == i) continue;
		temp = a[i];
		memmove(a + j + 1, a + j, sizeof *a * (i - j));
		a[j] = temp;
	}
}

static void pT_(test)(void) {
	struct t_(tree) tree = t_(tree)(), empty = t_(tree)();
	struct T_(cursor) cur;
	struct pT_(tree_test) test[800];
	const size_t test_size = sizeof test / sizeof *test;
#	ifdef TREE_VALUE
	pT_(value) *v;
#	endif
	pT_(key) k, k_prev;
	size_t i, n_unique = 0, n_unique2;
	char fn[64];

	errno = 0;

	/* Fill. */
	for(i = 0; i < test_size; i++) {
		struct pT_(tree_test) *const t = test + i;
		t->in = 0;
		/* This function must exist. */
#	ifdef TREE_VALUE
		t_(filler)(&t->key, &t->value);
#	else
		t_(filler)(&t->key);
#	endif
	}
	pT_(sort)(test, test_size);
	for(i = 0; i < test_size; i++) {
		struct pT_(tree_test) *const t = test + i;
		char z[12];
#	ifdef TREE_VALUE
		t_(to_string)(t->key, &t->value, &z);
#	else
		t_(to_string)(t->key, &z);
#	endif
		/*printf("%s\n", z);*/
	}

	/* Idle. */
	pT_(valid)(0);
	pT_(valid)(&tree);
	T_(graph_fn)(&tree, "graph/tree/" QUOTE(TREE_NAME) "-idle.gv");
	t_(tree_)(&tree), pT_(valid)(&tree);
	/* Not valid anymore.
	it = T_(tree_less)(0, test[0].key), assert(!it._.root); */
	/*fixme what?
	it = T_(less)(&tree, test[0].key), assert(it._.root && !it._.ref.node);*/

	/* Bulk, (simple.) */
	for(i = 0; i < test_size; i++) {
		/*char z[12];*/
		struct pT_(tree_test) *const t = test + i;
		/*
#	ifdef TREE_VALUE
		t_(to_string)(t->key, &t->value, &z);
#	else
		t_(to_string)(t->key, &z);
#	endif
		printf("%lu -- bulk adding <%s>.\n", (unsigned long)i, z);*/
		switch(
#	ifdef TREE_VALUE
		T_(bulk_assign)(&tree, t->key, &v)
#	else
		T_(bulk_add)(&tree, t->key)
#	endif
		) {
		case TREE_ERROR: perror("What?"); assert(0); break;
		case TREE_PRESENT:
			/*assert(T_(tree_get)(&tree, pT_(test_to_key)(t)));*/
			break;
		case TREE_ABSENT:
			n_unique++;
#	ifdef TREE_VALUE
			*v = t->value;
#	endif
			t->in = 1;
			break;
		}

#	if 0 /* fixme: is this even a thing anymore? `get_or`? */
#	ifdef TREE_VALUE
		/* Not a very good test. */
		value = T_(tree_get)(&tree, pT_(test_to_key)(t));
		assert(value);
#	else
		{
			pT_(key) *pk = T_(tree_get)(&tree, pT_(test_to_key)(t));
			assert(pk && !pT_(compare)(*pk, *t));
		}
#	endif
#	endif

		if(!(i & (i + 1)) || i == test_size - 1) {
			sprintf(fn, "graph/tree/" QUOTE(TREE_NAME) "-bulk-%lu.gv", i + 1);
			T_(graph_fn)(&tree, fn);
		}
	}
	printf("Finalize.\n");
	T_(bulk_finish)(&tree);
	printf("Finalize again. This should be idempotent.\n");
	T_(bulk_finish)(&tree);
	T_(graph_fn)(&tree, "graph/tree/" QUOTE(TREE_NAME) "-bulk-finish.gv");
	printf("Tree: %s.\n", T_(to_string)(&tree));

	/* Iteration; checksum. */
	memset(&k_prev, 0, sizeof k_prev);
	for(cur = T_(begin)(&tree), i = 0; T_(exists)(&cur); T_(next)(&cur)) {
		/*char z[12];*/
		k = T_(key)(&cur);
/*#	ifdef TREE_VALUE
		v = T_(tree_value)(&it);
		T_(to_string)(k, v, &z);
#	else
		T_(to_string)(k, &z);
#	endif
		printf("<%s>\n", z);*/
		if(i) { const int cmp = t_(less)(k, k_prev); assert(cmp > 0); }
		k_prev = k;
		if(++i > test_size) assert(0); /* Avoids loops. */
	}
	assert(i == n_unique);
	/*printf("\n");*/

#	if 0 /*fixme*/
	/* Going the other way. */
	for(cur = T_(end)(&tree), i = 0; T_(exists)(&cur); T_(previous)(&cur)) {
		/*char z[12];*/
		k = T_(key)(&cur);
/*#	ifdef TREE_VALUE
		v = T_(tree_value)(&it);
		T_(to_string)(k, v, &z);
#	else
		T_(to_string)(k, &z);
#	endif
		printf("<%s>\n", z);*/
		if(i) { const int cmp = t_(less)(k_prev, k); assert(cmp > 0); }
		k_prev = k;
		if(++i > test_size) assert(0); /* Avoids loops. */
	}
	assert(i == n_unique);
#	endif

#	if 0 /*fixme*/
	/* Deleting while iterating. */
	cur = T_(begin)(&tree);
	succ = T_(previous)(&cur);
	assert(succ);
	do {
		/*char z[12];*/
		pT_(key) key = T_(key)(&cur);
/*#	ifdef TREE_VALUE
		T_(to_string)(key, T_(tree_value)(&it), &z);
#	else
		T_(to_string)(key, &z);
#	endif
		printf("removing <%s>\n", z);*/
		succ = T_(remove)(&tree, key);
		assert(succ);
		succ = T_(remove)(&tree, key);
		assert(!succ);
		it = T_(tree_less)(&tree, key);
	} while(T_(tree_has_element)(&cur));
	printf("Individual delete tree: %s.\n", T_(tree_to_string)(&tree));
	assert(!tree.root.height && tree.root.node);
#	endif

	/* Clear. */
	T_(clear)(0);
	T_(clear)(&empty), assert(!empty.trunk.bough);
	T_(clear)(&tree), assert(tree.trunk.bough && !tree.trunk.height);
	n_unique = 0;

	/* Fill again, this time, don't sort. */
	for(i = 0; i < test_size; i++) {
		struct pT_(tree_test) *const t = test + i;
		t->in = 0;
		/* This function must exist. */
#	ifdef TREE_VALUE
		t_(filler)(&t->key, &t->value);
#	else
		t_(filler)(&t->key);
#	endif
	}

	/* Add. */
	for(i = 0; i < test_size; i++) {
		/*char z[12];*/
		struct pT_(tree_test) *const t = test + i;
/*#	ifdef TREE_VALUE
		t_(to_string)(t->key, &t->value, &z);
#	else
		t_(to_string)(t->key, &z);
#	endif
		printf("%lu -- adding <%s>.\n", (unsigned long)i, z);*/
		switch(
#	ifdef TREE_VALUE
		T_(assign)(&tree, t->key, &v)
#	else
		T_(add)(&tree, t->key)
#	endif
		) {
		case TREE_ERROR: perror("unexpected"); assert(0); return;
		case TREE_PRESENT: /*printf("<%s> already in tree\n", z);*/ break;
		case TREE_ABSENT:
			n_unique++;
#	ifdef TREE_VALUE
			*v = t->value;
#	endif
			t->in = 1;
			/*printf("<%s> added\n", z);*/ break;
		}
		if(!(i & (i + 1)) || i == test_size - 1) {
			sprintf(fn, "graph/tree/" QUOTE(TREE_NAME) "-add-%lu.gv", i + 1);
			T_(graph_fn)(&tree, fn);
		}
	}
	printf("Number of entries in the tree: %lu/%lu.\n",
		(unsigned long)n_unique, (unsigned long)test_size);

	/* Delete all. Removal invalidates iterator. */
	for(cur = T_(begin)(&tree), i = 0; T_(exists)(&cur); ) {
		/*char z[12];*/
		int succ;
		k = T_(key)(&cur);
/*#	ifdef TREE_VALUE
		v = T_(value)(&cur);
		t_(to_string)(k, v, &z);
#	else
		t_(to_string)(k, &z);
#	endif
		printf("Targeting <%s> for removal.\n", z);*/
		if(i) { const int cmp = t_(less)(k, k_prev); assert(cmp > 0); }
		k_prev = k;
		if(++i > test_size) assert(0); /* Avoids loops. */
		assert(T_(contains)(&tree, k));
		succ = T_(remove)(&tree, k);
		/*T_(graph_fn)(&tree, "graph/tree/" QUOTE(TREE_NAME) "-a-after.gv");*/
		assert(succ);
		assert(!T_(contains)(&tree, k));
		cur = T_(more)(&tree, k);
		/*printf("Iterator now %s:h%u:i%u.\n",
			orcify(cur.ref.bough), cur.ref.height, cur.ref.idx);*/
		if(!(i & (i + 1)) || i == test_size - 1) {
			sprintf(fn, "graph/tree/" QUOTE(TREE_NAME) "-rm-%lu.gv", i);
			T_(graph_fn)(&tree, fn);
		}
	}
	assert(i == n_unique);

	/* Add all back. */
	n_unique2 = 0;
	for(i = 0; i < test_size; i++) {
		struct pT_(tree_test) *const t = test + i;
		/*char z[12];*/
		if(!t->in) continue;
/*#	ifdef TREE_VALUE
		T_(to_string)(t->key, &t->value, &z);
#	else
		T_(to_string)(t->key, &z);
#	endif
		printf("Adding %s.\n", z);*/
		switch(
#	ifdef TREE_VALUE
		T_(assign)(&tree, t->key, &v)
#	else
		T_(add)(&tree, t->key)
#	endif
		) {
		case TREE_ERROR:
		case TREE_PRESENT: perror("unexpected"); assert(0); return;
		case TREE_ABSENT: n_unique2++; break;
		}
#	ifdef TREE_VALUE
		*v = t->value;
#	endif
	}
	printf("Re-add tree: %lu\n", (unsigned long)n_unique2);
	T_(graph_fn)(&tree, "graph/tree/" QUOTE(TREE_NAME) "-re-add.gv");
	assert(n_unique == n_unique2);
	i = T_(count)(&tree);
	printf("tree count: %lu; add count: %lu\n",
		(unsigned long)i, (unsigned long)n_unique2);
	assert(i == n_unique);

	/* Remove every 2nd. */
	for(cur = T_(begin)(&tree); T_(exists)(&cur); T_(next)(&cur)) {
		pT_(key) key = T_(key)(&cur);
		const int ret = T_(remove)(&tree, key);
		assert(ret);
		n_unique--;
		cur = T_(more)(&tree, key); /* Move past the erased keys. */
		if(!T_(exists)(&cur)) break;
	}
	i = T_(count)(&tree);
	printf("remove every 2nd: %lu\n", (unsigned long)i);
	assert(i == n_unique);

	printf("clear, destroy\n");
	T_(clear)(&tree);
	assert(!tree.trunk.height && tree.trunk.bough);

	/* Destroy. */
	t_(tree_)(&tree), assert(!tree.trunk.bough), pT_(valid)(&tree);
	assert(!errno);
}

#	define BOX_PUBLIC_OVERRIDE
#	include "../src/box.h"

/** Will be tested on stdout. Requires `TREE_TEST`, and not `NDEBUG` while
 defining `assert`. @allow */
static void T_(test)(void) {
	printf("<" QUOTE(TREE_NAME) ">tree"
		" of key <" QUOTE(TREE_KEY) ">;"
#ifdef TREE_VALUE
		" value <" QUOTE(TREE_VALUE) ">;"
#endif
		" testing:\n");
	pT_(test)();
	fprintf(stderr, "Done tests of <" QUOTE(TREE_NAME) ">tree.\n\n");
	/*(void)pT_(graph_horiz);*/ /* Not used in general. */
}

#	define BOX_PRIVATE_AGAIN
#	include "../src/box.h"

#	undef QUOTE
#	undef QUOTE_
#endif
