/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Unordered hash set

 ![Example of <string>set.](../web/set.png)

 <tag:<S>set> is a hash set of unordered <tag:<PS>key> that doesn't allow
 duplication. It must be supplied a hash function and equality function.

 This code is simple by design, and may not be suited for more complex
 situations. While enclosing a pointer <tag:<PS>key> in a larger `struct` can
 give an associative array, compile-time constant sets are better handled with
 [gperf](https://www.gnu.org/software/gperf/). Also,
 [CMPH](http://cmph.sourceforge.net/) is a minimal perfect hashing library that
 may be more useful, especially for large sets.

 @param[SET_NAME, SET_TYPE]
 `<S>` that satisfies `C` naming conventions when mangled and a valid
 <typedef:<PS>type> associated therewith; required. Type is copied extensively,
 so if it's a large, making it a pointer may improve performance. `<PS>` is
 private, whose names are prefixed in a manner to avoid collisions; any should
 be re-defined prior to use elsewhere.

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
#error SET_INVERSE_HASH has to store the hash; conflicts with SET_RECALCULATE.
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
#define SET_IDLE { 0, 0, 0, 0 }
/* Use negative values of <typedef:<PS>uint> to store special things. (These
 work on mathematically-impaired representations, such that range of an index
 is 2 less than the maximum; in practice, it causes one bit loss anyway, so we
 might as well have defined them (PS_(uint))-1.) */
#define SETm1 ((PS_(uint))~(PS_(uint))0)
#define SETm2 (SETm1 - 1)
#endif /* idempotent --> */


#if SET_TRAITS == 0 /* <!-- base code */


#ifndef SET_UINT
#define SET_UINT size_t
#endif

/** Unsigned integer type used for hash values as well as placing a limit on
 how many items can be in this set. <typedef:<PS>hash_fn> returns this type. */
typedef SET_UINT PS_(uint);

/** Valid tag type defined by `SET_TYPE`. */
typedef SET_TYPE PS_(type);
/** Used on read-only. @fixme Including `const` qualifier in `SET_TYPE` is not
 supported and will lead to errors. */
typedef const SET_TYPE PS_(ctype);

/** A map from <typedef:<PS>ctype> onto <typedef:<PS>uint>. Usually should use
 all the the argument and output should be as close as possible to a discrete
 uniform distribution. It is up to the user to provide an appropriate hash
 function. In general, see: <https://github.com/skeeto/hash-prospector>,
 <https://github.com/aappleby/smhasher/>,
 <https://github.com/sindresorhus/fnv1a>. */
typedef PS_(uint) (*PS_(hash_fn))(PS_(ctype));
/* Check that `SET_HASH` is a function implementing <typedef:<PS>hash_fn>. */
static const PS_(hash_fn) PS_(hash) = (SET_HASH);

#ifdef SET_INVERSE_HASH /* <!-- inv */
/** Defining `SET_INVERSE_HASH` says that the <typedef:<PS>type> forms a
 bijection with <typedef:<PS>uint>; this is inverse-mapping to
 <typedef:<PS>hash_fn>. This saves having to store the <typedef:<PS>type>. */
typedef PS_(type) (*PS_(inverse_hash_fn))(PS_(uint));
#error Fixme. Think about how to iterate. Maybe `it` has a temp field?
#endif /* inv --> */

/** Equivalence relation between <typedef:<PS>ctype> that satisfies
 `<PS>is_equal_fn(a, b) -> <PS>SET_HASH(a) == <PS>SET_HASH(b)`. */
typedef int (*PS_(is_equal_fn))(const PS_(ctype) a, const PS_(ctype) b);
/* Check that `SET_IS_EQUAL` is a function implementing
 <typedef:<PS>is_equal_fn>. */
static const PS_(is_equal_fn) PS_(equal) = (SET_IS_EQUAL);

/** Although one entry holds at most one key, like open hashing, (thus the load
 factor ca'n't go above one,) the buckets are linked. (However, there is no
 coalescing.) */
struct PS_(entry) {
	PS_(uint) next; /* -2 null, -1 end */
#ifndef SET_RECALCULATE /* <!-- cache */
	PS_(uint) hash;
#endif /* cache --> */
#ifndef SET_INVERSE_HASH /* <!-- !inv */
	PS_(type) key;
#endif /* !inv --> */
};

/** Fill `entry` with `key` and `hash`. The entry must be empty. */
static void PS_(fill_entry)(struct PS_(entry) *const entry,
	const PS_(type) key, const PS_(uint) hash) {
	assert(entry && entry->next == SETm2);
#ifndef SET_RECALCULATE /* <!-- cache */
	entry->hash = hash;
#else /* cache --><!-- !cache */
	(void)hash;
#endif /* !cache --> */
#ifndef SET_INVERSE_HASH /* <!-- !inv */
	entry->key = key;
#else /* !inv --><!-- inv */
	(void)key;
#endif /* inv --> */
	/* next of -1: _this_ entry's valid, but the next one isn't. */
	entry->next = SETm1;
}

/** Gets the hash of an occupied `entry`, which should be consistent. */
static PS_(uint) PS_(entry_hash)(const struct PS_(entry) *const entry) {
	assert(entry && entry->next != SETm2);
#ifdef SET_RECALCULATE
	return PS_(hash)(&entry->data);
#else
	return entry->hash;
#endif
}

/** Gets the key of an occupied `entry`. */
static PS_(type) PS_(entry_key)(const struct PS_(entry) *const entry) {
	assert(entry && entry->next != SETm2);
#ifdef SET_INVERSE_HASH
	return PS_(inverse_hash_fn)(&entry->hash);
#else
	return entry->key;
#endif
}

/** To initialize, see <fn:<S>hash>, `SET_IDLE`, `{0}` (`C99`,) or being
 `static`.

 ![States.](../web/states.png) */
struct S_(set) {
	struct PS_(entry) *entries; /* @ has zero/one key. */
	PS_(uint) size; /* How many keys, <= capacity. */
	PS_(uint) top; /* -1 no stack; collided entries growing from the back. */
	unsigned log_capacity; /* Applies to entries. */
};

/** The capacity of a non-idle `set` is always a power-of-two. */
static PS_(uint) PS_(capacity)(const struct S_(set) *const set)
	{ return assert(set && set->entries && set->log_capacity >= 3),
	(PS_(uint))1 << set->log_capacity; }

/** @return Indexes an entry from non-idle `set` given the `hash`. */
static PS_(uint) PS_(hash_to_index)(const struct S_(set) *const set,
	const PS_(uint) hash) { return hash & (PS_(capacity)(set) - 1); }

/** Moves the index `src` to the top of the collision stack in non-idle
 `set`. */
static void PS_(move_to_top)(struct S_(set) *const set, const PS_(uint) x) {
	struct PS_(entry) *dst, *src;
	PS_(uint) top, link/*, next*/;
	assert(set && set->size < PS_(capacity)(set) && x < PS_(capacity)(set));
	/* Search for an empty entry. Amortized: `n` decrements for `n` entries. */
	if((top = set->top) == SETm1) top = PS_(capacity)(set) - 1;
	else assert(top), top--; /* By size < capacity. */;
	while(set->entries[top].next) assert(top), top--;
	dst = set->entries + (set->top = top) - 1, src = set->entries + x;
	/* Occupied to unoccupied. */
	assert(dst->next == SETm2 && src->next != SETm2);
	/* Search for the previous link in the linked-list. */
	link = PS_(hash_to_index)(set, PS_(entry_hash)(src));

	printf("foo\n");
	struct PS_(entry) {
		PS_(uint) next;
	#ifndef SET_RECALCULATE /* <!-- cache */
		PS_(uint) hash;
	#endif /* cache --> */
	#ifndef SET_INVERSE_HASH /* <!-- !inv */
		PS_(type) key;
	#endif /* !inv --> */
	};
}

/** `set` will be searched linearly for `key` which has `hash`.
 @fixme Fix for inverse. @fixme Move to front? */
static struct PS_(entry) *PS_(get)(struct S_(set) *const set,
	const PS_(type) key, const PS_(uint) hash) {
	struct PS_(entry) *entry;
	PS_(uint) idx, next;
	assert(set && set->entries && set->log_capacity);
	entry = set->entries + (idx = PS_(hash_to_index)(set, hash));
	/* Not the start of a bucket: empty or in the collision stack. */
	if((next = entry->next) == SETm2
		|| set->top != SETm1 && set->top <= idx /* In range of stack. */
		&& idx != PS_(hash_to_index)(set, PS_(entry_hash)(entry))) return 0;
	for( ; ; ) {
#ifdef SET_RECALCULATE /* <!-- !cache: always go to next predicate. */
		const int hashes_are_equal = ((void)(hash), 1);
#else /* !cache --><!-- cache: quick out. */
		const int hashes_are_equal = hash == entry->hash;
#endif /* cache --> */
		if(hashes_are_equal && PS_(equal)(key, entry->key)) return entry;
		if(next == SETm1) return 0; /* -1 used to end the bucket. */
		idx = next;
		/* We are now in the collision stack. */
		assert(set->top <= idx && idx < 1 << set->log_capacity);
		entry = set->entries + idx;
		next = entry->next;
		assert(next != SETm2); /* -2 null: linked-list integrity. */
	}
}

/** Ensures that `set` has enough entries to fill `n` more than the size.
 @return Success; otherwise, `errno` will be hash. @throws[ERANGE] Tried
 allocating more then can fit in half <typedef:<PS>uint> or `realloc` doesn't
 follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html).
 @throws[realloc] */
static int PS_(buffer)(struct S_(set) *const set, const PS_(uint) n) {
	struct PS_(entry) *entries, *e, *e_end;
	/* fixme: sizeof does not lead to the correct results */
	const unsigned log_c0 = set->log_capacity,
		log_limit = sizeof(PS_(uint)) * CHAR_BIT - 1;
	unsigned log_c1;
	const PS_(uint) max = (PS_(uint))1 << log_limit;
	PS_(uint) c0 = (PS_(uint))1 << log_c0, c1, size1, i;
	assert(set && c0 && log_c0 <= log_limit
		&& (log_c0 >= 3 && set->entries || !log_c0 && !set->entries)
		&& n <= SETm1 && set->size <= SETm1 && max <= SETm1);
	printf("buffer: max %lu, limit %lu, entries %lu/%lu, new %lu\n",
		(unsigned long)SETm1, (unsigned long)max,
		(unsigned long)set->size, (unsigned long)c0, (unsigned long)n);
	printf("but: %lu\n", (unsigned long)(SETm2 - SETm1));
	if(SETm1 - set->size < n || max < (size1 = set->size + n))
		return errno = ERANGE, 0;
	if(set->entries) log_c1 = log_c0, c1 = c0;
	else             log_c1 = 3,      c1 = 8;
	while(c1 < size1) log_c1++, c1 <<= 1;
	if(log_c0 == log_c1) return 1;
	/* Need to allocate more. */
	if(!(entries = realloc(set->entries, sizeof *entries * c1)))
		{ if(!errno) errno = ERANGE; return 0; }
	set->entries = entries, set->log_capacity = log_c1;
	/* Want zero for initialization of extra entries and stack is re-done. */
	if(c0 == 1) c0 = 0, assert(!c0 || c0 >= 8);
	for(e = entries + c0, e_end = entries + c1; e < e_end; e++) e->next = SETm2;
	set->top = SETm1;
	printf("buffer: rehash %lu entries\n", (unsigned long)c0);
	/* Expectation value of rehashing a closed entry is the growth amount. */
	for(i = 0; i < c0; i++) {
		struct PS_(entry) *f;
		PS_(uint) hash, j;
		e = entries + i;
		if(e->next == SETm2) { /* Empty. */
			assert(n > 1 /* Must have been asking more, otherwise full. */
				&& (set->top == SETm1 || i < set->top) /* Stack full. */);
			printf("\t%lu: empty\n", (unsigned long)i);
			continue; /* Empty entry. */
		}
		if((j = PS_(hash_to_index)(set, hash = PS_(entry_hash)(e))) == i)
			{ printf("\t%lu: right where it's supposed to be\n",
			(unsigned long)i);
			continue; /* Right where it's supposed to be. */ }
		f = entries + j, assert(e < f); /* Rehash e=>f = entries + i=>j. */
		if(f->next == SETm2) { /* One we're moving it to is unoccupied. */
			PS_(fill_entry)(f, e->key, e->hash);
			e->next = SETm2; /* Now this is unoccupied; the stack is further. */
			printf("\t%lu: moved to unoccupied %lu\n",
				(unsigned long)i, (unsigned long)j);
			continue; /* Moved the entry to an unoccupied spot. */
		}
		/* Collision. */
		assert(0);
	}
	return 1;
}

/** A bi-predicate; returns true if the `replace` replaces the `original`. */
typedef int (*PS_(replace_fn))(PS_(type) original, PS_(type) replace);

/** Used in <fn:<S>set_policy_put> when `replace` is null; `original` and
 `replace` are ignored. @implements `<PS>replace_fn` */
static int PS_(false)(PS_(type) original, PS_(type) replace)
	{ (void)(original); (void)(replace); return 0; }

/* **********fixme*/
static void (*PS_(to_string))(const PS_(type) *, char (*)[12]);

/** Put `key` in `hash` as long as `replace` is null or returns true.
 @param[equal] If non-null, the equal element, if any. If `replace`
 returns false, the address of `key`.
 @return Success. @throws[malloc] @order amortized \O(1) */
static int PS_(put)(struct S_(set) *const set, const PS_(replace_fn) replace,
	PS_(type) key, PS_(type) *eject) {
	struct PS_(entry) *entry;
	PS_(uint) hash, idx, next = SETm1; /* The end of a linked-list. */
	char z[12];
	assert(set && key);
	PS_(to_string)(&key, &z);
	if(eject) *eject = 0;
	hash = PS_(hash)(key);
	if(set->entries && (entry = PS_(get)(set, key, hash))) goto replace;
	else goto expand;
replace:
	printf("put(%s)::replace\n", z);
	if(replace && !replace(PS_(entry_key)(entry), key))
		{ if(eject) *eject = key; return 1; }
	if(eject) *eject = PS_(entry_key)(entry);
	/* Cut the tail and put new element in the head. */
	next = entry->next, entry->next = SETm2, assert(next != SETm2);
	goto write;
expand:
	if(!PS_(buffer)(set, 1)) return 0; /* Amortized. */
	entry = set->entries + (idx = PS_(hash_to_index)(set, hash));
	printf("put::expand: index 0x%lu, hash 0x%lx, data %s\n",
		(unsigned long)idx, (unsigned long)hash, z);
	set->size++;
	if(entry->next == SETm2) goto write; /* Unoccupied. */
	PS_(move_to_top)(set, idx);
	/*...*/
	assert(entry->next != SETm2);
write:
	PS_(fill_entry)(entry, key, hash); /* fixme: Robin-hood. */
	entry->next = next;
	return 1;
}

/** Initialises `set` to idle. @order \Theta(1) @allow */
static void S_(set)(struct S_(set) *const set) { assert(set); set->entries = 0;
	set->log_capacity = 0; set->size = 0; set->top = 0; }

/** Destroys `set` and returns it to idle. @allow */
static void S_(set_)(struct S_(set) *const set)
	{ assert(set), free(set->entries); S_(set)(set); }

/** Clears and removes all entries from `hash`. The capacity and memory of the
 hash table is preserved, but all previous values are un-associated. The load
 factor will be less until it reaches it's previous size.
 @order \Theta(`set.entries`) @allow */
static void S_(set_clear)(struct S_(set) *const set) {
	struct PS_(entry) *b, *b_end;
	assert(set);
	if(!set->entries) { assert(!set->log_capacity); return; }
	for(b = set->entries, b_end = b + (1 << set->log_capacity); b < b_end; b++)
		b->next = SETm2;
	set->size = 0;
}

/** @return The value in `hash` which <typedef:<PS>is_equal_fn> `SET_IS_EQUAL`
 `key`, or, if no such value exists, null.
 @order Average \O(1), (hash distributes elements uniformly); worst \O(n).
 @allow */
static PS_(type) S_(set_get)(struct S_(set) *const set, const PS_(type) key) {
	struct PS_(entry) *b;
	assert(set);
	if(!set->entries) { assert(!set->log_capacity); return 0; }
	b = PS_(get)(set, key, PS_(hash)(key));
	return b ? PS_(entry_key)(b) : 0;
}

#if 0
/** Reserve at least `reserve`, divided by the maximum load factor, space in
 the entries of `hash`. @return Success.
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
	if(!hash || !hash->entries) return 0;
	hash = PS_(set)(data);
	if(!(to_x = PS_(entry_to)(PS_(get_entry)(hash, hash), hash, data)))
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
struct PS_(iterator) { const struct S_(set) *set; PS_(uint) idx; };

/** Loads `set` (can be null) into `it`. @implements begin */
static void PS_(begin)(struct PS_(iterator) *const it,
	const struct S_(set) *const set)
	{ assert(it), it->set = set, it->idx = 0; }

/** Advances `it`. @implements next */
static PS_(type) *PS_(next)(struct PS_(iterator) *const it) {
	PS_(uint) entry_end;
	assert(it);
	if(!it->set || !it->set->entries) return 0;
	entry_end = 1 << it->set->log_capacity;
	while(it->idx < entry_end) {
		struct PS_(entry) *b = it->set->entries + it->idx;
		it->idx++;
		if(b->next != SETm2) return &b->key; /* Fixme! how to return address? */
	}
	it->set = 0, it->idx = 0;
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
