/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Unordered set

 ![Example of <string>set.](../web/set.png)

 <tag:<S>set> is a collection of elements of <tag:<PS>type> that doesn't allow
 duplication.

 To do this, it must be supplied a hash function and equality function.
 Internally, it is a power-of-two hash table that points to a separate bucket
 pool. Maximum load factor is `ln 2`. It can be expanded to an associative
 array by supplying a pointer `<PS>type`, which becomes the key, and enclosing
 in another `struct` which includes the value.

 Note that constant compile-time sets are better handled with
 [gperf](https://www.gnu.org/software/gperf/), which will generate much faster
 and smaller code. Also, [CMPH](http://cmph.sourceforge.net/) is a minimal
 perfect hashing library that may be useful.

 @param[SET_NAME, SET_TYPE]
 `<S>` that satisfies `C` naming conventions when mangled and a valid
 <typedef:<PS>type> associated therewith; required. `<PS>` is private, whose
 names are prefixed in a manner to avoid collisions; any should be re-defined
 prior to use elsewhere.

 @param[SET_HASH]
 A function satisfying <typedef:<PS>hash_fn>; required.

 @param[SET_IS_EQUAL]
 A function satisfying <typedef:<PS>is_equal_fn>; required.

 @param[SET_UINT]
 This is <typedef:<PS>uint> and defaults to `size_t`; use when
 <typedef:<PS>hash_fn> is a specific hash length.

 @param[SET_RECALCULATE]
 Don't cache the hash, but calculate every time and discards it; for example,
 should be used when the hash calculation is trivial to avoid storing duplicate
 <typedef:<PS>uint> _per_ entry.

 @param[SET_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[SET_TO_STRING]
 To string trait contained in <to_string.h>; `<SZ>` that satisfies `C` naming
 conventions when mangled and function implementing
 <typedef:<PSZ>to_string_fn>. There can be multiple to string traits, but only
 one can omit `SET_TO_STRING_NAME`.

 @std C89 */

#if !defined(SET_NAME) || !defined(SET_TYPE) || !defined(SET_IS_EQUAL) \
	|| !defined(SET_HASH)
#error Name SET_NAME, tag type SET_TYPE, fns SET_IS_EQUAL, SET_HASH undefined.
#endif
#if defined(SET_TO_STRING_NAME) || defined(SET_TO_STRING)
#define SET_TO_STRING_TRAIT 1
#else
#define SET_TO_STRING_TRAIT 0
#endif
#define SET_TRAITS SET_TO_STRING_TRAIT
#if SET_TRAITS > 1
#error Only one trait per include is allowed; use SET_EXPECT_TRAIT.
#endif
#if defined(SET_TO_STRING_NAME) && !defined(SET_TO_STRING)
#error SET_TO_STRING_NAME requires SET_TO_STRING.
#endif

#ifndef SET_H /* <!-- idempotent */
#define SET_H
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>
#if defined(SET_CAT_) || defined(SET_CAT) || defined(S_) || defined(PS_) \
	|| defined(SET_IDLE)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define SET_CAT_(n, m) n ## _ ## m
#define SET_CAT(n, m) SET_CAT_(n, m)
#define S_(n) SET_CAT(SET_NAME, n)
#define PS_(n) SET_CAT(set, S_(n))
#define SET_IDLE { 0, 0, 0, { POOL_IDLE } }
#endif /* idempotent --> */


#if SET_TRAITS == 0 /* <!-- base code */


#ifndef SET_UINT
#define SET_UINT size_t
#endif

/** Unsigned integer type used for hash values. The hash map will saturate at
 `min(((ln 2)/2) \cdot range(<PS>uint), (1/8) \cdot range(size_t))`,
 at which point no new buckets can be added and the load factor will
 increase. */
typedef SET_UINT PS_(uint);

/** Valid tag type defined by `SET_TYPE`. */
typedef SET_TYPE PS_(type);

/** A map from <typedef:<PS>type> onto <typedef:<PS>uint>. Usually should use
 all the the argument and output should be as close as possible to a discrete
 uniform distribution. */
typedef PS_(uint) (*PS_(hash_fn))(const PS_(type));

/** Equivalence relation between <typedef:<PS>type> that satisfies
 `<PS>is_equal_fn(a, b) -> <PS>hash_fn(a) == <PS>hash_fn(b)`. */
typedef int (*PS_(is_equal_fn))(const PS_(type) a, const PS_(type) b);

/* Check that `SET_HASH` is a function implementing <typedef:<PS>hash_fn>. */
static const PS_(hash_fn) PS_(hash) = (SET_HASH);

/* Check that `SET_IS_EQUAL` is a function implementing
 <typedef:<PS>is_equal_fn>. */
static const PS_(is_equal_fn) PS_(equal) = (SET_IS_EQUAL);

/** A bi-predicate; returns true if the `replace` replaces the `original`. */
typedef int (*PS_(replace_fn))(PS_(type) original, PS_(type) replace);

/** Used in <fn:<S>set_policy_put> when `replace` is null; `original` and
 `replace` are ignored. @implements `<PS>replace_fn` */
static int PS_(false)(PS_(type) *original, PS_(type) *replace)
	{ (void)(original); (void)(replace); return 0; }

/** A bucket is a linked-list of entries, which store keys. */
struct PS_(entry) {
	struct PS_(entry) *next;
#ifndef SET_RECALCULATE /* <!-- cache */
	PS_(uint) hash;
#endif /* cache --> */
	PS_(type) key;
};

/* Uses pool to store unordered <typedef:<PS>entry>. */
#define POOL_NAME PS_(entry)
#define POOL_TYPE struct PS_(entry)
#include "pool.h"

/** Don't feel like having confusing double-pointers. */
struct PS_(bucket) { struct PS_(entry) *head; };

/** To initialize, see <fn:<S>set>, `SET_IDLE`, `{0}` (`C99`,) or being
 `static`.

 ![States.](../web/states.png) */
struct S_(set) {
	struct PS_(bucket) *buckets; /* An array of 1 << log_capacity (>3) or 0. */
	unsigned log_capacity, unused;
	struct PS_(entry_pool) entries;
};

/** Gets the hash of `entry`, which should be consistent. */
static PS_(uint) PS_(hash_from_entry)(const struct PS_(entry) *const entry) {
	/* assert(entry); */
#ifdef SET_RECALCULATE /* <!-- !cache */
	return PS_(hash)(&entry->data);
#else /* !cache --><!-- cache */
	return entry->hash;
#endif /* cache --> */
}

/** @return Retrieves a bucket from `set` given the `hash`. */
static struct PS_(bucket) *PS_(get_bucket)(struct S_(set) *const set,
	const PS_(uint) hash) {
	assert(set && set->buckets);
	return set->buckets + (hash & ((1 << set->log_capacity) - 1));
}

/** Linear search for `key` in `bucket`.
 @param[hash] Must match the hash of `key`.
 @return The link that points to the `key` or null. */
static struct PS_(entry) **PS_(bucket_link)(struct PS_(bucket) *const bucket,
	const PS_(uint) hash, const PS_(type) key) {
	struct PS_(entry) **to_entry = &bucket->head, *entry;
	assert(bucket);
	while(entry = *to_entry) {
#ifndef SET_RECALCULATE /* <!-- cache: quick out. */
		if(hash != entry->hash) continue;
#endif /* cache --> */
		if(PS_(equal)(key, entry->key)) return to_entry;
		to_entry = &entry->next;
	}
#ifdef SET_RECALCULATE /* <!-- !cache */
	(void)(hash);
#endif /* cache --> */
	return 0;
}

/** Ensures that `set` has enough buckets to hold `ln 2` times `min_capacity`.
 @param[min_capacity] If zero, does nothing.
 @return Success; otherwise, `errno` will be set. @throws[ERANGE] Tried
 allocating more then can fit in `size_t` or `realloc` doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html).
 @throws[realloc] */
static int PS_(reserve)(struct S_(set) *const set, const size_t min_capacity) {
	struct PS_(bucket) *buckets, *b, *b_end;
	const unsigned log_c0 = set->log_capacity,
		log_limit = sizeof(PS_(uint)) * CHAR_BIT - 1;
	unsigned log_c1;
	PS_(uint) c0 = (PS_(uint))(1 << log_c0), c1, mask;
	/* One did set `<PS>uint` to an unsigned type, right? */
	assert(set && c0 && log_c0 <= log_limit && (log_c0 >= 3 || !log_c0)
		&& (PS_(uint))-1 > 0);
	/* Ca'n't be expanded further; the load factor will increase. */
	if(log_c0 >= log_limit) { assert(log_c0 == log_limit); return 1; }
	/* `C99` `SIZE_MAX` min 65535 -> 5041; load factor `ln 2 ~= 0.693 ~= 9/13`.
	 Bucket number is a power of 2 in `[8, 1 << log_limit]`. */
	do {
		size_t no_buckets;
		/* Overflow multiplying numerator. */
		if(min_capacity > (size_t)-1 / 13) {
			if(min_capacity > 1ul << log_limit) {
				log_c1 = log_limit, c1 = 1ul << log_c1;
				break;
			} else {
				no_buckets = min_capacity / 9 * 13;
			}
		} else if((no_buckets = min_capacity * 13 / 9) > 1ul << log_limit) {
			log_c1 = log_limit, c1 = 1ul << log_c1;
			break;
		}
		if(log_c0 < 3) log_c1 = 3ul,    c1 = 8ul;
		else           log_c1 = log_c0, c1 = c0;
		while(c1 < no_buckets) log_c1++, c1 <<= 1;
	} while(0);
	/* It's under the critical load factor; don't need to do anything. */
	if(log_c0 == log_c1) return 1;
	if(!(buckets = realloc(set->buckets, sizeof *buckets * c1)))
		{ if(!errno) errno = ERANGE; return 0; }
	set->buckets = buckets;
	set->log_capacity = log_c1;
	/* The mask needs domain `c0 \in [1, max]`, but we want 0 for loops. */
	mask = (c1 - 1) ^ (c0 - 1), assert(mask);
	if(c0 == 1) c0 = 0, assert(!c0 || c0 >= 8);
	/* Initialize to contain no elements. */
	for(b = buckets + c0, b_end = buckets + c1; b < b_end; b++) b = 0;
	/* Expectation value of rehashing an entry is half. */
	for(b = buckets, b_end = buckets + c0; b < b_end; b++) {
		struct PS_(entry) **to_x, *x;
		PS_(uint) hash;
		assert(!((PS_(uint))(b - buckets) & mask));
		to_x = &b->head;
		while(x = *to_x) {
			struct PS_(bucket) *new_b;
			hash = PS_(hash_from_entry)(x);
			if(!(hash & mask)) { to_x = &x->next; continue; }
			*to_x = x->next; /* Remove. */
			new_b = PS_(get_bucket)(set, hash);
			x->next = new_b->head, new_b->head = x;
		}
	}
	return 1;
}

/** Put `key` in `set` and returns the collided element, if any, as long as
 `replace` is null or returns true. If `replace` returns false, returns
 `key`. */
static struct PS_(entry) *PS_(put)(struct S_(set) *const set,
	PS_(type) key, const PS_(replace_fn) replace) {
	struct PS_(bucket) *bucket;
	PS_(type) **to_x = 0, *x = 0;
	PS_(uint) hash;
	assert(set && key);
	hash = PS_(hash)(key);
#ifndef SET_RECALCULATE /* <!-- cache */
	//element->hash = hash;
#endif /* cache --> */
	if(!set->buckets) goto grow_table;
	/* Deal with duplicates. */
	bucket = PS_(get_bucket)(set, hash);
	if(!(to_x = PS_(entry_in_bucket)(bucket, hash, key))) goto grow_table;
	x = *to_x;
	if(replace && !replace(&x->key, &element->key)) return key;
	*to_x = x->next, x->next = 0;
	goto add_element;
grow_table:
	assert(set->size + 1 > set->size);
	/* Didn't <fn:<PS>reserve>, now one can't tell error except `errno`. */
	if(!PS_(reserve)(set, set->size + 1)) return 0;
	bucket = PS_(get_bucket)(set, hash);
	set->size++;
add_element:
	element->next = bucket->first, bucket->first = element;
	return x;
}

/** Initialises `set` to idle. @order \Theta(1) @allow */
static void S_(set)(struct S_(set) *const set)
	{ assert(set); set->buckets = 0; set->log_capacity = 0;
	PS_(entry_pool)(&set->entries); }

/** Destroys `set` and returns it to idle. @allow */
static void S_(set_)(struct S_(set) *const set)
	{ assert(set), free(set->buckets), PS_(entry_pool_)(&set->entries);
	S_(set)(set); }

/** Clears and removes all entries from `set`. The capacity and memory of the
 hash table is preserved, but all previous values are un-associated. The load
 factor will be less until it reaches it's previous size.
 @param[set] If null, does nothing. @order \Theta(`set.buckets`) @allow */
static void S_(set_clear)(struct S_(set) *const set) {
	struct PS_(bucket) *b, *b_end;
	assert(set);
	if(!set->log_capacity) return;
	for(b = set->buckets, b_end = b + (1 << set->log_capacity); b < b_end; b++)
		b->first = 0;
	set->size = 0;
}

/** @return The value in `set` which <typedef:<PS>is_equal_fn> `SET_IS_EQUAL`
 `data`, or, if no such value exists, null.
 @param[set] If null, returns null. @order Average \O(1), (hash distributes
 elements uniformly); worst \O(n). @allow */
static struct S_(setlink) *S_(set_get)(struct S_(set) *const set,
	const PS_(mtype) data) {
	struct S_(setlink) **to_x;
	PS_(uint) hash;
	if(!set || !set->buckets) return 0;
	hash = PS_(hash)(data);
	to_x = PS_(bucket_to)(PS_(get_bucket)(set, hash), hash, data);
	return to_x ? *to_x : 0;
}

/** Reserve at least `reserve`, divided by the maximum load factor, space in
 the buckets of `set`. @return Success.
 @throws[ERANGE] `reserve` plus the size would take a bigger number then could
 fit in a `size_t`. @throws[realloc] @allow */
static int S_(set_reserve)(struct S_(set) *const set, const size_t reserve)
	{ return set ? reserve > (size_t)-1 - set->size ? (errno = ERANGE, 0) :
	PS_(reserve)(set, set->size + reserve) : 0; }

/** Puts the `element` in `set`.
 @param[set, element] If null, returns null. @param[element] Should not be of a
 set because the integrity of that set will be compromised.
 @return Any ejected element or null. (An ejected element has
 <typedef:<PS>is_equal_fn> `SET_IS_EQUAL` the `element`.)
 @throws[realloc, ERANGE] There was an error with a re-sizing. Successfully
 calling <fn:<S>set_reserve> with at least one before ensures that this does
 not happen. @order Average amortised \O(1), (hash distributes elements
 uniformly); worst \O(n). @allow */
static struct S_(setlink) *S_(set_put)(struct S_(set) *const set,
	struct S_(setlink) *const element) { return PS_(put)(set, element, 0); }

/** Puts the `element` in `set` only if the entry is absent or if calling
 `replace` returns true.
 @param[set, element] If null, returns null.
 @param[element] Should not be of a set because the integrity of that set will
 be compromised.
 @param[replace] Called on collision and only replaces it if the function
 returns true. If null, doesn't do any replacement on collision.
 @return Any ejected element or null. On collision, if `replace` returns false
 or `replace` is null, returns `element` and leaves the other element in the
 set. @throws[realloc, ERANGE] There was an error with a re-sizing.
 Successfully calling <fn:<S>set_reserve> with at least one before ensures that
 this does not happen. @order Average amortised \O(1), (hash distributes
 elements uniformly); worst \O(n). @allow */
static struct S_(setlink) *S_(set_policy_put)(struct S_(set) *const set,
	struct S_(setlink) *const element, const PS_(replace_fn) replace)
	{ return PS_(put)(set, element, replace ? replace : &PS_(false)); }

/** Removes an element `data` from `set`.
 @return Successfully ejected element or null. This element is free to be put
 into another set or modify it's hash values. @order Average \O(1), (hash
 distributes elements uniformly); worst \O(n). @allow */
static struct S_(setlink) *S_(set_remove)(struct S_(set) *const set,
	const PS_(mtype) data) {
	PS_(uint) hash;
	struct S_(setlink) **to_x, *x;
	if(!set || !set->buckets) return 0;
	hash = PS_(hash)(data);
	if(!(to_x = PS_(bucket_to)(PS_(get_bucket)(set, hash), hash, data)))
		return 0;
	x = *to_x;
	*to_x = x->next;
	assert(set->size);
	set->size--;
	return x;
}

/* <!-- iterate interface */

/* Contains all iteration parameters. */
struct PS_(iterator)
	{ const struct S_(set) *set; size_t b; struct S_(setlink) *e; };

/** Loads `set` into `it`. @implements begin */
static void PS_(begin)(struct PS_(iterator) *const it,
	const struct S_(set) *const set)
	{ assert(it && set), it->set = set, it->b = 0, it->e = 0; }

/** Advances `it`. @implements next */
static const PS_(type) *PS_(next)(struct PS_(iterator) *const it) {
	const size_t b_end = 1 << it->set->log_capacity;
	assert(it && it->set);
	if(!it->set->buckets) return 0;
	while(it->b < b_end) {
		if(!it->e) it->e = it->set->buckets[it->b].first;
		else it->e = it->e->next;
		if(it->e) return &it->e->key; /*???*/
		it->b++;
	}
	it->set = 0, it->b = 0, it->e = 0;
	return 0;
}

/* iterate --> */

/* <!-- box (multiple traits) */
#define BOX_ PS_
#define BOX_CONTAINER struct S_(set)
#define BOX_CONTENTS PS_(type)

#ifdef SET_TEST /* <!-- test */
/* Forward-declare. */
static void (*PS_(to_string))(const PS_(type) *, char (*)[12]);
static const char *(*PS_(set_to_string))(const struct S_(set) *);
#include "../test/test_set.h"
#endif /* test --> */

static void PS_(unused_base_coda)(void);
static void PS_(unused_base)(void) {
	S_(set)(0); S_(set_)(0); S_(set_clear)(0); S_(set_get)(0, 0);
	S_(set_reserve)(0, 0); S_(set_put)(0, 0);  S_(set_policy_put)(0, 0, 0);
	S_(set_remove)(0, 0);
	PS_(begin)(0, 0); PS_(next)(0);
	PS_(unused_base_coda)();
}
static void PS_(unused_base_coda)(void) { PS_(unused_base)(); }


#elif defined(SET_TO_STRING) /* base code --><!-- to string trait */


#ifdef SET_TO_STRING_NAME
#define SZ_(n) SET_CAT(S_(set), SET_CAT(SET_TO_STRING_NAME, n))
#else
#define SZ_(n) SET_CAT(S_(set), n)
#endif
#define TO_STRING SET_TO_STRING
#define TO_STRING_LEFT '{'
#define TO_STRING_RIGHT '}'
#include "to_string.h" /** \include */
#ifdef SET_TEST /* <!-- expect: greedy satisfy forward-declared. */
#undef SET_TEST
static PSZ_(to_string_fn) PS_(to_string) = PSZ_(to_string);
static const char *(*PS_(set_to_string))(const struct S_(set) *)
	= &SZ_(to_string);
#endif /* expect --> */
#undef SZ_
#undef SET_TO_STRING
#ifdef SET_TO_STRING_NAME
#undef SET_TO_STRING_NAME
#endif


#endif /* traits --> */


#ifdef SET_EXPECT_TRAIT /* <!-- trait */
#undef SET_EXPECT_TRAIT
#else /* trait --><!-- !trait */
#ifdef SET_TEST
#error No SET_TO_STRING traits defined for SET_TEST.
#endif
#undef SET_NAME
#undef SET_TYPE
#undef SET_UINT
#undef SET_HASH
#undef SET_IS_EQUAL
#ifdef SET_POINTER
#undef SET_POINTER
#endif
#ifdef SET_RECALCULATE
#undef SET_RECALCULATE
#endif
#undef BOX_
#undef BOX_CONTAINER
#undef BOX_CONTENTS
/* box (multiple traits) --> */
#endif /* !trait --> */
#undef SET_TO_STRING_TRAIT
#undef SET_TRAITS
