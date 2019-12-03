/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 `<E>Set` is a collection of elements of type `E`, along with a hash function
 and equality function, that doesn't allow duplication. Internally, it is a
 separately chained hash set having a maximum load factor of `ln 2`. It
 requires the storage of <tag:<E>SetItem>. While in the set, the
 values cannot change. One can use this as the key in an associative array.

 @param[SET_NAME, SET_TYPE]
 `E` that satisfies `C` naming conventions when mangled; required.

 @param[SET_HASH]
 A function satisfying <typedef:<PE>Hash>; required.

 @param[SET_IS_EQUAL]
 A function satisfying <typedef:<PE>IsEqual>; required.

 @param[SET_NO_CACHE]
 Always calculates the hash every time and don't store it _per_ datum. Best
 used when the data to be hashed is very small, (_viz_, the hash calculation is
 trivial.)

 @param[SET_TO_STRING]
 Optional print function implementing <typedef:<PE>ToString>; makes available
 <fn:<E>SetToString>.

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
#ifdef SET_TO_STRING /* <!-- string */
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
#ifdef E
#undef E
#endif
#ifdef E_
#undef E_
#endif
#ifdef PE_
#undef PE_
#endif
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#define E_(thing) CAT(SET_NAME, thing)
#define PE_(thing) PCAT(set, PCAT(SET_NAME, thing)) /* "Private." */

/* Troubles? Check `SET_TYPE` is a valid type, whose definition is placed above
 inclusion. */
typedef SET_TYPE PE_(Type);
#define E PE_(Type)



/** Contains `E` as the element `data` along with data internal to the set.
 Storage of the `<E>SetItem` structure is the responsibility of the caller; it
 could be one part of a more complex super-structure, (thus using it as a hash
 table.) */
struct E_(SetItem);
struct E_(SetItem) {
	E data;
#ifndef SET_NO_CACHE /* <!-- cache */
	unsigned hash;
#endif /* cache --> */
	struct E_(SetItem) *next;
};



/** A `<E>Set`. To initialise, see <fn:<E>Set>. */
struct E_(Set);
struct E_(Set) {
	struct E_(SetItem) *buckets; /* An array of 1 << log_capacity (>3) or 0. */
	unsigned log_capacity;
	size_t size;
};
#define SET_ZERO { 0, 0, 0 }



/** A map from `E` onto `unsigned int`. Should be as close as possible to a
 discrete uniform distribution for maximum performance and, when computing,
 take all of `E` into account. */
typedef unsigned (*PE_(Hash))(const E);
/* Check that `SET_HASH` is a function implementing <typedef:<PE>Hash>. */
static const PE_(Hash) PE_(hash) = (SET_HASH);

/** A constant equivalence relation between `E` that satisfies
 `<PE>IsEqual(a, b) -> <PE>Hash(a) == <PE>Hash(b)`. */
typedef int (*PE_(IsEqual))(const E, const E);
/* Check that `SET_IS_EQUAL` is a function implementing
 <typedef:<PE>IsEqual>. */
static const PE_(IsEqual) PE_(is_equal) = (SET_IS_EQUAL);

/** Returns true if the `replace` replaces the `original`; used in
 <fn:<E>SetPutResolve>. */
typedef int (*PE_(Replace))(E *original, E *replace);

#ifdef SET_TO_STRING /* <!-- string */
/** Responsible for turning `E` (the first argument) into a 12 `char` string
 (the second.) */
typedef void (*PE_(ToString))(const E *const, char (*const)[12]);
/* Check that `SET_TO_STRING` is a function implementing
 <typedef:<PE>ToString>. */
static const PE_(ToString) PE_(to_string) = (SET_TO_STRING);
#endif /* string --> */

#ifdef SET_TEST /* <!-- test */
/** Used for `SET_TEST`. */
typedef void (*PE_(Action))(const E *const);
#endif /* test --> */



/** Gets the hash of `item`. */
static unsigned PE_(get_hash)(struct E_(SetItem) *item) {
#ifdef SET_NO_CACHE /* <!-- !cache */
	return PE_(hash)(item->data);
#else /* !cache --><!-- cache */
	return item->hash;
#endif /* cache --> */
}

/** Retrieves a bucket from `set` given the `hash`. Only call this function if
 non-empty. Will be invalidated upon a call to <fn:<PE>grow>.
 @return Given a `hash`, compute the bucket at it's index. */
static struct E_(SetItem) *PE_(get_bucket)(struct E_(Set) *const set,
	const unsigned hash) {
	assert(set && set->buckets);
	return set->buckets + (hash & ((1 << set->log_capacity) - 1));
}

/** 
 @param[bucket] Must match the bucket of `data`.
 @param[hash] Must match the hash of `data`.
 @return The link before the `data` in `bucket` or null. */
static struct E_(SetItem) *PE_(bucket_prev)(struct E_(SetItem) *const bucket,
	const E data, const unsigned hash) {
	struct E_(SetItem) *prev_x, *x;
	assert(bucket && data);
	for(prev_x = bucket; (x = prev_x->next); prev_x = x) {
#ifndef SET_NO_CACHE /* <!-- cache: a quick out. */
		if(hash != x->hash) continue;
#endif /* cache --> */
		if(PE_(is_equal)(data, x->data)) return prev_x;
	}
#ifdef SET_NO_CACHE /* <!-- !cache */
	(void)(hash);
#endif /* cache --> */
	return 0;
}

/** Private: grow the table until the capacity is at least
 `size / load factor = ln 2 = 0.69`, ranged from `[8, UINT]` entries.
 @param[size] How many entries are there going to be; `size > 1`.
 @return Success; otherwise, `errno` may be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t`.
 @throws[malloc]
 @order \O(1) amortized. */
static int PE_(grow)(struct E_(Set) *const set, const size_t size) {
	/* Size if we want each bucket to have one item. */
	const size_t eff_size = 1 + size / 0.693147180559945309417232121458176568;
	struct E_(SetItem) *buckets, *b, *b_end, *new_b, *prev_x, *x;
	const unsigned log_c0 = set->log_capacity,
		log_limit = sizeof(unsigned) * 8;
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
	/* The mask needs domain `c0 \in [1, max]`, but we want 0 for loops. */
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
			unsigned hash = PE_(get_hash)(x);
			if(!(hash & mask)) { prev_x = x; continue; }
			prev_x->next = x->next;
			new_b = PE_(get_bucket)(set, hash);
			x->next = new_b->next, new_b->next = x;
		}
	}
	return 1;
}

/** Zeros `set`, a well-defined state. */
static void PE_(set)(struct E_(Set) *const set) {
	assert(set);
	set->buckets      = 0;
	set->log_capacity = 0;
	set->size         = 0;
}

/** Destructor for `set`. After, it takes no memory and is in an empty state.
 @allow */
static void E_(Set_)(struct E_(Set) *const set) {
	if(!set) return;
	free(set->buckets);
	PE_(set)(set);
}

/** Initialises `set` to be take no memory and be in an empty state. If it is
 `static` data, then it is initialised by default. Alternatively, assigning
 `{0}` (`C99`+) or `SET_ZERO` as the initialiser also puts it in an empty
 state. Calling this on an active set will cause memory leaks.
 @param[set] If null, does nothing.
 @order \Theta(1)
 @allow */
static void E_(Set)(struct E_(Set) *const set) {
	if(!set) return;
	PE_(set)(set);
}

/** Clears and removes all entries from `set`. The capacity and memory of the
 hash table is preserved, but all previous values are un-associated. The load
 factor will be less until it reaches it's previous size.
 @param[set] If null, does nothing.
 @order \Theta(`set.buckets`)
 @allow */
static void E_(SetClear)(struct E_(Set) *const set) {
	struct E_(SetItem) *b, *b_end;
	if(!set || !set->log_capacity) return;
	for(b = set->buckets, b_end = b + (1 << set->log_capacity); b < b_end; b++)
		b->next = 0;
	set->size = 0;
}

/** @return The number of entries in the `set`.
 @param[set] If null, returns 0.
 @order \Theta(1) */
static size_t E_(SetSize)(const struct E_(Set) *const set) {
	if(!set) return 0;
	return set->size;
}

/** Gets `item` from `set`. This can be used for testing if the item is in the
 `set`.
 @return The value which <typedef:<PE>IsEqual> the `item`, or, if no such value
 exists, null.
 @order Average \O(1), (hash distributes elements uniformly); worst \O(n).
 @allow */
static const E *E_(SetGet)(struct E_(Set) *const set, const E data) {
	struct E_(SetItem) *prev;
	unsigned hash;
	if(!set || !set->buckets) return 0;
	hash = PE_(hash)(data);
	prev = PE_(bucket_prev)(PE_(get_bucket)(set, hash), data, hash);
	return prev ? &prev->next->data : 0;
}

/** Puts the `item` in `set`. Adding an element with the same `E`, according
 to <typedef:<PE>IsEqual> `SET_IS_EQUAL`, causes the old data to be ejected.
 @param[set, item] If null, returns false.
 @param[item] Should not be of a `set`, because the integrity of that `set`
 will be compromised.
 @return The removed item, or null.
 @throws[realloc, EDOM] This is a problem.
 @order Average amortised \O(1), (hash distributes elements uniformly);
 worst \O(n).
 @fixme Have a `<E>SetPutReplaced` that accepts a parameter. All of these will
 call a single private fn.
 @allow */
static int E_(SetPut)(struct E_(Set) *const set,
	struct E_(SetItem) *const item) {
	struct E_(SetItem) *bucket, *prev = 0;
	unsigned hash;
	if(!set || !item) return 0;
	hash = PE_(hash)(item->data);
	/* Delete any duplicate. */
	if(set->buckets) {
		bucket = PE_(get_bucket)(set, hash);
		if((prev = PE_(bucket_prev)(bucket, item->data, hash)))
			{ prev->next = prev->next->next; goto erased; }
	}
	/* New entry; the bucket may change. */
	assert(set->size + 1 > set->size);
	if(!PE_(grow)(set, set->size + 1)) return 0;
	bucket = PE_(get_bucket)(set, hash);
	set->size++;
erased:
	/* Stick the item on the head of the bucket. */
	item->next = bucket->next, bucket->next = item;
	return 1;
}

/** Puts the `item` in `set` only if the entry is absent or if calling
 `replace` returns true.
 @param[set, item] If null, returns false.
 @param[item] Must not be part this `set` or any other.
 @param[replace] If specified, gets called on collision and only replaces it if
 the function returns true. If null, doesn't do any replacement on collision.
 @return Successful operation, including doing nothing because the entry is
 already in the set.
 @throws[realloc]
 @order Constant time assuming the hash function is uniform; worst \O(n).
 @allow */
static int E_(SetPutResolve)(struct E_(Set) *const set,
	struct E_(SetItem) *const item, const PE_(Replace) replace) {
	struct E_(SetItem) *bucket, *prev;
	unsigned hash;
	if(!set || !item) return 0;
	hash = PE_(hash)(item->data);
	/* Delete any duplicate. */
	if(set->buckets) {
		bucket = PE_(get_bucket)(set, hash);
		if((prev = PE_(bucket_prev)(bucket, item->data, hash))) {
			struct E_(SetItem) *const collide = prev->next;
			if(replace && replace(&collide->data, &item->data))
				{ prev->next = prev->next->next; goto erased; }
			return 1;
		}
	}
	/* New entry; the bucket may change. */
	assert(set->size + 1 > set->size);
	if(!PE_(grow)(set, set->size + 1)) return 0;
	bucket = PE_(get_bucket)(set, hash);
	set->size++;
erased:
	/* Stick the item on the head of the bucket. */
	item->next = bucket->next, bucket->next = item;
	return 1;
}

/** Removes an element specified by `key` from `set`.
 @return Successfully removed an element or null.
 @order Constant time assuming the hash function is uniform; worst \O(n).
 @allow */
static struct E_(SetItem) *E_(SetRemove)(struct E_(Set) *const set,
	const E data) {
	unsigned hash;
	struct E_(SetItem) *prev, *removed;
	if(!set || !set->buckets) return 0;
	hash = PE_(hash)(data);
	if(!(prev = PE_(bucket_prev)(PE_(get_bucket)(set, hash), data, hash)))
		return 0;
	removed = prev->next;
	prev->next = removed->next;
	assert(set->size);
	set->size--;
	return removed;
}

#ifdef SET_TO_STRING /* <!-- print */

#ifndef SET_PRINT_THINGS /* <!-- once inside translation unit */
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
 `SET_TO_STRING` to a function implementing <typedef:<PE>ToString> to get this
 functionality.
 @return Prints `set` in a static buffer.
 @order \Theta(1); it has a 1024 character limit; every element takes some of
 it.
 @allow */
static const char *E_(SetToString)(const struct E_(Set) *const set) {
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
		struct E_(SetItem) *b, *b_end, *x;
		char a[12];
		assert(set->log_capacity >= 3 && set->log_capacity < 32);
		for(b = set->buckets, b_end = b + (1 << set->log_capacity);
			b < b_end; b++) {
			for(x = b->next; x; x = x->next) {
				if(!is_first) {
					set_super_cat(&cat, ", ");
					if(cat.is_truncated) break;
				} else {
					is_first = 0;
				}
				PE_(to_string)(&x->data, &a);
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

#ifdef SET_TEST /* <!-- test */
#include "../test/TestSet.h" /* need this file if one is going to run tests */
#endif /* test --> */

static void PE_(unused_coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 <http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code> */
static void PE_(unused_set)(void) {
	E_(Set_)(0);
	E_(Set)(0);
	E_(SetClear)(0);
	E_(SetSize)(0);
	E_(SetGet)(0, 0);
	E_(SetPut)(0, 0);
	E_(SetPutResolve)(0, 0, 0);
	E_(SetRemove)(0, 0);
#ifdef SET_TO_STRING
	E_(SetToString)(0);
#endif
	PE_(unused_coda)();
}
static void PE_(unused_coda)(void) { PE_(unused_set)(); }

/* Un-define all macros. */
#undef SET_NAME
#undef SET_TYPE
/* Undocumented; allows nestled inclusion so long as: `CAT`, _etc_ conform to
 the definitions, and `E` is not used. */
#ifdef SET_SUBTYPE /* <!-- sub */
#undef SET_SUBTYPE
#else /* sub --><!-- !sub */
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#endif /* !sub --> */
#undef E
#undef E_
#undef PE_
#undef SET_NAME
#undef SET_TYPE
#undef SET_HASH
#undef SET_IS_EQUAL
#ifdef SET_NO_CACHE /* <!-- !cache */
#undef SET_NO_CACHE
#endif /* !cache --> */
#ifdef SET_TO_STRING /* <!-- string */
#undef SET_TO_STRING
#endif /* string --> */
#ifdef SET_TEST /* <!-- test */
#undef SET_TEST
#endif /* test --> */
