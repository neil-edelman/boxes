/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Stand-alone header <../src/table.h>; examples <../test/test_table.c>;
 article <../doc/table/table.pdf>.

 @subtitle Hash table

 ![Example of <string>table.](../doc/table/table.png)

 <tag:<N>table> implements a set or map of <typedef:<PN>entry> as a hash table.
 It must be supplied <typedef:<PN>hash_fn> `<N>hash` and,
 <typedef:<PN>is_equal_fn> `<N>is_equal` or <typedef:<PN>unhash_fn>
 `<N>unhash`.

 It is contiguous and may rearrange elements—not stable.

 @param[TABLE_NAME, TABLE_KEY]
 `<N>` that satisfies `C` naming conventions when mangled and a valid
 <typedef:<PN>key> associated therewith; required. `<PN>` is private, whose
 names are prefixed in a manner to avoid collisions.

 @param[TABLE_UNHASH]
 By default it assumes that `<N>is_equal` is supplied; with this, instead
 requires `<N>unhash` satisfying <typedef:<PN>unhash_fn>.

 @param[TABLE_VALUE]
 An optional type that is the payload of the key, thus making this a map or
 associative array.

 @param[TABLE_UINT]
 This is <typedef:<PN>uint>, the unsigned type of hash of the key given by
 <typedef:<PN>hash_fn>; defaults to `size_t`.

 @param[TABLE_DEFAULT]
 Default trait; a <typedef:<PN>value> used in <fn:<N>table<D>get>.

 @param[TABLE_TO_STRING]
 To string trait `<STR>` contained in <src/to_string.h>. Require
 `<name>[<trait>]to_string` be declared as <typedef:<PSTR>to_string_fn>.

 @param[TABLE_EXPECT_TRAIT, TABLE_TRAIT]
 Named traits are obtained by including `table.h` multiple times with
 `TABLE_EXPECT_TRAIT` and then subsequently including the name in
 `TABLE_TRAIT`.

 These go together to allow exporting non-static data between compilation units
 by separating the `TABLE_BODY` refers to `TABLE_HEAD`, and identical

 @param[TABLE_HEAD, TABLE_BODY]
 These go together to allow exporting non-static data between compilation units
 by separating the header head from the code body. `TABLE_HEAD` needs identical
 `TABLE_NAME`, `TABLE_KEY`, `TABLE_UNHASH`, `TABLE_VALUE`, and `TABLE_UINT`.

 @std C89 */

#if !defined(TABLE_NAME) || !defined(TABLE_KEY)
#error Name TABLE_NAME or tag type TABLE_KEY undefined.
#endif
#if defined(TABLE_TRAIT) ^ defined(BOX_TYPE)
#error TABLE_TRAIT name must come after TABLE_EXPECT_TRAIT.
#endif
#if defined(TABLE_TEST) && (!defined(TABLE_TRAIT) && !defined(TABLE_TO_STRING) \
	|| defined(TABLE_TRAIT) && !defined(TABLE_HAS_TO_STRING))
#error Test requires to string.
#endif
#if defined TABLE_HEAD && (defined TABLE_BODY || defined TABLE_TRAIT)
#error Can not be simultaneously defined.
#endif

#ifndef TABLE_H /* <!-- idempotent */
#define TABLE_H
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#if defined(TABLE_CAT_) || defined(TABLE_CAT) || defined(N_) || defined(PN_)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define TABLE_CAT_(n, m) n ## _ ## m
#define TABLE_CAT(n, m) TABLE_CAT_(n, m)
#define N_(n) TABLE_CAT(TABLE_NAME, n)
#define PN_(n) TABLE_CAT(table, N_(n))
/* Use the sign bit to store out-of-band flags when a <typedef:<PN>uint>
 represents an address in the table, (such that range of an index is one bit
 less.) Choose representations that may save power? We cannot save this in an
 `enum` because we don't know maximum. */
#define TABLE_M1 ((PN_(uint))~(PN_(uint))0) /* 2's compliment -1. */
#define TABLE_HIGH ((TABLE_M1 >> 1) + 1) /* High-bit set: max cardinality. */
#define TABLE_END (TABLE_HIGH) /* Out-of-band signalling end of chain. */
#define TABLE_NULL (TABLE_HIGH + 1) /* Out-of-band signalling no item. */
#define TABLE_RESULT X(ERROR), X(ABSENT), X(PRESENT)
#define X(n) TABLE_##n
/** A result of modifying the table, of which `TABLE_ERROR` is false.
 
 ![A diagram of the result states.](../doc/table/result.png) */
enum table_result { TABLE_RESULT };
#undef X
#ifndef TABLE_HEAD /* <!-- body */
#define X(n) #n
/** A static array of strings describing the <tag:table_result>. */
static const char *const table_result_str[] = { TABLE_RESULT };
#undef X
#endif /* body --> */
#undef TABLE_RESULT
#endif /* idempotent --> */


#ifndef TABLE_TRAIT /* <!-- base code */

#ifndef TABLE_UINT
#define TABLE_UINT size_t
#endif

#ifndef TABLE_BODY /* <!-- head */

/** <typedef:<PN>hash_fn> returns this hash type by `TABLE_UINT`, which must be
 be an unsigned integer. Places a simplifying limit on the maximum number of
 elements of half the cardinality. */
typedef TABLE_UINT PN_(uint);

/** Valid tag type defined by `TABLE_KEY` used for keys. If `TABLE_UNHASH` is
 not defined, a copy of this value will be stored in the internal buckets. */
typedef TABLE_KEY PN_(key);
typedef const TABLE_KEY PN_(key_c); /* Works 90%? */

#if 0 /* <!-- documentation */
/** A map from <typedef:<PN>key_c> onto <typedef:<PN>uint>, called `<N>hash`,
 that, ideally, should be easy to compute while minimizing duplicate addresses.
 Must be consistent for each value while in the table. If <typedef:<PN>key> is
 a pointer, one is permitted to have null in the domain. */
typedef PN_(uint) (*PN_(hash_fn))(const PN_(key));
#ifdef TABLE_UNHASH /* <!-- inv */
/** Defining `TABLE_UNHASH` says <typedef:<PN>hash_fn> forms a bijection
 between the range in <typedef:<PN>key> and the image in <typedef:<PN>uint>,
 and the inverse is called `<N>unhash`. In this case, keys are not stored
 in the hash table, rather they are generated using this inverse-mapping. */
typedef PN_(key) (*PN_(unhash_fn))(PN_(uint));
#else /* inv --><!-- !inv */
/** Equivalence relation between <typedef:<PN>key> that satisfies
 `<PN>is_equal_fn(a, b) -> <PN>hash(a) == <PN>hash(b)`, called `<N>is_equal`.
 If `TABLE_UNHASH` is set, there is no need for this function because the
 comparison is done directly in hash space. */
typedef int (*PN_(is_equal_fn))(PN_(key_c) a, PN_(key_c) b);
#endif /* !inv --> */
#endif /* documentation --> */

#ifdef TABLE_VALUE /* <!-- value */
/** Defining `TABLE_VALUE` produces an associative map, otherwise it is the
 same as <typedef:<PN>key>. */
typedef TABLE_VALUE PN_(value);
/** Defining `TABLE_VALUE` creates this map from <typedef:<PN>key> to
 <typedef:<PN>value>, as an interface with table. */
struct N_(table_entry) { PN_(key) key; PN_(value) value; };
/** If `TABLE_VALUE`, this is <tag:<N>table_entry>; otherwise, it's the same as
 <typedef:<PN>key>. */
typedef struct N_(table_entry) PN_(entry);
#else /* value --><!-- !value */
typedef PN_(key) PN_(value);
typedef PN_(key) PN_(entry);
#endif /* !value --> */

/* Address is hash modulo size of table. Any occupied buckets at the head of
 the linked structure are closed, that is, the address equals the index. These
 form a linked table, possibly with other, open buckets that have the same
 address in vacant buckets. */
struct PN_(bucket) {
	PN_(uint) next; /* Bucket index, including `TABLE_NULL` and `TABLE_END`. */
	PN_(uint) hash;
#ifndef TABLE_UNHASH
	PN_(key) key;
#endif
#ifdef TABLE_VALUE
	PN_(value) value;
#endif
};

/** Returns true if the `replace` replaces the `original`.
 (Shouldn't it be entry?) */
typedef int (*PN_(policy_fn))(PN_(key) original, PN_(key) replace);

/** To initialize, see <fn:<N>table>, `TABLE_IDLE`, `{0}` (`C99`,) or being
 `static`. The fields should be treated as read-only; any modification is
 liable to cause the table to go into an invalid state.

 ![States.](../doc/table/states.png) */
struct N_(table) { /* "Padding size," good. */
	struct PN_(bucket) *buckets; /* @ has zero/one key specified by `next`. */
	/* `size <= capacity`; size is not needed but convenient and allows
	 short-circuiting. Top is an index of the stack, potentially lazy: MSB
	 stores whether this is a step ahead (which would make it less, the stack
	 grows from the bottom,) otherwise it is right at the top, */
	PN_(uint) log_capacity, size, top;
};

/* In no particular order, usually, but deterministic up to topology changes. */
struct PN_(iterator) { struct N_(table) *table; PN_(uint) i; };

/** ![States](../doc/table/it.png)

 Adding, deleting, successfully looking up entries, or any modification of the
 table's topology invalidates the iterator.
 Iteration usually not in any particular order. The asymptotic runtime of
 iterating though the whole table is proportional to the capacity. */
struct N_(table_iterator);
struct N_(table_iterator) { struct PN_(iterator) _; };

#endif /* head --> */
#ifndef TABLE_HEAD /* <!-- body */

/** Gets the key of an occupied `bucket`. */
static PN_(key) PN_(bucket_key)(const struct PN_(bucket) *const bucket) {
	assert(bucket && bucket->next != TABLE_NULL);
#ifdef TABLE_UNHASH
	/* On `TABLE_UNHASH`, this function must be defined by the user. */
	return N_(unhash)(bucket->hash);
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

/** The capacity of a non-idle `table` is always a power-of-two. */
static PN_(uint) PN_(capacity)(const struct N_(table) *const table)
	{ return assert(table && table->buckets && table->log_capacity >= 3),
	(PN_(uint))((PN_(uint))1 << table->log_capacity); }

/** @return Indexes the first (closed) bucket in the set of buckets with the
 same address from non-idle `table` given the `hash`. If the bucket is empty,
 it will have `next = TABLE_NULL` or it's own <fn:<PN>chain_head> not equal
 to the index (another open bucket). */
static PN_(uint) PN_(chain_head)(const struct N_(table) *const table,
	const PN_(uint) hash) { return hash & (PN_(capacity)(table) - 1); }

/** @return Search for the previous link in the bucket to `b` in `table`, if it
 exists, (by restarting and going though the list.)
 @order \O(`bucket size`) */
static struct PN_(bucket) *PN_(prev)(const struct N_(table) *const table,
	const PN_(uint) b) {
	const struct PN_(bucket) *const bucket = table->buckets + b;
	PN_(uint) to_next = TABLE_NULL, next;
	assert(table && bucket->next != TABLE_NULL);
	/* Note that this does not check for corrupted tables; would get assert. */
	for(next = PN_(chain_head)(table, bucket->hash);
		/* assert(next < capacity), */ next != b;
		to_next = next, next = table->buckets[next].next);
	return to_next != TABLE_NULL ? table->buckets + to_next : 0;
}

/* <!-- stack functions */
/** On return, the `top` of `table` will be empty and eager, but size is not
 incremented, leaving it in intermediate state. Amortized if you grow only. */
static void PN_(grow_stack)(struct N_(table) *const table) {
	/* Subtract one for eager. */
	PN_(uint) top = (table->top & ~TABLE_HIGH) - !(table->top & TABLE_HIGH);
	assert(table && table->buckets && table->top && top < PN_(capacity)(table));
	while(table->buckets[top].next != TABLE_NULL) assert(top), top--;
	table->top = top; /* Eager, since one is allegedly going to fill it. */
}
/** Force the evaluation of the stack of `table`, thereby making it eager. This
 is like searching for a bucket in open-addressing. @order \O(`buckets`) */
static void PN_(force_stack)(struct N_(table) *const table) {
	PN_(uint) top = table->top;
	if(top & TABLE_HIGH) { /* Lazy. */
		struct PN_(bucket) *bucket;
		top &= ~TABLE_HIGH;
		do bucket = table->buckets + ++top/*, assert(top < capacity)*/;
		while(bucket->next != TABLE_NULL
			&& PN_(chain_head)(table, bucket->hash) == top);
		table->top = top; /* Eager. */
	}
}
/** Is `i` in `table` possibly on the stack? (The stack grows from the high.) */
static int PN_(in_stack_range)(const struct N_(table) *const table,
	const PN_(uint) i) {
	return assert(table && table->buckets),
		(table->top & ~TABLE_HIGH) + !!(table->top & TABLE_HIGH) <= i;
}
/** Corrects newly-deleted `b` from `table` in the stack. */
static void PN_(shrink_stack)(struct N_(table) *const table,
	const PN_(uint) b) {
	assert(table && table->buckets && b < PN_(capacity)(table));
	assert(table->buckets[b].next == TABLE_NULL);
	if(!PN_(in_stack_range)(table, b)) return;
	PN_(force_stack)(table); /* Only have room for 1 step of laziness. */
	assert(PN_(in_stack_range)(table, b)); /* I think this is assured? Think. */
	if(b != table->top) {
		struct PN_(bucket) *const prev = PN_(prev)(table, table->top);
		table->buckets[b] = table->buckets[table->top];
		prev->next = b;
	}
	table->buckets[table->top].next = TABLE_NULL;
	table->top |= TABLE_HIGH; /* Lazy. */
}
/** Moves the `m` index in non-idle `table`, to the top of collision stack.
 This may result in an inconsistent state; one is responsible for filling that
 hole and linking it with top. */
static void PN_(move_to_top)(struct N_(table) *const table, const PN_(uint) m) {
	struct PN_(bucket) *move, *top, *prev;
	assert(table
		&& table->size < PN_(capacity)(table) && m < PN_(capacity)(table));
	PN_(grow_stack)(table); /* Leaves it in an eager state. */
	move = table->buckets + m, top = table->buckets + table->top;
	assert(move->next != TABLE_NULL && top->next == TABLE_NULL);
	if(prev = PN_(prev)(table, m)) prev->next = table->top; /* \O(|`bucket`|) */
	memcpy(top, move, sizeof *move), move->next = TABLE_NULL;
}
/* stack --> */

/** `TABLE_UNHASH` is injective, so in that case, we only compare hashes.
 @return `a` and `b`. */
static int PN_(equal_buckets)(PN_(key_c) a, PN_(key_c) b) {
#ifdef TABLE_UNHASH
	return (void)a, (void)b, 1;
#else
	/* Must have this function declared. */
	return N_(is_equal)(a, b);
#endif
}

/** `table` will be searched linearly for `key` which has `hash`. */
static struct PN_(bucket) *PN_(query)(struct N_(table) *const table,
	PN_(key_c) key, const PN_(uint) hash) {
	struct PN_(bucket) *bucket1;
	PN_(uint) head, b0 = TABLE_NULL, b1, b2;
	assert(table && table->buckets && table->log_capacity);
	bucket1 = table->buckets + (head = b1 = PN_(chain_head)(table, hash));
	/* Not the start of a bucket: empty or in the collision stack. */
	if((b2 = bucket1->next) == TABLE_NULL
		|| PN_(in_stack_range)(table, b1)
		&& b1 != PN_(chain_head)(table, bucket1->hash)) return 0;
	while(hash != bucket1->hash
		|| !PN_(equal_buckets)(key, PN_(bucket_key)(bucket1))) {
		if(b2 == TABLE_END) return 0;
		bucket1 = table->buckets + (b0 = b1, b1 = b2);
		assert(b1 < PN_(capacity)(table) && PN_(in_stack_range)(table, b1)
			&& b1 != TABLE_NULL);
		b2 = bucket1->next;
	}
#ifdef TABLE_DONT_SPLAY /* <!-- !splay: (No reason not to, practically.) */
	return bucket1;
#undef TABLE_DONT_SPLAY
#else /* !splay --><!-- splay: bring the MRU to the front. */
	if(b0 == TABLE_NULL) return bucket1;
	{
		struct PN_(bucket) *const bucket0 = table->buckets + b0,
			*const bucket_head = table->buckets + head, temp;
		bucket0->next = b2;
		memcpy(&temp, bucket_head, sizeof *bucket_head);
		memcpy(bucket_head, bucket1, sizeof *bucket1);
		memcpy(bucket1, &temp, sizeof temp);
		bucket_head->next = b1;
		return bucket_head;
	}
#endif /* splay --> */
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
		g = PN_(chain_head)(table, hash = idx->hash);
		/* It's a power-of-two size, so, like consistent hashing, `E[old/new]`
		 capacity that a closed bucket will remain where it is. */
		if(i == g) { idx->next = TABLE_END; continue; }
		if((go = table->buckets + g)->next == TABLE_NULL) {
			/* Priority is given to the first closed bucket; simpler later. */
			struct PN_(bucket) *head;
			PN_(uint) h = g & ~mask; assert(h <= g);
			if(h < g && i < h
				&& (head = table->buckets + h, assert(head->next != TABLE_NULL),
				PN_(chain_head)(table, head->hash) == g)) {
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

	/* Search waiting stack for buckets that moved concurrently. */
	{ PN_(uint) prev = TABLE_END, w = wait; while(w != TABLE_END) {
		struct PN_(bucket) *waiting = table->buckets + w;
		PN_(uint) cl = PN_(chain_head)(table, waiting->hash);
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

	/* Rebuild the top stack at the high numbers from the waiting at low. */
	while(wait != TABLE_END) {
		struct PN_(bucket) *const waiting = table->buckets + wait;
		PN_(uint) h = PN_(chain_head)(table, waiting->hash);
		struct PN_(bucket) *const head = table->buckets + h;
		struct PN_(bucket) *top;
		assert(h != wait && head->next != TABLE_NULL);
		PN_(grow_stack)(table), top = table->buckets + table->top;
		memcpy(top, waiting, sizeof *waiting);
		top->next = head->next, head->next = table->top;
		wait = waiting->next, waiting->next = TABLE_NULL; /* Pop. */
	}

	return 1;
}

/** Replace the `key` and `hash` of `bucket`. Don't touch next. */
static void PN_(replace_key)(struct PN_(bucket) *const bucket,
	const PN_(key) key, const PN_(uint) hash) {
	(void)key;
	bucket->hash = hash;
#ifndef TABLE_UNHASH
	bucket->key = key;
#endif
}

/** Evicts the spot where `hash` goes in `table`. This results in a space in
 the table. */
static struct PN_(bucket) *PN_(evict)(struct N_(table) *const table,
	const PN_(uint) hash) {
	PN_(uint) i;
	struct PN_(bucket) *bucket;
	if(!PN_(buffer)(table, 1)) return 0; /* Amortized. */
	bucket = table->buckets + (i = PN_(chain_head)(table, hash));/* Closed. */
	if(bucket->next != TABLE_NULL) { /* Occupied. */
		int in_stack = PN_(chain_head)(table, bucket->hash) != i;
		PN_(move_to_top)(table, i);
		bucket->next = in_stack ? TABLE_END : table->top;
	} else { /* Unoccupied. */
		bucket->next = TABLE_END;
	}
	table->size++;
	return bucket;
}

/** Put `key` in `table`. For collisions, only if `policy` exists and returns
 true do and displace it to `eject`, if non-null.
 @return A <tag:table_result>. @throws[malloc]
 @order Amortized \O(max bucket length); the key to another bucket may have to
 be moved to the top; the table might be full and have to be resized. */
static enum table_result PN_(put_key)(struct N_(table) *const table,
	const PN_(key) key, PN_(key) *eject, const PN_(policy_fn) policy) {
	struct PN_(bucket) *bucket;
	const PN_(uint) hash = N_(hash)(key);
	enum table_result result;
	assert(table);
	if(table->buckets && (bucket = PN_(query)(table, key, hash))) {
		if(!policy || !policy(PN_(bucket_key)(bucket), key))
			return TABLE_PRESENT;
		if(eject) *eject = PN_(bucket_key)(bucket);
		result = TABLE_PRESENT;
	} else {
		if(!(bucket = PN_(evict)(table, hash))) return TABLE_ERROR;
		result = TABLE_ABSENT;
	}
	PN_(replace_key)(bucket, key, hash);
	return result;
}

/** @return Before `table`. */
static struct PN_(iterator) PN_(iterator)(struct N_(table) *const table)
	{ struct PN_(iterator) it; it.table = table, it.i = 0, it.i--; return it; }
/** @return Element at valid non-null `it`. */
static struct PN_(bucket) *PN_(element)(const struct PN_(iterator) *const it)
	{ return it->table->buckets + it->i; }
/** @return Whether `it` even has a next. */
static int PN_(next)(struct PN_(iterator) *const it) {
	const struct N_(table) *const t = it->table;
	PN_(uint) limit;
	assert(it && it->table);
	if(!it->table->buckets) return 0; /* Idle. */
	limit = PN_(capacity)(t);
	while(++it->i < limit) if(t->buckets[it->i].next != TABLE_NULL) return 1;
	return 0;
}

/** Removes the entry at `it` and possibly corrects `it` so that calling
 <fn:<PN>next> will go through the entire list. @return Success. */
static int PN_(remove)(struct PN_(iterator) *const it) {
	struct N_(table) *table = it->table;
	struct PN_(bucket) *previous = 0, *current;
	PN_(uint) prv = TABLE_NULL, crnt;
	assert(it && table);
	if(!it->table->buckets) return 0;
	assert(it->i < PN_(capacity)(it->table));
	if(it->i >= PN_(capacity)(it->table)) return 0;
	/* This should be possible to simplify with <fn:<PN>prev>?
	 if(previous = PN_(prev)(table, it->i)) */
	/* Egregious code reuse from <fn:<N>table_remove>; because `it` contains
	 `i` and remove has a `key`, the counting is different. But the rest is the
	 same? Get the last bucket. */
	current = it->table->buckets + it->i, assert(current->next != TABLE_NULL);
	crnt = PN_(chain_head)(it->table, current->hash);
	while(crnt != it->i) assert(crnt < PN_(capacity)(it->table)),
		crnt = (previous = it->table->buckets + (prv = crnt))->next;
	if(prv != TABLE_NULL) { /* Open entry. */
		previous->next = current->next;
	} else if(current->next != TABLE_END) { /* Head closed entry and others. */
		const PN_(uint) scnd = current->next;
		struct PN_(bucket) *const second = table->buckets + scnd;
		assert(scnd < PN_(capacity)(table));
		memcpy(current, second, sizeof *second);
		/* Because we replace current with a bucket we haven't seen yet. */
		if(crnt < scnd) it->i--;
		crnt = scnd, current = second;
	}
	current->next = TABLE_NULL, table->size--, PN_(shrink_stack)(table, crnt);
	return 1;
}

/** Zeroed data (not all-bits-zero) is initialized. @return An idle array.
 @order \Theta(1) @allow */
static struct N_(table) N_(table)(void) {
	struct N_(table) table;
	table.buckets = 0; table.log_capacity = 0; table.size = 0; table.top = 0;
	return table;
}

/** If `table` is not null, destroys and returns it to idle. @allow */
static void N_(table_)(struct N_(table) *const table)
	{ if(table) free(table->buckets), *table = N_(table)(); }

/** Loads a non-null `table` into `it`. @allow */
static struct N_(table_iterator) N_(table_iterator)(struct N_(table) *const
	table) { struct N_(table_iterator) it; it._ = PN_(iterator)(table);
	return it; }
/** Advances `it`. @return Whether `it` has an element now. @allow */
static int N_(table_next)(struct N_(table_iterator) *const it) {
	return PN_(next)(&it->_);
}
/** @return If `it` has an element, returns it's key. @allow */
static PN_(key) N_(table_key)(const struct N_(table_iterator) *const it)
	{ return PN_(bucket_key)(it->_.table->buckets + it->_.i); }
#ifdef TABLE_VALUE /* <!-- value */
/** @return If `it` has an element, returns it's value, if `TABLE_VALUE`.
 @allow */
static PN_(value) *N_(table_value)(const struct N_(table_iterator) *const it)
	{ return &it->_.table->buckets[it->_.i].value; }
#endif /* value --> */
/** Removes the entry at `it`. Whereas <fn:<N>table_remove> invalidates the
 iterator, this corrects `it` so <fn:<N>table_next> is the next entry. To use
 the iterator after this, one must move to the next.
 @return Success, or there was no entry at the iterator's position, (anymore.)
 @allow */
static int N_(table_iterator_remove)(struct N_(table_iterator) *const it)
	{ return assert(it), PN_(remove)(&it->_); }

/** Reserve at least `n` more empty buckets in `table`. This may cause the
 capacity to increase and invalidates any pointers to data in the table.
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
	table->top = (PN_(capacity)(table) - 1) | TABLE_HIGH;
}

/** @return Whether `key` is in `table` (which can be null.) @allow */
static int N_(table_contains)(struct N_(table) *const table, const PN_(key) key) {
	/* This function must be defined by the user. */
	return table && table->buckets
		? !!PN_(query)(table, key, N_(hash)(key)) : 0;
}

#ifdef TABLE_VALUE /* <!-- map */
/** If there can be no default key, use this so separate a null, returns false,
 from a result. Otherwise, a more convenient function is <fn:<N>table_get_or>.
 @param[result] If null, behaves like <fn:<N>table_contains>, otherwise, a
 <typedef:<PN>key> which gets filled on true.
 @param[value] Only on a map with `TABLE_VALUE`. If not-null, stores the value.
 @return Whether `key` is in `table` (which can be null.) @allow */
static int N_(table_query)(struct N_(table) *const table, const PN_(key) key,
	PN_(key) *result, PN_(value) *value) {
	struct PN_(bucket) *bucket;
	if(!table || !table->buckets
		|| !(bucket = PN_(query)(table, key, N_(hash)(key)))) return 0;
	if(result) *result = PN_(bucket_key)(bucket);
	if(value) *value = bucket->value;
	return 1;
}
#else
/** `key` from `table` is stored in `result`. */
static int N_(table_query)(struct N_(table) *const table, const PN_(key) key,
	PN_(key) *result) {
	struct PN_(bucket) *bucket;
	if(!table || !table->buckets
		|| !(bucket = PN_(query)(table, key, N_(hash)(key)))) return 0;
	if(result) *result = PN_(bucket_key)(bucket);
	return 1;
}
#endif

/** @return The value associated with `key` in `table`, (which can be null.) If
 no such value exists, `default_value` is returned.
 @order Average \O(1); worst \O(n). @allow */
static PN_(value) N_(table_get_or)(struct N_(table) *const table,
	const PN_(key) key, PN_(value) default_value) {
	struct PN_(bucket) *bucket;
	/* This function must be defined by the user. */
	return table && table->buckets
		&& (bucket = PN_(query)(table, key, N_(hash)(key)))
		? PN_(bucket_value)(bucket) : default_value;
}

#ifndef TABLE_VALUE /* <!-- set */

/** Only if `TABLE_VALUE` is not set; see <fn:<N>table_assign> for a map. Puts
 `key` in set `table` only if absent.
 @return One of: `TABLE_ERROR`, tried putting the entry in the table but
 failed, the table is not modified; `TABLE_PRESENT`, does nothing if there is
 another entry with the same key; `TABLE_ABSENT`, put an entry in the table.
 @throws[realloc, ERANGE] On `TABLE_ERROR`.
 @order Average amortised \O(1); worst \O(n). @allow */
static enum table_result N_(table_try)(struct N_(table) *const table,
	PN_(key) key) { return PN_(put_key)(table, key, 0, 0); }

#else /* set --><!-- map */

/** Only if `TABLE_VALUE` is set. Ensures that `key` is in the `table` and
 update `content`. @throws[malloc] */
static enum table_result PN_(assign)(struct N_(table) *const table,
	PN_(key) key, PN_(value) **const content) {
	struct PN_(bucket) *bucket;
	const PN_(uint) hash = N_(hash)(key);
	enum table_result result;
	assert(table && content);
	if(table->buckets && (bucket = PN_(query)(table, key, hash))) {
		result = TABLE_PRESENT;
	} else {
		if(!(bucket = PN_(evict)(table, hash))) return TABLE_ERROR;
		PN_(replace_key)(bucket, key, hash);
		result = TABLE_ABSENT;
	}
	*content = &bucket->value;
	return result;
}

/** Only if `TABLE_VALUE` is set; see <fn:<N>table_try> for a set. Puts `key`
 in the map `table` and store the associated value in `content`.
 @return `TABLE_ERROR` does not set `content`; `TABLE_ABSENT`, the `content`
 will be a pointer to uninitialized memory; `TABLE_PRESENT`, gets the current
 `content`, (does not alter the keys, if they are distinguishable.)
 @throws[malloc, ERANGE] On `TABLE_ERROR`. @allow */
static enum table_result N_(table_assign)(struct N_(table) *const table,
	PN_(key) key, PN_(value) **const content)
	{ return PN_(assign)(table, key, content); }

#endif /* value --> */

/** Callback in <fn:<N>table_update>.
 @return `original` and `replace` ignored, true.
 @implements <typedef:<PN>policy_fn> */
static int PN_(always_replace)(const PN_(key) original,
	const PN_(key) replace) { return (void)original, (void)replace, 1; }

/** Puts `key` in `table`, replacing an equal-valued key. (If keys are
 indistinguishable, this function is not very useful, see <fn:<N>table_try> or
 <fn:<N>table_assign>.)
 @return One of: `TABLE_ERROR`, the table is not modified; `TABLE_ABSENT`, the
 `key` is new; `TABLE_PRESENT`, the `key` displaces another, and if non-null,
 `eject` will be filled with the previous entry.
 @throws[realloc, ERANGE] On `TABLE_ERROR`.
 @order Average amortised \O(1); worst \O(n). @allow */
static enum table_result N_(table_update)(struct N_(table) *const table,
	PN_(key) key, PN_(key) *eject)
	{ return PN_(put_key)(table, key, eject, &PN_(always_replace)); }

/** Puts `key` in `table` only if absent or if calling `policy` returns true.
 @return One of: `TABLE_ERROR`, the table is not modified; `TABLE_ABSENT`, the
 `key` is new; `TABLE_PRESENT`, `key` collides, if `policy` returns true,
 `eject`, if non-null, will be filled;
 @throws[realloc, ERANGE] On `TABLE_ERROR`.
 @order Average amortised \O(1); worst \O(n). @allow */
static enum table_result N_(table_policy)(struct N_(table) *const table,
	PN_(key) key, PN_(key) *eject, const PN_(policy_fn) policy)
	{ return PN_(put_key)(table, key, eject, policy); }

/** Removes `key` from `table` (which could be null.)
 @return Whether that `key` was in `table`. @order Average \O(1), (hash
 distributes elements uniformly); worst \O(n). @allow */
static int N_(table_remove)(struct N_(table) *const table,
	const PN_(key) key) {
	struct PN_(bucket) *current;
	PN_(uint) c, p = TABLE_NULL, n, hash = N_(hash)(key);
	if(!table || !table->size) return 0;
	assert(table->buckets);
	/* Find item and keep track of previous. */
	current = table->buckets + (c = PN_(chain_head)(table, hash));
	if((n = current->next) == TABLE_NULL /* No entry here. */
		|| PN_(in_stack_range)(table, c)
		&& c != PN_(chain_head)(table, current->hash)) return 0;
	/* Find prev? Why not <fn:<PN>prev>? */
	while(hash != current->hash
		&& !PN_(equal_buckets)(key, PN_(bucket_key)(current))) {
		if(n == TABLE_END) return 0;
		p = c, current = table->buckets + (c = n);
		assert(c < PN_(capacity)(table) && PN_(in_stack_range)(table, c)
			&& c != TABLE_NULL);
		n = current->next;
	}
	if(p != TABLE_NULL) { /* Open entry. */
		struct PN_(bucket) *previous = table->buckets + p;
		previous->next = current->next;
	} else if(current->next != TABLE_END) { /* Head closed entry and others. */
		struct PN_(bucket) *const second
			= table->buckets + (c = current->next);
		assert(current->next < PN_(capacity)(table));
		memcpy(current, second, sizeof *second);
		current = second;
	}
	current->next = TABLE_NULL, table->size--, PN_(shrink_stack)(table, c);
	return 1;
}

/* Box override information. */
#define BOX_TYPE struct N_(table)
#define BOX_CONTENT struct PN_(bucket)
#define BOX_ PN_
#define BOX_T_MAJOR_NAME table
#define BOX_NAME TABLE_NAME

#ifdef HAVE_ITERATE_H /* <!-- iterate */
#include "iterate.h" /** \include */
#endif /* iterate --> */

static void PN_(unused_base_coda)(void);
static void PN_(unused_base)(void) {
	PN_(entry) e; PN_(key) k; PN_(value) v;
	memset(&e, 0, sizeof e); memset(&k, 0, sizeof k); memset(&v, 0, sizeof v);
	PN_(element)(0);
	N_(table)(); N_(table_)(0);
	N_(table_iterator)(0); N_(table_key)(0); N_(table_next)(0);
	N_(table_buffer)(0, 0); N_(table_clear)(0); N_(table_contains)(0, k);
	N_(table_get_or)(0, k, v);
	N_(table_update)(0, k, 0); N_(table_policy)(0, k, 0, 0);
	N_(table_remove)(0, k); N_(table_iterator_remove)(0);
#ifdef TABLE_VALUE
	N_(table_value)(0); N_(table_query)(0, k, 0, 0); N_(table_assign)(0, k, 0);
#else
	N_(table_query)(0, k, 0); N_(table_try)(0, e);
#endif
	PN_(unused_base_coda)();
}
static void PN_(unused_base_coda)(void) { PN_(unused_base)(); }

#endif /* body --> */

#endif /* base code --> */


#ifdef TABLE_TRAIT /* <-- trait: Will be different on different includes. */
#define BOX_TRAIT_NAME TABLE_TRAIT
#define PNT_(n) PN_(ARRAY_CAT(TABLE_TRAIT, n))
#define NT_(n) N_(ARRAY_CAT(TABLE_TRAIT, n))
#else /* trait --><!-- !trait */
#define PNT_(n) PN_(n)
#define NT_(n) N_(n)
#endif /* !trait --> */


#ifdef TABLE_TO_STRING /* <!-- to string trait */
/** Thunk `b` -> `a`. */
static void PNT_(to_string)(const struct PN_(bucket) *const b,
	char (*const a)[12]) {
#ifdef TABLE_VALUE
	NT_(to_string)(PN_(bucket_key)(b), PN_(bucket_value)(b), a);
#else
	NT_(to_string)(PN_(bucket_key)(b), a);
#endif
}
#define TO_STRING_LEFT '{'
#define TO_STRING_RIGHT '}'
#include "to_string.h" /** \include */
#undef TABLE_TO_STRING
#ifndef TABLE_TRAIT
#define TABLE_HAS_TO_STRING
#endif
#endif /* to string trait --> */
#undef PNT_
#undef NT_


#if defined(TABLE_TEST) && !defined(TABLE_TRAIT) /* <!-- test base */
#include "../test/test_table.h"
#endif /* test base --> */


#ifdef TABLE_DEFAULT /* <!-- default trait */
#ifdef TABLE_TRAIT
#define N_D_(n, m) TABLE_CAT(N_(n), TABLE_CAT(TABLE_TRAIT, m))
#define PN_D_(n, m) TABLE_CAT(table, N_D_(n, m))
#else
#define N_D_(n, m) TABLE_CAT(N_(n), m)
#define PN_D_(n, m) TABLE_CAT(table, N_D_(n, m))
#endif
/* #include "../test/test_table_default.h", just test manually. */
/** This is functionally identical to <fn:<N>table_get_or>, but a with a trait
 specifying a constant default value.
 @return The value associated with `key` in `table`, (which can be null.) If
 no such value exists, the `TABLE_DEFAULT` is returned.
 @order Average \O(1); worst \O(n). @allow */
static PN_(value) N_D_(table, get)(struct N_(table) *const table,
	const PN_(key) key) {
	struct PN_(bucket) *bucket;
	/* `TABLE_DEFAULT` is a valid <tag:<PN>value>. */
	const PN_(value) PN_D_(default, value) = TABLE_DEFAULT;
	/* Function `<N>hash` must be defined by the user. */
	return table && table->buckets
		&& (bucket = PN_(query)(table, key, N_(hash)(key)))
		? PN_(bucket_value)(bucket) : PN_D_(default, value);
}
static void PN_D_(unused, default_coda)(void);
static void PN_D_(unused, default)(void) { PN_(key) k; memset(&k, 0, sizeof k);
	N_D_(table, get)(0, k); PN_D_(unused, default_coda)(); }
static void PN_D_(unused, default_coda)(void) { PN_D_(unused, default)(); }
#undef N_D_
#undef PN_D_
#undef TABLE_DEFAULT
#endif /* default trait --> */


#ifdef TABLE_EXPECT_TRAIT /* <!-- more */
#undef TABLE_EXPECT_TRAIT
#else /* more --><!-- done */
#undef BOX_TYPE
#undef BOX_CONTENT
#undef BOX_
#undef BOX_T_MAJOR_NAME
#undef BOX_NAME
#undef TABLE_NAME
#undef TABLE_KEY
#undef TABLE_UINT
#undef TABLE_HASH
#ifdef TABLE_IS_EQUAL
#undef TABLE_IS_EQUAL
#else
#undef TABLE_UNHASH
#endif
#ifdef TABLE_VALUE
#undef TABLE_VALUE
#endif
#ifdef TABLE_HAS_TO_STRING
#undef TABLE_HAS_TO_STRING
#endif
#ifdef TABLE_TEST
#undef TABLE_TEST
#endif
#ifdef TABLE_BODY
#undef TABLE_BODY
#endif
#ifdef TABLE_HEAD
#undef TABLE_HEAD
#endif
#endif /* done --> */
#ifdef TABLE_TRAIT
#undef TABLE_TRAIT
#undef BOX_TRAIT_NAME
#endif
