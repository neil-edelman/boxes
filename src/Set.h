/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 `<K>Set` is a collection of objects with a hash function and equality function
 that doesn't allow duplication. Collisions are handled by separate chaining.
 The maximum load factor is `ln 2`. While in the set, the values cannot change
 in way that affects their hash.

 @param[SET_NAME, SET_TYPE]
 `K` that satisfies `C` naming conventions when mangled; required.

 @param[SET_HASH]
 A function satisfying <typedef:<PK>Hash>; required.

 @param[SET_IS_EQUAL]
 A function satisfying <typedef:<PK>IsEqual>; required.

 @param[SET_TO_STRING]
 Optional print function implementing <typedef:<PK>ToString>; makes available
 <fn:<K>SetToString>.

 @param[SET_TEST]
 Unit testing framework, included in a separate header, <../test/SetTest.h>.
 Must be defined equal to a random filler function, satisfying
 <typedef:<PV>Action>. Requires `SET_TO_STRING`.

 @fixme Implement tests.
 @std C89/90 */

#include <stddef.h>	/* offsetof */
#include <stdlib.h> /* realloc free */
#include <assert.h>	/* assert */
#include <stdio.h>  /* perror fprintf */
#include <errno.h>  /* errno */
#ifdef SET_TO_STRING /* <-- string */
#include <string.h> /* strlen */
#endif /* string --> */



/* Check defines. */
#ifndef SET_NAME
#error Generic SET_NAME undefined.
#endif
#ifndef SET_TYPE
#error Type SET_TYPE undefined.
#endif
#ifndef SET_IS_EQUAL
#error Function SET_IS_EQUAL undefined.
#endif
#ifndef SET_HASH
#error Function SET_HASH undefined.
#endif
#if defined(SET_TEST) && !defined(SET_TO_STRING)
#error SET_TEST requires SET_TO_STRING.
#endif



/* Generics using the preprocessor;
 <http://stackoverflow.com/questions/16522341/pseudo-generics-in-c>. */
#ifdef CAT
#undef CAT
#endif
#ifdef CAT_
#undef CAT_
#endif
#ifdef PCAT
#undef PCAT
#endif
#ifdef PCAT_
#undef PCAT_
#endif
#ifdef K
#undef K
#endif
#ifdef K_
#undef K_
#endif
#ifdef PK_
#undef PK_
#endif
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#define K_(thing) CAT(SET_NAME, thing)
#define PK_(thing) PCAT(set, PCAT(SET_NAME, thing)) /* "Private." */

/* Troubles? Check `SET_TYPE` is a valid type, whose definition is placed above
 inclusion. */
typedef SET_TYPE PK_(Type);
#define K PK_(Type)



/** Contains `K` and more internal to the working of the hash. Storage of the
 `<K>SetItem` structure is the responsibility of the caller; it could be one
 part of a complicated structure. */
struct K_(SetItem);
struct K_(SetItem) {
	K data;
	unsigned hash;
	struct K_(SetItem) *next;
};

#define SET_ZERO { 0, 0, 0 }

/** A `<K>Set`. */
struct K_(Set);
struct K_(Set) {
	struct K_(SetItem) *buckets; /* An array of 1 << log_capacity (>3) or 0. */
	unsigned log_capacity;
	size_t size;
};



/** A map from `K` onto `unsigned int`. Should be as close as possible to a
 discrete uniform distribution for maximum performance and, when computing,
 take all of `K` into account. */
typedef unsigned (*PK_(Hash))(const K);
/* Check that `SET_HASH` is a function implementing <typedef:<PK>Hash>. */
static const PK_(Hash) PK_(hash) = (SET_HASH);

/** A constant equivalence relation between `K` that satisfies
 `<PK>IsEqual(a, b) -> <PK>Hash(a) == <PK>Hash(b)`. */
typedef int (*PK_(IsEqual))(const K, const K);
/* Check that `SET_IS_EQUAL` is a function implementing
 <typedef:<PK>IsEqual>. */
static const PK_(IsEqual) PK_(is_equal) = (SET_IS_EQUAL);

#ifdef SET_TO_STRING /* <-- string */
/** Responsible for turning `K` (the first argument) into a 12 `char` string
 (the second.) */
typedef void (*PK_(ToString))(const K *const, char (*const)[12]);
/* Check that {SET_TO_STRING} is a function implementing {<E>ToString}. */
static const PK_(ToString) PK_(to_string) = (SET_TO_STRING);
#endif /* string --> */

#ifdef SET_TEST /* <-- test */
/** Used for `SET_TEST`. */
typedef void (*PK_(Action))(const K *const);
#endif /* test --> */



/** Extracts the `K` from `item`. */
static const K *K_(SetItem)(const struct K_(SetItem) *const item) {
	if(!item) return 0;
	return &item->data;
}

static K *K_(SetVariableItem)(struct K_(SetItem) *const item) {
	if(!item) return 0;
	return &item->data;
}

/** @return Given a `hash`, compute the bucket at it's index. May be empty. */
static struct K_(SetItem) *PK_(get_bucket)(struct K_(Set) *const set,
	const unsigned hash) {
	assert(set);
	return set->buckets + (hash & ((1 << set->log_capacity) - 1));
}

/** Private: grow the table until the capacity is at least
 `size / load factor = ln 2 = 0.69`, ranged from `[8, UINT]` entries.
 @param[size] How many entries are there going to be; `size > 1`.
 @return Success; otherwise, `errno` may be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t`.
 @throws[malloc]
 @order \O(1) amortized. */
static int PK_(grow)(struct K_(Set) *const set, const size_t size) {
	/* Size if we want each bucket to have one item. */
	const size_t eff_size = 1 + size / 0.693147180559945309417232121458176568;
	struct K_(SetItem) *buckets, *b, *b_end, *new_b, *prev_x, *x;
	const unsigned log_c0 = set->log_capacity,
		log_limit = sizeof buckets->hash * 8;
	unsigned c0 = 1 << log_c0, log_c1, c1, mask;
	assert(set && size && log_c0 < log_limit
		&& (set->log_capacity >= 3 || !set->log_capacity));
	/* Starting bucket number is a power of 2 in [8, 1 << (log_limit - 1)]. */
	if(eff_size > 1u << (log_limit - 1)) {
		log_c1 = log_limit - 1;
		c1 = 1 << (log_limit - 1);
	} else {
		if(log_c0 < 3) log_c1 = 3u,     c1 = 8u;
		else           log_c1 = log_c0, c1 = c0;
		while(c1 < eff_size) log_c1++, c1 <<= 1;
	}
	/* It's under the critical load factor; don't need to do anything. */
	if(log_c0 == log_c1) return 1;
	/* Everything else is amortised. Allocate new space for an expansion. */
	if(!(buckets = realloc(set->buckets, sizeof *buckets * c1))) return 0;
	set->buckets = buckets;
	set->log_capacity = log_c1;
	/* The mask needs domain {c0 \in [1, max]}, but we want 0 for loops. */
	mask = (c1 - 1) ^ (c0 - 1), assert(mask);
	if(c0 == 1) c0 = 0, assert(!c0 || c0 >= 8);
	/* Initialize the new lists to contain no elements. */
	for(b = buckets + c0, b_end = buckets + c1; b < b_end; b++) b->next = 0;
	/* Rehash some entries into the new lists. */
	for(b = buckets, b_end = buckets + c0; b < b_end; b++) {
		assert(!((b - buckets) & mask));
		/* Skip the keys that go nowhere. Rehash to the higher buckets. */
		prev_x = b;
		while((x = prev_x->next)) {
			if(!(x->hash & mask)) { prev_x = x; continue; }
			prev_x->next = x->next;
			new_b = PK_(get_bucket)(set, x->hash);
			x->next = new_b->next, new_b->next = x;
		}
	}
	return 1;
}

/** Zeros `set`, a well-defined state. */
static void PK_(set)(struct K_(Set) *const set) {
	assert(set);
	set->buckets      = 0;
	set->log_capacity = 0;
	set->size         = 0;
}

/** Destructor for `set`. After, it takes no memory and is in an empty state.
 @allow */
static void K_(Set_)(struct K_(Set) *const set) {
	if(!set) return;
	free(set->buckets);
	PK_(set)(set);
}

/** Initialises `set` to be take no memory and be in an empty state. If it is
 `static` data, then it is initialised by default. Alternatively, assigning
 `{0}` (`C99+`) or `SET_ZERO` as the initialiser also puts it in an empty
 state. Calling this on an active set will cause memory leaks.
 @param[set] If null, does nothing.
 @order \Theta(1)
 @allow */
static void K_(Set)(struct K_(Set) *const set) {
	if(!set) return;
	PK_(set)(set);
}

/** Clears and removes all entries from `set`. The capacity and memory of the
 hash table is preserved, but all previous values are un-associated. Until the
 previous size is obtained, the load factor will be less.
 @param[set] If null, does nothing.
 @order \Theta(`set.buckets`)
 @allow */
static void K_(SetClear)(struct K_(Set) *const set) {
	struct K_(SetItem) *b, *b_end;
	if(!set || !set->log_capacity) return;
	for(b = set->buckets, b_end = b + (1 << set->log_capacity); b < b_end; b++)
		b->next = 0;
	set->size = 0;
}

/** @return The number of entries in the `set`.
 @param[set] If null, returns 0.
 @order \Theta(1) */
static size_t K_(SetSize)(const struct K_(Set) *const set) {
	if(!set) return 0;
	return set->size;
}

/** Gets `item` from `set`.
 @return The value which <typedef:<PK>IsEqual> the `item`, or, if no such value
 exists, null.
 @order Constant time assuming the hash function is uniform; worst \O(n).
 @allow */
static const K *K_(SetGet)(struct K_(Set) *const set, const K key) {
	unsigned hash;
	struct K_(SetItem) *bucket, *x;
	if(!set || !set->buckets) return 0;
	hash   = PK_(hash)(key);
	bucket = PK_(get_bucket)(set, hash);
	for(x = bucket->next; x; x = x->next)
		if(hash == x->hash && PK_(is_equal)(x->data, key)) return &x->data;
	return 0;
}

/** Puts the `item` in `set`. Adding an element with the same `K`, according
 to <typedef:<PK>IsEqual> `SET_IS_EQUAL`, causes the old data to be ejected.
 @param[set, item] If null, returns false.
 @param[item] Must not be part this `set` or any other, because the integrety
 of the other set will be compromised.
 @param[p_eject] If not-null, this address of a variable that will store the
 `K` that was replaced, if any. If null, does nothing.
 @return Success.
 @throws[realloc]
 @order Constant time assuming the hash function is uniform; worst \O(n).
 @allow */
static int K_(SetPut)(struct K_(Set) *const set,
	struct K_(SetItem) *const item, const struct K_(SetItem) **const p_eject) {
	struct K_(SetItem) *bucket, *eject = 0;
	unsigned hash;
	if(p_eject) *p_eject = 0;
	if(!set || !item) return 0;
	/* Calculate and cache the hash value of the key. */
	hash = item->hash = PK_(hash)(item->data);
	/* There can only be only be one entry `is_equal(entry)`. */
	if(set->buckets) {
		struct K_(SetItem) *prev_x, *x;
		bucket = PK_(get_bucket)(set, hash);
		for(prev_x = bucket; (x = prev_x->next); prev_x = x)
			if(hash == x->hash && PK_(is_equal)(item->data, x->data)) break;
		if(x) {
			prev_x->next = x->next;
			if(x->next == x) x->next = 0;
			if(eject) eject = x;
		}
	}
	/* If the entry is new, we grow. The bucket may have changed. */
	if(!eject) {
		assert(set->size + 1 > set->size);
		if(!PK_(grow)(set, set->size + 1)) return 0;
		bucket = PK_(get_bucket)(set, item->hash);
		set->size++;
	}
	/* Stick the item on the head of the bucket. */
	item->next = bucket->next, bucket->next = item;
	/* Request replaced? */
	if(p_eject) *p_eject = eject;
	return 1;
}

			/* fixme */
static const char *K_(SetToString)(const struct K_(Set) *const set);

/** Puts the `item` in `set` only if the entry is absent.
 @param[set, item] If null, returns false.
 @param[item] Must not be part this `set` or any other.
 @param[p_is_absent] If not-null, it will signal the successful placement.
 @return Successful operation, including doing nothing because the entry is
 already in the set.
 @throws[realloc]
 @order Constant time assuming the hash function is uniform; worst \O(n).
 @allow */
static int K_(SetPutIfAbsent)(struct K_(Set) *const set,
	struct K_(SetItem) *const item, int *const p_is_absent) {
	struct K_(SetItem) *bucket;
	unsigned hash;
	if(p_is_absent) *p_is_absent = 0;
	if(!set || !item) return 0;
	hash = item->hash = PK_(hash)(item->data);
	if(set->buckets) {
		char a[12];
		struct K_(SetItem) *prev_x, *x;
		PK_(to_string)(&item->data, &a);
		fprintf(stderr, "we are looking for %s$%u\n", a, item->hash);
		bucket = PK_(get_bucket)(set, hash);
		for(prev_x = bucket; (x = prev_x->next); prev_x = x) {
			PK_(to_string)(&x->data, &a);
			fprintf(stderr, "\t%s$%u\n", a, x->hash);
			if(hash == x->hash && PK_(is_equal)(item->data, x->data)) return 1;
		}
	}
	if(!PK_(grow)(set, set->size + 1)) return 0;
	bucket = PK_(get_bucket)(set, item->hash);
	item->next = bucket->next, bucket->next = item;
	set->size++;
	if(p_is_absent) *p_is_absent = 1;
	fprintf(stderr, "Now %s.\n", K_(SetToString)(set));
	return 1;
}

/** Removes an element specified by `key` from `set`.
 @return Successfully removed an element or null.
 @order Constant time assuming the hash function is uniform; worst \O(n).
 @allow */
static struct K_(SetItem) *K_(SetRemove)(struct K_(Set) *const set,
	const K key) {
	unsigned hash;
	struct K_(SetItem) *bucket, *prev_x, *x;
	if(!set || !set->size || !set->buckets) return 0;
	hash   = PK_(hash)(key);
	bucket = PK_(get_bucket)(set, hash);
	for(prev_x = bucket; (x = prev_x->next); prev_x = x)
		if(hash == x->hash && PK_(is_equal)(x->data, key)) break;
	if(!x) return 0;
	prev_x->next = x->next;
	x->next = 0;
	set->size--;
	return x;
}

#ifdef SET_TO_STRING /* <-- print */

#ifndef SET_PRINT_THINGS /* <-- once inside translation unit */
#define SET_PRINT_THINGS

struct Set_SuperCat {
	char *print, *cursor;
	size_t left;
	int is_truncated;
};
static void set_super_cat_init(struct Set_SuperCat *const cat,
	char *const print, const size_t print_size) {
	cat->print = cat->cursor = print;
	cat->left = print_size;
	cat->is_truncated = 0;
	print[0] = '\0';
}
static void set_super_cat(struct Set_SuperCat *const cat,
	const char *const append) {
	size_t lu_took; int took;
	if(cat->is_truncated) return;
	took = sprintf(cat->cursor, "%.*s", (int)cat->left, append);
	if(took < 0)  { cat->is_truncated = -1; return; } /*implementation defined*/
	if(took == 0) { return; }
	if((lu_took = (size_t)took) >= cat->left)
		cat->is_truncated = -1, lu_took = cat->left - 1;
	cat->cursor += lu_took, cat->left -= lu_took;
}

#endif /* once --> */

/** Can print 2 things at once before it overwrites. One must set
 `SET_TO_STRING` to a function implementing <typedef:<PK>ToString> to get this
 functionality.
 @return Prints `set` in a static buffer.
 @order \Theta(1); it has a 1024 character limit; every element takes some of
 it.
 @allow */
static const char *K_(SetToString)(const struct K_(Set) *const set) {
	static char buffer[2][1024];
	static unsigned buffer_i;
	struct Set_SuperCat cat;
	int is_first = 1;
	assert(4 /*...]*/ >= 2 /* ]*/ && sizeof buffer > 4 /*...]*/);
	set_super_cat_init(&cat, buffer[buffer_i],
		sizeof *buffer / sizeof **buffer - 4 /*...]*/);
	buffer_i++, buffer_i &= 1;
	if(!set) return set_super_cat(&cat, "null"), cat.print;
	set_super_cat(&cat, "[ ");
	if(set->buckets) {
		struct K_(SetItem) *b, *b_end, *x;
		char a[12];
		assert(set->log_capacity >= 3 && set->log_capacity < 32);
		for(b = set->buckets, b_end = b + (1 << set->log_capacity);
			b < b_end; b++) {
			if(!is_first) set_super_cat(&cat, "/");
			for(x = b->next; x; x = x->next) {
				if(!is_first) {
					set_super_cat(&cat, ", ");
					if(cat.is_truncated) break;
				} else {
					is_first = 0;
				}
				PK_(to_string)(&x->data, &a);
				set_super_cat(&cat, a);
				if(cat.is_truncated) break;
			}
			if(x) break;
		}
	}
	sprintf(cat.cursor, "%s", cat.is_truncated ? "...]" : " ]");
	return cat.print; /* Static buffer. */
}

#endif /* print --> */

#ifdef SET_TEST /* <-- test */
#include "../test/TestSet.h" /* need this file if one is going to run tests */
#endif /* test --> */

static void PK_(unused_coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 \url{ http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code } */
static void PK_(unused_set)(void) {
	K_(Set_)(0);
	K_(Set)(0);
	K_(SetClear)(0);
	K_(SetSize)(0);
	K_(SetGet)(0, 0);
	K_(SetPut)(0, 0, 0);
	K_(SetPutIfAbsent)(0, 0, 0);
	K_(SetRemove)(0, 0);
#ifdef SET_TO_STRING
	K_(SetToString)(0);
#endif
	PK_(unused_coda)();
}
static void PK_(unused_coda)(void) { PK_(unused_set)(); }

/* Un-define all macros. */
#undef SET_NAME
#undef SET_TYPE
/* Undocumented; allows nestled inclusion so long as: {CAT_}, {CAT}, {PCAT},
 {PCAT_} conform, and {K} is not used. */
#ifdef SET_SUBTYPE /* <-- sub */
#undef SET_SUBTYPE
#else /* sub --><-- !sub */
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#endif /* !sub --> */
#undef K
#undef K_
#undef PK_
#undef SET_KEY
#undef SET_VALUE
#undef SET_HASH
#undef SET_IS_EQUAL
#ifdef SET_TO_STRING /* <-- string */
#undef SET_TO_STRING
#endif /* string --> */
#ifdef SET_TEST /* <-- test */
#undef SET_TEST
#endif /* test --> */
