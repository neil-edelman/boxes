/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Header <../../src/table.h>; examples <../../test/test_table.c>;
 article <../table/table.pdf>.

 @subtitle Hash table

 ![Example of <string>table.](../doc/table/table.png)

 <tag:<t>table> implements a set or map of <typedef:<pT>entry> as an
 inline-chined hash-table. It must be supplied <typedef:<pT>hash_fn> `<t>hash`
 and, <typedef:<pT>is_equal_fn> `<t>is_equal` or <typedef:<pT>unhash_fn>
 `<t>unhash`. It is contiguous and not stable, and may rearrange elements.

 @param[TABLE_NAME, TABLE_KEY]
 `<t>` that satisfies `C` naming conventions when mangled and a valid
 <typedef:<pT>key> associated therewith; required.

 @param[TABLE_UNHASH]
 By default it assumes that `<t>is_equal` is supplied; with this, instead
 requires `<t>unhash` satisfying <typedef:<pT>unhash_fn>.

 @param[TABLE_VALUE]
 An optional type that is the payload of the key, thus making this a map or
 associative array.

 @param[TABLE_UINT]
 This is <typedef:<pT>uint>, the unsigned type of hash of the key given by
 <typedef:<pT>hash_fn>; defaults to `size_t`. Usually this can be set to the
 more sensible value `uint32_t` (or smaller) in C99's `stdint.h`.

 @param[TABLE_DEFAULT]
 Default trait; a <typedef:<pT>value> used in <fn:<T>table<R>get>.

 @param[TABLE_TO_STRING]
 To string trait contained in <../../src/to_string.h>. See
 <typedef:<pT>to_string_fn>.

 @param[TABLE_EXPECT_TRAIT, TABLE_TRAIT]
 Named traits are obtained by including `table.h` multiple times with
 `TABLE_EXPECT_TRAIT` and then subsequently including the name in
 `TABLE_TRAIT`.

 @param[TABLE_DECLARE_ONLY]
 For headers in different compilation units.

 @depend [box](../../src/box.h)
 @std C89 */

#if !defined(TABLE_NAME) || !defined(TABLE_KEY)
#	error Name TABLE_NAME or tag type TABLE_KEY undefined.
#endif
#if !defined(BOX_ENTRY1) && (defined(TABLE_TRAIT) ^ defined(BOX_MAJOR))
#	error TABLE_TRAIT name must come after TABLE_EXPECT_TRAIT.
#endif
#if defined(TABLE_TEST) && (!defined(TABLE_TRAIT) && !defined(TABLE_TO_STRING) \
	|| defined(TABLE_TRAIT) && !defined(TABLE_HAS_TO_STRING))
#error Test requires to string.
#endif
#if defined(BOX_TRAIT) && !defined(ARRAY_TRAIT)
#	error Unexpected.
#endif

#ifdef TABLE_TRAIT
#	define BOX_TRAIT TABLE_TRAIT /* Ifdef in <box.h>. */
#endif
#define BOX_START
#include "box.h"

#ifndef TABLE_H
#	define TABLE_H
/* Use the sign bit to store out-of-band flags when a <typedef:<PN>uint>
 represents an address in the table, (such that range of an index is one bit
 less.) Choose representations that may save power? We cannot save this in an
 `enum` because we don't know maximum. */
#	define TABLE_M1 ((pT_(uint))~(pT_(uint))0) /* 2's compliment -1. */
#	define TABLE_HIGH ((TABLE_M1 >> 1) + 1) /* High-bit set: max cardinality. */
#	define TABLE_END (TABLE_HIGH) /* Out-of-band signalling end of chain. */
#	define TABLE_NULL (TABLE_HIGH + 1) /* Out-of-band signalling no item. */
#	define TABLE_RESULT X(ERROR), X(ABSENT), X(PRESENT)
#	define X(n) TABLE_##n
/** A result of modifying the table, of which `TABLE_ERROR` is false.
 
 ![A diagram of the result states.](../doc/table/result.png) */
enum table_result { TABLE_RESULT };
#	undef X
#	ifndef TABLE_DECLARE_ONLY
#		define X(n) #n
/** A static array of strings describing the <tag:table_result>. */
static const char *const table_result_str[] = { TABLE_RESULT };
#		undef X
#	endif
#	undef TABLE_RESULT
#endif


#ifndef TABLE_TRAIT /* <!-- base code */
#	include <stdlib.h>
#	include <string.h>
#	include <errno.h>
#	include <assert.h>

#	define BOX_MAJOR table
#	define BOX_MINOR TABLE_NAME

#	ifndef TABLE_UINT
#		define TABLE_UINT size_t
#	endif

/** <typedef:<pT>hash_fn> returns this hash type by `TABLE_UINT`, which must be
 be an unsigned integer. Places a simplifying limit on the maximum number of
 elements of half the cardinality. */
typedef TABLE_UINT pT_(uint);

/** Valid tag type defined by `TABLE_KEY` used for keys. If `TABLE_UNHASH` is
 not defined, a copy of this value will be stored in the internal buckets. */
typedef TABLE_KEY pT_(key);
/*I don't think I need this? I said that last time.
 typedef const TABLE_KEY pT_(key_c);*/ /* Works 90%? */

/** A map from <typedef:<pT>key> onto <typedef:<pT>uint>, called `<t>hash`,
 that, ideally, should be easy to compute while minimizing duplicate addresses.
 Must be consistent for each value while in the table. If <typedef:<pT>key> is
 a pointer, one is permitted to have null in the domain. */
typedef pT_(uint) (*pT_(hash_fn))(const pT_(key));
#		ifdef TABLE_UNHASH
/** Defining `TABLE_UNHASH` says <typedef:<pT>hash_fn> forms a bijection
 between the range in <typedef:<pT>key> and the image in <typedef:<pT>uint>,
 and the inverse is called `<t>unhash`. In this case, keys are not stored
 in the hash table, rather they are generated using this inverse-mapping. (This
 provides a smaller and simpler hashing method where the information in the key
 being hashed is equal to the hash itself—such as numbers.) */
typedef pT_(key) (*pT_(unhash_fn))(pT_(uint));
#		else
/** Equivalence relation between <typedef:<pT>key> that satisfies
 `<t>is_equal_fn(a, b) -> <t>hash(a) == <t>hash(b)`, called `<t>is_equal`.
 If `TABLE_UNHASH` is set, there is no need for this function because the
 comparison is done directly in hash space. */
typedef int (*pT_(is_equal_fn))(const pT_(key) a, const pT_(key) b);
#		endif

#	ifdef TABLE_VALUE
/** Defining `TABLE_VALUE` produces an associative map, otherwise it is the
 same as <typedef:<pT>key>. */
typedef TABLE_VALUE pT_(value);
/** Defining `TABLE_VALUE` creates this map from <typedef:<pT>key> to
 <typedef:<pT>value>, as an interface with table. */
struct T_(entry) { pT_(key) key; pT_(value) value; };
/** If `TABLE_VALUE`, this is <tag:<T>entry>; otherwise, it's the same as
 <typedef:<pT>key>. */
typedef struct T_(entry) pT_(entry);
#	else
typedef pT_(key) pT_(value);
typedef pT_(key) pT_(entry);
#	endif

/* Address is hash modulo size of table. Any occupied buckets at the head of
 the linked structure are closed, that is, the address equals the index. These
 form a linked table, possibly with other, open buckets that have the same
 address in vacant buckets. */
struct pT_(bucket) {
	pT_(uint) next; /* Bucket index, including `TABLE_NULL` and `TABLE_END`. */
	pT_(uint) hash;
#	ifndef TABLE_UNHASH
	pT_(key) key;
#	endif
#	ifdef TABLE_VALUE
	pT_(value) value;
#	endif
};

/** Returns true if the `replace` replaces the `original`.
 (fixme: Shouldn't it be entry?) */
typedef int (*pT_(policy_fn))(pT_(key) original, pT_(key) replace);

/** To initialize, see <fn:<t>table>, `TABLE_IDLE`, `{0}` (`C99`,) or being
 `static`. The fields should be treated as read-only; any modification is
 liable to cause the table to go into an invalid state.

 ![States.](../doc/table/states.png) */
struct t_(table) { /* "Padding size," good. */
	struct pT_(bucket) *buckets; /* @ has zero/one key specified by `next`. */
	/* `size <= capacity`; size is not needed but convenient and allows
	 short-circuiting. Top is an index of the stack, potentially lazy: MSB
	 stores whether this is a step ahead (which would make it less, the stack
	 grows from the bottom,) otherwise it is right at the top, */
	pT_(uint) log_capacity, size, top;
};
typedef struct t_(table) pT_(box);

/** ![States](../doc/table/it.png)

 Adding, deleting, successfully looking up entries, or any modification of the
 table's topology invalidates the iterator.
 Iteration usually not in any particular order, but deterministic up to
 topology changes. The asymptotic runtime of iterating though the whole table
 is proportional to the capacity. */
struct T_(cursor) { struct t_(table) *table; pT_(uint) i; };

#	ifndef TABLE_DECLARE_ONLY /* <!-- body */

/** Gets the key of an occupied `bucket`. */
static pT_(key) pT_(bucket_key)(const struct pT_(bucket) *const bucket) {
	assert(bucket && bucket->next != TABLE_NULL);
#		ifdef TABLE_UNHASH
	/* On `TABLE_UNHASH`, this function must be defined by the user. */
	return t_(unhash)(bucket->hash);
#		else
	return bucket->key;
#		endif
}
/** Gets the value of an occupied `bucket`, which might be the same as the
 key. */
static pT_(value) pT_(bucket_value)(const struct pT_(bucket) *const bucket) {
	assert(bucket && bucket->next != TABLE_NULL);
#		ifdef TABLE_VALUE
	return bucket->value;
#		else
	return pT_(bucket_key)(bucket);
#		endif
}
/** The capacity of a non-idle `table` is always a power-of-two. */
static pT_(uint) pT_(capacity)(const struct t_(table) *const table)
	{ return assert(table && table->buckets && table->log_capacity >= 3),
	(pT_(uint))((pT_(uint))1 << table->log_capacity); }
/** @return Indexes the first (closed) bucket in the set of buckets with the
 same address from non-idle `table` given the `hash`. If the bucket is empty,
 it will have `next = TABLE_NULL` or it's own <fn:<pT>chain_head> not equal
 to the index (another open bucket). */
static pT_(uint) pT_(chain_head)(const struct t_(table) *const table,
	const pT_(uint) hash) { return hash & (pT_(capacity)(table) - 1); }
/** @return Search for the previous link in the bucket to `b` in `table`, if it
 exists, (by restarting and going though the list.)
 @order \O(`bucket size`) */
static struct pT_(bucket) *pT_(prev)(const struct t_(table) *const table,
	const pT_(uint) b) {
	const struct pT_(bucket) *const bucket = table->buckets + b;
	pT_(uint) to_next = TABLE_NULL, next;
	assert(table && bucket->next != TABLE_NULL);
	/* Note that this does not check for corrupted tables; would get assert. */
	for(next = pT_(chain_head)(table, bucket->hash);
		/* assert(next < capacity), */ next != b;
		to_next = next, next = table->buckets[next].next);
	return to_next != TABLE_NULL ? table->buckets + to_next : 0;
}

/* <!-- stack functions */
/** On return, the `top` of `table` will be empty and eager, but size is not
 incremented, leaving it in intermediate state. Amortized if you grow only. */
static void pT_(grow_stack)(struct t_(table) *const table) {
	/* Subtract one for eager. */
	pT_(uint) top = (table->top & ~TABLE_HIGH) - !(table->top & TABLE_HIGH);
	assert(table && table->buckets && table->top && top < pT_(capacity)(table));
	while(table->buckets[top].next != TABLE_NULL) assert(top), top--;
	table->top = top; /* Eager, since one is allegedly going to fill it. */
}
/** Force the evaluation of the stack of `table`, thereby making it eager. This
 is like searching for a bucket in open-addressing. @order \O(`buckets`) */
static void pT_(force_stack)(struct t_(table) *const table) {
	pT_(uint) top = table->top;
	if(top & TABLE_HIGH) { /* Lazy. */
		struct pT_(bucket) *bucket;
		top &= ~TABLE_HIGH;
		do bucket = table->buckets + ++top/*, assert(top < capacity)*/;
		while(bucket->next != TABLE_NULL
			&& pT_(chain_head)(table, bucket->hash) == top);
		table->top = top; /* Eager. */
	}
}
/** Is `i` in `table` possibly on the stack? (The stack grows from the high.) */
static int pT_(in_stack_range)(const struct t_(table) *const table,
	const pT_(uint) i) {
	return assert(table && table->buckets),
		(table->top & ~TABLE_HIGH) + !!(table->top & TABLE_HIGH) <= i;
}
/** Corrects newly-deleted `b` from `table` in the stack. */
static void pT_(shrink_stack)(struct t_(table) *const table,
	const pT_(uint) b) {
	assert(table && table->buckets && b < pT_(capacity)(table));
	assert(table->buckets[b].next == TABLE_NULL);
	if(!pT_(in_stack_range)(table, b)) return;
	pT_(force_stack)(table); /* Only have room for 1 step of laziness. */
	assert(pT_(in_stack_range)(table, b)); /* I think this is assured? Think. */
	if(b != table->top) {
		struct pT_(bucket) *const prev = pT_(prev)(table, table->top);
		table->buckets[b] = table->buckets[table->top];
		prev->next = b;
	}
	table->buckets[table->top].next = TABLE_NULL;
	table->top |= TABLE_HIGH; /* Lazy. */
}
/** Moves the `m` index in non-idle `table`, to the top of collision stack.
 This may result in an inconsistent state; one is responsible for filling that
 hole and linking it with top. */
static void pT_(move_to_top)(struct t_(table) *const table, const pT_(uint) m) {
	struct pT_(bucket) *move, *top, *prev;
	assert(table
		&& table->size < pT_(capacity)(table) && m < pT_(capacity)(table));
	pT_(grow_stack)(table); /* Leaves it in an eager state. */
	move = table->buckets + m, top = table->buckets + table->top;
	assert(move->next != TABLE_NULL && top->next == TABLE_NULL);
	if(prev = pT_(prev)(table, m)) prev->next = table->top; /* \O(|`bucket`|) */
	memcpy(top, move, sizeof *move), move->next = TABLE_NULL;
}
/* stack --> */

/** `TABLE_UNHASH` is injective, so in that case, we only compare hashes.
 @return `a` and `b`. */
static int pT_(equal_buckets)(/*pT_(key_c) a, pT_(key_c) b (maybe it's fixed?)*/
	const pT_(key) a, const pT_(key) b) {
#		ifdef TABLE_UNHASH
	return (void)a, (void)b, 1;
#		else
	/* Must have this function declared. */
	return t_(is_equal)(a, b);
#		endif
}
/** `table` will be searched linearly for `key` which has `hash`. */
static struct pT_(bucket) *pT_(query)(struct t_(table) *const table,
	/*pT_(key_c)*/const pT_(key) key, const pT_(uint) hash) {
	struct pT_(bucket) *bucket1;
	pT_(uint) head, b0 = TABLE_NULL, b1, b2;
	assert(table && table->buckets && table->log_capacity);
	bucket1 = table->buckets + (head = b1 = pT_(chain_head)(table, hash));
	/* Not the start of a bucket: empty or in the collision stack. */
	if((b2 = bucket1->next) == TABLE_NULL
		|| pT_(in_stack_range)(table, b1)
		&& b1 != pT_(chain_head)(table, bucket1->hash)) return 0;
	while(hash != bucket1->hash
		|| !pT_(equal_buckets)(key, pT_(bucket_key)(bucket1))) {
		if(b2 == TABLE_END) return 0;
		bucket1 = table->buckets + (b0 = b1, b1 = b2);
		assert(b1 < pT_(capacity)(table) && pT_(in_stack_range)(table, b1)
			&& b1 != TABLE_NULL);
		b2 = bucket1->next;
	}
	/* No reason not to splay, practically. As one's data gets bigger, this
	 probably causes more overhead, but the table is an unstable container
	 anyway, so it's unlikely that `<pT>bucket` is going to be that big. */
#		ifdef TABLE_DONT_SPLAY /* <!-- !splay */
#			undef TABLE_DONT_SPLAY
	return bucket1;
#		else /* !splay --><!-- splay: bring the MRU to the front. */
	if(b0 == TABLE_NULL) return bucket1;
	{
		struct pT_(bucket) *const bucket0 = table->buckets + b0,
			*const bucket_head = table->buckets + head, temp;
		bucket0->next = b2;
		memcpy(&temp, bucket_head, sizeof *bucket_head);
		memcpy(bucket_head, bucket1, sizeof *bucket1);
		memcpy(bucket1, &temp, sizeof temp);
		bucket_head->next = b1;
		return bucket_head;
	}
#		endif /* splay --> */
}
/** Ensures that `table` has enough buckets to fill `n` more than the size. May
 invalidate and re-arrange the order.
 @return Success; otherwise, `errno` will be set. @throws[realloc]
 @throws[ERANGE] Tried allocating more then can fit in half <typedef:<pT>uint>
 or `realloc` doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html). */
static int pT_(buffer)(struct t_(table) *const table, const pT_(uint) n) {
	struct pT_(bucket) *buckets;
	const pT_(uint) log_c0 = table->log_capacity,
		c0 = log_c0 ? (pT_(uint))((pT_(uint))1 << log_c0) : 0;
	pT_(uint) log_c1, c1, size1, i, wait, mask;
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
	{ struct pT_(bucket) *e = buckets + c0, *const e_end = buckets + c1;
		for( ; e < e_end; e++) e->next = TABLE_NULL; }
	mask = (pT_(uint))((((pT_(uint))1 << log_c0) - 1)
		^ (((pT_(uint))1 << log_c1) - 1));

	/* Rehash most closed buckets in the lower half. Create waiting
	 linked-stack by borrowing next. */
	wait = TABLE_END;
	for(i = 0; i < c0; i++) {
		struct pT_(bucket) *idx, *go;
		pT_(uint) g, hash;
		idx = table->buckets + i;
		if(idx->next == TABLE_NULL) continue;
		g = pT_(chain_head)(table, hash = idx->hash);
		/* It's a power-of-two size, so, like consistent hashing, `E[old/new]`
		 capacity that a closed bucket will remain where it is. */
		if(i == g) { idx->next = TABLE_END; continue; }
		if((go = table->buckets + g)->next == TABLE_NULL) {
			/* Priority is given to the first closed bucket; simpler later. */
			struct pT_(bucket) *head;
			pT_(uint) h = g & ~mask; assert(h <= g);
			if(h < g && i < h
				&& (head = table->buckets + h, assert(head->next != TABLE_NULL),
				pT_(chain_head)(table, head->hash) == g)) {
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
	{ pT_(uint) prev = TABLE_END, w = wait; while(w != TABLE_END) {
		struct pT_(bucket) *waiting = table->buckets + w;
		pT_(uint) cl = pT_(chain_head)(table, waiting->hash);
		struct pT_(bucket) *const closed = table->buckets + cl;
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
		struct pT_(bucket) *const waiting = table->buckets + wait;
		pT_(uint) h = pT_(chain_head)(table, waiting->hash);
		struct pT_(bucket) *const head = table->buckets + h;
		struct pT_(bucket) *top;
		assert(h != wait && head->next != TABLE_NULL);
		pT_(grow_stack)(table), top = table->buckets + table->top;
		memcpy(top, waiting, sizeof *waiting);
		top->next = head->next, head->next = table->top;
		wait = waiting->next, waiting->next = TABLE_NULL; /* Pop. */
	}

	return 1;
}
/** Replace the `key` and `hash` of `bucket`. Don't touch next. */
static void pT_(replace_key)(struct pT_(bucket) *const bucket,
	const pT_(key) key, const pT_(uint) hash) {
	(void)key;
	bucket->hash = hash;
#		ifndef TABLE_UNHASH
	bucket->key = key;
#		endif
}
/** Evicts the spot where `hash` goes in `table`. This results in a space in
 the table. */
static struct pT_(bucket) *pT_(evict)(struct t_(table) *const table,
	const pT_(uint) hash) {
	pT_(uint) i;
	struct pT_(bucket) *bucket;
	if(!pT_(buffer)(table, 1)) return 0; /* Amortized. */
	bucket = table->buckets + (i = pT_(chain_head)(table, hash)); /* Closed. */
	if(bucket->next != TABLE_NULL) { /* Occupied. */
		int in_stack = pT_(chain_head)(table, bucket->hash) != i;
		pT_(move_to_top)(table, i);
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
static enum table_result pT_(put_key)(struct t_(table) *const table,
	const pT_(key) key, pT_(key) *eject, const pT_(policy_fn) policy) {
	struct pT_(bucket) *bucket;
	const pT_(uint) hash = t_(hash)(key);
	enum table_result result;
	assert(table);
	if(table->buckets && (bucket = pT_(query)(table, key, hash))) {
		if(!policy || !policy(pT_(bucket_key)(bucket), key))
			return TABLE_PRESENT;
		if(eject) *eject = pT_(bucket_key)(bucket);
		result = TABLE_PRESENT;
	} else {
		if(!(bucket = pT_(evict)(table, hash))) return TABLE_ERROR;
		result = TABLE_ABSENT;
	}
	pT_(replace_key)(bucket, key, hash);
	return result;
}

/** @return At the first element of `table`. */
static struct T_(cursor) T_(begin)(struct t_(table) *const table)
	{ struct T_(cursor) it; it.table = table, it.i = 0; return it; }
/** @return Whether the `cur` points to an element. */
static int T_(exists)(/*const*/ struct T_(cursor) *const cur) {
	const struct t_(table) *t;
	pT_(uint) limit;
	if(!cur || !(t = cur->table) || !cur->table->buckets /* Idle */) return 0;
	limit = pT_(capacity)(t);
	/* This actually modifies the cursor, but it does it in a idempotent way;
	 I think we're good. Otherwise code duplication. Have to have another
	 function. */
	while(cur->i < limit) {
		if(t->buckets[cur->i].next != TABLE_NULL) return 1;
		cur->i++;
	}
	cur->table = 0;
	return 0;
}
/** @return Pointer to a bucket at valid non-null `cur`. */
static struct pT_(bucket) *T_(look)(const struct T_(cursor) *const cur)
	{ return cur->table->buckets + cur->i; }
/** @return If `cur` has an element, returns it's key. @allow */
static pT_(key) T_(key)(const struct T_(cursor) *const cur)
	{ return pT_(bucket_key)(cur->table->buckets + cur->i); }
#		ifdef TABLE_VALUE
/** @return If `cur` has an element, returns it's value, if `TABLE_VALUE`.
 @allow */
static pT_(value) *T_(value)(const struct T_(cursor) *const cur)
	{ return &cur->table->buckets[cur->i].value; }
#		endif
/** Move to next on `cur` that exists. */
static void T_(next)(struct T_(cursor) *const cur)
	{ cur->i++; /* Will be precisely set on exists. */ }
/** Removes the entry at `cur`. Whereas <fn:<T>remove> invalidates the
 cursor, this corrects `cur` so <fn:<T>next> is the next entry. To use
 the cursor after this, one must move to the next.
 @return Success, or there was no entry at the iterator's position, (anymore.)
 @allow */
static int T_(cursor_remove)(struct T_(cursor) *const cur) {
	struct t_(table) *table = cur->table;
	struct pT_(bucket) *previous = 0, *current;
	pT_(uint) prv = TABLE_NULL, crnt;
	assert(cur && table);
	if(!cur->table->buckets) return 0;
	assert(cur->i < pT_(capacity)(cur->table));
	if(cur->i >= pT_(capacity)(cur->table)) return 0;
	/* Get the last bucket. */
	current = cur->table->buckets + cur->i, assert(current->next != TABLE_NULL);
	crnt = pT_(chain_head)(cur->table, current->hash);
	while(crnt != cur->i) assert(crnt < pT_(capacity)(cur->table)),
		crnt = (previous = cur->table->buckets + (prv = crnt))->next;
	if(prv != TABLE_NULL) { /* Open entry. */
		previous->next = current->next;
	} else if(current->next != TABLE_END) { /* Head closed entry and others. */
		const pT_(uint) scnd = current->next;
		struct pT_(bucket) *const second = table->buckets + scnd;
		assert(scnd < pT_(capacity)(table));
		memcpy(current, second, sizeof *second);
		/* Because we replace current with a bucket we haven't seen yet. */
		if(crnt < scnd) cur->i--;
		crnt = scnd, current = second;
	}
	current->next = TABLE_NULL, table->size--, pT_(shrink_stack)(table, crnt);
	return 1;
}

/** Zeroed data (not all-bits-zero) is initialized. @return An idle array.
 @order \Theta(1) @allow */
static struct t_(table) t_(table)(void) {
	struct t_(table) table;
	table.buckets = 0; table.log_capacity = 0; table.size = 0; table.top = 0;
	return table;
}

/** If `table` is not null, destroys and returns it to idle. @allow */
static void t_(table_)(struct t_(table) *const table)
	{ if(table) free(table->buckets), *table = t_(table)(); }

/** Reserve at least `n` more empty buckets in `table`. This may cause the
 capacity to increase and invalidates any pointers to data in the table.
 @return Success.
 @throws[ERANGE] The request was unsatisfiable. @throws[realloc] @allow */
static int T_(buffer)(struct t_(table) *const table, const pT_(uint) n)
	{ return assert(table), pT_(buffer)(table, n); }

/** Clears and removes all buckets from `table`. The capacity and memory of the
 `table` is preserved, but all previous values are un-associated. (The load
 factor will be less until it reaches it's previous size.)
 @order \Theta(`table.capacity`) @allow */
static void T_(clear)(struct t_(table) *const table) {
	struct pT_(bucket) *b, *b_end;
	assert(table);
	if(!table->buckets) { assert(!table->log_capacity); return; }
	assert(table->log_capacity);
	for(b = table->buckets, b_end = b + pT_(capacity)(table); b < b_end; b++)
		b->next = TABLE_NULL;
	table->size = 0;
	table->top = (pT_(capacity)(table) - 1) | TABLE_HIGH;
}

/** @return Whether `key` is in `table` (which can be null.) @allow */
static int T_(contains)(struct t_(table) *const table, const pT_(key) key) {
	/* This function must be defined by the user. */
	return table && table->buckets
		? !!pT_(query)(table, key, t_(hash)(key)) : 0;
}

#		ifdef TABLE_VALUE /* <!-- map */
/** If can be no default key, use this to separate a null—returns false—from
 a result. Otherwise, a more convenient function is <fn:<T>get_or>.
 @param[result] If null, behaves like <fn:<T>contains>, otherwise, a
 <typedef:<pT>key> which gets filled on true.
 @param[value] Only on a map with `TABLE_VALUE`. If not-null, stores the value.
 @return Whether `key` is in `table` (which can be null.) @allow */
static int T_(query)(struct t_(table) *const table, const pT_(key) key,
	pT_(key) *result, pT_(value) *value) {
	struct pT_(bucket) *bucket;
	if(!table || !table->buckets
		|| !(bucket = pT_(query)(table, key, t_(hash)(key)))) return 0;
	if(result) *result = pT_(bucket_key)(bucket);
	if(value) *value = bucket->value;
	return 1;
}
#		else
/** `key` from `table` is stored in `result`. */
static int T_(query)(struct t_(table) *const table, const pT_(key) key,
	pT_(key) *result) {
	struct pT_(bucket) *bucket;
	if(!table || !table->buckets
		|| !(bucket = pT_(query)(table, key, t_(hash)(key)))) return 0;
	if(result) *result = pT_(bucket_key)(bucket);
	return 1;
}
#		endif

/** @return The value associated with `key` in `table`, (which can be null.) If
 no such value exists, `default_value` is returned.
 @order Average \O(1); worst \O(n). @allow */
static pT_(value) T_(get_or)(struct t_(table) *const table,
	const pT_(key) key, pT_(value) default_value) {
	struct pT_(bucket) *bucket;
	/* This function must be defined by the user. */
	return table && table->buckets
		&& (bucket = pT_(query)(table, key, t_(hash)(key)))
		? pT_(bucket_value)(bucket) : default_value;
}

#		ifndef TABLE_VALUE /* <!-- set */

/** Only if `TABLE_VALUE` is not set; see <fn:<T>assign> for a map. Puts `key`
 in set `table` only if absent.
 @return One of: `TABLE_ERROR`, tried putting the entry in the table but
 failed, the table is not modified; `TABLE_PRESENT`, does nothing if there is
 another entry with the same key; `TABLE_ABSENT`, put an entry in the table.
 @throws[realloc, ERANGE] On `TABLE_ERROR`.
 @order Average amortised \O(1); worst \O(n). @allow */
static enum table_result T_(try)(struct t_(table) *const table,
	pT_(key) key) { return pT_(put_key)(table, key, 0, 0); }

#		else /* set --><!-- map */

/** Only if `TABLE_VALUE` is set. Ensures that `key` is in the `table` and
 update `content`. @throws[malloc] */
static enum table_result pT_(assign)(struct t_(table) *const table,
	pT_(key) key, pT_(value) **const content) {
	struct pT_(bucket) *bucket;
	const pT_(uint) hash = t_(hash)(key);
	enum table_result result;
	assert(table && content);
	if(table->buckets && (bucket = pT_(query)(table, key, hash))) {
		result = TABLE_PRESENT;
	} else {
		if(!(bucket = pT_(evict)(table, hash))) return TABLE_ERROR;
		pT_(replace_key)(bucket, key, hash);
		result = TABLE_ABSENT;
	}
	*content = &bucket->value;
	return result;
}

/** Only if `TABLE_VALUE` is set; see <fn:<T>try> for a set. Puts `key` in the
 map `table` and store the associated value in `content`.
 @return `TABLE_ERROR` does not set `content`; `TABLE_ABSENT`, the `content`
 will be a pointer to uninitialized memory; `TABLE_PRESENT`, gets the current
 `content`, (does not alter the keys, if they are distinguishable.)
 @throws[malloc, ERANGE] On `TABLE_ERROR`. @allow */
static enum table_result T_(assign)(struct t_(table) *const table,
	pT_(key) key, pT_(value) **const content)
	{ return pT_(assign)(table, key, content); }

#		endif /* value --> */

/** Callback in <fn:<T>update>.
 @return `original` and `replace` ignored, true.
 @implements <typedef:<pT>policy_fn> */
static int pT_(always_replace)(const pT_(key) original,
	const pT_(key) replace) { return (void)original, (void)replace, 1; }

/** Puts `key` in `table`, replacing an equal-valued key. (If keys are
 indistinguishable, this function is not very useful, see <fn:<T>try> or
 <fn:<T>assign>.)
 @return One of: `TABLE_ERROR`, the table is not modified; `TABLE_ABSENT`, the
 `key` is new; `TABLE_PRESENT`, the `key` displaces another, and if non-null,
 `eject` will be filled with the previous entry.
 @throws[realloc, ERANGE] On `TABLE_ERROR`.
 @order Average amortised \O(1); worst \O(n). @allow */
static enum table_result T_(update)(struct t_(table) *const table,
	pT_(key) key, pT_(key) *eject)
	{ return pT_(put_key)(table, key, eject, &pT_(always_replace)); }

/** Puts `key` in `table` only if absent or if calling `policy` returns true.
 @return One of: `TABLE_ERROR`, the table is not modified; `TABLE_ABSENT`, the
 `key` is new; `TABLE_PRESENT`, `key` collides, if `policy` returns true,
 `eject`, if non-null, will be filled;
 @throws[realloc, ERANGE] On `TABLE_ERROR`.
 @order Average amortised \O(1); worst \O(n). @allow */
static enum table_result T_(policy)(struct t_(table) *const table,
	pT_(key) key, pT_(key) *eject, const pT_(policy_fn) policy)
	{ return pT_(put_key)(table, key, eject, policy); }

/** Removes `key` from `table` (which could be null.)
 @return Whether that `key` was in `table`. @order Average \O(1), (hash
 distributes elements uniformly); worst \O(n). @allow */
static int T_(remove)(struct t_(table) *const table, const pT_(key) key) {
	struct pT_(bucket) *current;
	pT_(uint) c, p = TABLE_NULL, n, hash = t_(hash)(key);
	if(!table || !table->size) return 0;
	assert(table->buckets);
	/* Find item and keep track of previous. */
	current = table->buckets + (c = pT_(chain_head)(table, hash));
	if((n = current->next) == TABLE_NULL /* No entry here. */
		|| pT_(in_stack_range)(table, c)
		&& c != pT_(chain_head)(table, current->hash)) return 0;
	/* Find prev? Why not <fn:<PN>prev>? */
	while(hash != current->hash
		&& !pT_(equal_buckets)(key, pT_(bucket_key)(current))) {
		if(n == TABLE_END) return 0;
		p = c, current = table->buckets + (c = n);
		assert(c < pT_(capacity)(table) && pT_(in_stack_range)(table, c)
			&& c != TABLE_NULL);
		n = current->next;
	}
	if(p != TABLE_NULL) { /* Open entry. */
		struct pT_(bucket) *previous = table->buckets + p;
		previous->next = current->next;
	} else if(current->next != TABLE_END) { /* Head closed entry and others. */
		struct pT_(bucket) *const second
			= table->buckets + (c = current->next);
		assert(current->next < pT_(capacity)(table));
		memcpy(current, second, sizeof *second);
		current = second;
	}
	current->next = TABLE_NULL, table->size--, pT_(shrink_stack)(table, c);
	return 1;
}

#		ifdef HAVE_ITERATE_H /* <!-- iterate */
#			include "iterate.h" /** \include */
#		endif /* iterate --> */

static void pT_(unused_base_coda)(void);
static void pT_(unused_base)(void) {
	pT_(entry) e; pT_(key) k; pT_(value) v;
	memset(&e, 0, sizeof e); memset(&k, 0, sizeof k); memset(&v, 0, sizeof v);
	T_(begin)(0); T_(exists)(0); T_(look)(0); T_(key)(0);
	T_(cursor_remove)(0);
	t_(table)(); t_(table_)(0);
	T_(buffer)(0, 0); T_(clear)(0); T_(contains)(0, k); T_(get_or)(0, k, v);
	T_(update)(0, k, 0); T_(policy)(0, k, 0, 0); T_(remove)(0, k);
#		ifdef TABLE_VALUE
	T_(value)(0); T_(query)(0, k, 0, 0); T_(assign)(0, k, 0);
#		else
	T_(query)(0, k, 0); T_(try)(0, e);
#		endif
	pT_(unused_base_coda)();
}
static void pT_(unused_base_coda)(void) { pT_(unused_base)(); }

#	endif /* body --> */
#endif /* base code --> */

#ifndef TABLE_DECLARE_ONLY /* Produce code. */

#	if defined(TABLE_TO_STRING)
#		undef TABLE_TO_STRING
#		ifndef ARRAY_TRAIT
#			ifdef TABLE_VALUE
/** The type of the required `<tr>to_string`. Responsible for turning the
 read-only argument into a 12-max-`char` output string. `<pT>value` is omitted
 when it's a set. */
typedef void (*pT_(to_string_fn))(const pT_(key), const pT_(value) *,
	char (*)[12]);
#			else
typedef void (*pT_(to_string_fn))(const pT_(key), char (*)[12]);
#			endif
#		endif
/** Thunk(`cur`, `a`). One must implement `<tr>to_string`. */
static void pTR_(to_string)(const struct T_(cursor) *const cur,
	char (*const a)[12]) {
	const struct pT_(bucket) *const b = cur->table->buckets + cur->i;
#		ifdef TABLE_VALUE
	tr_(to_string)(pT_(bucket_key)(b), pT_(bucket_value)(b), a);
#		else
	tr_(to_string)(pT_(bucket_key)(b), a);
#		endif
}
#		define TO_STRING_LEFT '{'
#		define TO_STRING_RIGHT '}'
#		include "to_string.h" /** \include */
#		ifndef TABLE_TRAIT
#			define TABLE_HAS_TO_STRING /* Warning about tests. */
#		endif
#	endif

#	if defined(TABLE_TEST) && !defined(TABLE_TRAIT)
#		include "../test/test_table.h"
#	endif

#	ifdef TABLE_DEFAULT /* <!-- default trait */
/*#		include "../test/test_table_default.h", just test manually. */
/** This is functionally identical to <fn:<T>get_or>, but a with a trait
 specifying a constant default value. This is the most convenient access
 method, but it needs to have a `TABLE_DEFAULT`.
 @return The value associated with `key` in `table`, (which can be null.) If
 no such value exists, the `TABLE_DEFAULT` for this trait is returned.
 @order Average \O(1); worst \O(n). @allow */
static pT_(value) T_R_(table, get)(struct t_(table) *const table,
	const pT_(key) key) {
	struct pT_(bucket) *bucket;
	/* `TABLE_DEFAULT` is a valid <tag:<pT>value>. */
	const pT_(value) pTR_(default_value) = TABLE_DEFAULT;
	/* Function `<N>hash` must be defined by the user. */
	return table && table->buckets
		&& (bucket = pT_(query)(table, key, t_(hash)(key)))
		? pT_(bucket_value)(bucket) : pTR_(default_value);
}
static void pTR_(unused_default_coda)(void);
static void pTR_(unused_default)(void) { pT_(key) k; memset(&k, 0, sizeof k);
	T_R_(table, get)(0, k); pTR_(unused_default_coda)(); }
static void pTR_(unused_default_coda)(void) { pTR_(unused_default)(); }
#		undef TABLE_DEFAULT
#	endif /* default trait --> */

#endif /* Produce code. */
#ifdef TABLE_TRAIT
#	undef TABLE_TRAIT
#	undef BOX_TRAIT
#endif


#ifdef TABLE_EXPECT_TRAIT
#	undef TABLE_EXPECT_TRAIT
#else
#	undef BOX_MINOR
#	undef BOX_MAJOR
#	undef TABLE_NAME
#	undef TABLE_KEY
#	undef TABLE_UINT
#	undef TABLE_HASH
#	ifdef TABLE_IS_EQUAL
#		undef TABLE_IS_EQUAL
#	else
#		undef TABLE_UNHASH
#	endif
#	ifdef TABLE_VALUE
#		undef TABLE_VALUE
#	endif
#	ifdef TABLE_HAS_TO_STRING
#		undef TABLE_HAS_TO_STRING
#	endif
#	ifdef TABLE_TEST
#		undef TABLE_TEST
#	endif
#	ifdef TABLE_DECLARE_ONLY
#		undef TABLE_DECLARE_ONLY
#	endif
#endif
#define BOX_END
#include "box.h"
