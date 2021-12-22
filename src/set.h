/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Unordered hash set

 ![Example of <string>set.](../web/set.png)

 <tag:<S>set> is a hash set of unordered <tag:<PS>key> that doesn't allow
 duplication. It must be supplied a hash function and equality function. It
 accepts null.

 This code is simple by design. While enclosing a pointer <tag:<PS>key> in a
 larger `struct` can give an associative array, compile-time constant sets are
 better handled with [gperf](https://www.gnu.org/software/gperf/). Also,
 [CMPH](http://cmph.sourceforge.net/) is a minimal perfect hashing library that
 may be more useful for large projects.

 @param[SET_NAME, SET_TYPE]
 `<S>` that satisfies `C` naming conventions when mangled and a valid
 <typedef:<PS>type> associated therewith; required. `<PS>` is private, whose
 names are prefixed in a manner to avoid collisions; any should be re-defined
 prior to use elsewhere.

 @param[SET_HASH, SET_IS_EQUAL]
 A function satisfying <typedef:<PS>SET_HASH> and <typedef:<PS>is_equal_fn>;
 required.

 @param[SET_UINT]
 This is <typedef:<PS>uint>, the unsigned hash type, and defaults to `size_t`.

 @param[SET_RECALCULATE]
 Don't cache the hash, but calculate every time; this avoids storing
 <typedef:<PS>uint> _per_ entry, but can be slower when the hash is
 non-trivial to compute.

 @param[SET_INVERSE_HASH]
 Function satisfying <typedef:<PS>inverse_SET_HASH> that avoids storing the key,
 but calculates it from the hashed value. As such, incompatible with
 `SET_RECALCULATE`.

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
#if defined(SET_RECALCULATE) && defined(SET_INVERSE_HASH)
#error SET_INVERSE_HASH has to cache the hash; conflicts with SET_RECALCULATE.
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
#define SET_IDLE { 0, 0, 0, 0, POOL_IDLE }
#endif /* idempotent --> */


#if SET_TRAITS == 0 /* <!-- base code */


#ifndef SET_UINT
#define SET_UINT size_t
#endif

/** Unsigned integer type used for hash values. Set this according to
 <typedef:<PS>hash_fn>. Half the range of this type presents an upper-limit to
 how many entries are possible. */
typedef SET_UINT PS_(uint);

/** This is the same type as <typedef:<PS>uint>, but one bit is reserved, thus
 has half the range. */
typedef SET_UINT PS_(uint_token);

/** Valid tag type defined by `SET_TYPE`. */
typedef SET_TYPE PS_(type);

/** Used on read-only; including `const` annotation in `SET_TYPE` is not
 supported, (yet.) */
typedef const SET_TYPE PS_(ctype);

/** A map from <typedef:<PS>ctype> onto <typedef:<PS>uint>. Usually should use
 all the the argument and output should be as close as possible to a discrete
 uniform distribution. It is up to the user to provide an appropriate hash
 function. In general, see: <https://github.com/skeeto/hash-prospector>,
 <https://github.com/aappleby/smhasher/>,
 <https://github.com/sindresorhus/fnv1a>. */
typedef PS_(uint) (*PS_(hash_fn))(PS_(ctype));

/* Check that `SET_HASH` is a function implementing <typedef:<PS>SET_HASH>. */
static const PS_(hash_fn) PS_(hash) = (SET_HASH);

#ifdef SET_INVERSE_HASH /* <!-- inv */
/** Defining `SET_INVERSE_HASH` says that the <typedef:<PS>type> forms a
 bijection with <typedef:<PS>uint>; this is inverse-mapping to
 <typedef:<PS>hash_fn>. This saves having to store the <typedef:<PS>type>. */
typedef PS_(type) (*PS_(inverse_hash_fn))(PS_(uint));
#error Not working.
#endif /* inv --> */

/** Equivalence relation between <typedef:<PS>ctype> that satisfies
 `<PS>is_equal_fn(a, b) -> <PS>SET_HASH(a) == <PS>SET_HASH(b)`. */
typedef int (*PS_(is_equal_fn))(const PS_(ctype) a, const PS_(ctype) b);

/* Check that `SET_IS_EQUAL` is a function implementing
 <typedef:<PS>is_equal_fn>. */
static const PS_(is_equal_fn) PS_(equal) = (SET_IS_EQUAL);

/** The structure is like coalesced-hashing: this is an open-addressing scheme,
 in that it stores collision keys in another bucket; like separate-chaining,
 each bucket is a linked-list of keys. However, coalescing is impossible
 because of amortized work on insertion, so it's not that. */
struct PS_(bucket) {
	PS_(uint_token) next;
#ifndef SET_RECALCULATE /* <!-- cache */
	PS_(uint) hash;
#endif /* cache --> */
#ifndef SET_INVERSE_HASH /* <!-- !inv */
	PS_(type) key;
#endif /* !inv --> */
};

/** To initialize, see <fn:<S>hash>, `SET_IDLE`, `{0}` (`C99`,) or being
 `static`.

 ![States.](../web/states.png) */
struct S_(set) {
	struct PS_(bucket) *buckets;
	unsigned log_capacity;
	PS_(uint_token) top;
};

/* We cannot check for null in <typedef:<PS>type> because it possibly has no
 null element. We could require it to, or require a pointer, but this is
 potentially too useful in some sets; or write different cases or have
 `is_full` field in bucket. Design decision to shrink the range by half and use
 one bit from all the <typedef:<PS>info_uint> fields; if one needs a lot of
 entries, one is going to have to use a size bigger <typedef:<PS>uint>. We
 could use signed values, but that would probably be very confusing to the
 user. */

/** Null-check for `token`. */
static int PS_(token_exists)(const PS_(uint_token) token)
	{ return !token; }

/** Does `token` have a next? <fn:<PS>token_next> -> <fn:<PS>token_exists> */
static int PS_(token_has_next)(const PS_(uint_token) token)
	{ return token & 1; }

/** Gets the <typedef:<PS>uint> value of `token`. !<fn:<PS>token_exists> -> 0 */
static PS_(uint) PS_(token_value)(const PS_(uint_token) token)
	{ return token >> 1; }

/** Gets the hash of an occupied `bucket`, which should be consistent. */
static PS_(uint) PS_(bucket_hash)(const struct PS_(bucket) *const bucket) {
	assert(bucket && PS_(token_exists)(bucket->next));
#ifdef SET_RECALCULATE /* <!-- !cache */
	return PS_(hash)(&bucket->data);
#else /* !cache --><!-- cache */
	return bucket->hash;
#endif /* cache --> */
}

/** Gets the key of an occupied `bucket`. */
static PS_(type) PS_(bucket_key)(const struct PS_(bucket) *const bucket) {
	assert(bucket && PS_(token_exists)(bucket->next));
#ifdef SET_INVERSE_HASH
	return PS_(inverse_hash_fn)(&bucket->hash);
#else
	return bucket->key;
#endif
}

/** @return Indexes a bucket from `set` given the `hash`. */
static PS_(uint) PS_(hash_to_index)(struct S_(set) *const set,
	const PS_(uint) hash)
	{ return hash & ((1 << set->log_capacity) - 1); }

/** `to_bucket` will be search linearly for `key`.
 @param[hash] Must match the hash of `key`.
 @fixme Fix for inverse. */
static PS_(type) *PS_(get)(struct S_(set) *const set,
	const PS_(type) key, const PS_(uint) hash) {
	struct PS_(bucket) *bucket;
	PS_(uint_token) next;
	assert(set);
	bucket = set->buckets + PS_(hash_to_index)(set, hash);
	next = bucket->next;
	if(!PS_(token_exists)(next)) return 0; /* This is not an occupied bucket. */
	do {
#ifndef SET_RECALCULATE /* <!-- cache: quick out. */
		if(hash != bucket->hash) continue;
#endif /* cache --> */
		/* fixme: Move to front! */
		if(PS_(equal)(key, bucket->key)) return &bucket->key;
	} while(PS_(token_has_next)(next)
		&& (bucket = set->buckets + PS_(token_value)(next),
		next = bucket->next, 1));
#ifdef SET_RECALCULATE /* <!-- !cache */
	(void)(hash); /* Not used in this case. */
#endif /* !cache --> */
	return 0;
}

/** Ensures that `set` has enough buckets to fill `min_capacity`.
 @param[min_capacity] If zero, does nothing.
 @return Success; otherwise, `errno` will be hash. @throws[ERANGE] Tried
 allocating more then can fit in `size_t` or `realloc` doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html).
 @throws[realloc] */
static int PS_(reserve)(struct S_(set) *const set,
	const size_t min_capacity) {
	struct PS_(bucket) *buckets, *b, *b_end;
	const unsigned log_c0 = set->log_capacity,
		log_limit = sizeof(PS_(uint)) * CHAR_BIT - 1;
	unsigned log_c1;
	PS_(uint) c0 = (PS_(uint))1 << log_c0, c1, mask;
	/* One did hash <typedef:<PS>uint> to an unsigned type, right? */
	assert(set && c0 && log_c0 <= log_limit && (log_c0 >= 3 || !log_c0)
		&& (PS_(uint))-1 > 0);
	/* Ca'n't be expanded further; the load factor will increase. */
	if(log_c0 >= log_limit) { assert(log_c0 == log_limit); return 1; }
	{
		if(log_c0 < 3) log_c1 = 3ul,    c1 = 8ul;
		else           log_c1 = log_c0, c1 = c0;
		while(c1 < no_buckets) log_c1++, c1 <<= 1;
	}
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
	for(b = buckets + c0, b_end = buckets + c1; b < b_end; b++) b->head = 0;
	/* Expectation value of rehashing an entry is half, (usually.) */
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
			new_b = PS_(get_bucket)(hash, hash);
			x->next = new_b->head, new_b->head = x;
		}
	}
	return 1;
}

/** A bi-predicate; returns true if the `replace` replaces the `original`. */
typedef int (*PS_(replace_fn))(PS_(type) original, PS_(type) replace);

/** Used in <fn:<S>set_policy_put> when `replace` is null; `original` and
 `replace` are ignored. @implements `<PS>replace_fn` */
static int PS_(false)(PS_(type) original, PS_(type) replace)
	{ (void)(original); (void)(replace); return 0; }

/** Put `key` in `hash` as long as `replace` is null or returns true.
 @param[collide] If non-null, the collided element, if any. If `replace`
 returns false, the address of `key`.
 @return Success. @throws[malloc] */
static int PS_(put)(struct S_(set) *const set, const PS_(replace_fn) replace,
	PS_(type) key, PS_(type) *collide) {
	struct PS_(bucket) *bucket;
	struct PS_(entry) **to_x = 0, *x = 0;
	PS_(uint) hash;
	assert(set && key);
	hash = PS_(hash)(key);
	if(collide) *collide = 0;
	if(!set->buckets) goto grow_table;
	bucket = PS_(get_bucket)(set, set);
	if(!(to_x = PS_(bucket_link)(bucket, set, key))) goto grow_table;
	x = *to_x; /* Deal with duplicates. */
	if(replace && !replace(x->key, key))
		{ if(collide) *collide = key; return 1; }
	if(collide) *collide = x->key;
	*to_x = x->next;
	goto add_element;
grow_table:
	if((hash->entry_size < (size_t)-1
		&& !PS_(bucket_reserve)(hash, hash->entry_size + 1))
		|| !(x = PS_(new_entry)(hash))) return 0;
	bucket = PS_(get_bucket)(set, set); /* Possibly invalidated. */
add_element:
	x->key = key;
	x->next = bucket->head, bucket->head = x;
#ifndef SET_RECALCULATE /* <!-- cache: quick out. */
	x->hash = hash;
#endif /* cache --> */
	return 1;
}

/** Initialises `hash` to idle. @order \Theta(1) @allow */
static void S_(set)(struct S_(set) *const hash)
	{ assert(hash); hash->buckets = 0; hash->entry_size = 0; hash->log_capacity = 0;
	PS_(entry_pool)(&hash->entries); }

/** Destroys `hash` and returns it to idle. @allow */
static void S_(set_)(struct S_(set) *const hash)
	{ assert(hash), free(hash->buckets), PS_(entry_pool_)(&hash->entries);
	S_(set)(hash); }

/** Clears and removes all entries from `hash`. The capacity and memory of the
 hash table is preserved, but all previous values are un-associated. The load
 factor will be less until it reaches it's previous size.
 @param[hash] If null, does nothing. @order \Theta(`hash.buckets`) @allow */
static void S_(set_clear)(struct S_(set) *const hash) {
	struct PS_(bucket) *b, *b_end;
	assert(hash);
	if(!hash->log_capacity) return;
	for(b = hash->buckets, b_end = b + (1 << hash->log_bucket_size); b < b_end; b++)
		b->head = 0;
	hash->entry_size = 0;
	PS_(entry_pool_clear)(&hash->entries);
}

/** @return The value in `hash` which <typedef:<PS>is_equal_fn> `SET_IS_EQUAL`
 `key`, or, if no such value exists, null.
 @order Average \O(1), (hash distributes elements uniformly); worst \O(n).
 @allow */
static PS_(type) S_(set_get)(struct S_(set) *const hash, const PS_(type) key) {
	struct PS_(entry) **to_x;
	PS_(uint) hash;
	assert(hash);
	if(!hash->buckets) return 0;
	hash = PS_(set)(key);
	to_x = PS_(bucket_link)(PS_(get_bucket)(hash, hash), hash, key);
	return to_x ? (*to_x)->key : 0;
}

#if 0
/** Reserve at least `reserve`, divided by the maximum load factor, space in
 the buckets of `hash`. @return Success.
 @throws[ERANGE] `reserve` plus the size would take a bigger number then could
 fit in a `size_t`. @throws[realloc] @allow */
static int S_(set_reserve(buffer?))(struct S_(set) *const hash, const size_t reserve)
	{ return hash ? reserve > (size_t)-1 - hash->size ? (errno = ERANGE, 0) :
	PS_(reserve)(hash, hash->size + reserve) : 0; }
#endif

/** Puts `key` in `hash`.
 @return Any ejected key or null.
 @throws[realloc, ERANGE] There was an error with a re-sizing. It is not
 always possible to tell the difference between an error and a unique key.
 Successfully calling <fn:<S>set_buffer> before ensures that this does not
 happen, setting `errno` to zero before also distinguishes.
 @order Average amortised \O(1), (hash distributes keys uniformly); worst \O(n). @allow */
static PS_(type) S_(set_put)(struct S_(set) *const hash, const PS_(type) key) {
	PS_(type) collide;
	/* No error information. */
	return PS_(put)(hash, 0, key, &collide) ? collide : 0;
}

/** Puts `key` in `hash` only if the entry is absent or if calling `replace`
 returns true.
 @param[replace] If null, doesn't do any replacement on collision.
 @return Any ejected element or null. On collision, if `replace` returns false
 or `replace` is null, returns `key` and leaves the other element in the hash.
 @throws[realloc, ERANGE] There was an error with a re-sizing.
 Successfully calling <fn:<S>set_reserve> ensures that this does not happen.
 @order Average amortised \O(1), (hash distributes keys uniformly); worst \O(n). @allow */
static PS_(type) S_(set_policy_put)(struct S_(set) *const hash,
	const PS_(type) key, const PS_(replace_fn) replace) {
	PS_(type) collide;
	/* No error information. */
	return PS_(put)(hash, replace ? replace : &PS_(false), key, &collide)
		? collide : 0;
}

#if 0
/** Removes an element `data` from `hash`.
 @return Successfully ejected element or null. @order Average \O(1), (hash
 distributes elements uniformly); worst \O(n). @allow */
static struct S_(setlink) *S_(set_remove)(struct S_(set) *const hash,
	const PS_(mtype) data) {
	PS_(uint) hash;
	struct S_(setlink) **to_x, *x;
	if(!hash || !hash->buckets) return 0;
	hash = PS_(set)(data);
	if(!(to_x = PS_(bucket_to)(PS_(get_bucket)(hash, hash), hash, data)))
		return 0;
	x = *to_x;
	*to_x = x->next;
	assert(hash->size);
	hash->size--;
	return x;
}
#endif

/* <!-- iterate interface */

/* Contains all iteration parameters. */
struct PS_(iterator)
	{ const struct S_(set) *hash; size_t bucket_idx; struct PS_(entry) *entry; };

/** Loads `hash` into `it`. @implements begin */
static void PS_(begin)(struct PS_(iterator) *const it,
	const struct S_(set) *const hash)
	{ assert(it && hash), it->hash = hash, it->bucket_idx = 0, it->entry = 0; }

/** Advances `it`. @implements next */
static PS_(type) *PS_(next)(struct PS_(iterator) *const it) {
	const size_t bucket_end = 1 << it->hash->log_capacity;
	assert(it && it->hash);
	if(!it->hash->buckets) return 0;
	while(it->bucket_idx < bucket_end) {
		if(!it->entry) it->entry = it->hash->buckets[it->bucket_idx].head;
		else it->entry = it->entry->next;
		if(it->entry) return &it->entry->key;
		it->bucket_idx++;
	}
	it->hash = 0, it->bucket_idx = 0, it->entry = 0;
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
	/*S_(set_reserve)(0, 0);*/ S_(set_put)(0, 0);  S_(set_policy_put)(0, 0, 0);
	/*S_(set_remove)(0, 0);*/
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
