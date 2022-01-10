/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Hash table

 ![Example of <string>hash.](../web/hash.png)

 <tag:<M>hash> is an unordered set or map (associative array) of
 <typedef:<M>entry> implemented as a compact hash table. It must be
 supplied a hash and equality function.

 [CMPH](http://cmph.sourceforge.net/) is a minimal perfect hashing library
 that provides performance for large sets. Compile-time constant sets are
 can be better handled with [gperf](https://www.gnu.org/software/gperf/).

 @param[HASH_NAME, HASH_KEY]
 `<M>` that satisfies `C` naming conventions when mangled and a valid
 <typedef:<PM>key> associated therewith; required. `<PM>` is private, whose
 names are prefixed in a manner to avoid collisions; any should be re-defined
 prior to use elsewhere.

 @param[HASH_VALUE]
 An optional type that is the payload of the key, thus making this an
 associative array. Should be used when a map is desired and the key and value
 are associated but in independent memory locations; if the key is part of an
 aggregate value, it will be more efficient and more robust to use a type
 conversion from the key.

 @param[HASH_CODE, HASH_IS_EQUAL]
 A function satisfying <typedef:<PM>code_fn> and <typedef:<PM>is_equal_fn>;
 required.

 @param[HASH_UINT]
 This is <typedef:<PM>uint>, the unsigned type of hash code of the key given by
 <typedef:<PM>code_fn>; defaults to `size_t`.

 @param[HASH_NO_CACHE]
 Calculate every time; this avoids storing <typedef:<PM>uint> _per_ entry, but
 can be slower when the hash code is non-trivial to compute.

 @param[HASH_INVERSE]
 Function satisfying <typedef:<PM>inverse_code_fn> that avoids storing the key,
 but calculates it from the hashed value. As such, incompatible with
 `HASH_NO_CACHE`.

 @param[HASH_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[HASH_TO_STRING]
 To string trait contained in <to_string.h>; `<SZ>` that satisfies `C` naming
 conventions when mangled and function implementing
 <typedef:<PSZ>to_string_fn>. There can be multiple to string traits, but only
 one can omit `HASH_TO_STRING_NAME`.

 @std C89 */
 /* Bug in cdoc. @fixme Implement move-to-front like splay-trees.
 @fixme Implement HASH_NO_CACHE and HASH_INVERSE. */

#if !defined(HASH_NAME) || !defined(HASH_KEY) || !defined(HASH_IS_EQUAL) \
	|| !defined(HASH_CODE)
#error Name HASH_NAME, tag type HASH_KEY, fns HASH_IS_EQUAL, or HASH_CODE, undefined.
#endif
#if defined(HASH_TO_STRING_NAME) || defined(HASH_TO_STRING)
#define HASH_TO_STRING_TRAIT 1
#else
#define HASH_TO_STRING_TRAIT 0
#endif
#define HASH_TRAITS HASH_TO_STRING_TRAIT
#if HASH_TRAITS > 1
#error Only one trait per include is allowed; use HASH_EXPECT_TRAIT.
#endif
#if defined(HASH_NO_CACHE) && defined(HASH_INVERSE)
#error HASH_INVERSE has to store the hash code; conflicts with HASH_NO_CACHE.
#endif
#if defined(HASH_TO_STRING_NAME) && !defined(HASH_TO_STRING)
#error HASH_TO_STRING_NAME requires HASH_TO_STRING.
#endif

#ifndef HASH_H /* <!-- idempotent */
#define HASH_H
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#if defined(HASH_CAT_) || defined(HASH_CAT) || defined(M_) || defined(PM_) \
	|| defined(HASH_IDLE)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define HASH_CAT_(n, m) n ## _ ## m
#define HASH_CAT(n, m) HASH_CAT_(n, m)
#define M_(n) HASH_CAT(HASH_NAME, n) /* H is taken by heap; M map. */
#define PM_(n) HASH_CAT(hash, M_(n))
#define HASH_IDLE { 0, 0, 0, 0, 0 }
/* When a <typedef:<PM>uint> represents an address in the table, use the sign
 bit to store out-of-band flags, (such that range of an index is one bit less.)
 Choose representations that probably save power, since there are lots. We
 cannot save this in an `enum` because don't know maximum. */
#define SETm1 ((PM_(uint))~(PM_(uint))0) /* 2's compliment -1. */
#define SETmax (SETm1 >> 1) /* Sign bit excluded. */
#define SETlimit (SETmax + 1) /* Cardinality. */
#define SETend (SETlimit)
#define SETnull (SETlimit + 1)
#endif /* idempotent --> */


#if HASH_TRAITS == 0 /* <!-- base code */


#ifndef HASH_UINT
#define HASH_UINT size_t
#endif

/** Unsigned integer hash code; <typedef:<PM>code_fn> returns this type. Also
 places a simplifying limit on the maximum number of items in this hash table
 of half the cardinality of this type. */
typedef HASH_UINT PM_(uint);

/** Valid tag type defined by `HASH_KEY`. This code makes the simplifying
 assumption that this is not `const`-qualified. */
typedef HASH_KEY PM_(key);
/** Used on read-only. If one sees a duplicate `const` warning, consider
 X-macros defining an `enum` instead. */
typedef const HASH_KEY PM_(ctype);

/** A map from <typedef:<PM>ctype> onto <typedef:<PM>uint>. Usually should use
 all the the argument and output should be as close as possible to a discrete
 uniform distribution. It is up to the user to provide an appropriate code
 function. */
typedef PM_(uint) (*PM_(code_fn))(PM_(ctype));
/* Check that `HASH_CODE` is a function implementing <typedef:<PM>code_fn>. */
static const PM_(code_fn) PM_(code) = (HASH_CODE);

#ifdef HASH_INVERSE /* <!-- inv */
/** Defining `HASH_INVERSE` says that the <typedef:<PM>key> forms a bijection
 with <typedef:<PM>uint>; this is inverse-mapping to <typedef:<PM>code_fn>.
 Used to avoid having to store the <typedef:<PM>key>. */
typedef PM_(key) (*PM_(inverse_code_fn))(PM_(uint));
#endif /* inv --> */

/*Type is copied extensively,
 so if it's large, making it a pointer may improve performance.*/

/** Equivalence relation between <typedef:<PM>ctype> that satisfies
 `<PM>is_equal_fn(a, b) -> <PM>HASH_CODE(a) == <PM>HASH_CODE(b)`. */
typedef int (*PM_(is_equal_fn))(PM_(ctype) a, PM_(ctype) b);
/* Check that `HASH_IS_EQUAL` is a function implementing
 <typedef:<PM>is_equal_fn>. */
static const PM_(is_equal_fn) PM_(equal) = (HASH_IS_EQUAL);

#ifdef HASH_VALUE /* <!-- value */
/** Defining `HASH_VALUE` creates another entry for associative maps. */
typedef HASH_VALUE PM_(value);
#endif /* value --> */

/** Buckets are linked-lists of entries, and entries are stored in a code
 table. When a collision occurs, we push the entry out to an unoccupied stack
 in the same table. */
struct PM_(entry) {
	PM_(uint) next; /* `SETnull`, `SETend`, accepted, half the size. */
#ifndef HASH_NO_CACHE /* <!-- cache */
	PM_(uint) code;
#endif /* cache --> */
#ifndef HASH_INVERSE /* <!-- !inv */
	PM_(key) key;
#endif /* !inv --> */
#ifdef HASH_VALUE
	/*...place it with the key in a separate <tag:P_(entry)>. */
#endif
};

/** Fill `entry` with `key` and `code`. The entry must be empty. */
static void PM_(fill_entry)(struct PM_(entry) *const entry,
	const PM_(key) key, const PM_(uint) code) {
	assert(entry && entry->next == SETnull);
#ifndef HASH_NO_CACHE /* <!-- cache */
	entry->code = code;
#else /* cache --><!-- !cache */
	(void)code;
#endif /* !cache --> */
#ifndef HASH_INVERSE /* <!-- !inv */
	entry->key = key;
#else /* !inv --><!-- inv */
	(void)key;
#endif /* inv --> */
	entry->next = SETend;
}

/** Gets the code of an occupied `entry`, which should be consistent. */
static PM_(uint) PM_(entry_code)(const struct PM_(entry) *const entry) {
	assert(entry && entry->next != SETnull);
#ifdef HASH_NO_CACHE
	return PM_(code)(entry->key);
#else
	return entry->code;
#endif
}

/** Gets the key of an occupied `entry`. */
static PM_(key) PM_(entry_key)(const struct PM_(entry) *const entry) {
	assert(entry && entry->next != SETnull);
#ifdef HASH_INVERSE
	return PM_(inverse_code_fn)(&entry->code);
#else
	return entry->key;
#endif
}

/** To initialize, see <fn:<M>hash>, `HASH_IDLE`, `{0}` (`C99`,) or being
 `static`.

 ![States.](../web/states.png) */
struct M_(hash) {
	struct PM_(entry) *entries; /* @ has zero/one key specified by `next`. */
	unsigned log_capacity, unused; /* Applies to entries. */
	PM_(uint) size, top; /* size <= capacity; collided stack, `SETend` used. */
};

/** The capacity of a non-idle `hash` is always a power-of-two. */
static PM_(uint) PM_(capacity)(const struct M_(hash) *const hash)
	{ return assert(hash && hash->entries && hash->log_capacity >= 3),
	(PM_(uint))((PM_(uint))1 << hash->log_capacity); }

/** @return Indexes the first `hash.entries` in the bucket (a closed entry) from
 non-idle `hash` given the `code`. */
static PM_(uint) PM_(code_to_bucket)(const struct M_(hash) *const hash,
	const PM_(uint) code) { return code & (PM_(capacity)(hash) - 1); }

/** This is amortized; every value takes at most one top. On return, the `top`
 of `hash` will be empty. */
static void PM_(grow_stack)(struct M_(hash) *const hash) {
	PM_(uint) top = hash->top;
	assert(hash && hash->entries && top);
	top = (top == SETend ? PM_(capacity)(hash) : top) - 1;
	while(hash->entries[top].next != SETnull) assert(top), top--;
	hash->top = top;
}

/** Is `idx` is `hash` possibly on the stack? (Got tired of changing every time
 I wanted to change the direction.) */
static int PM_(in_stack_range)(const struct M_(hash) *const hash,
	const PM_(uint) idx)
	{ return assert(hash), hash->top != SETend && hash->top <= idx; }

/***********fixme*/
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)
#ifdef HASH_TEST
static void PM_(graph)(const struct M_(hash) *const hash, const char *const fn);
static void (*PM_(to_string))(PM_(ctype), char (*)[12]);
#else
static void PM_(graph)(const struct M_(hash) *const hash, const char *const fn) {
	(void)hash, (void)fn;
}
static void PM_(to_string)(PM_(ctype) data, char (*z)[12])
	{ (void)data, strcpy(*z, "<key>"); }
#endif

/** Moves the index `victim` to the top of the collision stack in non-idle
 `hash`. This is an inconsistent state; one is responsible for filling that hole
 and linking it with top. */
static void PM_(move_to_top)(struct M_(hash) *const hash,
	const PM_(uint) victim) {
	struct PM_(entry) *top, *vic;
	PM_(uint) to_next, next;
	char z[12];
	const PM_(uint) capacity = PM_(capacity)(hash);
	assert(hash->size < capacity && victim < capacity);
	PM_(grow_stack)(hash);
	vic = hash->entries + victim, top = hash->entries + hash->top;
	assert(vic->next != SETnull && top->next == SETnull); /* Occupied to vacant. */
	PM_(to_string)(PM_(entry_key)(vic), &z);
	printf("move_to_top: victim \"%s\" moved from 0x%lx to top 0x%lx\n",
		z, (unsigned long)victim, (unsigned long)hash->top);
	/* Search for the previous link in the linked-list. \O(|bucket|). */
	for(to_next = SETnull, next = PM_(code_to_bucket)(hash, PM_(entry_code)(vic));
		assert(next < capacity), PM_(to_string)(hash->entries[next].key, &z), printf("searching for victim in bucket: \"%s\" 0x%lx\n", z, (unsigned long)next), next != victim;
		to_next = next, next = hash->entries[next].next);
	printf("got \"%s\"\n", z);
	/* Move `vic` to `top`. */
	if(to_next != SETnull) hash->entries[to_next].next = hash->top;
	memcpy(top, vic, sizeof *vic), vic->next = SETnull;
}

/** `hash` will be searched linearly for `key` which has `code`.
 @fixme Fix for inverse. @fixme Move to front like splay trees? */
static struct PM_(entry) *PM_(get)(struct M_(hash) *const hash,
	const PM_(key) key, const PM_(uint) code) {
	struct PM_(entry) *entry;
	PM_(uint) idx, next;
	assert(hash && hash->entries && hash->log_capacity);
	entry = hash->entries + (idx = PM_(code_to_bucket)(hash, code));
	/* Not the start of a bucket: empty or in the collision stack. */
	if((next = entry->next) == SETnull
		|| PM_(in_stack_range)(hash, idx)
		&& idx != PM_(code_to_bucket)(hash, PM_(entry_code)(entry))) return 0;
	for( ; ; ) {
#ifdef HASH_NO_CACHE /* <!-- !cache: always go to next predicate. */
		const int codees_are_equal = ((void)(code), 1);
#else /* !cache --><!-- cache: quick out. */
		const int codees_are_equal = code == entry->code;
#endif /* cache --> */
		if(codees_are_equal && PM_(equal)(key, entry->key)) return entry;
		if(next == SETend) return 0; /* -1 used to end the bucket. */
		idx = next;
		assert(idx < PM_(capacity)(hash) && PM_(in_stack_range)(hash, idx));
		entry = hash->entries + idx;
		next = entry->next;
		assert(next != SETnull); /* -2 null: linked-list integrity. */
	}
}

/** Ensures that `hash` has enough entries to fill `n` more than the size.
 May invalidate `entries` and re-arrange the order.
 @return Success; otherwise, `errno` will be hash. @throws[realloc]
 @throws[ERANGE] Tried allocating more then can fit in half <typedef:<PM>uint>
 or `realloc` doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html). */
static int PM_(buffer)(struct M_(hash) *const hash, const PM_(uint) n) {
	struct PM_(entry) *entries;
	const unsigned log_c0 = hash->log_capacity;
	unsigned log_c1;
	/* fixme: this will have to be updated because it relies on -1. */
	const PM_(uint) c0 = log_c0 ? (PM_(uint))((PM_(uint))1 << log_c0) : 0;
	PM_(uint) c1, size1, i, wait, mask;
	char fn[64];
	assert(hash && hash->size <= SETlimit
		&& (!hash->entries && !hash->size && !log_c0 && !c0
		|| hash->entries && hash->size <= c0 && log_c0 >= 3));
	printf("buffer: limit %lu, entries %lu/%lu, new %lu\n",
		(unsigned long)SETlimit,
		(unsigned long)hash->size, (unsigned long)c0, (unsigned long)n);

	/* Can we satisfy `n` growth from the buffer? */
	if(SETm1 - hash->size < n || SETlimit < (size1 = hash->size + n))
		return errno = ERANGE, 0;
	if(hash->entries) log_c1 = log_c0, c1 = c0 ? c0 : 1;
	else             log_c1 = 3,      c1 = 8;
	while(c1 < size1) log_c1++, c1 <<= 1;
	if(log_c0 == log_c1) return 1;

	sprintf(fn, "graph/" QUOTE(HASH_NAME) "-resize-%u-%u-a-before.gv",
		log_c0, log_c1), PM_(graph)(hash, fn);

	/* Otherwise, need to allocate more. */
	printf("buffer: %lu -> %lu to satisfy %lu.\n",
		(unsigned long)c0, (unsigned long)c1, (unsigned long)size1);
	if(!(entries = realloc(hash->entries, sizeof *entries * c1)))
		{ if(!errno) errno = ERANGE; return 0; }
	hash->top = SETend; /* Idle `top` initialized, or rehash. */
	hash->entries = entries, hash->log_capacity = log_c1;

	/* Initialize new values. Rehash the stack. Mask off the added bits. */
	{ struct PM_(entry) *e = entries + c0, *const e_end = entries + c1;
		for( ; e < e_end; e++) e->next = SETnull; }
	mask = (PM_(uint))((((PM_(uint))1 << log_c0) - 1)
		^ (((PM_(uint))1 << log_c1) - 1));

	/* Recode most closed entries in the lower half. */
	printf("buffer: recode %lu entries; mask 0x%lx.\n",
		(unsigned long)c0, (unsigned long)mask);
	wait = SETend;
	for(i = 0; i < c0; i++) {
		struct PM_(entry) *idx, *go;
		PM_(uint) g, code;
		idx = hash->entries + i;
		printf("A.\t0x%lx: ", (unsigned long)i);
		/* Empty; don't have to do anything. */
		if(idx->next == SETnull) { printf("empty.\n"); continue; }
		/* Where it is closed. */
		g = PM_(code_to_bucket)(hash, code = PM_(entry_code)(idx));
		{
			char z[12];
			PM_(to_string)(PM_(entry_key)(idx), &z);
			printf("\"%s\"->0x%lx ", z, (unsigned long)g);
		}
		/* Like consistent codeing, because it's a power-of-two size,
		 `E[old/new]` capacity that a closed entry will remain where it is. */
		if(i == g) { idx->next = SETend; printf("chill.\n"); continue; }
		if((go = hash->entries + g)->next == SETnull) {
			/* Priority is given to the closed head entries; simpler later. */
			struct PM_(entry) *head;
			PM_(uint) h = g & ~mask; assert(h <= g);
			if(h < g && i < h /* Lookahead to the first entry in the bucket. */
				&& (head = hash->entries + h, assert(head->next != SETnull),
				g == PM_(code_to_bucket)(hash, PM_(entry_code)(head)))) {
				char y[12];
				PM_(to_string)(head->key, &y);
				printf("future 0x%lx \"%s\"->0x%lx will go instead, ",
					(unsigned long)h, y, (unsigned long)g);
				memcpy(go, head, sizeof *head);
				go->next = SETend, head->next = SETnull;
				/* Fall-though -- the entry still needs to be put on waiting. */
			} else {
				/* If the new entry is available and this entry is first. */
				memcpy(go, idx, sizeof *idx);
				go->next = SETend, idx->next = SETnull;
				printf("to vacant.\n");
				continue;
			}
		}
		printf("wait.\n");
		idx->next = wait, wait = i; /* Push for next sweep. */
	}

	/*{
		PM_(uint) w = wait;
		printf("waiting: { ");
		while(w != SETend) {
			printf("0x%lx ", (unsigned long)w);
			w = hash->entries[w].next;
		}
		printf("} checking for stragglers.\n");
	}*/
	sprintf(fn, "graph/" QUOTE(HASH_NAME) "-resize-%u-%u-b-obvious.gv",
		log_c0, log_c1), PM_(graph)(hash, fn);

	/* Search waiting stack for rest of the closed that moved concurrently. */
	{ PM_(uint) prev = SETend, w = wait; while(w != SETend) {
		char z[12];
		struct PM_(entry) *waiting = hash->entries + w;
		PM_(uint) cl = PM_(code_to_bucket)(hash, PM_(entry_code)(waiting));
		struct PM_(entry) *const closed = hash->entries + cl;
		assert(cl != w);
		{
			PM_(to_string)(PM_(entry_key)(waiting), &z);
			printf("B.\t0x%lx: \"%s\"->%lx ",
				(unsigned long)w, z, (unsigned long)cl);
		}
		if(closed->next == SETnull) {
			memcpy(closed, waiting, sizeof *waiting), closed->next = SETend;
			printf("to vacant.\n");
			if(prev != SETend) hash->entries[prev].next = waiting->next;
			if(wait == w) wait = waiting->next; /* First, modify head. */
			w = waiting->next, waiting->next = SETnull;
		} else {
			/* Not in the wait stack. */
			assert(closed->next == SETend);
			printf("wait.\n");
			prev = w, w = waiting->next;
		}
	}}

	/*{
		PM_(uint) w = wait;
		printf("waiting: { ");
		while(w != SETend) {
			printf("0x%lx ", (unsigned long)w);
			w = hash->entries[w].next;
		}
		printf("} moving to stack.\n");
	}*/
	sprintf(fn, "graph/" QUOTE(HASH_NAME) "-resize-%u-%u-c-closed.gv",
		log_c0, log_c1), PM_(graph)(hash, fn);

	/* Rebuild the (smaller?) top stack (high) from the waiting (low). */
	while(wait != SETend) {
		char z[12];
		struct PM_(entry) *const waiting = hash->entries + wait;
		PM_(uint) h = PM_(code_to_bucket)(hash, PM_(entry_code)(waiting));
		struct PM_(entry) *const head = hash->entries + h;
		struct PM_(entry) *top;
		assert(h != wait && head->next != SETnull);
		PM_(grow_stack)(hash), top = hash->entries + hash->top;
		{
			PM_(to_string)(PM_(entry_key)(waiting), &z);
			printf("\t0x%lx: \"%s\" to stack 0x%lx.\n",
				(unsigned long)wait, z, (unsigned long)hash->top);
		}
		memcpy(top, waiting, sizeof *waiting);
		top->next = head->next, head->next = hash->top;
		wait = waiting->next, waiting->next = SETnull; /* Pop. */
	}

	sprintf(fn, "graph/" QUOTE(HASH_NAME) "-resize-%u-%u-d-final.gv",
		log_c0, log_c1), PM_(graph)(hash, fn);

	/*{ PM_(uint) j;
	printf("buffer::recode: final top 0x%lx\n", (long)hash->top);
	for(j = 0; j < PM_(capacity)(hash); j++) {
		struct PM_(entry) *je = hash->entries + j;
		char z[12];
		printf("\t0x%lx: ", (unsigned long)j);
		if(je->next == SETnull) { printf("--\n"); continue; }
		PM_(to_string)(PM_(entry_key)(je), &z);
		printf("\"%s\"", z);
		if(je->next == SETend) { printf("\n"); continue; }
		printf(" -> 0x%lx\n", (unsigned long)je->next);
	}}*/
	return 1;
}

#undef QUOTE_
#undef QUOTE

/** A bi-predicate; returns true if the `replace` replaces the `original`. */
typedef int (*PM_(replace_fn))(PM_(key) original, PM_(key) replace);

/** Used in <fn:<M>hash_policy_put> when `replace` is null; `original` and
 `replace` are ignored. @implements `<PM>replace_fn` */
static int PM_(false)(PM_(key) original, PM_(key) replace)
	{ (void)(original); (void)(replace); return 0; }

/** Put `key` in `hash` as long as `replace` is null or returns true.
 @param[eject] If non-null, the equal element, if any. If `replace`
 returns false, the address of `key`.
 @return Success. @throws[malloc] @order amortized \O(1) */
static int PM_(put)(struct M_(hash) *const hash, const PM_(replace_fn) replace,
	PM_(key) key, PM_(key) *eject) {
	struct PM_(entry) *entry;
	PM_(uint) code, idx, next = SETend /* The end of a linked-list. */, size;
	char z[12];
	assert(hash /*&& key Null keys possible. */);
	PM_(to_string)(key, &z);
	if(eject) *eject = 0;
	code = PM_(code)(key);
	size = hash->size;
	printf("put: \"%s\" code 0x%lx.\n", z, (unsigned long)code);
	if(hash->entries && (entry = PM_(get)(hash, key, code))) { /* Replace. */
		if(replace && !replace(PM_(entry_key)(entry), key))
			{ if(eject) *eject = key; return 1; } /* Decided not to replace. */
		if(eject) *eject = PM_(entry_key)(entry);
		/* Cut the tail and put new element in the head. */
		next = entry->next, entry->next = SETnull, assert(next != SETnull);
	} else { /* Expand. */
		if(!PM_(buffer)(hash, 1)) return 0; /* Amortized. */
		entry = hash->entries + (idx = PM_(code_to_bucket)(hash, code));
		/*printf("\tput expand: \"%s\" index 0x%lx from code 0x%lx\n",
			z, (unsigned long)idx, (unsigned long)code);*/
		size++;
		if(entry->next != SETnull) { /* Unoccupied. */
			int already_in_stack = PM_(code_to_bucket)(hash,
				PM_(entry_code)(entry)) != idx;
			/*printf("\tis_in_stack 0x%lx: %d\n", (unsigned long)idx, is_in_stack);*/
			PM_(move_to_top)(hash, idx);
			next = already_in_stack ? SETend : hash->top;
			assert(entry->next == SETnull
				&& (next == SETend || hash->entries[next].next != SETnull));
		}
	}
	PM_(fill_entry)(entry, key, code);
	entry->next = next;
	hash->size = size;
	return 1;
}

/** Initialises `hash` to idle. @order \Theta(1) @allow */
static void M_(hash)(struct M_(hash) *const hash) { assert(hash);
	hash->entries = 0; hash->log_capacity = 0; hash->size = 0; hash->top = 0; }

/** Destroys `hash` and returns it to idle. @allow */
static void M_(hash_)(struct M_(hash) *const hash)
	{ assert(hash), free(hash->entries); M_(hash)(hash); }

/** Clears and removes all entries from `hash`. The capacity and memory of the
 code table is preserved, but all previous values are un-associated. The load
 factor will be less until it reaches it's previous size.
 @order \Theta(`hash.entries`) @allow */
static void M_(hash_clear)(struct M_(hash) *const hash) {
	struct PM_(entry) *b, *b_end;
	assert(hash);
	if(!hash->entries) { assert(!hash->log_capacity); return; }
	for(b = hash->entries, b_end = b + (1 << hash->log_capacity); b < b_end; b++)
		b->next = SETnull;
	hash->size = 0;
}

/** @return The value in `hash` which <typedef:<PM>is_equal_fn> `HASH_IS_EQUAL`
 `key`, or, if no such value exists, null.
 @order Average \O(1), (code distributes elements uniformly); worst \O(n).
 @allow */
static PM_(key) M_(hash_get)(struct M_(hash) *const hash, const PM_(key) key) {
	struct PM_(entry) *b;
	assert(hash);
	if(!hash->entries) { assert(!hash->log_capacity); return 0; }
	b = PM_(get)(hash, key, PM_(code)(key));
	return b ? PM_(entry_key)(b) : 0;
}

#if 0
/** Reserve at least `reserve`, divided by the maximum load factor, space in
 the entries of `code`. @return Success.
 @throws[ERANGE] `reserve` plus the size would take a bigger number then could
 fit in a `size_t`. @throws[realloc] @allow */
static int M_(hash_buffer)(struct M_(hash) *const code, const size_t reserve)
	{ return code ? reserve > (size_t)-1 - code->size ? (errno = ERANGE, 0) :
	PM_(reserve)(code, code->size + reserve) : 0; }
#endif

/* fixme: Buffering changes the outcome if it's already in the table, it
 creates a new code anyway. This is not a pleasant situation. */
/** Puts `key` in `code`, and, for keys already in the code, replaces them.
 @return Any ejected key or null.
 @throws[realloc, ERANGE] There was an error with a re-sizing. It is not
 always possible to tell the difference between an error and a unique key.
 If needed, before calling this, successfully calling <fn:<M>hash_buffer>, or
 hashting `errno` to zero. @order Average amortised \O(1), (code distributes
 keys uniformly); worst \O(n) (are you sure that's up to date?). @allow */
static PM_(key) M_(hash_replace)(struct M_(hash) *const code,
	const PM_(key) key) {
	PM_(key) collide;
	/* No error information. */
	return PM_(put)(code, 0, key, &collide) ? collide : 0;
}

/** Puts `key` in `code` only if the entry is absent or if calling `replace`
 returns true.
 @param[replace] If null, doesn't do any replacement on collision.
 @return Any ejected element or null. On collision, if `replace` returns false
 or `replace` is null, returns `key` and leaves the other element in the code.
 @throws[realloc, ERANGE] There was an error with a re-sizing.
 Successfully calling <fn:<M>hash_buffer> ensures that this does not happen.
 @order Average amortised \O(1), (code distributes keys uniformly); worst \O(n).
 @allow */
static PM_(key) M_(hash_policy_put)(struct M_(hash) *const code,
	const PM_(key) key, const PM_(replace_fn) replace) {
	PM_(key) collide;
	/* No error information. */
	return PM_(put)(code, replace ? replace : &PM_(false), key, &collide)
		? collide : 0;
}

#if 0
/** Removes an element `data` from `code`.
 @return Successfully ejected element or null. @order Average \O(1), (code
 distributes elements uniformly); worst \O(n). @allow */
static struct M_(hashlink) *M_(hash_remove)(struct M_(hash) *const code,
	const PM_(mtype) data) {
	PM_(uint) code;
	struct M_(hashlink) **to_x, *x;
	if(!code || !code->entries) return 0;
	code = PM_(hash)(data);
	if(!(to_x = PM_(entry_to)(PM_(get_entry)(code, code), code, data)))
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
struct PM_(iterator) { const struct M_(hash) *hash; PM_(uint) idx; };

/** Loads `hash` (can be null) into `it`. @implements begin */
static void PM_(begin)(struct PM_(iterator) *const it,
	const struct M_(hash) *const hash)
	{ assert(it), it->hash = hash, it->idx = 0; }

/** Helper to skip the entries of `it` that are not there.
 @return Whether it found another index. */
static int PM_(skip)(struct PM_(iterator) *const it) {
	const struct M_(hash) *const hash = it->hash;
	const PM_(uint) limit = PM_(capacity)(hash);
	assert(it && it->hash && it->hash->entries);
	while(it->idx < limit) {
		struct PM_(entry) *const e = hash->entries + it->idx;
		if(e->next != SETnull) return 1;
		it->idx++;
	}
	return 0;
}

/** Advances `it`. @implements next */
static struct PM_(entry) *PM_(next)(struct PM_(iterator) *const it) {
	assert(it);
	if(!it->hash || !it->hash->entries) return 0;
	if(PM_(skip)(it)) return it->hash->entries + it->idx++;
	it->hash = 0, it->idx = 0;
	return 0;
}

/* iterate --> */

/** Iteration usually not in any particular order, as it goes by entry. The
 asymptotic runtime is proportional to the hash capacity. (This will be the
 power-of-two equal-to or greater then the maximum size plus buffering of the
 history of the hash.) */
struct M_(hash_iterator) { struct PM_(iterator) it; };

/** Loads `hash` (can be null) into `it`. */
static void M_(hash_begin)(struct M_(hash_iterator) *const it,
	const struct M_(hash) *const hash) { PM_(begin)(&it->it, hash); }

/** @return Whether the hash specified by <fn:<M>hash_begin> has a next entry. */
static int M_(hash_has_next)(struct M_(hash_iterator) *const it) {
	assert(it);
	/* <tag:<PM>entry> is fine for private returning, but <typedef:<PM>key>
	 may not even be nullable. */
	return it->it.hash && it->it.hash->entries && PM_(skip)(&it->it);
}

/** Advances `it`. @return The next key or zero. */
static PM_(key) M_(hash_next_key)(struct M_(hash_iterator) *const it) {
	const struct PM_(entry) *e = PM_(next)(&it->it);
	return e ? PM_(entry_key)(e) : 0;
}

/* fixme: and value, if VALUE? */

/* <!-- box (multiple traits) */
#define BOX_ PM_
#define BOX_CONTAINER struct M_(hash)
#define BOX_CONTENTS struct PM_(entry)

#ifdef HASH_TEST /* <!-- test */
/* Forward-declare. */
static void (*PM_(to_string))(PM_(ctype), char (*)[12]);
static const char *(*PM_(hash_to_string))(const struct M_(hash) *);
#include "../test/test_hash.h"
#endif /* test --> */

static void PM_(unused_base_coda)(void);
static void PM_(unused_base)(void) {
	M_(hash)(0); M_(hash_)(0); M_(hash_clear)(0); M_(hash_get)(0, 0);
	/*M_(hash_reserve)(0, 0);*/ M_(hash_replace)(0, 0);  M_(hash_policy_put)(0, 0, 0);
	/*M_(hash_remove)(0, 0);*/
	M_(hash_begin)(0, 0); M_(hash_has_next)(0); M_(hash_next_key)(0);
	PM_(unused_base_coda)();
}
static void PM_(unused_base_coda)(void) { PM_(unused_base)(); }


#elif defined(HASH_TO_STRING) /* base code --><!-- to string trait */


#ifdef HASH_TO_STRING_NAME
#define SZ_(n) HASH_CAT(M_(hash), HASH_CAT(HASH_TO_STRING_NAME, n))
#else
#define SZ_(n) HASH_CAT(M_(hash), n)
#endif
#define TSZ_(n) HASH_CAT(hash_sz, SZ_(n))
/* Check that `HASH_TO_STRING` is a function implementing this prototype. */
static void (*const TSZ_(actual_to_string))(PM_(ctype), char (*const)[12])
	= (HASH_TO_STRING);
/** This is to line up the hash, which can have <typedef:<PM>key> a pointer or
 not, with to string, which requires a pointer. Call
 <data:<TSZ>actual_to_string> with key of `entry` and `z`. */
static void TSZ_(thunk_to_string)(const struct PM_(entry) *const entry,
	char (*const z)[12]) { TSZ_(actual_to_string)(PM_(entry_key)(entry), z); }
#define TO_STRING &TSZ_(thunk_to_string)
#define TO_STRING_LEFT '{'
#define TO_STRING_RIGHT '}'
#include "to_string.h" /** \include */
#ifdef HASH_TEST /* <!-- expect: greedy satisfy forward-declared. */
#undef HASH_TEST
static void (*PM_(to_string))(PM_(ctype), char (*const)[12])
	= TSZ_(actual_to_string);
static const char *(*PM_(hash_to_string))(const struct M_(hash) *)
	= &SZ_(to_string);
#endif /* expect --> */
#undef TSZ_
#undef SZ_
#undef HASH_TO_STRING
#ifdef HASH_TO_STRING_NAME
#undef HASH_TO_STRING_NAME
#endif


#endif /* traits --> */


#ifdef HASH_EXPECT_TRAIT /* <!-- trait */
#undef HASH_EXPECT_TRAIT
#else /* trait --><!-- !trait */
#ifdef HASH_TEST
#error No HASH_TO_STRING traits defined for HASH_TEST.
#endif
#undef HASH_NAME
#undef HASH_KEY
#undef HASH_UINT
#undef HASH_CODE
#undef HASH_IS_EQUAL
#ifdef HASH_VALUE
#undef HASH_VALUE
#endif
#ifdef HASH_NO_CACHE
#undef HASH_NO_CACHE
#endif
#undef BOX_
#undef BOX_CONTAINER
#undef BOX_CONTENTS
/* box (multiple traits) --> */
#endif /* !trait --> */
#undef HASH_TO_STRING_TRAIT
#undef HASH_TRAITS
