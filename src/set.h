/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Hash table

 ![Example of <string>set.](../web/set.png)

 <tag:<S>set> is a set or map of <typedef:<PS>entry> implemented as a hash
 table. It must be supplied a <typedef:<PS>hash_fn> and
 <typedef:<PS>is_equal_fn>.

 @param[SET_NAME, SET_KEY]
 `<S>` that satisfies `C` naming conventions when mangled and a valid
 <typedef:<PS>key> associated therewith; required. `<PS>` is private, whose
 names are prefixed in a manner to avoid collisions.

 @param[SET_HASH, SET_IS_EQUAL]
 A function satisfying <typedef:<PS>hash_fn> and <typedef:<PS>is_equal_fn>.
 `SET_HASH` and either `SET_IS_EQUAL` or `SET_INVERSE`, but not both, are
 required.

 @param[SET_VALUE]
 An optional type that is the payload of the key, thus making this an
 associative array. If the key is part of an aggregate value, it will be
 more efficient and robust to use a type conversion instead of storing
 related pointers.

 @param[SET_UINT]
 This is <typedef:<PS>uint>, the unsigned type of hash hash of the key given by
 <typedef:<PS>hash_fn>; defaults to `size_t`.

 @param[SET_INVERSE]
 Function satisfying <typedef:<PS>inverse_hash_fn>; this avoids storing the
 key, but calculates it from the hashed value. The hashes are now unique, so
 there is no need for a `SET_IS_EQUAL`.

 @param[SET_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[SET_TO_STRING_NAME, SET_TO_STRING]
 To string trait contained in <to_string.h>; `<SZ>` that satisfies `C` naming
 conventions when mangled and function implementing
 <typedef:<PSZ>to_string_fn>. There can be multiple to string traits, but only
 one can omit `SET_TO_STRING_NAME`.

 @std C89 */

#if !defined(SET_NAME) || !defined(SET_KEY) || !defined(SET_HASH) \
	|| !(defined(SET_IS_EQUAL) ^ defined(SET_INVERSE))
#error Name SET_NAME, tag type SET_KEY, functions SET_HASH, and, \
	SET_IS_EQUAL or SET_INVERSE (but not both) undefined.
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
/* Use the sign bit to store out-of-band flags when a <typedef:<PS>uint>
 represents an address in the table, (such that range of an index is one bit
 less.) Choose representations that probably save power. We cannot save this in
 an `enum` because we don't know maximum. */
#define SET_M1 ((PS_(uint))~(PS_(uint))0) /* 2's compliment -1. */
#define SET_LIMIT ((SET_M1 >> 1) + 1) /* Cardinality. */
#define SET_END (SET_LIMIT)
#define SET_NULL (SET_LIMIT + 1)
#define SET_RESULT X(ERROR), X(UNIQUE), X(YIELD), X(REPLACE)
/* These are not returned by any of the editing functions; micromanaging has
 been simplified. X(REPLACE_KEY), X(REPLACE_VALUE) */
#define X(n) SET_##n
/** This is the result of modifying the table. An `enum` of `SET_*`, of which
 `SET_ERROR` is false. ![A diagram of the result states.](../web/put.png) */
enum set_result { SET_RESULT };
#undef X
#define X(n) #n
/** A static array of strings describing the <tag:set_result>. */
static const char *const set_result_str[] = { SET_RESULT };
#undef X
#undef SET_RESULT
#endif /* idempotent --> */


#if SET_TRAITS == 0 /* <!-- base set */


#ifndef SET_UINT
#define SET_UINT size_t
#endif

/** <typedef:<PS>hash_fn> returns this hash type by `SET_UINT`, which must be
 be an unsigned integer. Places a simplifying limit on the maximum number of
 items in this container of half the cardinality. */
typedef SET_UINT PS_(uint);

/** Valid tag type defined by `SET_KEY` used for keys. */
typedef SET_KEY PS_(key);
/** Read-only <typedef:<PS>key>. Makes the simplifying assumption that this is
 not `const`-qualified. */
typedef const SET_KEY PS_(ckey);

/** A map from <typedef:<PS>ckey> onto <typedef:<PS>uint>, (any will do, but
 the performance may suffer if too many entries are hashed to the same
 buckets.) If <typedef:<PS>key> is a pointer, one is permitted to have null in
 the domain. */
typedef PS_(uint) (*PS_(hash_fn))(PS_(ckey));
/* Check that `SET_HASH` is a function implementing <typedef:<PS>hash_fn>. */
static const PS_(hash_fn) PS_(hash) = (SET_HASH);

#ifdef SET_INVERSE /* <!-- inv */

/** Defining `SET_INVERSE` says <typedef:<PS>hash_fn> forms a bijection between
 the range in <typedef:<PS>key> and the image in <typedef:<PS>uint>. This is
 the inverse-mapping. */
typedef PS_(key) (*PS_(inverse_hash_fn))(PS_(uint));
/* Check that `SET_INVERSE` is a function implementing
 <typedef:<PS>inverse_hash_fn>. */
static const PS_(inverse_hash_fn) PS_(inverse_hash) = (SET_INVERSE);

#else /* inv --><!-- !inv */

/** Equivalence relation between <typedef:<PS>key> that satisfies
 `<PS>is_equal_fn(a, b) -> <PS>hash(a) == <PS>hash(b)`. */
typedef int (*PS_(is_equal_fn))(PS_(ckey) a, PS_(ckey) b);
/* Check that `SET_IS_EQUAL` is a function implementing
 <typedef:<PS>is_equal_fn>. */
static const PS_(is_equal_fn) PS_(equal) = (SET_IS_EQUAL);

#endif /* !inv --> */

#ifdef SET_VALUE /* <!-- value */
/** Defining `SET_VALUE` produces an associative map, otherwise it is the same
 as <typedef:<PS>key>. */
typedef SET_VALUE PS_(value);
/** Defining `SET_VALUE` creates this map from <typedef:<PS>key> to
 <typedef:<PS>value> as an interface with set. */
struct S_(set_entry) { PS_(key) key; PS_(value) value; };
/** If `SET_VALUE`, this is <tag:<S>set_entry>; otherwise, it's the same as
 <typedef:<PS>key>. */
typedef struct S_(set_entry) PS_(entry);
#else /* value --><!-- !value */
typedef PS_(key) PS_(value);
typedef PS_(key) PS_(entry);
#endif /* !value --> */

/** @return Key from `e`. */
static PS_(key) PS_(entry_key)(PS_(entry) e) {
#ifdef SET_VALUE
	return e.key;
#else
	return e;
#endif
}

/* Address is hash modulo size of table. Any occupied buckets at the start of
 the linked structure are closed, that is, the address equals the index. These
 form a linked set, possibly with other, open buckets that have the same
 address in vacant buckets. */
struct PS_(bucket) {
	PS_(uint) next; /* Bucket index, including `SET_NULL` and `SET_END`. */
	PS_(uint) hash;
#ifndef SET_INVERSE
	PS_(key) key;
#endif
#ifdef SET_VALUE
	PS_(value) value;
#endif
};

/** Gets the key of an occupied `bucket`. */
static PS_(key) PS_(bucket_key)(const struct PS_(bucket) *const bucket) {
	assert(bucket && bucket->next != SET_NULL);
#ifdef SET_INVERSE
	return PS_(inverse_hash)(bucket->hash);
#else
	return bucket->key;
#endif
}

/** Fills `entry`, a public structure, with the information of `bucket`. */
static void PS_(to_entry)(const struct PS_(bucket) *const bucket,
	PS_(entry) *const entry) {
	assert(bucket && entry);
#ifdef SET_VALUE /* entry { <PS>key key; <PS>value value; } */
	entry->key = PS_(bucket_key)(bucket);
	memcpy(&entry->value, &bucket->value, sizeof bucket->value);
#else /* entry <PS>key */
	*entry = PS_(bucket_key)(bucket);
#endif
}

/** Returns true if the `replace` replaces the `original`. */
typedef int (*PS_(policy_fn))(PS_(key) original, PS_(key) replace);

/** To initialize, see <fn:<S>set>, `SET_IDLE`, `{0}` (`C99`,) or being
 `static`. The fields should be treated as read-only; any modification is
 liable to cause the set to go into an invalid state.

 ![States.](../web/states.png) */
struct S_(set) { /* "Padding size," good. */
	struct PS_(bucket) *buckets; /* @ has zero/one key specified by `next`. */
	/* Buckets; `size <= capacity`; open stack, including `SET_END`. */
	PS_(uint) log_capacity, size, top;
	/* Size is not needed but convenient and allows short-circuiting. */
};

/** The capacity of a non-idle `set` is always a power-of-two. */
static PS_(uint) PS_(capacity)(const struct S_(set) *const set)
	{ return assert(set && set->buckets && set->log_capacity >= 3),
	(PS_(uint))((PS_(uint))1 << set->log_capacity); }

/** @return Indexes the first closed bucket in the set of buckets with the same
 address from non-idle `set` given the `hash`. If the bucket is empty, it will
 have `next = SET_NULL` or it's own <fn:<PS>to_bucket> not equal to the index.*/
static PS_(uint) PS_(to_bucket)(const struct S_(set) *const set,
	const PS_(uint) hash) { return hash & (PS_(capacity)(set) - 1); }

/** On return, the `top` of `set` will be empty, but size is not incremented,
 leaving it in intermediate state. This is amortized; every value takes at most
 one. */
static void PS_(grow_stack)(struct S_(set) *const set) {
	PS_(uint) top = set->top;
	assert(set && set->buckets && top);
	top = (top == SET_END ? PS_(capacity)(set) : top) - 1;
	while(set->buckets[top].next != SET_NULL) assert(top), top--;
	set->top = top;
}

/** Is `i` in `set` possibly on the stack? (The stack grows from the high.) */
static int PS_(in_stack_range)(const struct S_(set) *const set,
	const PS_(uint) i)
	{ return assert(set), set->top != SET_END && set->top <= i; }

/***********fixme*/
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)
#ifdef SET_TEST
/** `f` `g` */
static void PS_(graph)(const struct S_(set) *f, const char *g);
static void (*PS_(to_string))(PS_(ckey), char (*)[12]);
#else
/** `set` `fn` */
static void PS_(graph)(const struct S_(set) *const set, const char *const fn) {
	(void)set, (void)fn;
}
/** `key` `z` */
static void PS_(to_string)(PS_(ckey) key, char (*z)[12])
	{ (void)key, strcpy(*z, "<key>"); }
#endif

/** Moves the `target` index in non-idle `set`, to the top of collision stack.
 This may result in an inconsistent state; one is responsible for filling that
 hole and linking it with top. */
static void PS_(move_to_top)(struct S_(set) *const set,
	const PS_(uint) target) {
	struct PS_(bucket) *tgt, *top;
	PS_(uint) to_next, next;
	const PS_(uint) capacity = PS_(capacity)(set);
	assert(set->size < capacity && target < capacity);
	PS_(grow_stack)(set);
	tgt = set->buckets + target, top = set->buckets + set->top;
	assert(tgt->next != SET_NULL && top->next == SET_NULL);
	/* Search for the previous link in the bucket, \O(|bucket|). */
	for(to_next = SET_NULL,
		next = PS_(to_bucket)(set, tgt->hash);
		assert(next < capacity), next != target;
		to_next = next, next = set->buckets[next].next);
	/* Move `tgt` to `top`. */
	if(to_next != SET_NULL) set->buckets[to_next].next = set->top;
	memcpy(top, tgt, sizeof *tgt), tgt->next = SET_NULL;
}

/** `set` will be searched linearly for `key` which has `hash`.
 @fixme Move to front like splay trees? this is awkward. */
static struct PS_(bucket) *PS_(query)(struct S_(set) *const set,
	const PS_(key) key, const PS_(uint) hash) {
	struct PS_(bucket) *bucket;
	PS_(uint) i, next;
	assert(set && set->buckets && set->log_capacity);
	bucket = set->buckets + (i = PS_(to_bucket)(set, hash));
	/* Not the start of a bucket: empty or in the collision stack. */
	if((next = bucket->next) == SET_NULL
		|| PS_(in_stack_range)(set, i)
		&& i != PS_(to_bucket)(set, bucket->hash)) return 0;
	for( ; ; ) {
		if(hash == bucket->hash) {
			int entries_are_equal;
#ifdef SET_INVERSE
			entries_are_equal = ((void)(key), 1); /* Injective. */
#else
			entries_are_equal = PS_(equal)(key, bucket->key);
#endif
			if(entries_are_equal) return bucket;
		}
		if(next == SET_END) return 0;
		bucket = set->buckets + (i = next);
		assert(i < PS_(capacity)(set) && PS_(in_stack_range)(set, i) &&
			i != SET_NULL);
		next = bucket->next;
	}
}

/** Ensures that `set` has enough buckets to fill `n` more than the size. May
 invalidate and re-arrange the order.
 @return Success; otherwise, `errno` will be set. @throws[realloc]
 @throws[ERANGE] Tried allocating more then can fit in half <typedef:<PS>uint>
 or `realloc` doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html). */
static int PS_(buffer)(struct S_(set) *const set, const PS_(uint) n) {
	struct PS_(bucket) *buckets;
	const PS_(uint) log_c0 = set->log_capacity,
		c0 = log_c0 ? (PS_(uint))((PS_(uint))1 << log_c0) : 0;
	PS_(uint) log_c1, c1, size1, i, wait, mask;
	char fn[64];
	assert(set && set->size <= SET_LIMIT && (!set->buckets && !set->size
		&& !log_c0 && !c0 || set->buckets && set->size <= c0 && log_c0>=3));
	/* Can we satisfy `n` growth from the buffer? */
	if(SET_M1 - set->size < n || SET_LIMIT < (size1 = set->size + n))
		return errno = ERANGE, 0;
	if(set->buckets)  log_c1 = log_c0, c1 = c0 ? c0 : 1;
	else              log_c1 = 3,      c1 = 8;
	while(c1 < size1) log_c1++, c1 <<= 1;
	if(log_c0 == log_c1) return 1;

	sprintf(fn, "graph/" QUOTE(SET_NAME) "-resize-%lu-%lu-a-before.gv",
		(unsigned long)log_c0, (unsigned long)log_c1), PS_(graph)(set, fn);

	/* Otherwise, need to allocate more. */
	if(!(buckets = realloc(set->buckets, sizeof *buckets * c1)))
		{ if(!errno) errno = ERANGE; return 0; }
	set->top = SET_END; /* Idle `top` initialized or reset. */
	set->buckets = buckets, set->log_capacity = log_c1;

	/* Initialize new values. Mask to identify the added bits. */
	{ struct PS_(bucket) *e = buckets + c0, *const e_end = buckets + c1;
		for( ; e < e_end; e++) e->next = SET_NULL; }
	mask = (PS_(uint))((((PS_(uint))1 << log_c0) - 1)
		^ (((PS_(uint))1 << log_c1) - 1));

	/* Rehash most closed buckets in the lower half. Create waiting
	 linked-stack by borrowing next. */
	wait = SET_END;
	for(i = 0; i < c0; i++) {
		struct PS_(bucket) *idx, *go;
		PS_(uint) g, hash;
		idx = set->buckets + i;
		if(idx->next == SET_NULL) continue;
		g = PS_(to_bucket)(set, hash = idx->hash);
		/* It's a power-of-two size, so, like consistent hashing, `E[old/new]`
		 capacity that a closed bucket will remain where it is. */
		if(i == g) { idx->next = SET_END; continue; }
		if((go = set->buckets + g)->next == SET_NULL) {
			/* Priority is given to the first closed bucket; simpler later. */
			struct PS_(bucket) *head;
			PS_(uint) h = g & ~mask; assert(h <= g);
			if(h < g && i < h
				&& (head = set->buckets + h, assert(head->next != SET_NULL),
				PS_(to_bucket)(set, head->hash) == g)) {
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

	sprintf(fn, "graph/" QUOTE(SET_NAME) "-resize-%lu-%lu-b-obvious.gv",
		(unsigned long)log_c0, (unsigned long)log_c1), PS_(graph)(set, fn);

	/* Search waiting stack for buckets that moved concurrently. */
	{ PS_(uint) prev = SET_END, w = wait; while(w != SET_END) {
		struct PS_(bucket) *waiting = set->buckets + w;
		PS_(uint) cl = PS_(to_bucket)(set, waiting->hash);
		struct PS_(bucket) *const closed = set->buckets + cl;
		assert(cl != w);
		if(closed->next == SET_NULL) {
			memcpy(closed, waiting, sizeof *waiting), closed->next = SET_END;
			if(prev != SET_END) set->buckets[prev].next = waiting->next;
			if(wait == w) wait = waiting->next; /* First, modify head. */
			w = waiting->next, waiting->next = SET_NULL;
		} else {
			assert(closed->next == SET_END); /* Not in the wait stack. */
			prev = w, w = waiting->next;
		}
	}}

	sprintf(fn, "graph/" QUOTE(SET_NAME) "-resize-%lu-%lu-c-closed.gv",
		(unsigned long)log_c0, (unsigned long)log_c1), PS_(graph)(set, fn);

	/* Rebuild the top stack at the high numbers from the waiting at low. */
	while(wait != SET_END) {
		struct PS_(bucket) *const waiting = set->buckets + wait;
		PS_(uint) h = PS_(to_bucket)(set, waiting->hash);
		struct PS_(bucket) *const head = set->buckets + h;
		struct PS_(bucket) *top;
		assert(h != wait && head->next != SET_NULL);
		PS_(grow_stack)(set), top = set->buckets + set->top;
		memcpy(top, waiting, sizeof *waiting);
		top->next = head->next, head->next = set->top;
		wait = waiting->next, waiting->next = SET_NULL; /* Pop. */
	}

	sprintf(fn, "graph/" QUOTE(SET_NAME) "-resize-%lu-%lu-d-final.gv",
		(unsigned long)log_c0, (unsigned long)log_c1), PS_(graph)(set, fn);

	return 1;
}

#undef QUOTE_
#undef QUOTE

/** Replace the `key` and `hash` of `bucket`. Don't touch next.
 @fixme `memcpy`? */
static void PS_(replace_key)(struct PS_(bucket) *const bucket,
	const PS_(key) key, const PS_(uint) hash) {
#ifndef SET_NO_CACHE
	bucket->hash = hash;
	(void)key;
#endif
#ifndef SET_INVERSE
	memcpy(&bucket->key, &key, sizeof key);
	(void)hash;
#endif
}

/** Replace the entire `entry` and `hash` of `bucket`. Don't touch next.
 @fixme `memcpy`? */
static void PS_(replace_entry)(struct PS_(bucket) *const bucket,
	const PS_(entry) entry, const PS_(uint) hash) {
	PS_(replace_key)(bucket, PS_(entry_key)(entry), hash);
#ifdef SET_VALUE
	memcpy(&bucket->value, &entry.value, sizeof(entry.value));
#endif
}

/** Evicts the spot where `hash` goes in `set`. Must have at least one free
 bucket. This results in a space in the table, with the next set. */
static struct PS_(bucket) *PS_(evict)(struct S_(set) *const set,
	const PS_(uint) hash) {
	PS_(uint) i;
	struct PS_(bucket) *bucket;
	if(!PS_(buffer)(set, 1)) return 0; /* Amortized. */
	bucket = set->buckets + (i = PS_(to_bucket)(set, hash)); /* Closed. */
	if(bucket->next != SET_NULL) { /* Occupied. */
		int in_stack = PS_(to_bucket)(set, bucket->hash) != i;
		PS_(move_to_top)(set, i);
		bucket->next = in_stack ? SET_END : set->top;
	} else { /* Unoccupied. */
		bucket->next = SET_END;
	}
	set->size++;
	return bucket;
}

/** Put `entry` in `set`. For collisions, only if `update` exists and returns
 true do and displace it to `eject`, if non-null.
 @return A <tag:set_result>. @throws[malloc]
 @order Amortized \O(max bucket length); the key to another bucket may have to
 be moved to the top; the table might be full and have to be resized. */
static enum set_result PS_(put)(struct S_(set) *const set,
	PS_(entry) entry, PS_(entry) *eject, const PS_(policy_fn) update) {
	struct PS_(bucket) *bucket;
	const PS_(key) key = PS_(entry_key)(entry);
	const PS_(uint) hash = PS_(hash)(key);
	enum set_result result;
	assert(set);
	if(set->buckets && (bucket = PS_(query)(set, key, hash))) {
		if(!update || !update(PS_(bucket_key)(bucket), key)) return SET_YIELD;
		if(eject) PS_(to_entry)(bucket, eject);
		result = SET_REPLACE;
	} else {
		if(!(bucket = PS_(evict)(set, hash))) return SET_ERROR;
		result = SET_UNIQUE;
	}
	PS_(replace_entry)(bucket, entry, hash);
	return result;
}

#ifdef SET_VALUE /* <!-- value */

/** On `SET_VALUE`, try to put `key` into `set`, and update `value` to be
 a pointer to the current value.
 @return `SET_ERROR` does not set `value`; `SET_ABSENT`, the `value` will be
 uninitialized; `SET_YIELD`, gets the current `value`. @throws[malloc] */
static enum set_result PS_(compute)(struct S_(set) *const set,
	PS_(key) key, PS_(value) **const value) {
	struct PS_(bucket) *bucket;
	const PS_(uint) hash = PS_(hash)(key);
	enum set_result result;
	assert(set && value);
	if(set->buckets && (bucket = PS_(query)(set, key, hash))) {
		result = SET_YIELD;
	} else {
		if(!(bucket = PS_(evict)(set, hash))) return SET_ERROR;
		PS_(replace_key)(bucket, key, hash);
		result = SET_UNIQUE;
	}
	*value = &bucket->value;
	return result;
}

#endif /* value --> */

/** Initialises `set` to idle. @order \Theta(1) @allow */
static void S_(set)(struct S_(set) *const set) {
	assert(set);
	set->buckets = 0;
	set->log_capacity = 0; set->size = 0; set->top = 0;
}

/** Destroys `set` and returns it to idle. @allow */
static void S_(set_)(struct S_(set) *const set)
	{ assert(set), free(set->buckets); S_(set)(set); }

/** Reserve at least `n` space for buckets of `set`. This will ensure that
 there is space for those buckets and may increase iteration time.
 @return Success.
 @throws[ERANGE] The request was unsatisfiable. @throws[realloc] @allow */
static int S_(set_buffer)(struct S_(set) *const set, const PS_(uint) n)
	{ return assert(set), PS_(buffer)(set, n); }

/** Clears and removes all buckets from `set`. The capacity and memory of the
 `set` is preserved, but all previous values are un-associated. (The load
 factor will be less until it reaches it's previous size.)
 @order \Theta(`set.capacity`) @allow */
static void S_(set_clear)(struct S_(set) *const set) {
	struct PS_(bucket) *b, *b_end;
	assert(set);
	if(!set->buckets) { assert(!set->log_capacity); return; }
	for(b = set->buckets, b_end = b + PS_(capacity)(set); b < b_end; b++)
		b->next = SET_NULL;
	set->size = 0;
}

/* set_shrink: if shrinkable, reserve the exact amount in a separate buffer
 and move all. Does not, and indeed cannot, respect the most-recently used
 heuristic. */

/** @return Is `key` in `set`? @allow */
static int S_(set_is)(struct S_(set) *const set, const PS_(key) key)
	{ return assert(set),set->buckets && PS_(query)(set, key, PS_(hash)(key)); }

/** @param[result] If non-null, a <typedef:<PS>entry> which gets filled on true.
 @return Is `key` in `set`? @allow */
static int S_(set_query)(struct S_(set) *const set, const PS_(key) key,
	PS_(entry) *const result) {
	struct PS_(bucket) *b;
	assert(set);
	if(!set->buckets || !(b = PS_(query)(set, key, PS_(hash)(key)))) return 0;
	if(result) PS_(to_entry)(b, result);
	return 1;
}

/* set_get_or, set_<P>_get */
#if 0
/** @return The value in `hash` which is equal `key`, or, if no such value
 exists, null. @order Average \O(1), (hash distributes elements uniformly);
 worst \O(n). @allow */
static PS_(key) S_(set_get)(struct S_(set) *const hash,
	const PS_(key) key) {
	struct PS_(bucket) *e;
	assert(hash);
	if(!hash->buckets) { assert(!hash->log_capacity); return 0; }
	e = PS_(get)(hash, key, PS_(hash)(key));
	return e ? PS_(entry_key)(e) : 0;
}
#endif

/* set_try(), set_replace() */

/** Puts `entry` in `set` only if absent or if calling `update` returns true.
 @return One of: `SET_ERROR` the set is not modified; `SET_REPLACE` if
 `update` is non-null and returns true, `eject`, if non-null, will be filled;
 `SET_YIELD` if `replace` is null or false; `SET_GROW`, on unique entry.
 @throws[realloc, ERANGE] There was an error with resizing.
 @order Average amortised \O(1), (hash distributes keys uniformly); worst \O(n).
 @allow */
static enum set_result S_(set_update)(struct S_(set) *const set,
	PS_(entry) entry, PS_(entry) *eject, const PS_(policy_fn) update)
	{ return PS_(put)(set, entry, eject, update); }

#ifdef SET_VALUE /* <!-- value */
/** If `SET_VALUE`, try to put `key` into `set`, and store the value in `value`.
 @return `SET_ERROR` does not set `value`; `SET_GROW`, the `value` will be
 uninitialized; `SET_YIELD`, gets the current `value`.
 @throws[malloc] On `SET_ERROR`. @allow */
static enum set_result S_(set_compute)(struct S_(set) *const set,
	PS_(key) key, PS_(value) **const value)
	{ return PS_(compute)(set, key, value); }
#endif /* value --> */

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
struct PS_(iterator) {
	const struct S_(set) *set;
	union {
		const void *const do_not_warn;
		PS_(uint) b;
	} _;
};

/** Loads `hash` (can be null) into `it`. @implements begin */
static void PS_(begin)(struct PS_(iterator) *const it,
	const struct S_(set) *const hash)
	{ assert(it), it->set = hash, it->_.b = 0; }

/** Helper to skip the buckets of `it` that are not there.
 @return Whether it found another index. */
static int PS_(skip)(struct PS_(iterator) *const it) {
	const struct S_(set) *const hash = it->set;
	const PS_(uint) limit = PS_(capacity)(hash);
	assert(it && it->set && it->set->buckets);
	while(it->_.b < limit) {
		struct PS_(bucket) *const bucket = hash->buckets + it->_.b;
		if(bucket->next != SET_NULL) return 1;
		it->_.b++;
	}
	return 0;
}

/** Advances `it`. @implements next */
static struct PS_(bucket) *PS_(next)(struct PS_(iterator) *const it) {
	assert(it);
	if(!it->set || !it->set->buckets) return 0;
	if(PS_(skip)(it)) return it->set->buckets + it->_.b++;
	it->set = 0, it->_.b = 0;
	return 0;
}

/* iterate --> */

/** Iteration usually not in any particular order. The asymptotic runtime is
 proportional to the hash capacity. */
struct S_(set_iterator) { struct PS_(iterator) it; };

/** Loads `set` (can be null) into `it`. @allow */
static void S_(set_begin)(struct S_(set_iterator) *const it,
	const struct S_(set) *const set) { PS_(begin)(&it->it, set); }

/** Advances `it`.
 @param[entry] If non-null, the entry is filled with the next element only if
 it has a next. @return Whether it had a next element. @allow */
static int S_(set_next)(struct S_(set_iterator) *const it, PS_(entry) *entry) {
	const struct PS_(bucket) *b = PS_(next)(&it->it);
	return b ? (PS_(to_entry)(b, entry), 1) : 0;
}

/** @return Whether the set specified to `it` in <fn:<S>set_begin> has a next.
 @order Amortized on the capacity, \O(1). @allow */
static int S_(set_has_next)(struct S_(set_iterator) *const it) {
	assert(it);
	return it->it.set && it->it.set->buckets && PS_(skip)(&it->it);
}

#ifdef SET_VALUE /* <!-- value */

/** If `SET_VALUE`, advances `it` when <fn:<S>set_has_next>.
 @return The next key. @allow */
static PS_(key) S_(set_next_key)(struct S_(set_iterator) *const it)
	{ return PS_(bucket_key)(PS_(next)(&it->it)); }

/** If `SET_VALUE`, advances `it` when <fn:<S>set_has_next>.
 @return The next value. @allow */
static PS_(value) S_(set_next_value)(struct S_(set_iterator) *const it)
	{ return PS_(next)(&it->it)->value; }

#endif /* value --> */

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
	PS_(entry) e;
	PS_(key) k;
	memset(&e, 0, sizeof e);
	memset(&k, 0, sizeof k);
	S_(set)(0); S_(set_)(0); S_(set_buffer)(0, 0); S_(set_clear)(0);
	S_(set_is)(0, k); S_(set_query)(0, k, 0);
	S_(set_update)(0, e, 0, 0);
#ifdef SET_VALUE
	S_(set_compute)(0, k, 0); S_(set_next_key)(0); S_(set_next_value)(0);
#endif
	/*S_(set_remove)(0, 0);*/
	S_(set_begin)(0, 0); S_(set_next)(0, 0); S_(set_has_next)(0);
	PS_(unused_base_coda)();
}
static void PS_(unused_base_coda)(void) { PS_(unused_base)(); }


#elif defined(SET_TO_STRING) /* base hash --><!-- to string trait */


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
	char (*const z)[12]) { TSZ_(actual_to_string)(PS_(bucket_key)(bucket), z); }
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
#undef SET_HASH
#undef SET_IS_EQUAL
#ifdef SET_VALUE
#undef SET_VALUE
#endif
#ifdef SET_INVERSE
#undef SET_INVERSE
#endif
#undef BOX_
#undef BOX_CONTAINER
#undef BOX_CONTENTS
/* box (multiple traits) --> */
#endif /* !trait --> */
#undef SET_TO_STRING_TRAIT
#undef SET_TRAITS
