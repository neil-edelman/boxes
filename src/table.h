/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Hash table

 ![Example of <string>table.](../web/table.png)

 <tag:<N>set> implements a set or map of <typedef:<PN>entry> as a hash table.
 It must be supplied a <typedef:<PN>hash_fn> and, <typedef:<PN>is_equal_fn> or
 <typedef:<PN>inverse_hash_fn>.

 @param[TABLE_NAME, TABLE_KEY]
 `<N>` that satisfies `C` naming conventions when mangled and a valid
 <typedef:<PN>key> associated therewith; required. `<PN>` is private, whose
 names are prefixed in a manner to avoid collisions.

 @param[TABLE_HASH, TABLE_IS_EQUAL]
 A function satisfying <typedef:<PN>hash_fn> and <typedef:<PN>is_equal_fn>.
 `TABLE_HASH` and either `TABLE_IS_EQUAL` or `TABLE_INVERSE`, but not both, are
 required.

 @param[TABLE_VALUE]
 An optional type that is the payload of the key, thus making this an
 associative array. If the key is part of an aggregate value, it will be
 more efficient and robust to use a type conversion instead of storing
 related pointers.

 @param[TABLE_UINT]
 This is <typedef:<PN>uint>, the unsigned type of hash hash of the key given by
 <typedef:<PN>hash_fn>; defaults to `size_t`.

 @param[TABLE_INVERSE]
 Function satisfying <typedef:<PN>inverse_hash_fn>; this avoids storing the
 key, but calculates it from the hashed value. The hashes are now unique, so
 there is no need for a `TABLE_IS_EQUAL`.

 @param[TABLE_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[TABLE_DEFAULT_NAME, TABLE_DEFAULT]
 A name that satisfies `C` naming conventions when mangled and a
 <typedef:<PN>value> used in <fn:<N>set<D>get>. There can be multiple to
 defaults, but only one can omit `TABLE_DEFAULT_NAME`.

 @param[TABLE_TO_STRING_NAME, TABLE_TO_STRING]
 To string trait contained in <to_string.h>; `<SZ>` that satisfies `C` naming
 conventions when mangled and function implementing
 <typedef:<PSZ>to_string_fn>. There can be multiple to string traits, but only
 one can omit `TABLE_TO_STRING_NAME`.

 @std C89 */

#if !defined(TABLE_NAME) || !defined(TABLE_KEY) || !defined(TABLE_HASH) \
	|| !(defined(TABLE_IS_EQUAL) ^ defined(TABLE_INVERSE))
#error Name TABLE_NAME, tag type TABLE_KEY, functions TABLE_HASH, and, \
	TABLE_IS_EQUAL or TABLE_INVERSE (but not both) undefined.
#endif
#if defined(TABLE_DEFAULT_NAME) || defined(TABLE_DEFAULT)
#define TABLE_DEFAULT_TRAIT 1
#else
#define TABLE_DEFAULT_TRAIT 0
#endif
#if defined(TABLE_TO_STRING_NAME) || defined(TABLE_TO_STRING)
#define TABLE_TO_STRING_TRAIT 1
#else
#define TABLE_TO_STRING_TRAIT 0
#endif
#define TABLE_TRAITS TABLE_DEFAULT_TRAIT + TABLE_TO_STRING_TRAIT
#if TABLE_TRAITS > 1
#error Only one trait per include is allowed; use TABLE_EXPECT_TRAIT.
#endif
#if defined(TABLE_DEFAULT_NAME) && !defined(TABLE_DEFAULT)
#error TABLE_DEFAULT_NAME requires TABLE_DEFAULT.
#endif
#if defined(TABLE_TO_STRING_NAME) && !defined(TABLE_TO_STRING)
#error TABLE_TO_STRING_NAME requires TABLE_TO_STRING.
#endif

#ifndef TABLE_H /* <!-- idempotent */
#define TABLE_H
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#if defined(TABLE_CAT_) || defined(TABLE_CAT) || defined(N_) || defined(PN_) \
	|| defined(TABLE_IDLE)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define TABLE_CAT_(n, m) n ## _ ## m
#define TABLE_CAT(n, m) TABLE_CAT_(n, m)
#define N_(n) TABLE_CAT(TABLE_NAME, n)
#define PN_(n) TABLE_CAT(set, N_(n))
#define TABLE_IDLE { 0, 0, 0, 0 }
/* Use the sign bit to store out-of-band flags when a <typedef:<PN>uint>
 represents an address in the table, (such that range of an index is one bit
 less.) Choose representations that probably save power. We cannot save this in
 an `enum` because we don't know maximum. */
#define TABLE_M1 ((PN_(uint))~(PN_(uint))0) /* 2's compliment -1. */
#define TABLE_LIMIT ((TABLE_M1 >> 1) + 1) /* Cardinality. */
#define TABLE_END (TABLE_LIMIT)
#define TABLE_NULL (TABLE_LIMIT + 1)
#define TABLE_RESULT X(ERROR), X(UNIQUE), X(YIELD), X(REPLACE)
/* These are not returned by any of the editing functions; micromanaging has
 been simplified. X(REPLACE_KEY), X(REPLACE_VALUE) */
#define X(n) TABLE_##n
/** This is the result of modifying the table. An `enum` of `TABLE_*`, of which
 `TABLE_ERROR` is false. ![A diagram of the result states.](../web/put.png) */
enum set_result { TABLE_RESULT };
#undef X
#define X(n) #n
/** A static array of strings describing the <tag:set_result>. */
static const char *const set_result_str[] = { TABLE_RESULT };
#undef X
#undef TABLE_RESULT
#endif /* idempotent --> */


#if TABLE_TRAITS == 0 /* <!-- base set */


#ifndef TABLE_UINT
#define TABLE_UINT size_t
#endif

/** <typedef:<PN>hash_fn> returns this hash type by `TABLE_UINT`, which must be
 be an unsigned integer. Places a simplifying limit on the maximum number of
 items in this container of half the cardinality. */
typedef TABLE_UINT PN_(uint);

/** Valid tag type defined by `TABLE_KEY` used for keys. */
typedef TABLE_KEY PN_(key);
/** Read-only <typedef:<PN>key>. Makes the simplifying assumption that this is
 not `const`-qualified. */
typedef const TABLE_KEY PN_(ckey);

/** A map from <typedef:<PN>ckey> onto <typedef:<PN>uint> that, ideally, should
 be easy to compute while minimizing duplications of <typedef:<PN>uint> mod
 hash table capacity for the domain of the <typedef:<PN>key>. Must be
 consistent while in the set. If <typedef:<PN>key> is a pointer, one is
 permitted to have null in the domain. */
typedef PN_(uint) (*PN_(hash_fn))(PN_(ckey));
/* Check that `TABLE_HASH` is a function implementing <typedef:<PN>hash_fn>. */
static const PN_(hash_fn) PN_(hash) = (TABLE_HASH);

#ifdef TABLE_INVERSE /* <!-- inv */

/** Defining `TABLE_INVERSE` says <typedef:<PN>hash_fn> forms a bijection between
 the range in <typedef:<PN>key> and the image in <typedef:<PN>uint>. This is
 the inverse-mapping. */
typedef PN_(key) (*PN_(inverse_hash_fn))(PN_(uint));
/* Check that `TABLE_INVERSE` is a function implementing
 <typedef:<PN>inverse_hash_fn>. */
static const PN_(inverse_hash_fn) PN_(inverse_hash) = (TABLE_INVERSE);

#else /* inv --><!-- !inv */

/** Equivalence relation between <typedef:<PN>key> that satisfies
 `<PN>is_equal_fn(a, b) -> <PN>hash(a) == <PN>hash(b)`. */
typedef int (*PN_(is_equal_fn))(PN_(ckey) a, PN_(ckey) b);
/* Check that `TABLE_IS_EQUAL` is a function implementing
 <typedef:<PN>is_equal_fn>. */
static const PN_(is_equal_fn) PN_(equal) = (TABLE_IS_EQUAL);

#endif /* !inv --> */

#ifdef TABLE_VALUE /* <!-- value */
/** Defining `TABLE_VALUE` produces an associative map, otherwise it is the same
 as <typedef:<PN>key>. */
typedef TABLE_VALUE PN_(value);
/** Defining `TABLE_VALUE` creates this map from <typedef:<PN>key> to
 <typedef:<PN>value> as an interface with set. */
struct N_(set_entry) { PN_(key) key; PN_(value) value; };
/** If `TABLE_VALUE`, this is <tag:<N>set_entry>; otherwise, it's the same as
 <typedef:<PN>key>. */
typedef struct N_(set_entry) PN_(entry);
#else /* value --><!-- !value */
typedef PN_(key) PN_(value);
typedef PN_(key) PN_(entry);
#endif /* !value --> */

/** @return Key from `e`. */
static PN_(key) PN_(entry_key)(PN_(entry) e) {
#ifdef TABLE_VALUE
	return e.key;
#else
	return e;
#endif
}

/* Address is hash modulo size of table. Any occupied buckets at the start of
 the linked structure are closed, that is, the address equals the index. These
 form a linked set, possibly with other, open buckets that have the same
 address in vacant buckets. */
struct PN_(bucket) {
	PN_(uint) next; /* Bucket index, including `TABLE_NULL` and `TABLE_END`. */
	PN_(uint) hash;
#ifndef TABLE_INVERSE
	PN_(key) key;
#endif
#ifdef TABLE_VALUE
	PN_(value) value;
#endif
};

/** Gets the key of an occupied `bucket`. */
static PN_(key) PN_(bucket_key)(const struct PN_(bucket) *const bucket) {
	assert(bucket && bucket->next != TABLE_NULL);
#ifdef TABLE_INVERSE
	return PN_(inverse_hash)(bucket->hash);
#else
	return bucket->key;
#endif
}

/** Gets the value of an occupied `bucket`. */
static PN_(value) PN_(bucket_value)(const struct PN_(bucket) *const bucket) {
	assert(bucket && bucket->next != TABLE_NULL);
#ifdef TABLE_VALUE
	return bucket->value;
#else
	return PN_(bucket_key)(bucket);
#endif
}

/** Fills `entry`, a public structure, with the information of `bucket`. */
static void PN_(to_entry)(const struct PN_(bucket) *const bucket,
	PN_(entry) *const entry) {
	assert(bucket && entry);
#ifdef TABLE_VALUE /* entry { <PN>key key; <PN>value value; } */
	entry->key = PN_(bucket_key)(bucket);
	memcpy(&entry->value, &bucket->value, sizeof bucket->value);
#else /* entry <PN>key */
	*entry = PN_(bucket_key)(bucket);
#endif
}

/** Returns true if the `replace` replaces the `original`. */
typedef int (*PN_(policy_fn))(PN_(key) original, PN_(key) replace);

/** To initialize, see <fn:<N>set>, `TABLE_IDLE`, `{0}` (`C99`,) or being
 `static`. The fields should be treated as read-only; any modification is
 liable to cause the set to go into an invalid state.

 ![States.](../web/states.png) */
struct N_(set) { /* "Padding size," good. */
	struct PN_(bucket) *buckets; /* @ has zero/one key specified by `next`. */
	/* Buckets; `size <= capacity`; open stack, including `TABLE_END`. */
	PN_(uint) log_capacity, size, top;
	/* Size is not needed but convenient and allows short-circuiting. */
};

/** The capacity of a non-idle `set` is always a power-of-two. */
static PN_(uint) PN_(capacity)(const struct N_(set) *const set)
	{ return assert(set && set->buckets && set->log_capacity >= 3),
	(PN_(uint))((PN_(uint))1 << set->log_capacity); }

/** @return Indexes the first closed bucket in the set of buckets with the same
 address from non-idle `set` given the `hash`. If the bucket is empty, it will
 have `next = TABLE_NULL` or it's own <fn:<PN>to_bucket> not equal to the index.*/
static PN_(uint) PN_(to_bucket)(const struct N_(set) *const set,
	const PN_(uint) hash) { return hash & (PN_(capacity)(set) - 1); }

/** On return, the `top` of `set` will be empty, but size is not incremented,
 leaving it in intermediate state. This is amortized; every value takes at most
 one. */
static void PN_(grow_stack)(struct N_(set) *const set) {
	PN_(uint) top = set->top;
	assert(set && set->buckets && top);
	top = (top == TABLE_END ? PN_(capacity)(set) : top) - 1;
	while(set->buckets[top].next != TABLE_NULL) assert(top), top--;
	set->top = top;
}

/** Is `i` in `set` possibly on the stack? (The stack grows from the high.) */
static int PN_(in_stack_range)(const struct N_(set) *const set,
	const PN_(uint) i)
	{ return assert(set), set->top != TABLE_END && set->top <= i; }

/***********fixme*/
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)
#ifdef TABLE_TEST
/** `f` `g` */
static void PN_(graph)(const struct N_(set) *f, const char *g);
static void (*PN_(to_string))(PN_(ckey), char (*)[12]);
#else
/** `set` `fn` */
static void PN_(graph)(const struct N_(set) *const set, const char *const fn) {
	(void)set, (void)fn;
}
/** `key` `z` */
static void PN_(to_string)(PN_(ckey) key, char (*z)[12])
	{ (void)key, strcpy(*z, "<key>"); }
#endif

/** Moves the `target` index in non-idle `set`, to the top of collision stack.
 This may result in an inconsistent state; one is responsible for filling that
 hole and linking it with top. */
static void PN_(move_to_top)(struct N_(set) *const set,
	const PN_(uint) target) {
	struct PN_(bucket) *tgt, *top;
	PN_(uint) to_next, next;
	const PN_(uint) capacity = PN_(capacity)(set);
	assert(set->size < capacity && target < capacity);
	PN_(grow_stack)(set);
	tgt = set->buckets + target, top = set->buckets + set->top;
	assert(tgt->next != TABLE_NULL && top->next == TABLE_NULL);
	/* Search for the previous link in the bucket, \O(|bucket|). */
	for(to_next = TABLE_NULL,
		next = PN_(to_bucket)(set, tgt->hash);
		assert(next < capacity), next != target;
		to_next = next, next = set->buckets[next].next);
	/* Move `tgt` to `top`. */
	if(to_next != TABLE_NULL) set->buckets[to_next].next = set->top;
	memcpy(top, tgt, sizeof *tgt), tgt->next = TABLE_NULL;
}

/** `set` will be searched linearly for `key` which has `hash`.
 @fixme Move to front like splay trees? this is awkward. */
static struct PN_(bucket) *PN_(query)(struct N_(set) *const set,
	const PN_(key) key, const PN_(uint) hash) {
	struct PN_(bucket) *bucket;
	PN_(uint) i, next;
	assert(set && set->buckets && set->log_capacity);
	bucket = set->buckets + (i = PN_(to_bucket)(set, hash));
	/* Not the start of a bucket: empty or in the collision stack. */
	if((next = bucket->next) == TABLE_NULL
		|| PN_(in_stack_range)(set, i)
		&& i != PN_(to_bucket)(set, bucket->hash)) return 0;
	for( ; ; ) {
		if(hash == bucket->hash) {
			int entries_are_equal;
#ifdef TABLE_INVERSE
			entries_are_equal = ((void)(key), 1); /* Injective. */
#else
			entries_are_equal = PN_(equal)(key, bucket->key);
#endif
			if(entries_are_equal) return bucket;
		}
		if(next == TABLE_END) return 0;
		bucket = set->buckets + (i = next);
		assert(i < PN_(capacity)(set) && PN_(in_stack_range)(set, i) &&
			i != TABLE_NULL);
		next = bucket->next;
	}
}

/** Ensures that `set` has enough buckets to fill `n` more than the size. May
 invalidate and re-arrange the order.
 @return Success; otherwise, `errno` will be set. @throws[realloc]
 @throws[ERANGE] Tried allocating more then can fit in half <typedef:<PN>uint>
 or `realloc` doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html). */
static int PN_(buffer)(struct N_(set) *const set, const PN_(uint) n) {
	struct PN_(bucket) *buckets;
	const PN_(uint) log_c0 = set->log_capacity,
		c0 = log_c0 ? (PN_(uint))((PN_(uint))1 << log_c0) : 0;
	PN_(uint) log_c1, c1, size1, i, wait, mask;
	char fn[64];
	assert(set && set->size <= TABLE_LIMIT && (!set->buckets && !set->size
		&& !log_c0 && !c0 || set->buckets && set->size <= c0 && log_c0>=3));
	/* Can we satisfy `n` growth from the buffer? */
	if(TABLE_M1 - set->size < n || TABLE_LIMIT < (size1 = set->size + n))
		return errno = ERANGE, 0;
	if(set->buckets)  log_c1 = log_c0, c1 = c0 ? c0 : 1;
	else              log_c1 = 3,      c1 = 8;
	while(c1 < size1) log_c1++, c1 <<= 1;
	if(log_c0 == log_c1) return 1;

	sprintf(fn, "graph/" QUOTE(TABLE_NAME) "-resize-%lu-%lu-a-before.gv",
		(unsigned long)log_c0, (unsigned long)log_c1), PN_(graph)(set, fn);

	/* Otherwise, need to allocate more. */
	if(!(buckets = realloc(set->buckets, sizeof *buckets * c1)))
		{ if(!errno) errno = ERANGE; return 0; }
	set->top = TABLE_END; /* Idle `top` initialized or reset. */
	set->buckets = buckets, set->log_capacity = log_c1;

	/* Initialize new values. Mask to identify the added bits. */
	{ struct PN_(bucket) *e = buckets + c0, *const e_end = buckets + c1;
		for( ; e < e_end; e++) e->next = TABLE_NULL; }
	mask = (PN_(uint))((((PN_(uint))1 << log_c0) - 1)
		^ (((PN_(uint))1 << log_c1) - 1));

	/* Rehash most closed buckets in the lower half. Create waiting
	 linked-stack by borrowing next. */
	wait = TABLE_END;
	for(i = 0; i < c0; i++) {
		struct PN_(bucket) *idx, *go;
		PN_(uint) g, hash;
		idx = set->buckets + i;
		if(idx->next == TABLE_NULL) continue;
		g = PN_(to_bucket)(set, hash = idx->hash);
		/* It's a power-of-two size, so, like consistent hashing, `E[old/new]`
		 capacity that a closed bucket will remain where it is. */
		if(i == g) { idx->next = TABLE_END; continue; }
		if((go = set->buckets + g)->next == TABLE_NULL) {
			/* Priority is given to the first closed bucket; simpler later. */
			struct PN_(bucket) *head;
			PN_(uint) h = g & ~mask; assert(h <= g);
			if(h < g && i < h
				&& (head = set->buckets + h, assert(head->next != TABLE_NULL),
				PN_(to_bucket)(set, head->hash) == g)) {
				memcpy(go, head, sizeof *head);
				go->next = TABLE_END, head->next = TABLE_NULL;
				/* Fall-though -- the bucket still needs to be put on waiting. */
			} else {
				/* If the new bucket is available and this bucket is first. */
				memcpy(go, idx, sizeof *idx);
				go->next = TABLE_END, idx->next = TABLE_NULL;
				continue;
			}
		}
		idx->next = wait, wait = i; /* Push for next sweep. */
	}

	sprintf(fn, "graph/" QUOTE(TABLE_NAME) "-resize-%lu-%lu-b-obvious.gv",
		(unsigned long)log_c0, (unsigned long)log_c1), PN_(graph)(set, fn);

	/* Search waiting stack for buckets that moved concurrently. */
	{ PN_(uint) prev = TABLE_END, w = wait; while(w != TABLE_END) {
		struct PN_(bucket) *waiting = set->buckets + w;
		PN_(uint) cl = PN_(to_bucket)(set, waiting->hash);
		struct PN_(bucket) *const closed = set->buckets + cl;
		assert(cl != w);
		if(closed->next == TABLE_NULL) {
			memcpy(closed, waiting, sizeof *waiting), closed->next = TABLE_END;
			if(prev != TABLE_END) set->buckets[prev].next = waiting->next;
			if(wait == w) wait = waiting->next; /* First, modify head. */
			w = waiting->next, waiting->next = TABLE_NULL;
		} else {
			assert(closed->next == TABLE_END); /* Not in the wait stack. */
			prev = w, w = waiting->next;
		}
	}}

	sprintf(fn, "graph/" QUOTE(TABLE_NAME) "-resize-%lu-%lu-c-closed.gv",
		(unsigned long)log_c0, (unsigned long)log_c1), PN_(graph)(set, fn);

	/* Rebuild the top stack at the high numbers from the waiting at low. */
	while(wait != TABLE_END) {
		struct PN_(bucket) *const waiting = set->buckets + wait;
		PN_(uint) h = PN_(to_bucket)(set, waiting->hash);
		struct PN_(bucket) *const head = set->buckets + h;
		struct PN_(bucket) *top;
		assert(h != wait && head->next != TABLE_NULL);
		PN_(grow_stack)(set), top = set->buckets + set->top;
		memcpy(top, waiting, sizeof *waiting);
		top->next = head->next, head->next = set->top;
		wait = waiting->next, waiting->next = TABLE_NULL; /* Pop. */
	}

	sprintf(fn, "graph/" QUOTE(TABLE_NAME) "-resize-%lu-%lu-d-final.gv",
		(unsigned long)log_c0, (unsigned long)log_c1), PN_(graph)(set, fn);

	return 1;
}

#undef QUOTE_
#undef QUOTE

/** Replace the `key` and `hash` of `bucket`. Don't touch next.
 @fixme `memcpy`? */
static void PN_(replace_key)(struct PN_(bucket) *const bucket,
	const PN_(key) key, const PN_(uint) hash) {
#ifndef TABLE_NO_CACHE
	bucket->hash = hash;
	(void)key;
#endif
#ifndef TABLE_INVERSE
	memcpy(&bucket->key, &key, sizeof key);
	(void)hash;
#endif
}

/** Replace the entire `entry` and `hash` of `bucket`. Don't touch next.
 @fixme `memcpy`? */
static void PN_(replace_entry)(struct PN_(bucket) *const bucket,
	const PN_(entry) entry, const PN_(uint) hash) {
	PN_(replace_key)(bucket, PN_(entry_key)(entry), hash);
#ifdef TABLE_VALUE
	memcpy(&bucket->value, &entry.value, sizeof(entry.value));
#endif
}

/** Evicts the spot where `hash` goes in `set`. Must have at least one free
 bucket. This results in a space in the table, with the next set. */
static struct PN_(bucket) *PN_(evict)(struct N_(set) *const set,
	const PN_(uint) hash) {
	PN_(uint) i;
	struct PN_(bucket) *bucket;
	if(!PN_(buffer)(set, 1)) return 0; /* Amortized. */
	bucket = set->buckets + (i = PN_(to_bucket)(set, hash)); /* Closed. */
	if(bucket->next != TABLE_NULL) { /* Occupied. */
		int in_stack = PN_(to_bucket)(set, bucket->hash) != i;
		PN_(move_to_top)(set, i);
		bucket->next = in_stack ? TABLE_END : set->top;
	} else { /* Unoccupied. */
		bucket->next = TABLE_END;
	}
	set->size++;
	return bucket;
}

/** Put `entry` in `set`. For collisions, only if `update` exists and returns
 true do and displace it to `eject`, if non-null.
 @return A <tag:set_result>. @throws[malloc]
 @order Amortized \O(max bucket length); the key to another bucket may have to
 be moved to the top; the table might be full and have to be resized. */
static enum set_result PN_(put)(struct N_(set) *const set,
	PN_(entry) entry, PN_(entry) *eject, const PN_(policy_fn) update) {
	struct PN_(bucket) *bucket;
	const PN_(key) key = PN_(entry_key)(entry);
	const PN_(uint) hash = PN_(hash)(key);
	enum set_result result;
	assert(set);
	if(set->buckets && (bucket = PN_(query)(set, key, hash))) {
		if(!update || !update(PN_(bucket_key)(bucket), key)) return TABLE_YIELD;
		if(eject) PN_(to_entry)(bucket, eject);
		result = TABLE_REPLACE;
	} else {
		if(!(bucket = PN_(evict)(set, hash))) return TABLE_ERROR;
		result = TABLE_UNIQUE;
	}
	PN_(replace_entry)(bucket, entry, hash);
	return result;
}

#ifdef TABLE_VALUE /* <!-- value */

/** On `TABLE_VALUE`, try to put `key` into `set`, and update `value` to be
 a pointer to the current value.
 @return `TABLE_ERROR` does not set `value`; `TABLE_ABSENT`, the `value` will be
 uninitialized; `TABLE_YIELD`, gets the current `value`. @throws[malloc] */
static enum set_result PN_(compute)(struct N_(set) *const set,
	PN_(key) key, PN_(value) **const value) {
	struct PN_(bucket) *bucket;
	const PN_(uint) hash = PN_(hash)(key);
	enum set_result result;
	assert(set && value);
	if(set->buckets && (bucket = PN_(query)(set, key, hash))) {
		result = TABLE_YIELD;
	} else {
		if(!(bucket = PN_(evict)(set, hash))) return TABLE_ERROR;
		PN_(replace_key)(bucket, key, hash);
		result = TABLE_UNIQUE;
	}
	*value = &bucket->value;
	return result;
}

#endif /* value --> */

/** Initialises `set` to idle. @order \Theta(1) @allow */
static void N_(set)(struct N_(set) *const set) {
	assert(set);
	set->buckets = 0;
	set->log_capacity = 0; set->size = 0; set->top = 0;
}

/** Destroys `set` and returns it to idle. @allow */
static void N_(set_)(struct N_(set) *const set)
	{ assert(set), free(set->buckets); N_(set)(set); }

/** Reserve at least `n` space for buckets of `set`. This will ensure that
 there is space for those buckets and may increase iteration time.
 @return Success.
 @throws[ERANGE] The request was unsatisfiable. @throws[realloc] @allow */
static int N_(set_buffer)(struct N_(set) *const set, const PN_(uint) n)
	{ return assert(set), PN_(buffer)(set, n); }

/** Clears and removes all buckets from `set`. The capacity and memory of the
 `set` is preserved, but all previous values are un-associated. (The load
 factor will be less until it reaches it's previous size.)
 @order \Theta(`set.capacity`) @allow */
static void N_(set_clear)(struct N_(set) *const set) {
	struct PN_(bucket) *b, *b_end;
	assert(set);
	if(!set->buckets) { assert(!set->log_capacity); return; }
	for(b = set->buckets, b_end = b + PN_(capacity)(set); b < b_end; b++)
		b->next = TABLE_NULL;
	set->size = 0;
}

/* set_shrink: if shrinkable, reserve the exact amount in a separate buffer
 and move all. Does not, and indeed cannot, respect the most-recently used
 heuristic. */

/** @return Is `key` in `set`? (which can be null.) @allow */
static int N_(set_is)(struct N_(set) *const set, const PN_(key) key)
	{ return set && set->buckets && PN_(query)(set, key, PN_(hash)(key)); }
/* Fixme: a lot of copying for nothing, are you sure it's optimized? */

/** @param[result] If null, behaves like <fn:<N>set_is>, otherwise, a
 <typedef:<PN>entry> which gets filled on true.
 @return Is `key` in `set`? (which can be null.) @allow */
static int N_(set_query)(struct N_(set) *const set, const PN_(key) key,
	PN_(entry) *const result) {
	struct PN_(bucket) *b;
	if(!set || !set->buckets || !(b = PN_(query)(set, key, PN_(hash)(key))))
		return 0;
	if(result) PN_(to_entry)(b, result);
	return 1;
}

/* set_<P>_get */

/** @return The value associated with `key` in `set`, (which can be null.) If
 no such value exists, the `default_value` is returned.
 @order Average \O(1); worst \O(n).
 @allow */
static PN_(value) N_(set_get_or)(struct N_(set) *const set,
	const PN_(key) key, PN_(value) default_value) {
	struct PN_(bucket) *b;
	return set && set->buckets && (b = PN_(query)(set, key, PN_(hash)(key)))
		? PN_(bucket_value)(b) : default_value;
}

/** Puts `entry` in `set` only if absent.
 @return One of: `TABLE_ERROR` the set is not modified; `TABLE_YIELD` not modified
 if there is another entry with the same key; `TABLE_UNIQUE`, put an entry in the
 set.
 @throws[realloc, ERANGE] There was an error with resizing.
 @order Average amortised \O(1); worst \O(n).
 @allow */
static enum set_result N_(set_try)(struct N_(set) *const set,
	PN_(entry) entry) { return PN_(put)(set, entry, 0, 0); }

/* Callback in <fn:<N>set_replace>.
 @return `original` and `replace` ignored, true. @implements <PN>policy_fn */
static int PN_(always_replace)(const PN_(key) original,
	const PN_(key) replace) { return (void)original, (void)replace, 1; }

/** Puts `entry` in `set`.
 @return One of: `TABLE_ERROR` the set is not modified; `TABLE_REPLACE`, `eject`,
 if non-null, will be filled; `TABLE_UNIQUE`, on a unique entry.
 @throws[realloc, ERANGE] There was an error with resizing.
 @order Average amortised \O(1); worst \O(n).
 @allow */
static enum set_result N_(set_replace)(struct N_(set) *const set,
	PN_(entry) entry, PN_(entry) *eject) {
	return PN_(put)(set, entry, eject, &PN_(always_replace));
}

/** Puts `entry` in `set` only if absent or if calling `update` returns true.
 @return One of: `TABLE_ERROR` the set is not modified; `TABLE_REPLACE` if
 `update` is non-null and returns true, `eject`, if non-null, will be filled;
 `TABLE_YIELD` if `update` is null or false; `TABLE_UNIQUE`, on unique entry.
 @throws[realloc, ERANGE] There was an error with resizing.
 @order Average amortised \O(1); worst \O(n).
 @allow */
static enum set_result N_(set_update)(struct N_(set) *const set,
	PN_(entry) entry, PN_(entry) *eject, const PN_(policy_fn) update)
	{ return PN_(put)(set, entry, eject, update); }

#ifdef TABLE_VALUE /* <!-- value */
/** If `TABLE_VALUE`, try to put `key` into `set`, and store the value in `value`.
 @return `TABLE_ERROR` does not set `value`; `TABLE_GROW`, the `value` will be
 uninitialized; `TABLE_YIELD`, gets the current `value`.
 @throws[malloc] On `TABLE_ERROR`. @allow */
static enum set_result N_(set_compute)(struct N_(set) *const set,
	PN_(key) key, PN_(value) **const value)
	{ return PN_(compute)(set, key, value); }
#endif /* value --> */

#if 0
/** Removes an element `data` from `hash`.
 @return Successfully ejected element or null. @order Average \O(1), (hash
 distributes elements uniformly); worst \O(n). @allow */
static struct N_(setlink) *N_(set_remove)(struct N_(set) *const hash,
	const PN_(mtype) data) {
	PN_(uint) hash;
	struct N_(setlink) **to_x, *x;
	if(!hash || !hash->buckets) return 0;
	hash = PN_(set)(data);
	if(!(to_x = PN_(entry_to)(PN_(get_entry)(hash, hash), hash, data)))
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
struct PN_(iterator) {
	const struct N_(set) *set;
	union {
		const void *const do_not_warn;
		PN_(uint) b;
	} _;
};

/** Loads `hash` (can be null) into `it`. @implements begin */
static void PN_(begin)(struct PN_(iterator) *const it,
	const struct N_(set) *const hash)
	{ assert(it), it->set = hash, it->_.b = 0; }

/** Helper to skip the buckets of `it` that are not there.
 @return Whether it found another index. */
static int PN_(skip)(struct PN_(iterator) *const it) {
	const struct N_(set) *const hash = it->set;
	const PN_(uint) limit = PN_(capacity)(hash);
	assert(it && it->set && it->set->buckets);
	while(it->_.b < limit) {
		struct PN_(bucket) *const bucket = hash->buckets + it->_.b;
		if(bucket->next != TABLE_NULL) return 1;
		it->_.b++;
	}
	return 0;
}

/** Advances `it`. @implements next */
static struct PN_(bucket) *PN_(next)(struct PN_(iterator) *const it) {
	assert(it);
	if(!it->set || !it->set->buckets) return 0;
	if(PN_(skip)(it)) return it->set->buckets + it->_.b++;
	it->set = 0, it->_.b = 0;
	return 0;
}

/* iterate --> */

/** Iteration usually not in any particular order. The asymptotic runtime is
 proportional to the hash capacity. */
struct N_(set_iterator) { struct PN_(iterator) it; };

/** Loads `set` (can be null) into `it`. @allow */
static void N_(set_begin)(struct N_(set_iterator) *const it,
	const struct N_(set) *const set) { PN_(begin)(&it->it, set); }

/** Advances `it`.
 @param[entry] If non-null, the entry is filled with the next element only if
 it has a next. @return Whether it had a next element. @allow */
static int N_(set_next)(struct N_(set_iterator) *const it, PN_(entry) *entry) {
	const struct PN_(bucket) *b = PN_(next)(&it->it);
	return b ? (PN_(to_entry)(b, entry), 1) : 0;
}

/** @return Whether the set specified to `it` in <fn:<N>set_begin> has a next.
 @order Amortized on the capacity, \O(1). @allow */
static int N_(set_has_next)(struct N_(set_iterator) *const it) {
	assert(it);
	return it->it.set && it->it.set->buckets && PN_(skip)(&it->it);
}

#ifdef TABLE_VALUE /* <!-- value */

/** If `TABLE_VALUE`, advances `it` when <fn:<N>set_has_next>.
 @return The next key. @allow */
static PN_(key) N_(set_next_key)(struct N_(set_iterator) *const it)
	{ return PN_(bucket_key)(PN_(next)(&it->it)); }

/** If `TABLE_VALUE`, advances `it` when <fn:<N>set_has_next>.
 @return The next value. @allow */
static PN_(value) N_(set_next_value)(struct N_(set_iterator) *const it)
	{ return PN_(next)(&it->it)->value; }

#endif /* value --> */

/* <!-- box (multiple traits) */
#define BOX_ PN_
#define BOX_CONTAINER struct N_(set)
#define BOX_CONTENTS struct PN_(bucket)

#ifdef TABLE_TEST /* <!-- test */
/* Forward-declare. */
static void (*PN_(to_string))(PN_(ckey), char (*)[12]);
static const char *(*PN_(set_to_string))(const struct N_(set) *);
#include "../test/test_table.h"
#endif /* test --> */

static void PN_(unused_base_coda)(void);
static void PN_(unused_base)(void) {
	PN_(entry) e;
	PN_(key) k;
	PN_(value) v;
	memset(&e, 0, sizeof e);
	memset(&k, 0, sizeof k);
	memset(&v, 0, sizeof v);
	N_(set)(0); N_(set_)(0); N_(set_buffer)(0, 0); N_(set_clear)(0);
	N_(set_is)(0, k); N_(set_query)(0, k, 0); N_(set_get_or)(0, k, v);
	N_(set_try)(0, e); N_(set_replace)(0, e, 0); N_(set_update)(0, e, 0, 0);
	/*N_(set_remove)(0, 0);*/
	N_(set_begin)(0, 0); N_(set_next)(0, 0); N_(set_has_next)(0);
	PN_(unused_base_coda)();
#ifdef TABLE_VALUE
	N_(set_compute)(0, k, 0); N_(set_next_key)(0); N_(set_next_value)(0);
#endif
}
static void PN_(unused_base_coda)(void) { PN_(unused_base)(); }


#elif defined(TABLE_DEFAULT) /* base --><!-- default */

#ifdef TABLE_DEFAULT_NAME
#define ND_(n, m) TABLE_CAT(N_(n), TABLE_CAT(TABLE_DEFAULT_NAME, m))
#else
#define ND_(n, m) TABLE_CAT(N_(n), m)
#endif
#define PND_(n) TABLE_CAT(set_d, ND_(private, n))

/* Check that `TABLE_DEFAULT` is a valid <tag:<PN>value> and that only one time
 can the `TABLE_DEFAULT_NAME` be omitted. */
static const PN_(value) PND_(default_value) = (TABLE_DEFAULT);

/** @return The value associated with `key` in `set`, (which can be null.) If
 no such value exists, the `TABLE_DEFAULT` is returned.
 @order Average \O(1); worst \O(n).
 @allow */
static PN_(value) ND_(set, get)(struct N_(set) *const set, const PN_(key) key) {
	struct PN_(bucket) *b;
	return set && set->buckets && (b = PN_(query)(set, key, PN_(hash)(key)))
		? PN_(bucket_value)(b) : PND_(default_value);
}

static void PND_(unused_default_coda)(void);
static void PND_(unused_default)(void) { PN_(key) k; memset(&k, 0, sizeof k);
	ND_(set, get)(0, k); PND_(unused_default_coda)(); }
static void PND_(unused_default_coda)(void) { PND_(unused_default)(); }

#undef ND_
#undef TABLE_DEFAULT
#ifdef TABLE_DEFAULT_NAME
#undef TABLE_DEFAULT_NAME
#endif


#elif defined(TABLE_TO_STRING) /* default --><!-- to string trait */


#ifdef TABLE_TO_STRING_NAME
#define SZ_(n) TABLE_CAT(N_(set), TABLE_CAT(TABLE_TO_STRING_NAME, n))
#else
#define SZ_(n) TABLE_CAT(N_(set), n)
#endif
#define TSZ_(n) TABLE_CAT(set_sz, SZ_(n))
/* Check that `TABLE_TO_STRING` is a function implementing this prototype. */
static void (*const TSZ_(actual_to_string))(PN_(ckey), char (*const)[12])
	= (TABLE_TO_STRING);
/** This is to line up the hash, which can have <typedef:<PN>key> a pointer or
 not, with to string, which requires a pointer. Call
 <data:<TSZ>actual_to_string> with key of `bucket` and `z`. */
static void TSZ_(thunk_to_string)(const struct PN_(bucket) *const bucket,
	char (*const z)[12]) { TSZ_(actual_to_string)(PN_(bucket_key)(bucket), z); }
#define TO_STRING &TSZ_(thunk_to_string)
#define TO_STRING_LEFT '{'
#define TO_STRING_RIGHT '}'
#include "to_string.h" /** \include */
#ifdef TABLE_TEST /* <!-- expect: greedy satisfy forward-declared. */
#undef TABLE_TEST
static void (*PN_(to_string))(PN_(ckey), char (*const)[12])
	= TSZ_(actual_to_string);
static const char *(*PN_(set_to_string))(const struct N_(set) *)
	= &SZ_(to_string);
#endif /* expect --> */
#undef TSZ_
#undef SZ_
#undef TABLE_TO_STRING
#ifdef TABLE_TO_STRING_NAME
#undef TABLE_TO_STRING_NAME
#endif


#endif /* traits --> */


#ifdef TABLE_EXPECT_TRAIT /* <!-- trait */
#undef TABLE_EXPECT_TRAIT
#else /* trait --><!-- !trait */
#ifdef TABLE_TEST
#error No TABLE_TO_STRING traits defined for TABLE_TEST.
#endif
#undef TABLE_NAME
#undef TABLE_KEY
#undef TABLE_UINT
#undef TABLE_HASH
#ifdef TABLE_IS_EQUAL
#undef TABLE_IS_EQUAL
#else
#undef TABLE_INVERSE
#endif
#ifdef TABLE_VALUE
#undef TABLE_VALUE
#endif
#undef BOX_
#undef BOX_CONTAINER
#undef BOX_CONTENTS
/* box (multiple traits) --> */
#endif /* !trait --> */
#undef TABLE_DEFAULT_TRAIT
#undef TABLE_TO_STRING_TRAIT
#undef TABLE_TRAITS
