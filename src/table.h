/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Hash table

 ![Example of <string>table.](../web/table.png)

 <tag:<N>table> implements a set or map of <typedef:<PN>entry> as a hash table.
 It must be supplied a <typedef:<PN>hash_fn> and, <typedef:<PN>is_equal_fn> or
 <typedef:<PN>inverse_hash_fn>.

 @param[TABLE_NAME, TABLE_KEY]
 `<N>` that satisfies `C` naming conventions when mangled and a valid
 <typedef:<PN>key> associated therewith; required. `<PN>` is private, whose
 names are prefixed in a manner to avoid collisions.

 @param[TABLE_HASH, TABLE_IS_EQUAL, TABLE_INVERSE]
 `TABLE_HASH`, and either `TABLE_IS_EQUAL` or `TABLE_INVERSE`, but not both,
 are required. Function satisfying <typedef:<PN>hash_fn>, and <typedef:<PN>is_equal_fn> or <typedef:<PN>inverse_hash_fn>.

 @param[TABLE_VALUE]
 An optional type that is the payload of the key, thus making this an
 associative array. If the key is part of an aggregate value, it will be
 more efficient and robust to use a type conversion instead of storing
 related pointers.

 @param[TABLE_UINT]
 This is <typedef:<PN>uint>, the unsigned type of hash hash of the key given by
 <typedef:<PN>hash_fn>; defaults to `size_t`.

 @param[TABLE_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[TABLE_DEFAULT_NAME, TABLE_DEFAULT]
 Default trait; a name that satisfies `C` naming conventions when mangled and a
 <typedef:<PN>value> used in <fn:<N>table<D>get>. There can be multiple to
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
#define PN_(n) TABLE_CAT(table, N_(n))
#define TABLE_IDLE { 0, 0, 0, 0 }
/* Use the sign bit to store out-of-band flags when a <typedef:<PN>uint>
 represents an address in the table, (such that range of an index is one bit
 less.) Choose representations that probably save power. We cannot save this in
 an `enum` because we don't know maximum. */
#define TABLE_M1 ((PN_(uint))~(PN_(uint))0) /* 2's compliment -1. */
#define TABLE_HIGH ((TABLE_M1 >> 1) + 1) /* Cardinality. */
#define TABLE_END (TABLE_HIGH)
#define TABLE_NULL (TABLE_HIGH + 1)
#define TABLE_RESULT X(ERROR), X(UNIQUE), X(YIELD), X(REPLACE)
#define X(n) TABLE_##n
/** This is the result of modifying the table, of which `TABLE_ERROR` is false.
 ![A diagram of the result states.](../web/put.png) */
enum table_result { TABLE_RESULT };
#undef X
#define X(n) #n
/** A static array of strings describing the <tag:table_result>. */
static const char *const table_result_str[] = { TABLE_RESULT };
#undef X
#undef TABLE_RESULT
#endif /* idempotent --> */


#if TABLE_TRAITS == 0 /* <!-- base table */


#ifndef TABLE_UINT
#define TABLE_UINT size_t
#endif

/** <typedef:<PN>hash_fn> returns this hash type by `TABLE_UINT`, which must be
 be an unsigned integer. Places a simplifying limit on the maximum number of
 elements of half the cardinality. */
typedef TABLE_UINT PN_(uint);

/** Valid tag type defined by `TABLE_KEY` used for keys. */
typedef TABLE_KEY PN_(key);
/** Read-only <typedef:<PN>key>. Makes the simplifying assumption that this is
 not `const`-qualified. */
typedef const TABLE_KEY PN_(ckey);

/** A map from <typedef:<PN>ckey> onto <typedef:<PN>uint> that, ideally, should
 be easy to compute while minimizing duplicate addresses. Must be consistent
 while in the table. If <typedef:<PN>key> is a pointer, one is permitted to
 have null in the domain. */
typedef PN_(uint) (*PN_(hash_fn))(PN_(ckey));
/* Check that `TABLE_HASH` is a function implementing <typedef:<PN>hash_fn>. */
static const PN_(hash_fn) PN_(hash) = (TABLE_HASH);

#ifdef TABLE_INVERSE /* <!-- inv */

/** Defining `TABLE_INVERSE` says <typedef:<PN>hash_fn> forms a bijection
 between the range in <typedef:<PN>key> and the image in <typedef:<PN>uint>.
 The keys are not stored in the hash table at all, but rely on this, the
 inverse-mapping. */
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
/** Defining `TABLE_VALUE` produces an associative map, otherwise it is the
 same as <typedef:<PN>key>. */
typedef TABLE_VALUE PN_(value);
/** Defining `TABLE_VALUE` creates this map from <typedef:<PN>key> to
 <typedef:<PN>value> as an interface with table. In general, reducing the size
 of these elements will be better for performance. */
struct N_(table_entry) { PN_(key) key; PN_(value) value; };
/** If `TABLE_VALUE`, this is <tag:<N>table_entry>; otherwise, it's the same as
 <typedef:<PN>key>. */
typedef struct N_(table_entry) PN_(entry);
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

/* Address is hash modulo size of table. Any occupied buckets at the head of
 the linked structure are closed, that is, the address equals the index. These
 form a linked table, possibly with other, open buckets that have the same
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

/** Gets the value of an occupied `bucket`, which might be the same as the
 key. */
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

/** To initialize, see <fn:<N>table>, `TABLE_IDLE`, `{0}` (`C99`,) or being
 `static`. The fields should be treated as read-only; any modification is
 liable to cause the table to go into an invalid state.

 ![States.](../web/states.png) */
struct N_(table) { /* "Padding size," good. */
	struct PN_(bucket) *buckets; /* @ has zero/one key specified by `next`. */
	/* `size <= capacity`; size is not needed but convenient and allows
	 short-circuiting. Index of the top of the stack; however, we are really
	 lazy, so MSB store is the top a step ahead? Thereby, hysteresis. */
	PN_(uint) log_capacity, size, top;
};
/***********fixme*/
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)
#ifdef TABLE_TEST
/** `f` `g` */
static void PN_(graph)(const struct N_(table) *f, const char *g);
static void (*PN_(to_string))(PN_(ckey), char (*)[12]);
#else
/** `table` `fn` */
static void PN_(graph)(const struct N_(table) *const table,
	const char *const fn) { (void)table, (void)fn; }
/** `key` `z` */
static void PN_(to_string)(PN_(ckey) key, char (*z)[12])
	{ (void)key, strcpy(*z, "<key>"); }
#endif

/** The capacity of a non-idle `table` is always a power-of-two. */
static PN_(uint) PN_(capacity)(const struct N_(table) *const table)
	{ return assert(table && table->buckets && table->log_capacity >= 3),
	(PN_(uint))((PN_(uint))1 << table->log_capacity); }

/** @return Indexes the first closed bucket in the set of buckets with the same
 address from non-idle `table` given the `hash`. If the bucket is empty, it
 will have `next = TABLE_NULL` or it's own <fn:<PN>to_bucket> not equal to the
 index. */
static PN_(uint) PN_(to_bucket)(const struct N_(table) *const table,
	const PN_(uint) hash) { return hash & (PN_(capacity)(table) - 1); }

/** @return Search for the previous link in the bucket to `b` in `table`, if it
 exists, (by restarting and going though the list.) @order \O(`bucket size`) */
static struct PN_(bucket) *PN_(prev)(const struct N_(table) *const table,
	const PN_(uint) b) {
	const struct PN_(bucket) *const bucket = table->buckets + b;
	const PN_(uint) capacity = PN_(capacity)(table);
	PN_(uint) to_next = TABLE_NULL, next;
	assert(table && bucket->next != TABLE_NULL);
	/* Note that this does not check for corrupted tables; would get assert. */
	for(next = PN_(to_bucket)(table, bucket->hash);
		assert(next < capacity), next != b;
		to_next = next, next = table->buckets[next].next);
	return to_next != TABLE_NULL ? table->buckets + to_next : 0;
}

/* <!-- stack */

/** On return, the `top` of `table` will be empty and eager, but size is not
 incremented, leaving it in intermediate state. Amortized if you grow only. */
static void PN_(grow_stack)(struct N_(table) *const table) {
	/* Subtract one for eager. */
	PN_(uint) top = (table->top & ~TABLE_HIGH) - !(table->top & TABLE_HIGH);
	char z[12];
	assert(table && table->buckets && table->top && top < PN_(capacity)(table));
	while(table->buckets[top].next != TABLE_NULL)
		assert(top), PN_(to_string)(PN_(bucket_key)(table->buckets + top), &z),
		printf("grow_stack: skipping %lu: %s.\n", (unsigned long)top, z),
		top--;
	table->top = top; /* Eager, since one is allegedly going to fill it. */
}

/** Force the evaluation of the stack of `table`, thereby making it eager. This
 is like searching for a bucket in open-addressing. @order \O(`buckets`) */
static void PN_(force_stack)(struct N_(table) *const table) {
	PN_(uint) top = table->top;
	if(top & TABLE_HIGH) { /* Lazy. */
		const PN_(uint) cap = PN_(capacity)(table);
		struct PN_(bucket) *bucket;
		char z[12];
		top &= ~TABLE_HIGH;
		do bucket = table->buckets + ++top, assert(top < cap);
		while(bucket->next != TABLE_NULL
			&& (PN_(to_string)(PN_(bucket_key)(bucket), &z), printf("skip %s\n", z), PN_(to_bucket)(table, bucket->hash) == top));
		table->top = top; /* Eager. */
	}
}

/** Is `i` in `table` possibly on the stack? (The stack grows from the high.) */
static int PN_(in_stack_range)(const struct N_(table) *const table,
	const PN_(uint) i) {
	return assert(table && table->buckets),
		(table->top & ~TABLE_HIGH) + !!(table->top & TABLE_HIGH) <= i;
}

/** Corrects newly-deleted `i` from `table` in the stack. */
static void PN_(shrink_stack)(struct N_(table) *const table,
	const PN_(uint) i) {
	assert(table && table->buckets && i < PN_(capacity)(table));
	assert(table->buckets[i].next == TABLE_NULL);
	printf("shrink_stack(%lx)\n", (unsigned long)i);
	if(!PN_(in_stack_range)(table, i)) return;
	PN_(force_stack)(table); /* Only have room for 1 step of laziness. */
	assert(PN_(in_stack_range)(table, i)); /* I think this is assured? Think. */
	if(i != table->top) {
		struct PN_(bucket) *const prev = PN_(prev)(table, table->top);
		memcpy(table->buckets + i, table->buckets + table->top,
			sizeof *table->buckets);
		prev->next = i;
		printf("top 0x%lx\n", (unsigned long)table->top);
		printf("i 0x%lx\n", (unsigned long)i);
		printf("prev 0x%lx\n", (unsigned long)(prev - table->buckets));
	}
	table->buckets[table->top].next = TABLE_NULL;
	table->top |= TABLE_HIGH; /* Lazy. */
}

/** Moves the `m` index in non-idle `table`, to the top of collision stack.
 This may result in an inconsistent state; one is responsible for filling that
 hole and linking it with top. */
static void PN_(move_to_top)(struct N_(table) *const table, const PN_(uint) m) {
	struct PN_(bucket) *move, *top, *prev;
	const PN_(uint) capacity = PN_(capacity)(table);
	assert(table->size < capacity && m < capacity);
	PN_(grow_stack)(table); /* Leaves it in an eager state. */
	move = table->buckets + m, top = table->buckets + table->top;
	assert(move->next != TABLE_NULL && top->next == TABLE_NULL);
	if(prev = PN_(prev)(table, m)) prev->next = table->top; /* \O(|`bucket`|) */
	memcpy(top, move, sizeof *move), move->next = TABLE_NULL;
}

/* stack --> */


/** `TABLE_INVERSE` is injective, so in that case, we only compare hashes.
 @return `a` and `b`. */
static int PN_(equal_buckets)(const PN_(ckey) a, const PN_(ckey) b) {
#ifdef TABLE_INVERSE
	return (void)a, (void)b, 1;
#else
	return PN_(equal)(a, b);
#endif
}

/** `table` will be searched linearly for `key` which has `hash`.
 @fixme Move to front like splay trees? this is awkward. */
static struct PN_(bucket) *PN_(query)(struct N_(table) *const table,
	const PN_(ckey) key, const PN_(uint) hash) {
	struct PN_(bucket) *bucket;
	PN_(uint) i, next;
	assert(table && table->buckets && table->log_capacity);
	bucket = table->buckets + (i = PN_(to_bucket)(table, hash));
	/* Not the start of a bucket: empty or in the collision stack. */
	if((next = bucket->next) == TABLE_NULL
		|| PN_(in_stack_range)(table, i)
		&& i != PN_(to_bucket)(table, bucket->hash)) return 0;
	while(hash != bucket->hash
		|| !PN_(equal_buckets)(key, PN_(bucket_key)(bucket))) {
		if(next == TABLE_END) return 0;
		bucket = table->buckets + (i = next);
		assert(i < PN_(capacity)(table) && PN_(in_stack_range)(table, i) &&
			i != TABLE_NULL);
		next = bucket->next;
	}
	return bucket;
}

/** Ensures that `table` has enough buckets to fill `n` more than the size. May
 invalidate and re-arrange the order.
 @return Success; otherwise, `errno` will be set. @throws[realloc]
 @throws[ERANGE] Tried allocating more then can fit in half <typedef:<PN>uint>
 or `realloc` doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html). */
static int PN_(buffer)(struct N_(table) *const table, const PN_(uint) n) {
	struct PN_(bucket) *buckets;
	const PN_(uint) log_c0 = table->log_capacity,
		c0 = log_c0 ? (PN_(uint))((PN_(uint))1 << log_c0) : 0;
	PN_(uint) log_c1, c1, size1, i, wait, mask;
	char fn[64];
	assert(table && table->size <= TABLE_HIGH
		&& (!table->buckets && !table->size && !log_c0 && !c0
		|| table->buckets && table->size <= c0 && log_c0>=3));
	/* Can we satisfy `n` growth from the buffer? */
	if(TABLE_M1 - table->size < n || TABLE_HIGH < (size1 = table->size + n))
		return errno = ERANGE, 0;
	if(table->buckets) log_c1 = log_c0, c1 = c0 ? c0 : 1;
	else               log_c1 = 3,      c1 = 8;
	while(c1 < size1)  log_c1++,        c1 <<= 1;
	if(log_c0 == log_c1) return 1;

	sprintf(fn, "graph/" QUOTE(TABLE_NAME) "-resize-%lu-%lu-a-before.gv",
		(unsigned long)log_c0, (unsigned long)log_c1), PN_(graph)(table, fn);

	/* Otherwise, need to allocate more. */
	if(!(buckets = realloc(table->buckets, sizeof *buckets * c1)))
		{ if(!errno) errno = ERANGE; return 0; }
	table->top = (c1 - 1) | TABLE_HIGH; /* No stack. */
	table->buckets = buckets, table->log_capacity = log_c1;

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
		idx = table->buckets + i;
		if(idx->next == TABLE_NULL) continue;
		g = PN_(to_bucket)(table, hash = idx->hash);
		/* It's a power-of-two size, so, like consistent hashing, `E[old/new]`
		 capacity that a closed bucket will remain where it is. */
		if(i == g) { idx->next = TABLE_END; continue; }
		if((go = table->buckets + g)->next == TABLE_NULL) {
			/* Priority is given to the first closed bucket; simpler later. */
			struct PN_(bucket) *head;
			PN_(uint) h = g & ~mask; assert(h <= g);
			if(h < g && i < h
				&& (head = table->buckets + h, assert(head->next != TABLE_NULL),
				PN_(to_bucket)(table, head->hash) == g)) {
				memcpy(go, head, sizeof *head);
				go->next = TABLE_END, head->next = TABLE_NULL;
				/* Fall-though -- the bucket still needs to be put on wait. */
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
		(unsigned long)log_c0, (unsigned long)log_c1), PN_(graph)(table, fn);

	/* Search waiting stack for buckets that moved concurrently. */
	{ PN_(uint) prev = TABLE_END, w = wait; while(w != TABLE_END) {
		struct PN_(bucket) *waiting = table->buckets + w;
		PN_(uint) cl = PN_(to_bucket)(table, waiting->hash);
		struct PN_(bucket) *const closed = table->buckets + cl;
		assert(cl != w);
		if(closed->next == TABLE_NULL) {
			memcpy(closed, waiting, sizeof *waiting), closed->next = TABLE_END;
			if(prev != TABLE_END) table->buckets[prev].next = waiting->next;
			if(wait == w) wait = waiting->next; /* First, modify head. */
			w = waiting->next, waiting->next = TABLE_NULL;
		} else {
			assert(closed->next == TABLE_END); /* Not in the wait stack. */
			prev = w, w = waiting->next;
		}
	}}

	sprintf(fn, "graph/" QUOTE(TABLE_NAME) "-resize-%lu-%lu-c-closed.gv",
		(unsigned long)log_c0, (unsigned long)log_c1), PN_(graph)(table, fn);

	/* Rebuild the top stack at the high numbers from the waiting at low. */
	while(wait != TABLE_END) {
		struct PN_(bucket) *const waiting = table->buckets + wait;
		PN_(uint) h = PN_(to_bucket)(table, waiting->hash);
		struct PN_(bucket) *const head = table->buckets + h;
		struct PN_(bucket) *top;
		assert(h != wait && head->next != TABLE_NULL);
		PN_(grow_stack)(table), top = table->buckets + table->top;
		memcpy(top, waiting, sizeof *waiting);
		top->next = head->next, head->next = table->top;
		wait = waiting->next, waiting->next = TABLE_NULL; /* Pop. */
	}

	sprintf(fn, "graph/" QUOTE(TABLE_NAME) "-resize-%lu-%lu-d-final.gv",
		(unsigned long)log_c0, (unsigned long)log_c1), PN_(graph)(table, fn);

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

/** Evicts the spot where `hash` goes in `table`. This results in a space in
 the table. */
static struct PN_(bucket) *PN_(evict)(struct N_(table) *const table,
	const PN_(uint) hash) {
	PN_(uint) i;
	struct PN_(bucket) *bucket;
	if(!PN_(buffer)(table, 1)) return 0; /* Amortized. */
	bucket = table->buckets + (i = PN_(to_bucket)(table, hash)); /* Closed. */
	if(bucket->next != TABLE_NULL) { /* Occupied. */
		int in_stack = PN_(to_bucket)(table, bucket->hash) != i;
		PN_(move_to_top)(table, i);
		bucket->next = in_stack ? TABLE_END : table->top;
	} else { /* Unoccupied. */
		bucket->next = TABLE_END;
	}
	table->size++;
	return bucket;
}

/** Put `entry` in `table`. For collisions, only if `update` exists and returns
 true do and displace it to `eject`, if non-null.
 @return A <tag:table_result>. @throws[malloc]
 @order Amortized \O(max bucket length); the key to another bucket may have to
 be moved to the top; the table might be full and have to be resized. */
static enum table_result PN_(put)(struct N_(table) *const table,
	PN_(entry) entry, PN_(entry) *eject, const PN_(policy_fn) update) {
	struct PN_(bucket) *bucket;
	const PN_(key) key = PN_(entry_key)(entry);
	const PN_(uint) hash = PN_(hash)(key);
	enum table_result result;
	assert(table);
	if(table->buckets && (bucket = PN_(query)(table, key, hash))) {
		if(!update || !update(PN_(bucket_key)(bucket), key)) return TABLE_YIELD;
		if(eject) PN_(to_entry)(bucket, eject);
		result = TABLE_REPLACE;
	} else {
		if(!(bucket = PN_(evict)(table, hash))) return TABLE_ERROR;
		result = TABLE_UNIQUE;
	}
	PN_(replace_entry)(bucket, entry, hash);
	return result;
}

#ifdef TABLE_VALUE /* <!-- value */

/** On `TABLE_VALUE`, try to put `key` into `table`, and update `value` to be
 a pointer to the current value.
 @return `TABLE_ERROR` does not set `value`; `TABLE_ABSENT`, the `value` will be
 uninitialized; `TABLE_YIELD`, gets the current `value`. @throws[malloc] */
static enum table_result PN_(compute)(struct N_(table) *const table,
	PN_(key) key, PN_(value) **const value) {
	struct PN_(bucket) *bucket;
	const PN_(uint) hash = PN_(hash)(key);
	enum table_result result;
	assert(table && value);
	if(table->buckets && (bucket = PN_(query)(table, key, hash))) {
		result = TABLE_YIELD;
	} else {
		if(!(bucket = PN_(evict)(table, hash))) return TABLE_ERROR;
		PN_(replace_key)(bucket, key, hash);
		result = TABLE_UNIQUE;
	}
	*value = &bucket->value;
	return result;
}

#endif /* value --> */

/** Initialises `table` to idle. @order \Theta(1) @allow */
static void N_(table)(struct N_(table) *const table) {
	assert(table);
	table->buckets = 0;
	table->log_capacity = 0; table->size = 0; table->top = 0;
}

/** Destroys `table` and returns it to idle. @allow */
static void N_(table_)(struct N_(table) *const table)
	{ assert(table), free(table->buckets); N_(table)(table); }

/** Reserve at least `n` space for buckets of `table`. This will ensure that
 there is space for those buckets and may increase iteration time.
 @return Success.
 @throws[ERANGE] The request was unsatisfiable. @throws[realloc] @allow */
static int N_(table_buffer)(struct N_(table) *const table, const PN_(uint) n)
	{ return assert(table), PN_(buffer)(table, n); }

/** Clears and removes all buckets from `table`. The capacity and memory of the
 `table` is preserved, but all previous values are un-associated. (The load
 factor will be less until it reaches it's previous size.)
 @order \Theta(`table.capacity`) @allow */
static void N_(table_clear)(struct N_(table) *const table) {
	struct PN_(bucket) *b, *b_end;
	assert(table);
	if(!table->buckets) { assert(!table->log_capacity); return; }
	assert(table->log_capacity);
	for(b = table->buckets, b_end = b + PN_(capacity)(table); b < b_end; b++)
		b->next = TABLE_NULL;
	table->size = 0;
	table->top = (PN_(capacity)(table) - 1) & TABLE_HIGH;
}

/* table_shrink: if shrinkable, reserve the exact amount in a separate buffer
 and move all. Does not, and indeed cannot, respect the most-recently used
 heuristic. */

/** @return Whether `key` is in `table` (which can be null.) @allow */
static int N_(table_is)(struct N_(table) *const table, const PN_(key) key)
	{ return table && table->buckets
		? !!PN_(query)(table, key, PN_(hash)(key)) : 0; }

/** @param[result] If null, behaves like <fn:<N>table_at>, otherwise, a
 <typedef:<PN>entry> which gets filled on true.
 @return Is `key` in `table`? (which can be null.) @allow */
static int N_(table_query)(struct N_(table) *const table, const PN_(key) key,
	PN_(entry) *const result) {
	struct PN_(bucket) *bucket;
	if(!table || !table->buckets
		|| !(bucket = PN_(query)(table, key, PN_(hash)(key)))) return 0;
	if(result) PN_(to_entry)(bucket, result);
	return 1;
}

/** @return The value associated with `key` in `table`, (which can be null.) If
 no such value exists, the `default_value` is returned.
 @order Average \O(1); worst \O(n).
 @allow */
static PN_(value) N_(table_get_or)(struct N_(table) *const table,
	const PN_(key) key, PN_(value) default_value) {
	struct PN_(bucket) *bucket;
	return table && table->buckets
		&& (bucket = PN_(query)(table, key, PN_(hash)(key)))
		? PN_(bucket_value)(bucket) : default_value;
}

/** Puts `entry` in `table` only if absent.
 @return One of: `TABLE_ERROR`, the table is not modified; `TABLE_YIELD`, not
 modified if there is another entry with the same key; `TABLE_UNIQUE`, put an
 entry in the table.
 @throws[realloc, ERANGE] There was an error with resizing.
 @order Average amortised \O(1); worst \O(n). @allow */
static enum table_result N_(table_try)(struct N_(table) *const table,
	PN_(entry) entry) { return PN_(put)(table, entry, 0, 0); }

/** Callback in <fn:<N>table_replace>.
 @return `original` and `replace` ignored, true.
 @implements <typedef:<PN>policy_fn> */
static int PN_(always_replace)(const PN_(key) original,
	const PN_(key) replace) { return (void)original, (void)replace, 1; }

/** Puts `entry` in `table`.
 @return One of: `TABLE_ERROR`, the table is not modified; `TABLE_REPLACE`, the
 `entry` is put if the table, and, if non-null, `eject` will be filled;
 `TABLE_UNIQUE`, on a unique entry.
 @throws[realloc, ERANGE] There was an error with resizing.
 @order Average amortised \O(1); worst \O(n). @allow */
static enum table_result N_(table_replace)(struct N_(table) *const table,
	PN_(entry) entry, PN_(entry) *eject) {
	return PN_(put)(table, entry, eject, &PN_(always_replace));
}

/** Puts `entry` in `table` only if absent or if calling `update` returns true.
 @return One of: `TABLE_ERROR`, the table is not modified; `TABLE_REPLACE`, if
 `update` is non-null and returns true, if non-null, `eject` will be filled;
 `TABLE_YIELD`, if `update` is null or false; `TABLE_UNIQUE`, on unique entry.
 @throws[realloc, ERANGE] There was an error with resizing.
 @order Average amortised \O(1); worst \O(n). @allow */
static enum table_result N_(table_update)(struct N_(table) *const table,
	PN_(entry) entry, PN_(entry) *eject, const PN_(policy_fn) update)
	{ return PN_(put)(table, entry, eject, update); }

#ifdef TABLE_VALUE /* <!-- value */
/** If `TABLE_VALUE`, try to put `key` into `table`, and store the value in
 `value`. @return `TABLE_ERROR` does not set `value`; `TABLE_GROW`, the `value`
 will be uninitialized; `TABLE_YIELD`, gets the current `value`.
 @throws[malloc] On `TABLE_ERROR`. @allow */
static enum table_result N_(table_compute)(struct N_(table) *const table,
	PN_(key) key, PN_(value) **const value)
	{ return PN_(compute)(table, key, value); }
#endif /* value --> */

/** Removes `key` from `table` (which could be null.)
 @return Whether that `key` was in `table`. @order Average \O(1), (hash
 distributes elements uniformly); worst \O(n). @allow */
static int N_(table_remove)(struct N_(table) *const table,
	const PN_(key) key) {
	struct PN_(bucket) *target;
	PN_(uint) i, prv = TABLE_NULL, nxt, hash = PN_(hash)(key);
	char z[12];
	assert(table);
	if(!table || !table->buckets || !table->size) return 0;
	/* Find item and keep track of previous. */
	target = table->buckets + (i = PN_(to_bucket)(table, hash));
	if((nxt = target->next) == TABLE_NULL
		|| PN_(in_stack_range)(table, i)
		&& i != PN_(to_bucket)(table, target->hash)) return 0;
	while(hash != target->hash
		|| !PN_(equal_buckets)(key, PN_(bucket_key)(target))) {
		if(nxt == TABLE_END) return 0;
		prv = i, target = table->buckets + (i = nxt);
		assert(i < PN_(capacity)(table) && PN_(in_stack_range)(table, i)
			&& i != TABLE_NULL);
		nxt = target->next;
	}
	PN_(to_string)(key, &z);
	printf("remove key %s: prev %lx, i %lx, next %lx\n",
		z, (unsigned long)prv, (unsigned long)i, (unsigned long)nxt);
	if(prv != TABLE_NULL) { /* Open entry. */
		struct PN_(bucket) *previous = table->buckets + prv;
		PN_(to_string)(PN_(bucket_key)(previous), &z);
		printf("prev was %s, making it point to next\n", z);
		previous->next = target->next;
	} else if(target->next != TABLE_END) { /* Head closed entry and others. */
		struct PN_(bucket) *const second = table->buckets + (i = target->next);
		printf("closed head %s, replacing it with next\n", z);
		assert(target->next < PN_(capacity)(table));
		memcpy(target, second, sizeof *second);
		target = second;
	}
	target->next = TABLE_NULL, table->size--, PN_(shrink_stack)(table, i);
	return 1;
}

/* <!-- private iterate interface */

/* Contains all iteration parameters. */
struct PN_(iterator) {
	const struct N_(table) *table;
	union {
		const void *const do_not_warn;
		PN_(uint) b;
	} _;
};

/** Loads `hash` (can be null) into `it`. @implements begin */
static void PN_(begin)(struct PN_(iterator) *const it,
	const struct N_(table) *const hash)
	{ assert(it), it->table = hash, it->_.b = 0; }

/** Helper to skip the buckets of `it` that are not there.
 @return Whether it found another index. */
static int PN_(skip)(struct PN_(iterator) *const it) {
	const struct N_(table) *const hash = it->table;
	const PN_(uint) limit = PN_(capacity)(hash);
	assert(it && it->table && it->table->buckets);
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
	if(!it->table || !it->table->buckets) return 0;
	if(PN_(skip)(it)) return it->table->buckets + it->_.b++;
	it->table = 0, it->_.b = 0;
	return 0;
}

/* iterate --> */

/** Iteration usually not in any particular order. The asymptotic runtime of
 iterating though the whole table is proportional to the capacity. */
struct N_(table_iterator) { struct PN_(iterator) it; };

/** Loads `table` (can be null) into `it`. @allow */
static void N_(table_begin)(struct N_(table_iterator) *const it,
	const struct N_(table) *const table) { PN_(begin)(&it->it, table); }

/** Advances `it`.
 @param[entry] If non-null, the entry is filled with the next element only if
 it has a next. @return Whether it had a next element. @allow */
static int N_(table_next)(struct N_(table_iterator) *const it,
	PN_(entry) *entry) {
	const struct PN_(bucket) *b = PN_(next)(&it->it);
	return b ? (PN_(to_entry)(b, entry), 1) : 0;
}

/** @return Whether the table specified to `it` in <fn:<N>table_begin> has a
 next element. @order Amortized on the capacity, \O(1). @allow */
static int N_(table_has_next)(struct N_(table_iterator) *const it) {
	assert(it);
	return it->it.table && it->it.table->buckets && PN_(skip)(&it->it);
}

#ifdef TABLE_VALUE /* <!-- value */

/** If `TABLE_VALUE`, advances `it` when <fn:<N>table_has_next>.
 @return The next key. @allow */
static PN_(key) N_(table_next_key)(struct N_(table_iterator) *const it)
	{ return PN_(bucket_key)(PN_(next)(&it->it)); }

/** If `TABLE_VALUE`, advances `it` when <fn:<N>table_has_next>.
 @return The next value. @allow */
static PN_(value) N_(table_next_value)(struct N_(table_iterator) *const it)
	{ return PN_(next)(&it->it)->value; }

#endif /* value --> */

/* <!-- box (multiple traits) */
#define BOX_ PN_
#define BOX_CONTAINER struct N_(table)
#define BOX_CONTENTS struct PN_(bucket)

#ifdef TABLE_TEST /* <!-- test */
/* Forward-declare. */
static void (*PN_(to_string))(PN_(ckey), char (*)[12]);
static const char *(*PN_(table_to_string))(const struct N_(table) *);
#include "../test/test_table.h"
#endif /* test --> */

static void PN_(unused_base_coda)(void);
static void PN_(unused_base)(void) {
	PN_(entry) e; PN_(key) k; PN_(value) v;
	memset(&e, 0, sizeof e); memset(&k, 0, sizeof k); memset(&v, 0, sizeof v);
	N_(table)(0); N_(table_)(0); N_(table_buffer)(0, 0); N_(table_clear)(0);
	N_(table_is)(0, k); N_(table_query)(0, k, 0); N_(table_get_or)(0, k, v);
	N_(table_try)(0, e); N_(table_replace)(0, e, 0); N_(table_update)(0,e,0,0);
	N_(table_remove)(0, 0); N_(table_begin)(0, 0); N_(table_next)(0, 0);
	N_(table_has_next)(0); PN_(unused_base_coda)();
#ifdef TABLE_VALUE
	N_(table_compute)(0, k, 0); N_(table_next_key)(0); N_(table_next_value)(0);
#endif
}
static void PN_(unused_base_coda)(void) { PN_(unused_base)(); }


#elif defined(TABLE_DEFAULT) /* base --><!-- default */


#ifdef TABLE_DEFAULT_NAME
#define N_D_(n, m) TABLE_CAT(N_(n), TABLE_CAT(TABLE_DEFAULT_NAME, m))
#define PN_D_(n, m) TABLE_CAT(table, N_D_(n, m))
#else
#define N_D_(n, m) TABLE_CAT(N_(n), m)
#define PN_D_(n, m) TABLE_CAT(table, N_D_(n, m))
#endif

/* Check that `TABLE_DEFAULT` is a valid <tag:<PN>value> and that only one
 `TABLE_DEFAULT_NAME` is omitted. */
static const PN_(value) PN_D_(default, value) = (TABLE_DEFAULT);

/** This is functionally identical to <fn:<N>table_get_or>, but a with a trait
 specifying a constant default value, (such as zero.)
 @return The value associated with `key` in `table`, (which can be null.) If
 no such value exists, the `TABLE_DEFAULT` is returned.
 @order Average \O(1); worst \O(n). @allow */
static PN_(value) N_D_(table, get)(struct N_(table) *const table,
	const PN_(key) key) {
	struct PN_(bucket) *bucket;
	return table && table->buckets
		&& (bucket = PN_(query)(table, key, PN_(hash)(key)))
		? PN_(bucket_value)(bucket) : PN_D_(default, value);
}

static void PN_D_(unused, default_coda)(void);
static void PN_D_(unused, default)(void) { PN_(key) k; memset(&k, 0, sizeof k);
	N_D_(table, get)(0, k); PN_D_(unused, default_coda)(); }
static void PN_D_(unused, default_coda)(void) { PN_D_(unused, default)(); }

#undef N_D_
#undef PN_D_
#undef TABLE_DEFAULT
#ifdef TABLE_DEFAULT_NAME
#undef TABLE_DEFAULT_NAME
#endif


#elif defined(TABLE_TO_STRING) /* default --><!-- to string trait */


#ifdef TABLE_TO_STRING_NAME
#define SZ_(n) TABLE_CAT(N_(table), TABLE_CAT(TABLE_TO_STRING_NAME, n))
#else
#define SZ_(n) TABLE_CAT(N_(table), n)
#endif
#define TSZ_(n) TABLE_CAT(table_sz, SZ_(n))
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
static const char *(*PN_(table_to_string))(const struct N_(table) *)
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
