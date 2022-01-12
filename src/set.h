/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Hash table

 ![Example of <string>hash.](../web/hash.png)

 <tag:<S>set> is an unordered set or map (associative array) of
 <typedef:<PS>bucket> implemented as a hash table. It must be supplied a hash
 and equality function.

 [CMPH](http://cmph.sourceforge.net/) is a minimal perfect hashing library
 that provides performance for large sets. Compile-time constant sets are
 can be better handled with [gperf](https://www.gnu.org/software/gperf/).

 @param[SET_NAME, SET_KEY]
 `<S>` that satisfies `C` naming conventions when mangled and a valid
 <typedef:<PS>key> associated therewith; required. `<PS>` is private, whose
 names are prefixed in a manner to avoid collisions; any should be re-defined
 prior to use elsewhere.

 @param[SET_VALUE]
 An optional type that is the payload of the set key, thus making this an
 associative array. Should be used when a map is desired and the key and value
 are associated but in independent memory locations; if the key is part of an
 aggregate value, it will be more efficient and more robust to use a type
 conversion from the key.

 @param[SET_CODE, SET_IS_EQUAL]
 A function satisfying <typedef:<PS>code_fn> and <typedef:<PS>is_equal_fn>;
 required.

 @param[SET_UINT]
 This is <typedef:<PS>uint>, the unsigned type of hash code of the key given by
 <typedef:<PS>code_fn>; defaults to `size_t`.

 @param[SET_NO_CACHE]
 Calculate every time; this avoids storing <typedef:<PS>uint> _per_ bucket, but
 can be slower when the hash code is non-trivial to compute.

 @param[SET_INVERSE]
 Function satisfying <typedef:<PS>inverse_code_fn> that avoids storing the key,
 but calculates it from the hashed value. As such, incompatible with
 `SET_NO_CACHE`.

 @param[SET_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[SET_TO_STRING]
 To string trait contained in <to_string.h>; `<SZ>` that satisfies `C` naming
 conventions when mangled and function implementing
 <typedef:<PSZ>to_string_fn>. There can be multiple to string traits, but only
 one can omit `SET_TO_STRING_NAME`.

 @std C89 */
 /* Bug in cdoc. @fixme Implement move-to-front like splay-trees.
 @fixme Implement SET_NO_CACHE and SET_INVERSE. */

#if !defined(SET_NAME) || !defined(SET_KEY) || !defined(SET_IS_EQUAL) \
	|| !defined(SET_CODE)
#error Name SET_NAME, tag type SET_KEY, fns SET_IS_EQUAL, or SET_CODE, undefined.
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
#if defined(SET_NO_CACHE) && defined(SET_INVERSE)
#error SET_INVERSE has to store the hash code; conflicts with SET_NO_CACHE.
#endif
#if defined(SET_TO_STRING_NAME) && !defined(SET_TO_STRING)
#error SET_TO_STRING_NAME requires SET_TO_STRING.
#endif

#ifndef SET_H /* <!-- idempotent */
#define SET_H
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#if defined(SET_CAT_) || defined(SET_CAT) || defined(S_) || defined(PS_) \
	|| defined(SET_IDLE)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define SET_CAT_(n, m) n ## _ ## m
#define SET_CAT(n, m) SET_CAT_(n, m)
#define S_(n) SET_CAT(SET_NAME, n) /* H is taken by heap; M map. */
#define PS_(n) SET_CAT(hash, S_(n))
#define SET_IDLE { 0, 0, 0, 0, 0 }
/* When a <typedef:<PS>uint> represents an address in the table, use the sign
 bit to store out-of-band flags, (such that range of an index is one bit less.)
 Choose representations that probably save power, since there are potently
 lots. We cannot save this in an `enum` because don't know maximum. */
#define SET_M1 ((PS_(uint))~(PS_(uint))0) /* 2's compliment -1. */
#define SET_LIMIT ((SET_M1 >> 1) + 1) /* Cardinality. */
#define SET_END (SET_LIMIT)
#define SET_NULL (SET_LIMIT + 1)
#endif /* idempotent --> */


#if SET_TRAITS == 0 /* <!-- base code */


#ifndef SET_UINT
#define SET_UINT size_t
#endif

/** Unsigned integer hash code; <typedef:<PS>code_fn> returns this type. Also
 places a simplifying limit on the maximum number of items in this hash table
 of half the cardinality of this type. */
typedef SET_UINT PS_(uint);

/** Valid tag type defined by `SET_KEY`. */
typedef SET_KEY PS_(key);
/** Used on read-only. This code makes the simplifying assumption that this is
 not `const`-qualified. */
typedef const SET_KEY PS_(ckey);

/** A map from <typedef:<PS>ckey> onto <typedef:<PS>uint>. In general, a good
 hash should use all the the argument and should as close as possible to a
 discrete uniform distribution. It is up to the user to provide an appropriate
 hash code function. */
typedef PS_(uint) (*PS_(code_fn))(PS_(ckey));
/* Check that `SET_CODE` is a function implementing <typedef:<PS>code_fn>. */
static const PS_(code_fn) PS_(code) = (SET_CODE);

#ifdef SET_INVERSE /* <!-- inv */
/** Defining `SET_INVERSE` says that the <typedef:<PS>key> forms a bijection
 with <typedef:<PS>uint>; this is inverse-mapping to <typedef:<PS>code_fn>.
 Used to avoid storing the key itself. */
typedef PS_(key) (*PS_(inverse_code_fn))(PS_(uint));
static const PS_(inverse_code_fn) PS_(inverse_code) = (SET_INVERSE);
#endif /* inv --> */

/** Equivalence relation between <typedef:<PS>ckey> that satisfies
 `<PS>is_equal_fn(a, b) -> <PS>SET_CODE(a) == <PS>SET_CODE(b)`. */
typedef int (*PS_(is_equal_fn))(PS_(ckey) a, PS_(ckey) b);
/* Check that `SET_IS_EQUAL` is a function implementing
 <typedef:<PS>is_equal_fn>. */
static const PS_(is_equal_fn) PS_(equal) = (SET_IS_EQUAL);

#ifdef SET_VALUE /* <!-- value */
/** Defining `SET_VALUE` creates another bucket for associative maps, otherwise
 it is the same as <typedef:<PS>key>. */
typedef SET_VALUE PS_(value);
/** Defining `SET_VALUE` creates this bucket association from key to value.
 @fixme Ugly. Maybe don't need? */
struct S_(set_map) { PS_(key) key; PS_(value) value; };
/** If `SET_VALUE`, then this is a <tag:<S>hash_map> from <typedef:<PS>ckey>,
 the domain, to <typedef:<PS>value>, the codomain; otherwise, it's a set, and
 this is the same as <typedef:<PS>key>. */
typedef struct S_(set_map) PS_(map);
static PS_(key) PS_(map_key)(PS_(map) map) { return map.key; }
#else /* value --><!-- !value */
typedef PS_(key) PS_(value);
typedef PS_(key) PS_(map);
static PS_(key) PS_(map_key)(PS_(map) map) { return map; }
#endif /* !value --> */

/** Entries are what make up the hash table. A linked bucket is a set of all
 buckets having the same address, that is, hash code mod table capacity. Each
 occupied bucket has a closed bucket (address equals the index) at it's start. */
struct PS_(bucket) {
	PS_(uint) next; /* In bucket, including `SET_NULL` and `SET_END`. */
#ifndef SET_NO_CACHE
	PS_(uint) code;
#endif
#ifndef SET_INVERSE
	PS_(key) key;
#endif
#ifdef SET_VALUE
	PS_(value) value;
#endif
};

/** Gets the code of an occupied `bucket`, which should be consistent. */
static PS_(uint) PS_(entry_code)(const struct PS_(bucket) *const bucket) {
	assert(bucket && bucket->next != SET_NULL);
#ifdef SET_NO_CACHE
	return PS_(code)(bucket->key);
#else
	return bucket->code;
#endif
}

/** Gets the key of an occupied `bucket`. */
static PS_(key) PS_(entry_key)(const struct PS_(bucket) *const bucket) {
	assert(bucket && bucket->next != SET_NULL);
#ifdef SET_INVERSE
	return PS_(inverse_code_fn)(&bucket->code);
#else
	return bucket->key;
#endif
}

static void PS_(entry_to_map)(const struct PS_(bucket) *const bucket,
	PS_(map) *const map) {
	assert(bucket && map);
#ifdef SET_VALUE /* map { <PS>key key; <PS>value value; } */
	map->key = PS_(entry_key)(bucket);
	memcpy(&map->value, &bucket->value, sizeof bucket->value);
#else /* map <PS>key */
	*map = PS_(entry_key)(bucket);
#endif
}

#if 0
/*
typedef SET_KEY PS_(key);
!VALUE typedef PS_(key) PS_(map);
VALUE typedef SET_VALUE PS_(value);
VALUE struct S_(set_map) { PS_(key) key; PS_(value) value; };
VALUE typedef struct S_(set_map) PS_(map);
*/
static void PS_(map_to_entry)(const PS_(map) *const map,
	struct PS_(bucket) *const bucket) {
	bucket->next = SET_END;
#ifndef SET_NO_CACHE /* <!-- cache */
#ifdef SET_VALUE
	bucket->code = PS_(code)(map->key);
#else
	bucket->code = PS_(code)(map);
#endif
#endif /* cache --> */
#ifndef SET_INVERSE /* <!-- !inv */
	PS_(key) key;
#endif /* !inv --> */
#ifdef SET_VALUE
	PS_(value) value;
#endif
#ifdef SET_VALUE /* <!-- value */
#error Unfinished.
#ifdef SET_INVERSE /* <!-- inv */
	bucket->code = PS_(code)(map->key);
#else /* inv --><!-- !inv */
	memcpy(bucket, map, sizeof *map);
#endif /* !inv --> */
#else /* value --><!-- !value */
	assert(sizeof(PS_(key)) == sizeof(PS_(value))
		&& sizeof(PS_(key)) == sizeof(PS_(map)));
#ifdef SET_INVERSE /* <!-- inv */
	bucket = PS_(inverse_code)(map);
#else /* inv --><!-- !inv */
	memcpy(bucket, map, sizeof *map);
#endif /* !inv --> */
#endif /* !value --> */
}
#endif

/** To initialize, see <fn:<S>hash>, `SET_IDLE`, `{0}` (`C99`,) or being
 `static`.

 ![States.](../web/states.png) */
struct S_(set) {
	struct PS_(bucket) *buckets; /* @ has zero/one key specified by `next`. */
	unsigned log_capacity, unused; /* Applies to buckets. fixme: type? */
	PS_(uint) size, top; /* size <= capacity; open stack, including `SET_END`. */
};

/** The capacity of a non-idle `hash` is always a power-of-two. */
static PS_(uint) PS_(capacity)(const struct S_(set) *const hash)
	{ return assert(hash && hash->buckets && hash->log_capacity >= 3),
	(PS_(uint))((PS_(uint))1 << hash->log_capacity); }

/** @return Indexes the first `hash.buckets` (closed) from non-idle `hash`
 given the `code`. */
static PS_(uint) PS_(code_to_entry)(const struct S_(set) *const hash,
	const PS_(uint) code) { return code & (PS_(capacity)(hash) - 1); }

/** This is amortized; every value takes at most one top. On return, the `top`
 of `hash` will be empty, but size is not incremented. */
static void PS_(grow_stack)(struct S_(set) *const hash) {
	PS_(uint) top = hash->top;
	assert(hash && hash->buckets && top);
	top = (top == SET_END ? PS_(capacity)(hash) : top) - 1;
	while(hash->buckets[top].next != SET_NULL) assert(top), top--;
	hash->top = top;
}

/** Is `idx` is `hash` possibly on the stack? */
static int PS_(in_stack_range)(const struct S_(set) *const hash,
	const PS_(uint) idx)
	{ return assert(hash), hash->top != SET_END && hash->top <= idx; }

/***********fixme*/
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)
#ifdef SET_TEST
static void PS_(graph)(const struct S_(set) *const hash, const char *const fn);
static void (*PS_(to_string))(PS_(ckey), char (*)[12]);
#else
static void PS_(graph)(const struct S_(set) *const hash, const char *const fn) {
	(void)hash, (void)fn;
}
static void PS_(to_string)(PS_(ckey) data, char (*z)[12])
	{ (void)data, strcpy(*z, "<key>"); }
#endif

/** Moves the `target` index in the collision stack to the top, in non-idle
 `hash`. This results in an inconsistent state; one is responsible for filling
 that hole and linking it with top. */
static void PS_(move_to_top)(struct S_(set) *const hash,
	const PS_(uint) target) {
	struct PS_(bucket) *tgt, *top;
	PS_(uint) to_next, next;
	const PS_(uint) capacity = PS_(capacity)(hash);
	assert(hash->size < capacity && target < capacity);
	PS_(grow_stack)(hash);
	tgt = hash->buckets + target, top = hash->buckets + hash->top;
	assert(tgt->next != SET_NULL && top->next == SET_NULL);
	/* Search for the previous link in the bucket, \O(|bucket|). */
	for(to_next = SET_NULL,
		next = PS_(code_to_entry)(hash, PS_(entry_code)(tgt));
		assert(next < capacity), next != target;
		to_next = next, next = hash->buckets[next].next);
	/* Move `tgt` to `top`. */
	if(to_next != SET_NULL) hash->buckets[to_next].next = hash->top;
	memcpy(top, tgt, sizeof *tgt), tgt->next = SET_NULL;
}

/** `hash` will be searched linearly for `key` which has `code`.
 @fixme Move to front like splay trees? */
static struct PS_(bucket) *PS_(get)(struct S_(set) *const hash,
	const PS_(key) key, const PS_(uint) code) {
	struct PS_(bucket) *bucket;
	PS_(uint) idx, next;
	assert(hash && hash->buckets && hash->log_capacity);
	bucket = hash->buckets + (idx = PS_(code_to_entry)(hash, code));
	/* Not the start of a bucket: empty or in the collision stack. */
	if((next = bucket->next) == SET_NULL
		|| PS_(in_stack_range)(hash, idx)
		&& idx != PS_(code_to_entry)(hash, PS_(entry_code)(bucket))) return 0;
	for( ; ; ) {
		int codes_are_equal;
#ifdef SET_NO_CACHE
		codes_are_equal = ((void)(code), 1);
#else
		codes_are_equal = code == bucket->code;
#endif
		if(codes_are_equal) {
			int entries_are_equal;
#ifdef SET_INVERSE
			entries_are_equal = ((void)(key), 1);
#else
			entries_are_equal = PS_(equal)(key, bucket->key);
#endif
			if(entries_are_equal) return bucket;
		}
		if(next == SET_END) return 0;
		bucket = hash->buckets + (idx = next);
		assert(idx < PS_(capacity)(hash) && PS_(in_stack_range)(hash, idx) &&
			idx != SET_NULL);
		next = bucket->next;
	}
}

/** Ensures that `hash` has enough buckets to fill `n` more than the size.
 May invalidate `buckets` and re-arrange the order.
 @return Success; otherwise, `errno` will be hash. @throws[realloc]
 @throws[ERANGE] Tried allocating more then can fit in half <typedef:<PS>uint>
 or `realloc` doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html). */
static int PS_(buffer)(struct S_(set) *const hash, const PS_(uint) n) {
	struct PS_(bucket) *buckets;
	const unsigned log_c0 = hash->log_capacity;
	unsigned log_c1;
	const PS_(uint) c0 = log_c0 ? (PS_(uint))((PS_(uint))1 << log_c0) : 0;
	PS_(uint) c1, size1, i, wait, mask;
	char fn[64];
	assert(hash && hash->size <= SET_LIMIT && (!hash->buckets && !hash->size
		&& !log_c0 && !c0 || hash->buckets && hash->size <= c0 && log_c0 >= 3));

	/* Can we satisfy `n` growth from the buffer? */
	if(SET_M1 - hash->size < n || SET_LIMIT < (size1 = hash->size + n))
		return errno = ERANGE, 0;
	if(hash->buckets) log_c1 = log_c0, c1 = c0 ? c0 : 1;
	else              log_c1 = 3,      c1 = 8;
	while(c1 < size1) log_c1++, c1 <<= 1;
	if(log_c0 == log_c1) return 1;

	sprintf(fn, "graph/" QUOTE(SET_NAME) "-resize-%u-%u-a-before.gv",
		log_c0, log_c1), PS_(graph)(hash, fn);

	/* Otherwise, need to allocate more. */
	if(!(buckets = realloc(hash->buckets, sizeof *buckets * c1)))
		{ if(!errno) errno = ERANGE; return 0; }
	hash->top = SET_END; /* Idle `top` initialized or reset. */
	hash->buckets = buckets, hash->log_capacity = log_c1;

	/* Initialize new values. Mask to identify the added bits. */
	{ struct PS_(bucket) *e = buckets + c0, *const e_end = buckets + c1;
		for( ; e < e_end; e++) e->next = SET_NULL; }
	mask = (PS_(uint))((((PS_(uint))1 << log_c0) - 1)
		^ (((PS_(uint))1 << log_c1) - 1));

	/* Recode most closed buckets in the lower half. */
	wait = SET_END;
	for(i = 0; i < c0; i++) {
		struct PS_(bucket) *idx, *go;
		PS_(uint) g, code;
		idx = hash->buckets + i;
		if(idx->next == SET_NULL) continue;
		g = PS_(code_to_entry)(hash, code = PS_(entry_code)(idx));
		/* It's a power-of-two size, so, consistent hashing, `E[old/new]`
		 capacity that a closed bucket will remain where it is. */
		if(i == g) { idx->next = SET_END; continue; }
		if((go = hash->buckets + g)->next == SET_NULL) {
			/* Priority is given to the first closed bucket; simpler later. */
			struct PS_(bucket) *head;
			PS_(uint) h = g & ~mask; assert(h <= g);
			if(h < g && i < h
				&& (head = hash->buckets + h, assert(head->next != SET_NULL),
				PS_(code_to_entry)(hash, PS_(entry_code)(head)) == g)) {
				memcpy(go, head, sizeof *head);
				go->next = SET_END, head->next = SET_NULL;
				/* Fall-though -- the bucket still needs to be put on waiting. */
			} else {
				/* If the new bucket is available and this bucket is first. */
				memcpy(go, idx, sizeof *idx);
				go->next = SET_END, idx->next = SET_NULL;
				continue;
			}
		}
		idx->next = wait, wait = i; /* Push for next sweep. */
	}

	sprintf(fn, "graph/" QUOTE(SET_NAME) "-resize-%u-%u-b-obvious.gv",
		log_c0, log_c1), PS_(graph)(hash, fn);

	/* Search waiting stack for buckets that moved concurrently. */
	{ PS_(uint) prev = SET_END, w = wait; while(w != SET_END) {
		struct PS_(bucket) *waiting = hash->buckets + w;
		PS_(uint) cl = PS_(code_to_entry)(hash, PS_(entry_code)(waiting));
		struct PS_(bucket) *const closed = hash->buckets + cl;
		assert(cl != w);
		if(closed->next == SET_NULL) {
			memcpy(closed, waiting, sizeof *waiting), closed->next = SET_END;
			if(prev != SET_END) hash->buckets[prev].next = waiting->next;
			if(wait == w) wait = waiting->next; /* First, modify head. */
			w = waiting->next, waiting->next = SET_NULL;
		} else {
			assert(closed->next == SET_END); /* Not in the wait stack. */
			prev = w, w = waiting->next;
		}
	}}

	sprintf(fn, "graph/" QUOTE(SET_NAME) "-resize-%u-%u-c-closed.gv",
		log_c0, log_c1), PS_(graph)(hash, fn);

	/* Rebuild the (smaller?) top stack (high) from the waiting (low). */
	while(wait != SET_END) {
		struct PS_(bucket) *const waiting = hash->buckets + wait;
		PS_(uint) h = PS_(code_to_entry)(hash, PS_(entry_code)(waiting));
		struct PS_(bucket) *const head = hash->buckets + h;
		struct PS_(bucket) *top;
		assert(h != wait && head->next != SET_NULL);
		PS_(grow_stack)(hash), top = hash->buckets + hash->top;
		memcpy(top, waiting, sizeof *waiting);
		top->next = head->next, head->next = hash->top;
		wait = waiting->next, waiting->next = SET_NULL; /* Pop. */
	}

	sprintf(fn, "graph/" QUOTE(SET_NAME) "-resize-%u-%u-d-final.gv",
		log_c0, log_c1), PS_(graph)(hash, fn);

	return 1;
}

#undef QUOTE_
#undef QUOTE

/** A bi-predicate; returns true if the `replace` replaces the `original`. */
typedef int (*PS_(replace_fn))(PS_(key) original, PS_(key) replace);

/** Put `key` in `hash` as long as `replace` is null or returns true.
 @param[eject] If non-null, replaced with the equal element, if any. If
 `replace` returns false, the `map`.
 @return True except exception. @throws[malloc] @order amortized \O(1) */
static int PS_(put)(struct S_(set) *const hash, PS_(map) map, PS_(map) *eject,
	const PS_(replace_fn) replace) {
	struct PS_(bucket) *bucket;
	const PS_(key) key = PS_(map_key)(map);
	PS_(uint) code, idx, next = SET_END /* The end of a linked-list. */, size;
	assert(hash);
	code = PS_(code)(key);
	{
		char z[12];
		PS_(to_string)(key, &z);
		printf("put: \"%s\" code 0x%lx.\n", z, (unsigned long)code);
	}
	size = hash->size;
	if(hash->buckets && (bucket = PS_(get)(hash, key, code))) { /* Replace. */
		/* Decided not to replace. */
		if(replace && !replace(PS_(entry_key)(bucket), key))
			{ if(eject) memcpy(eject, &map, sizeof map); return 1; }
		if(eject) PS_(entry_to_map)(bucket, eject);
		/* Cut the tail and put new element in the head. */
		next = bucket->next, bucket->next = SET_NULL, assert(next != SET_NULL);
	} else { /* Expand. */
		if(!PS_(buffer)(hash, 1)) return 0; /* Amortized. */
		bucket = hash->buckets + (idx = PS_(code_to_entry)(hash, code));
		size++;
		if(bucket->next != SET_NULL) { /* Unoccupied. */
			int already_in_stack = PS_(code_to_entry)(hash,
				PS_(entry_code)(bucket)) != idx;
			PS_(move_to_top)(hash, idx);
			next = already_in_stack ? SET_END : hash->top;
			assert(bucket->next == SET_NULL
				&& (next == SET_END || hash->buckets[next].next != SET_NULL));
		}
	}
	/* Fill `bucket`. The bucket must be empty. */
	assert(bucket && bucket->next == SET_NULL);
	bucket->next = next;
	#ifndef SET_NO_CACHE
	bucket->code = code;
	#endif
	#ifndef SET_INVERSE
	memcpy(&bucket->key, &key, sizeof key);
	#endif
	#ifdef SET_VALUE /* <!-- value */
	/*memcpy(&bucket->value, );*/
//#error Pending.
	#endif /* value --> */
	hash->size = size;
	return 1;
}

/** Initialises `hash` to idle. @order \Theta(1) @allow */
static void S_(set)(struct S_(set) *const hash) { assert(hash);
	hash->buckets = 0; hash->log_capacity = 0; hash->size = 0; hash->top = 0; }

/** Destroys `hash` and returns it to idle. @allow */
static void S_(set_)(struct S_(set) *const hash)
	{ assert(hash), free(hash->buckets); S_(set)(hash); }

/** Reserve at least `n` space for buckets of `hash`. This will ensure that
 there is space for those buckets and may increase iteration time.
 @return Success.
 @throws[ERANGE] The request was unsatisfiable. @throws[realloc] @allow */
static int S_(set_buffer)(struct S_(set) *const hash, const PS_(uint) n)
	{ return assert(hash), PS_(buffer)(hash, n); }

/** Clears and removes all buckets from `hash`. The capacity and memory of the
 hash table is preserved, but all previous values are un-associated. (The load
 factor will be less until it reaches it's previous size.)
 @order \Theta(`hash.capacity`) @allow */
static void S_(set_clear)(struct S_(set) *const hash) {
	struct PS_(bucket) *b, *b_end;
	assert(hash);
	if(!hash->buckets) { assert(!hash->log_capacity); return; }
	for(b = hash->buckets, b_end = b + (1 << hash->log_capacity); b < b_end; b++)
		b->next = SET_NULL;
	hash->size = 0;
}

/** @param[map] If non-null, a <typedef:<PS>map> which gets filled on true.
 @return Is `key` in `hash` (which could be null)? */
static int S_(set_query)(struct S_(set) *const hash, const PS_(key) key,
	PS_(map) *const map) {
	struct PS_(bucket) *e;
	if(!hash || !hash->buckets || !(e = PS_(get)(hash, key, PS_(code)(key))))
		return 0;
	if(map) PS_(entry_to_map)(e, map);
	return 1;
}

#if 0
/* fixme: get_or_default (get_or?) /\, otherwise have a get that has a
 parameter, so one could have multiple \/. "get" doesn't work with non-nullible
 types, (in C++ they made it work returning zero? eww.) */

/** @return The value in `hash` which is equal `key`, or, if no such value
 exists, null. @order Average \O(1), (code distributes elements uniformly);
 worst \O(n). @allow */
static PS_(key) S_(set_get)(struct S_(set) *const hash,
	const PS_(key) key) {
	struct PS_(bucket) *e;
	assert(hash);
	if(!hash->buckets) { assert(!hash->log_capacity); return 0; }
	e = PS_(get)(hash, key, PS_(code)(key));
	return e ? PS_(entry_key)(e) : 0;
}
#endif

/** Puts `map` in `hash` only if the bucket is absent or if calling `replace`
 returns true.
 @param[replace] If null, doesn't do any replacement on collision.
 @return Any ejected element or null. On collision, if `replace` returns false
 or `replace` is null, returns `key` and leaves the other element in the code.
 @throws[realloc, ERANGE] There was an error with a re-sizing.
 Successfully calling <fn:<S>hash_buffer> ensures that this does not happen.
 @order Average amortised \O(1), (code distributes keys uniformly); worst \O(n).
 @allow */
static int S_(set_policy_put)(struct S_(set) *const hash, PS_(map) map,
	PS_(map) *eject, const PS_(replace_fn) replace)
	{ return PS_(put)(hash, map, eject, replace); }

#if 0
/* fixme: Buffering changes the outcome if it's already in the table, it
 creates a new code anyway. This is not a pleasant situation. */
/* fixme: also have a hash_try */
/** Puts `key` in `code`, and, for keys already in the code, replaces them.
 @return Any ejected key or null.
 @throws[realloc, ERANGE] There was an error with a re-sizing. It is not
 always possible to tell the difference between an error and a unique key.
 If needed, before calling this, successfully calling <fn:<S>hash_buffer>, or
 hashting `errno` to zero. @order Average amortised \O(1), (code distributes
 keys uniformly); worst \O(n) (are you sure that's up to date?). @allow */
static PS_(map) S_(set_replace)(struct S_(set) *const hash,
	const PS_(map) map) {
	PS_(map) collide;
	/* No error information. */
	return PS_(put)(hash, 0, map, &collide) ? collide : 0;
}

/** Removes an element `data` from `code`.
 @return Successfully ejected element or null. @order Average \O(1), (code
 distributes elements uniformly); worst \O(n). @allow */
static struct S_(setlink) *S_(set_remove)(struct S_(set) *const code,
	const PS_(mtype) data) {
	PS_(uint) code;
	struct S_(setlink) **to_x, *x;
	if(!code || !code->buckets) return 0;
	code = PS_(set)(data);
	if(!(to_x = PS_(entry_to)(PS_(get_entry)(code, code), code, data)))
		return 0;
	x = *to_x;
	*to_x = x->next;
	assert(code->size);
	code->size--;
	return x;
}
#endif

/* <!-- iterate interface */

/* Contains all iteration parameters. */
struct PS_(iterator) { const struct S_(set) *hash; PS_(uint) idx; };

/** Loads `hash` (can be null) into `it`. @implements begin */
static void PS_(begin)(struct PS_(iterator) *const it,
	const struct S_(set) *const hash)
	{ assert(it), it->hash = hash, it->idx = 0; }

/** Helper to skip the buckets of `it` that are not there.
 @return Whether it found another index. */
static int PS_(skip)(struct PS_(iterator) *const it) {
	const struct S_(set) *const hash = it->hash;
	const PS_(uint) limit = PS_(capacity)(hash);
	assert(it && it->hash && it->hash->buckets);
	while(it->idx < limit) {
		struct PS_(bucket) *const e = hash->buckets + it->idx;
		if(e->next != SET_NULL) return 1;
		it->idx++;
	}
	return 0;
}

/** Advances `it`. @implements next */
static struct PS_(bucket) *PS_(next)(struct PS_(iterator) *const it) {
	assert(it);
	if(!it->hash || !it->hash->buckets) return 0;
	if(PS_(skip)(it)) return it->hash->buckets + it->idx++;
	it->hash = 0, it->idx = 0;
	return 0;
}

/* iterate --> */

/** Iteration usually not in any particular order, as it goes by bucket. The
 asymptotic runtime is proportional to the hash capacity. (This will be the
 power-of-two equal-to or greater then the maximum size plus buffering of the
 history of the hash.) */
struct S_(set_iterator) { struct PS_(iterator) it; };

/** Loads `hash` (can be null) into `it`. */
static void S_(set_begin)(struct S_(set_iterator) *const it,
	const struct S_(set) *const hash) { PS_(begin)(&it->it, hash); }

/** @return Whether the hash specified by <fn:<S>hash_begin> has a next bucket. */
static int S_(set_has_next)(struct S_(set_iterator) *const it) {
	assert(it);
	/* <tag:<PS>bucket> is fine for private returning, but <typedef:<PS>key>
	 may not even be nullable. */
	return it->it.hash && it->it.hash->buckets && PS_(skip)(&it->it);
}

/** Advances `it`. @return The next key or zero. */
static PS_(key) S_(set_next_key)(struct S_(set_iterator) *const it) {
	const struct PS_(bucket) *e = PS_(next)(&it->it);
	return e ? PS_(entry_key)(e) : 0;
}

/* fixme: and value, if VALUE? */

/* <!-- box (multiple traits) */
#define BOX_ PS_
#define BOX_CONTAINER struct S_(set)
#define BOX_CONTENTS struct PS_(bucket)

#ifdef SET_TEST /* <!-- test */
/* Forward-declare. */
static void (*PS_(to_string))(PS_(ckey), char (*)[12]);
static const char *(*PS_(set_to_string))(const struct S_(set) *);
#include "../test/test_set.h"
#endif /* test --> */

static void PS_(unused_base_coda)(void);
static void PS_(unused_base)(void) {
	PS_(map) m;
	PS_(key) k;
	memset(&m, 0, sizeof m);
	memset(&k, 0, sizeof k);
	S_(set)(0); S_(set_)(0); S_(set_buffer)(0, 0); S_(set_clear)(0);
	S_(set_query)(0, k, 0);
	S_(set_policy_put)(0, m, 0, 0);
	/*S_(set_remove)(0, 0);*/
	S_(set_begin)(0, 0); S_(set_has_next)(0); S_(set_next_key)(0);
	PS_(unused_base_coda)();
}
static void PS_(unused_base_coda)(void) { PS_(unused_base)(); }


#elif defined(SET_TO_STRING) /* base code --><!-- to string trait */


#ifdef SET_TO_STRING_NAME
#define SZ_(n) SET_CAT(S_(set), SET_CAT(SET_TO_STRING_NAME, n))
#else
#define SZ_(n) SET_CAT(S_(set), n)
#endif
#define TSZ_(n) SET_CAT(hash_sz, SZ_(n))
/* Check that `SET_TO_STRING` is a function implementing this prototype. */
static void (*const TSZ_(actual_to_string))(PS_(ckey), char (*const)[12])
	= (SET_TO_STRING);
/** This is to line up the hash, which can have <typedef:<PS>key> a pointer or
 not, with to string, which requires a pointer. Call
 <data:<TSZ>actual_to_string> with key of `bucket` and `z`. */
static void TSZ_(thunk_to_string)(const struct PS_(bucket) *const bucket,
	char (*const z)[12]) { TSZ_(actual_to_string)(PS_(entry_key)(bucket), z); }
#define TO_STRING &TSZ_(thunk_to_string)
#define TO_STRING_LEFT '{'
#define TO_STRING_RIGHT '}'
#include "to_string.h" /** \include */
#ifdef SET_TEST /* <!-- expect: greedy satisfy forward-declared. */
#undef SET_TEST
static void (*PS_(to_string))(PS_(ckey), char (*const)[12])
	= TSZ_(actual_to_string);
static const char *(*PS_(set_to_string))(const struct S_(set) *)
	= &SZ_(to_string);
#endif /* expect --> */
#undef TSZ_
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
#undef SET_KEY
#undef SET_UINT
#undef SET_CODE
#undef SET_IS_EQUAL
#ifdef SET_VALUE
#undef SET_VALUE
#endif
#ifdef SET_NO_CACHE
#undef SET_NO_CACHE
#endif
#undef BOX_
#undef BOX_CONTAINER
#undef BOX_CONTENTS
/* box (multiple traits) --> */
#endif /* !trait --> */
#undef SET_TO_STRING_TRAIT
#undef SET_TRAITS
