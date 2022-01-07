/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Hash set

 ![Example of <string>set.](../web/set.png)

 <tag:<S>set> is implemented as a hash table of unordered <typedef:<PS>type>
 that doesn't allow duplication. It must be supplied a hash function and
 equality function.

 Enclosing a pointer <typedef:<PS>type> in a larger `struct` can give an
 associative array. Compile-time constant sets are better handled with
 [gperf](https://www.gnu.org/software/gperf/). Also,
 [CMPH](http://cmph.sourceforge.net/) is a minimal perfect hashing library
 that provides performance for large sets.

 @param[SET_NAME, SET_TYPE]
 `<S>` that satisfies `C` naming conventions when mangled and a valid
 <typedef:<PS>type> associated therewith; required. Type is copied extensively,
 so if it's large, making it a pointer may improve performance. `<PS>` is
 private, whose names are prefixed in a manner to avoid collisions; any should
 be re-defined prior to use elsewhere.

 @param[SET_HASH, SET_IS_EQUAL]
 A function satisfying <typedef:<PS>hash_fn> and <typedef:<PS>is_equal_fn>;
 required.

 @param[SET_UINT]
 This is <typedef:<PS>uint>, the unsigned hash type, and defaults to `size_t`.

 @param[SET_RECALCULATE]
 Don't cache the hash, but calculate every time; this avoids storing
 <typedef:<PS>uint> _per_ entry, but can be slower when the hash is
 non-trivial to compute.

 @param[SET_INVERSE_HASH]
 Function satisfying <typedef:<PS>inverse_hash_fn> that avoids storing the key,
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
 /* Bug in cdoc. @fixme Implement move-to-front like splay-trees.
 @fixme Implement SET_RECALCULATE and SET_INVERSE_HASH. */

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
#define SET_IDLE { 0, 0, 0, 0, 0 }
/* fixme: ~0^(~0>>1)[+1] would be more energy efficient, since we're not using
 half the range. Make sure one splits <fn:<PS>buffer> from the rest, whose
 value is reliant on -1. */
/* Use negative values of <typedef:<PS>uint> to store special things, such that
 range of an index is 3 less than the maximum. (I think these work on
 mathematically-impaired representations, ones', s&m, and odd TI padding.) */
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
 supported and will probably lead to warnings. */
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
 <typedef:<PS>hash_fn>. Used to avoid having to store the <typedef:<PS>type>. */
typedef PS_(type) (*PS_(inverse_hash_fn))(PS_(uint));
#error Fixme. Think about how to iterate. Maybe `it` has a temp field?
#endif /* inv --> */

/** Equivalence relation between <typedef:<PS>ctype> that satisfies
 `<PS>is_equal_fn(a, b) -> <PS>SET_HASH(a) == <PS>SET_HASH(b)`. */
typedef int (*PS_(is_equal_fn))(const PS_(ctype) a, const PS_(ctype) b);
/* Check that `SET_IS_EQUAL` is a function implementing
 <typedef:<PS>is_equal_fn>. */
static const PS_(is_equal_fn) PS_(equal) = (SET_IS_EQUAL);

/** Buckets are linked-lists of entries, and entries are stored in a hash
 table. When a collision occurs, we push the entry out to an unoccupied stack
 in the same table. */
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

/** To initialize, see <fn:<S>set>, `SET_IDLE`, `{0}` (`C99`,) or being
 `static`.

 ![States.](../web/states.png) */
struct S_(set) {
	struct PS_(entry) *entries; /* @ has zero/one key specified by `next`. */
	PS_(uint) size; /* How many keys, <= capacity. */
	PS_(uint) top; /* -1 no stack; collided entries. */
	unsigned log_capacity, unused; /* Applies to entries. */
};

/** The capacity of a non-idle `set` is always a power-of-two. */
static PS_(uint) PS_(capacity)(const struct S_(set) *const set)
	{ return assert(set && set->entries && set->log_capacity >= 3),
	(PS_(uint))1 << set->log_capacity; }

/** @return Indexes the first `set.entries` in the bucket (a closed entry) from
 non-idle `set` given the `hash`. */
static PS_(uint) PS_(hash_to_bucket)(const struct S_(set) *const set,
	const PS_(uint) hash) { return hash & (PS_(capacity)(set) - 1); }

/** This is amortized; every value takes at most one top. On return, the `top`
 of `set` will be empty. */
static void PS_(grow_stack)(struct S_(set) *const set) {
	PS_(uint) top = set->top;
	assert(set && set->entries && top);
	top = (top == SETm1 ? PS_(capacity)(set) : top) - 1;
	while(set->entries[top].next != SETm2) assert(top), top--;
	set->top = top;
}

/** Is `idx` is `set` possibly on the stack? (Got tired of changing every time
 I wanted to change the direction.) */
static int PS_(in_stack_range)(const struct S_(set) *const set,
	const PS_(uint) idx)
	{ return assert(set), set->top != SETm1 && set->top <= idx; }

/***********fixme*/
static void (*PS_(to_string))(const PS_(type) *, char (*)[12]);

/** Moves the index `victim` to the top of the collision stack in non-idle
 `set`. This is an inconsistent state; one is responsible for filling that hole
 and linking it with top. */
static void PS_(move_to_top)(struct S_(set) *const set,
	const PS_(uint) victim) {
	struct PS_(entry) *top, *vic;
	PS_(uint) to_next, next;
	char z[12];
	const PS_(uint) capacity = PS_(capacity)(set);
	assert(set->size < capacity && victim < capacity);
	PS_(grow_stack)(set);
	vic = set->entries + victim, top = set->entries + set->top;
	assert(vic->next != SETm2 && top->next == SETm2); /* Occupied to vacant. */
	PS_(to_string)(&vic->key, &z);
	printf("move_to_top: victim \"%s\" moved from 0x%lx to top 0x%lx\n",
		z, (unsigned long)victim, (unsigned long)set->top);
	/* Search for the previous link in the linked-list. \O(|bucket|). */
	for(to_next = SETm2, next = PS_(hash_to_bucket)(set, PS_(entry_hash)(vic));
		assert(next < capacity), PS_(to_string)(&set->entries[next].key, &z), printf("searching for victim in bucket: \"%s\" 0x%lx\n", z, (unsigned long)next), next != victim;
		to_next = next, next = set->entries[next].next);
	printf("got \"%s\"\n", z);
	/* Move `vic` to `top`. */
	if(to_next != SETm2) set->entries[to_next].next = set->top;
	memcpy(top, vic, sizeof *vic), vic->next = SETm2;
}

/** `set` will be searched linearly for `key` which has `hash`.
 @fixme Fix for inverse. @fixme Move to front like splay trees? */
static struct PS_(entry) *PS_(get)(struct S_(set) *const set,
	const PS_(type) key, const PS_(uint) hash) {
	struct PS_(entry) *entry;
	PS_(uint) idx, next;
	assert(set && set->entries && set->log_capacity);
	entry = set->entries + (idx = PS_(hash_to_bucket)(set, hash));
	/* Not the start of a bucket: empty or in the collision stack. */
	if((next = entry->next) == SETm2
		|| PS_(in_stack_range)(set, idx)
		&& idx != PS_(hash_to_bucket)(set, PS_(entry_hash)(entry))) return 0;
	for( ; ; ) {
#ifdef SET_RECALCULATE /* <!-- !cache: always go to next predicate. */
		const int hashes_are_equal = ((void)(hash), 1);
#else /* !cache --><!-- cache: quick out. */
		const int hashes_are_equal = hash == entry->hash;
#endif /* cache --> */
		if(hashes_are_equal && PS_(equal)(key, entry->key)) return entry;
		if(next == SETm1) return 0; /* -1 used to end the bucket. */
		idx = next;
		assert(idx < PS_(capacity)(set) && PS_(in_stack_range)(set, idx));
		entry = set->entries + idx;
		next = entry->next;
		assert(next != SETm2); /* -2 null: linked-list integrity. */
	}
}

#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)
static void PS_(graph)(const struct S_(set) *const set, const char *const fn);

/** Ensures that `set` has enough entries to fill `n` more than the size.
 May invalidate `entries` and re-arrange the order.
 @return Success; otherwise, `errno` will be set. @throws[realloc]
 @throws[ERANGE] Tried allocating more then can fit in half <typedef:<PS>uint>
 or `realloc` doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html). */
static int PS_(buffer)(struct S_(set) *const set, const PS_(uint) n) {
	struct PS_(entry) *entries;
	const unsigned log_c0 = set->log_capacity;
	unsigned log_c1;
	/* fixme: this will have to be updated because it relies on -1. */
	const PS_(uint) limit = SETm1 ^ (SETm1 >> 1) /* TI C6000, _etc_ works? */,
		c0 = log_c0 ? (PS_(uint))1 << log_c0 : 0;
	PS_(uint) c1, size1, i, wait, mask;
	char fn[64];

	assert(set && n <= SETm1 && set->size <= SETm1 && limit && limit <= SETm1);
	assert((!set->entries && !set->size && !log_c0 && !c0
		|| set->entries && set->size <= c0 && log_c0 >= 3));
	printf("buffer: max %lu, limit %lu, entries %lu/%lu, new %lu\n",
		(unsigned long)SETm1, (unsigned long)limit,
		(unsigned long)set->size, (unsigned long)c0, (unsigned long)n);

	/* Can we satisfy `n` growth from the buffer? */
	if(SETm1 - set->size < n || limit < (size1 = set->size + n))
		return errno = ERANGE, 0;
	if(set->entries) log_c1 = log_c0, c1 = c0 ? c0 : 1;
	else             log_c1 = 3,      c1 = 8;
	while(c1 < size1) log_c1++, c1 <<= 1;
	if(log_c0 == log_c1) return 1;

	sprintf(fn, "graph/" QUOTE(SET_NAME) "-resize-%u-%u-a-before.gv",
		log_c0, log_c1);
	PS_(graph)(set, fn);

	/* Otherwise, need to allocate more. */
	printf("buffer: %lu -> %lu to satisfy %lu.\n",
		(unsigned long)c0, (unsigned long)c1, (unsigned long)size1);
	if(!(entries = realloc(set->entries, sizeof *entries * c1)))
		{ if(!errno) errno = ERANGE; return 0; }
	if(!set->entries) set->top = SETm1; /* Idle `top` initialized here. */
	set->entries = entries, set->log_capacity = log_c1;

	/* Initialize new values. Reset the stack. Mask off the added bits. */
	{ struct PS_(entry) *e = entries + c0, *const e_end = entries + c1;
		for( ; e < e_end; e++) e->next = SETm2; }
	set->top = SETm1;
	mask = (((PS_(uint))1 << log_c0) - 1) ^ (((PS_(uint))1 << log_c1) - 1);

	/* Rehash most closed entries in the lower half. */
	printf("buffer: rehash %lu entries; mask 0x%lx.\n",
		(unsigned long)c0, (unsigned long)mask);
	wait = SETm1;
	for(i = 0; i < c0; i++) {
		struct PS_(entry) *idx, *go;
		PS_(uint) g, hash;
		idx = set->entries + i;
		printf("A.\t0x%lx: ", (unsigned long)i);
		/* Empty; don't have to do anything. */
		if(idx->next == SETm2) { printf("empty.\n"); continue; }
		/* Where it is closed. */
		g = PS_(hash_to_bucket)(set, hash = PS_(entry_hash)(idx));
		{
			PS_(type) key = PS_(entry_key)(idx);
			char z[12];
			PS_(to_string)(&key, &z);
			printf("\"%s\"->0x%lx ", z, (unsigned long)g);
		}
		/* Like consistent hashing, because it's a power-of-two size,
		 `E[old/new]` capacity that a closed entry will remain where it is. */
		if(i == g) { idx->next = SETm1; printf("chill.\n"); continue; }
		if((go = set->entries + g)->next == SETm2) {
			/* Priority is given to the closed head entries; simpler later. */
			struct PS_(entry) *head;
			PS_(uint) h = g & ~mask; assert(h <= g);
			if(h < g && i < h /* Lookahead to the first entry in the bucket. */
				&& (head = set->entries + h, assert(head->next != SETm2),
				g == PS_(hash_to_bucket)(set, PS_(entry_hash)(head)))) {
				char y[12];
				PS_(to_string)(&head->key, &y);
				printf("future 0x%lx \"%s\"->0x%lx will go instead, ",
					(unsigned long)h, y, (unsigned long)g);
				memcpy(go, head, sizeof *head);
				go->next = SETm1, head->next = SETm2;
				/* Fall-though -- the entry still needs to be put on waiting. */
			} else {
				/* If the new entry is available and this entry is first. */
				memcpy(go, idx, sizeof *idx);
				go->next = SETm1, idx->next = SETm2;
				printf("to vacant.\n");
				continue;
			}
		}
		printf("wait.\n");
		idx->next = wait, wait = i; /* Push for next sweep. */
	}

	sprintf(fn, "graph/" QUOTE(SET_NAME) "-resize-%u-%u-b-waiting.gv",
		log_c0, log_c1);
	PS_(graph)(set, fn);

	{
		PS_(uint) w = wait;
		printf("waiting: { ");
		while(w != SETm1) {
			printf("0x%lx ", (unsigned long)w);
			w = set->entries[w].next;
		}
		printf("} checking for stragglers.\n");
	}

	/* Search waiting stack for rest of the closed that moved concurrently. */
	{ PS_(uint) prev = SETm1, w = wait; while(w != SETm1) {
		char z[12];
		struct PS_(entry) *waiting = set->entries + w;
		PS_(uint) cl = PS_(hash_to_bucket)(set, PS_(entry_hash)(waiting));
		struct PS_(entry) *const closed = set->entries + cl;
		assert(cl != w);
		{
			PS_(type) key = PS_(entry_key)(waiting);
			PS_(to_string)(&key, &z);
			printf("B.\t0x%lx: \"%s\"->%lx ",
				(unsigned long)w, z, (unsigned long)cl);
		}
		if(closed->next == SETm2) {
			memcpy(closed, waiting, sizeof *waiting), closed->next = SETm1;
			printf("to vacant.\n");
			if(prev != SETm1) set->entries[prev].next = waiting->next;
			if(wait == w) wait = waiting->next; /* First, modify head. */
			w = waiting->next, waiting->next = SETm2;
		} else {
			/* Not in the wait stack. */
			assert(closed->next == SETm1);
			printf("wait.\n");
			prev = w, w = waiting->next;
		}
	}}

	{
		PS_(uint) w = wait;
		printf("waiting: { ");
		while(w != SETm1) {
			printf("0x%lx ", (unsigned long)w);
			w = set->entries[w].next;
		}
		printf("} moving to stack.\n");
	}

	/* Rebuild the (smaller?) top stack (high) from the waiting (low). */
	while(wait != SETm1) {
		char z[12];
		struct PS_(entry) *const waiting = set->entries + wait;
		PS_(uint) h = PS_(hash_to_bucket)(set, PS_(entry_hash)(waiting));
		struct PS_(entry) *const head = set->entries + h;
		struct PS_(entry) *top;
		assert(h != wait && head->next != SETm2);
		PS_(grow_stack)(set), top = set->entries + set->top;
		{
			PS_(type) key = PS_(entry_key)(waiting);
			PS_(to_string)(&key, &z);
			printf("\t0x%lx: \"%s\" to stack 0x%lx.\n",
				(unsigned long)wait, z, (unsigned long)set->top);
		}
		memcpy(top, waiting, sizeof *waiting);
		top->next = head->next, head->next = set->top;
		wait = waiting->next, waiting->next = SETm2; /* Pop. */
	}

	{ PS_(uint) j;
	printf("buffer::rehash: final top 0x%lx\n", (long)set->top);
	for(j = 0; j < PS_(capacity)(set); j++) {
		struct PS_(entry) *je = set->entries + j;
		PS_(type) key;
		char z[12];
		printf("\t0x%lx: ", (unsigned long)j);
		if(je->next == SETm2) { printf("--\n"); continue; }
		key = PS_(entry_key)(je);
		PS_(to_string)(&key, &z);
		printf("\"%s\"", z);
		if(je->next == SETm1) { printf("\n"); continue; }
		printf(" -> 0x%lx\n", (unsigned long)je->next);
	}}
	return 1;
}

#undef QUOTE_
#undef QUOTE

/** A bi-predicate; returns true if the `replace` replaces the `original`. */
typedef int (*PS_(replace_fn))(PS_(type) original, PS_(type) replace);

/** Used in <fn:<S>set_policy_put> when `replace` is null; `original` and
 `replace` are ignored. @implements `<PS>replace_fn` */
static int PS_(false)(PS_(type) original, PS_(type) replace)
	{ (void)(original); (void)(replace); return 0; }

/** Put `key` in `set` as long as `replace` is null or returns true.
 @param[eject] If non-null, the equal element, if any. If `replace`
 returns false, the address of `key`.
 @return Success. @throws[malloc] @order amortized \O(1) */
static int PS_(put)(struct S_(set) *const set, const PS_(replace_fn) replace,
	PS_(type) key, PS_(type) *eject) {
	struct PS_(entry) *entry;
	PS_(uint) hash, idx, next = SETm1 /* The end of a linked-list. */, size;
	char z[12];
	assert(set && key);
	PS_(to_string)(&key, &z);
	if(eject) *eject = 0;
	hash = PS_(hash)(key);
	size = set->size;
	printf("put: \"%s\" hash 0x%lx.\n", z, (unsigned long)hash);
	if(set->entries && (entry = PS_(get)(set, key, hash))) { /* Replace. */
		if(replace && !replace(PS_(entry_key)(entry), key))
			{ if(eject) *eject = key; return 1; } /* Decided not to replace. */
		if(eject) *eject = PS_(entry_key)(entry);
		/* Cut the tail and put new element in the head. */
		next = entry->next, entry->next = SETm2, assert(next != SETm2);
	} else { /* Expand. */
		if(!PS_(buffer)(set, 1)) return 0; /* Amortized. */
		entry = set->entries + (idx = PS_(hash_to_bucket)(set, hash));
		/*printf("\tput expand: \"%s\" index 0x%lx from hash 0x%lx\n",
			z, (unsigned long)idx, (unsigned long)hash);*/
		size++;
		if(entry->next != SETm2) { /* Unoccupied. */
			int already_in_stack = PS_(hash_to_bucket)(set,
				PS_(entry_hash)(entry)) != idx;
			/*printf("\tis_in_stack 0x%lx: %d\n", (unsigned long)idx, is_in_stack);*/
			PS_(move_to_top)(set, idx);
			next = already_in_stack ? SETm1 : set->top;
			assert(entry->next == SETm2
				&& (next == SETm1 || set->entries[next].next != SETm2));
		}
	}
	PS_(fill_entry)(entry, key, hash);
	entry->next = next;
	set->size = size;
	return 1;
}

/** Initialises `set` to idle. @order \Theta(1) @allow */
static void S_(set)(struct S_(set) *const set) { assert(set); set->entries = 0;
	set->log_capacity = 0; set->size = 0; set->top = 0; }

/** Destroys `set` and returns it to idle. @allow */
static void S_(set_)(struct S_(set) *const set)
	{ assert(set), free(set->entries); S_(set)(set); }

/** Clears and removes all entries from `set`. The capacity and memory of the
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

/** @return The value in `set` which <typedef:<PS>is_equal_fn> `SET_IS_EQUAL`
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
static int S_(set_buffer)(struct S_(set) *const hash, const size_t reserve)
	{ return hash ? reserve > (size_t)-1 - hash->size ? (errno = ERANGE, 0) :
	PS_(reserve)(hash, hash->size + reserve) : 0; }
#endif

/* fixme: Buffering changes the outcome if it's already in the table, it
 creates a new hash anyway. This is not a pleasant situation. */
/** Puts `key` in `hash`.
 @return Any ejected key or null.
 @throws[realloc, ERANGE] There was an error with a re-sizing. It is not
 always possible to tell the difference between an error and a unique key.
 If needed, before calling this, successfully calling <fn:<S>set_buffer>, or
 setting `errno` to zero. @order Average amortised \O(1), (hash distributes
 keys uniformly); worst \O(n) (are you sure that's up to date?). @allow */
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
 Successfully calling <fn:<S>set_buffer> ensures that this does not happen.
 @order Average amortised \O(1), (hash distributes keys uniformly); worst \O(n).
 @allow */
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
#define TSZ_(n) SET_CAT(set_sz, SZ_(n))
/* Check that `SET_TO_STRING` is a function implementing this prototype. */
static void (*const TSZ_(actual_to_string))(const PS_(ctype),
	char (*const)[12]) = (SET_TO_STRING);
/** This is to line up the set, which can have <typedef:<PS>type> a pointer or
 not, with to string, which requires a pointer. Call
 <data:<TSZ>actual_to_string> with a dereference on `indirect` and `z`. */
static void TSZ_(thunk_to_string)(const PS_(ctype) *const indirect,
	char (*const z)[12]) { TSZ_(actual_to_string)(*indirect, z); }
#define TO_STRING &TSZ_(thunk_to_string)
#define TO_STRING_LEFT '{'
#define TO_STRING_RIGHT '}'
#include "to_string.h" /** \include */
#ifdef SET_TEST /* <!-- expect: greedy satisfy forward-declared. */
#undef SET_TEST
static PSZ_(to_string_fn) PS_(to_string) = PSZ_(to_string);
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
