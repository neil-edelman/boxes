/** 2018 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 {<KV>Map} is a hash map, (hash table, dictionary,) of {<KV>Entry},
 implementing an associative array. Storage of {<KV>Entry} structure, or
 structures with this in it, is the responsibility of the caller. Collisions
 are handled with linked-lists that provide separate chaining. The key is
 hashed to a {uint32_t} on entry into the map, and placed in a bucket
 implemented as a power-of-two dynamically allocated array of lists for easy
 rehashing. One should not the change the key in a way that affects the hash
 while inside the map. The maximum load factor is {ln 2} until the size reaches
 the limits of a 32-bit hash.

 The parameters are preprocessor macros and all are all undefined at the end of
 the file for convenience.

 @param MAP_NAME
 A unique (among {Map}) name associated with {<KV>} that satisfies {C} naming
 conventions when mangled; required.

 @param MAP_KEY
 This type becomes {<K>}, the key; required. Typically a pointer or elementary
 data type for speed; required.

 @param MAP_VALUE
 This type becomes {<V>}, the value associated with the key; required.

 @param MAP_KEY_IN_VALUE
 Use when the value, {<V>}, contains the key, {<K>}, within the data, instead
 of separate items for key and value. A function satisfying {<PKV>KeyInValue}.

 @param MAP_HASH
 A function satisfying {<PKV>Hash}; required.

 @param MAP_IS_EQUAL
 A function satisfying {<PKV>IsEqual}; required.

 @param MAP_TO_STRING
 Optional print function implementing {<PKV>ToString}; makes available
 \see{<KV>MapToString}.

 @param MAP_DEFAULT
 A {<KV>Entry} variable such that in \see{<KV>MapGet}, where it would return
 null, this is returned instead.

 @param MAP_STRICT_ANSI
 This doesn't include {C99} {<stdint.h>}; this file is not guaranteed to be in
 {C89}. When this is defined, one needs to specify a {typedef} for {uint32_t}
 that is 32-bits unsigned integer before including this file. (On old versions
 of {MSVC}, {typedef unsigned uint32_t}.)

 @param MAP_TEST
 Unit testing framework, included in a separate header, {../test/MapTest.h}.
 Must be defined equal to a (random) filler function, satisfying
 {<PKV>Action}. If {NDEBUG} is not defined, turns on {assert} private function
 integrity testing. Requires {MAP_TO_STRING}.

 @title   Map.h
 @author  Neil
 @std     C89/90 + C99 {stdint.h:uint32_t} if not {MAP_STRICT_ANSI}.
 @version 2019-03 Merging Map and Entry; changed memory allocation.
 @since   2018-03
 @fixme   {replace}, {merge}, {putAll}. */



#include <stddef.h>	/* offsetof */
#include <stdlib.h> /* realloc free */
#include <assert.h>	/* assert */
#include <stdio.h>  /* perror fprintf */
#include <errno.h>  /* errno */
#ifdef MAP_TO_STRING /* <-- string */
#include <string.h> /* strlen */
#endif /* string --> */
#ifndef MAP_STRICT_ANSI /* <-- !ansi */
#include <stdint.h> /* uint32_t uintptr_t (C99) */
#endif /* !ansi --> */



/* Check defines. */
#ifndef MAP_NAME
#error Map generic MAP_NAME undefined.
#endif
#ifndef MAP_KEY
#error Map type MAP_KEY undefined.
#endif
#ifndef MAP_VALUE
#error Map type MAP_VALUE undefined.
#endif
#ifndef MAP_IS_EQUAL
#error Map function MAP_IS_EQUAL undefined.
#endif
#ifndef MAP_HASH
#error Map function MAP_HASH undefined.
#endif
#if defined(MAP_TEST) && !defined(MAP_TO_STRING)
#error MAP_TEST requires MAP_TO_STRING.
#endif
#if !defined(MAP_TEST) && !defined(NDEBUG)
#define MAP_NDEBUG
#define NDEBUG
#endif



/* Generics using the preprocessor;
 \url{ http://stackoverflow.com/questions/16522341/pseudo-generics-in-c }. */
#ifdef CAT
#undef CAT
#endif
#ifdef CAT_
#undef CAT_
#endif
#ifdef PCAT
#undef PCAT
#endif
#ifdef PCAT_
#undef PCAT_
#endif
#ifdef K
#undef K
#endif
#ifdef V
#undef V
#endif
#ifdef KV_
#undef KV_
#endif
#ifdef PKV_
#undef PKV_
#endif
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#define KV_(thing) CAT(MAP_NAME, thing)
#define PKV_(thing) PCAT(map, PCAT(MAP_NAME, thing)) /* Private. */

/* Troubles with this line? check to ensure that {MAP_KEY} is a valid type,
 whose definition is placed above {#include "Map.h"}. Note that this works best
 if this is a simple data-type such as a pointer. */
typedef MAP_KEY PKV_(Key);
#define K PKV_(Key)

/* Troubles with this line? check to ensure that {MAP_VALUE} is a valid type,
 whose definition is placed above {#include "Map.h"}. */
typedef MAP_VALUE PKV_(Value);
#define V PKV_(Value)



/* A singly-linked list. */
struct PKV_(X) { struct PKV_(X) *next; };

/** An entry. Storage of the {<KV>Entry} structure is the responsibility of the
 caller. */
struct KV_(Entry);
struct KV_(Entry) {
#ifndef MAP_KEY_IN_VALUE /* <-- !kiv */
	K key;
#endif /* !kiv --> */
	V value;
	uint32_t hash; /* {uint32_t} is hard-coded. */
	struct PKV_(X) x;
};

/** A {Map} or dictionary, see \see{<KV>Map}. */
struct KV_(Map);
struct KV_(Map) {
	struct PKV_(X) *buckets; /* An array of 1 << log_capacity (>3) or 0. */
	unsigned log_capacity;
	size_t size;
};



#ifdef MAP_KEY_IN_VALUE /* <-- kiv */
/** Returns {<K> \in <V>} if {MAP_KEY_IN_VALUE}. */
typedef K (*PKV_(KeyInValue))(const V *const);
/* Check that {MAP_KEY_IN_VALUE} is a function implementing {<PKV>Key}. */
static const PKV_(KeyInValue) PKV_(key_in_value) = (MAP_KEY_IN_VALUE);
#endif /* kiv --> */



/* Constants across multiple includes in the same translation unit. Before
 definitions so you can this as {MAP_HASH} the first time. */
#ifndef MAP_H /* <-- MAP_H */
#define MAP_H
/** Perform a 32 bit Fowler/Noll/Vo FNV-1a hash on a string,
 \url{http://www.isthe.com/chongo/tech/comp/fnv/}. Provided for convenience
 when using constant string keys, {eg}, {#define MAP_KEY const char *},
 {#define MAP_HASH &map_fnv_32a_str}.
 @allow */
static uint32_t map_fnv_32a_str(const char *str) {
	const unsigned char *s = (const unsigned char *)str;
	/* 32 bit FNV-1 and FNV-1a non-zero initial basis, FNV1_32A_INIT */
	uint32_t hval = (uint32_t)0x811c9dc5;
	/* Take multiples of 4 bytes; FNV magic prime {FNV_32_PRIME 0x01000193}. */
	while(*s) {
		hval ^= *s++;
		hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
	}
	return hval;
}
#endif /* MAP_H */



/** A map from {<K>} onto {uint32_t}; should be as close as possible to a
 discrete uniform distribution for maximum performance. */
typedef uint32_t (*PKV_(Hash))(const K);
/* Check that {MAP_HASH} is a function implementing {<PT>Hash}. */
static const PKV_(Hash) PKV_(hash) = (MAP_HASH);

/** A constant equivalence relation between {<K>} that satisfies
 {<PKV>IsEqual(a, b) -> <PKV>Hash(a) == <PKV>Hash(b)}. */
typedef int (*PKV_(IsEqual))(const K, const K);
/* Check that {MAP_IS_EQUAL} is a function implementing {<PKV>IsEqual}. */
static const PKV_(IsEqual) PKV_(is_equal) = (MAP_IS_EQUAL);

#ifdef MAP_TO_STRING /* <-- string */
/** Responsible for turning {<K>} (the first argument) into a 12 {char}
 null-terminated output string (the second.) */
typedef void (*PKV_(ToString))(const struct KV_(Entry) *const,
	char (*const)[12]);
/* Check that {MAP_TO_STRING} is a function implementing {<E>ToString}. */
static const PKV_(ToString) PKV_(to_string) = (MAP_TO_STRING);
#endif /* string --> */

#ifdef MAP_TEST /* <-- test */
/** Used for {MAP_TEST}. */
typedef void (*PKV_(Action))(struct KV_(Entry) *const);
#endif /* test --> */



/** <KV>Entry -> K; depends on {KAP_KEY_IN_VALUE}, so one should always use. */
static K PKV_(key)(const struct KV_(Entry) *const entry) {
	assert(entry);
#ifdef MAP_KEY_IN_VALUE /* <-- kiv */
	return PKV_(key_in_value)(&entry->value);
#else /* kiv --><-- !kiv */
	return entry->key;
#endif /* !kiv --> */
}

/** Private: {container_of}. */
static struct KV_(Entry) *PKV_(x_upcast)(struct PKV_(X) *const x) {
	return (struct KV_(Entry) *)(void *)
		((char *)x - offsetof(struct KV_(Entry), x));
}

/** @return Given a {hash}, compute the bucket at it's index. May be empty. */
static struct PKV_(X) *PKV_(get_bucket)(struct KV_(Map) *const map,
	const uint32_t hash) {
	assert(map && map->log_capacity >= 3 && map->log_capacity < 32);
	return map->buckets + (hash & ((1 << map->log_capacity) - 1));
}

/** Private: grow the table until the capacity is at least
 {size / load factor = ln 2 = 0.69}, ranged from (8, MAX_32) entries.
 @param size: How many entries are there going to be; {size > 1}.
 @return Success; otherwise, {errno} may be set.
 @throws ERANGE: Tried allocating more then can fit in {size_t}.
 @throws ENOMEM?: {IEEE Std 1003.1-2001}; C standard does not say.
 @order O(1) amortized */
static int PKV_(grow)(struct KV_(Map) *const map, const size_t size) {
	/* Effective, the load factor. */
	const size_t eff_size = 1 + size / 0.693147180559945309417232121458176568;
	struct PKV_(X) *buckets, *b, *b_end, *new_b, *prev_x, *x;
	const uint32_t log_c0 = map->log_capacity;
	uint32_t c0 = 1 << log_c0, log_c1, c1, mask;
	assert(map && size && map->log_capacity < 32
		&& (map->log_capacity >= 3 || !map->log_capacity));
	assert(sizeof(uint32_t) <= sizeof(size_t)); /* This is guaranteed? */
	/* Starting bucket number is a power of 2 in [8, 0x80000000]. */
	if(eff_size > 0x40000000 || eff_size < size) {
		log_c1 = 31;
		c1 = 0x80000000;
	} else {
		if(log_c0 < 3) log_c1 = 3,      c1 = 8;
		else           log_c1 = log_c0, c1 = c0;
		while(c1 < eff_size) log_c1++, c1 <<= 1;
	}
	/* It's under the critical load factor; don't need to do anything. */
	if(log_c0 == log_c1) return 1;
	/* Everything else is amortised. Allocate new space for an expansion. */
	if(!(buckets = realloc(map->buckets, sizeof *buckets * c1))) return 0;
	map->buckets = buckets;
	map->log_capacity = log_c1;
	/* The mask needs domain {c0 \in [1, max]}, but we want 0 for loops. */
	mask = (c1 - 1) ^ (c0 - 1), assert(mask);
	if(c0 == 1) c0 = 0, assert(!c0 || c0 >= 8);
	/* Initialize the new lists to contain no elements. */
	for(b = buckets + c0, b_end = buckets + c1; b < b_end; b++) b->next = 0;
	/* Rehash some entries into the new lists. */
	for(b = buckets, b_end = buckets + c0; b < b_end; b++) {
		struct KV_(Entry) *e;
		assert(!((b - buckets) & mask));
		/* Skip the keys that go nowhere. Rehash to the higher buckets. */
		prev_x = b;
		while((x = prev_x->next)) {
			e = PKV_(x_upcast)(x);
			if(!(e->hash & mask)) { prev_x = x; continue; }
			prev_x->next = x->next;
			new_b = PKV_(get_bucket)(map, e->hash);
			x->next = new_b->next, new_b->next = x;
		}
	}
	return 1;
}

/** Zeros {map}, a well-defined state. */
static void PKV_(map)(struct KV_(Map) *const map) {
	assert(map);
	map->buckets      = 0;
	map->log_capacity = 0;
	map->size         = 0;
}

/** @return The key, {K}. Note that one should not change the key in a way that
 affects hash while in the map.
 @allow */
static K KV_(EntryKey)(const struct KV_(Entry) *const entry) {
	if(!entry) return 0;
	return PKV_(key)(entry);
}

/** @return A pointer to the value, {V}.
 @allow */
static V *KV_(EntryValue)(struct KV_(Entry) *const entry) {
	if(!entry) return 0;
	return &entry->value;
}
			
/** @return A pointer to the value, {V}.
 @allow */
static const V *KV_(EntryConstValue)(const struct KV_(Entry) *const entry) {
	if(!entry) return 0;
	return &entry->value;
}

/** Destructor for {map}. After, {map} takes no memory.
 @allow */
static void KV_(Map_)(struct KV_(Map) *const map) {
	if(!map) return;
	free(map->buckets);
	PKV_(map)(map);
}

/** Initialises {map} to be empty and zero and take no memory. If it is
 {static} data, then it is initialised by default.
 @order \Theta(1)
 @allow */
static void KV_(Map)(struct KV_(Map) *const map) {
	if(!map) return;
	PKV_(map)(map);
}

/** Clears and removes all entries from {map}. The capacity and memory of the
 hash table is preserved, but all previous values are un-associated.
 @param map: If null, does nothing.
 @order \Theta({map.buckets})
 @allow */
static void KV_(MapClear)(struct KV_(Map) *const map) {
	struct PKV_(X) *b, *b_end;
	if(!map || !map->log_capacity) return;
	for(b = map->buckets, b_end = b + (1 << map->log_capacity); b < b_end; b++)
		b->next = 0;
	map->size = 0;
}

/** @return The number of entries in the map.
 @order \Theta(1) */
static size_t KV_(MapSize)(const struct KV_(Map) *const map) {
	if(!map) return 0;
	return map->size;
}

/** Gets {key} from {map}.
 @return The entry with the specified {key}. A key which is not in the {map},
 when {MAP_DEFAULT} has been specified, returns the default, if not, null.
 @order {O(1)}, (assuming the hash function is uniform); worst {O(n)}.
 @allow */
static struct KV_(Entry) *KV_(MapGet)(struct KV_(Map) *const map, K key) {
#ifdef MAP_DEFAULT /* <-- def */
	/* Troubles with this line? check to ensure that {MAP_DEFAULT} is a valid
	 variable, whose definition is placed above {#include "Map.h"}. */
	static struct KV_(Entry) *const PKV_(default) = MAP_DEFAULT;
#endif /* def --> */
	uint32_t hash;
	struct PKV_(X) *bucket, *x;
	struct KV_(Entry) *e;
	if(!map || !map->buckets)
#ifdef MAP_DEFAULT /* <-- def */
		return PKV_(default);
#else /* def --><-- !def */
		return 0;
#endif /* !def --> */
	hash   = PKV_(hash)(key);
	bucket = PKV_(get_bucket)(map, hash);
	for(x = bucket->next; x; x = x->next) {
		e = PKV_(x_upcast)(x);
		if(hash == e->hash && PKV_(is_equal)(PKV_(key)(e), key)) return e;
	}
#ifdef MAP_DEFAULT /* <-- def */
	return PKV_(default);
#else /* def --><-- !def */
	return 0;
#endif /* !def --> */
}

/** Puts the {entry} in {map}. Adding an element with the same {K}, according
 to {MAP_IS_EQUAL}, causes the old data to be ejected.
 @param map, entry: If null, returns false.
 @param entry: {entry} must not be part this {map} or any other.
 @param p_eject: If not-null, this address of a variable that will store the
 {<KV>Entry} that was replaced, if any. If null, does nothing.
 @return Success.
 @throws {realloc} errors.
 @order {O(1)}, (assuming the hash function is uniform); worst {O(n)}.
 @allow */
static int KV_(MapPut)(struct KV_(Map) *const map,
	struct KV_(Entry) *const entry, struct KV_(Entry) **const p_eject) {
	struct PKV_(X) *bucket;
	struct KV_(Entry) *eject = 0;
	uint32_t hash;
	K key;
	if(p_eject) *p_eject = 0;
	if(!map || !entry) return 0;
	/* Calculate and cache the hash value of the key. */
	key = PKV_(key)(entry);
	hash = entry->hash = PKV_(hash)(key);
	/* There will only be one entry {is_equal(entry)}; remove another. */
	if(map->buckets) {
		struct PKV_(X) *prev_x, *x;
		struct KV_(Entry) *e;
		bucket = PKV_(get_bucket)(map, entry->hash);
		for(prev_x = bucket; (x = prev_x->next); prev_x = x) {
			e = PKV_(x_upcast)(x);
			if(hash == e->hash && PKV_(is_equal)(PKV_(key)(e), key)) break;
		}
		if(x) {
			prev_x->next = x->next;
			if(x->next == x) x->next = 0;
			eject = e;
		}
	}
	/* If the entry is new, we grow. The bucket may have changed. */
	if(!eject) {
		if(!PKV_(grow)(map, map->size + 1)) return 0;
		bucket = PKV_(get_bucket)(map, entry->hash);
		map->size++;
	}
	/* Stick the entry on the head of the bucket. @fixme should order? */
	entry->x.next = bucket->next, bucket->next = &entry->x;
	/* Request replaced? */
	if(p_eject) *p_eject = eject;
	return 1;
}

/** Puts the {entry} in {map} only if the entry is absent.
 @param map, entry: If null, returns false.
 @param entry: {entry} must not be part this {map} or any other.
 @param p_is_absent: If not-null, it will signal the successful placement.
 @return Successful operation, including doing nothing because the entry is
 already in the map.
 @throws {realloc} errors.
 @order {O(1)}, (assuming the hash function is uniform); worst {O(n)}.
 @allow */
static int KV_(MapPutIfAbsent)(struct KV_(Map) *const map,
	struct KV_(Entry) *const entry, int *const p_is_absent) {
	struct PKV_(X) *bucket;
	uint32_t hash;
	K key;
	if(p_is_absent) *p_is_absent = 0;
	if(!map || !entry) return 0;
	key = PKV_(key)(entry);
	hash = entry->hash = PKV_(hash)(key);
	if(map->buckets) {
		struct PKV_(X) *prev_x, *x;
		struct KV_(Entry) *e;
		bucket = PKV_(get_bucket)(map, entry->hash);
		for(prev_x = bucket; (x = prev_x->next); prev_x = x) {
			e = PKV_(x_upcast)(x);
			if(hash == e->hash && PKV_(is_equal)(PKV_(key)(e), key)) return 1;
		}
	}
	if(!PKV_(grow)(map, map->size + 1)) return 0;
	bucket = PKV_(get_bucket)(map, entry->hash);
	entry->x.next = bucket->next, bucket->next = &entry->x;
	map->size++;
	if(p_is_absent) *p_is_absent = 1;
	return 1;
}

/** Removes an element specified by {key} from {map}.
 @return Successfully removed an element or null.
 @order {O(1)}, (assuming the hash function is uniform); worst {O(n)}.
 @allow */
static struct KV_(Entry) *KV_(MapRemove)(struct KV_(Map) *const map, K key) {
	uint32_t hash;
	struct PKV_(X) *bucket, *prev_x, *x;
	struct KV_(Entry) *e;
	if(!map || !map->size || !map->buckets) return 0;
	hash   = PKV_(hash)(key);
	bucket = PKV_(get_bucket)(map, hash);
	for(prev_x = bucket; (x = prev_x->next); prev_x = x) {
		e = PKV_(x_upcast)(x);
		if(hash == e->hash && PKV_(is_equal)(PKV_(key)(e), key)) break;
	}
	if(!x) return 0;
	prev_x->next = x->next;
	x->next = 0;
	map->size--;
	return e;
}

#ifdef MAP_TO_STRING /* <-- print */

#ifndef MAP_PRINT_THINGS /* <-- once inside translation unit */
#define MAP_PRINT_THINGS

static const char *const map_cat_start     = "[\n";
static const char *const map_cat_end       = " ]";
static const char *const map_cat_alter_end = "...]";
static const char *const map_cat_sep       = ", ";
static const char *const map_cat_star      = "*";
static const char *const map_cat_null      = "null";

struct Map_SuperCat {
	char *print, *cursor;
	size_t left;
	int is_truncated;
};
static void map_super_cat_init(struct Map_SuperCat *const cat,
	char *const print, const size_t print_size) {
	cat->print = cat->cursor = print;
	cat->left = print_size;
	cat->is_truncated = 0;
	print[0] = '\0';
}
static void map_super_cat(struct Map_SuperCat *const cat,
	const char *const append) {
	size_t lu_took; int took;
	if(cat->is_truncated) return;
	took = sprintf(cat->cursor, "%.*s", (int)cat->left, append);
	if(took < 0)  { cat->is_truncated = -1; return; } /*implementation defined*/
	if(took == 0) { return; }
	if((lu_took = (size_t)took) >= cat->left)
		cat->is_truncated = -1, lu_took = cat->left - 1;
	cat->cursor += lu_took, cat->left -= lu_took;
}
#endif /* once --> */

/** Can print 2 things at once before it overwrites. One must set
 {MAP_TO_STRING} to a function implementing {<KV>ToString} to get this
 functionality.
 @return Prints {map} in a static buffer.
 @order {\Theta(1)}; it has a 1024 character limit; every element takes some of
 it.
 @allow */
static const char *KV_(MapToString)(const struct KV_(Map) *const map) {
	static char buffer[2][1024];
	static unsigned buffer_i;
	struct Map_SuperCat cat;
	assert(strlen(map_cat_alter_end) >= strlen(map_cat_end));
	assert(sizeof buffer > strlen(map_cat_alter_end));
	map_super_cat_init(&cat, buffer[buffer_i],
		sizeof *buffer / sizeof **buffer - strlen(map_cat_alter_end));
	buffer_i++, buffer_i &= 1;
	if(!map) {
		map_super_cat(&cat, map_cat_null);
		return cat.print;
	}
	map_super_cat(&cat, map_cat_start);
	if(map->buckets) {
		struct PKV_(X) *b, *b_end, *x;
		char a[12];
		assert(map->log_capacity >= 3 && map->log_capacity < 32);
		for(b = map->buckets, b_end = b + (1 << map->log_capacity);
			b < b_end; b++) {
			for(x = b->next; x; x = x->next) {
				PKV_(to_string)(PKV_(x_upcast)(x), &a);
				map_super_cat(&cat, a);
				if(cat.is_truncated) break;
			}
			if(x) break;
		}
	}
	sprintf(cat.cursor, "%s",
		cat.is_truncated ? map_cat_alter_end : map_cat_end);
	return cat.print; /* Static buffer. */
}

#endif /* print --> */

#ifdef MAP_TEST /* <-- test */
#include "../test/TestMap.h" /* need this file if one is going to run tests */
#endif /* test --> */

static void PKV_(unused_coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 \url{ http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code } */
static void PKV_(unused_map)(void) {
	KV_(EntryKey)(0);
	KV_(EntryValue)(0);
	KV_(EntryConstValue)(0);
	KV_(Map_)(0);
	KV_(Map)(0);
	KV_(MapClear)(0);
	KV_(MapSize)(0);
	KV_(MapGet)(0, 0);
	KV_(MapPut)(0, 0, 0);
	KV_(MapPutIfAbsent)(0, 0, 0);
	KV_(MapRemove)(0, 0);
#ifdef MAP_TO_STRING
	KV_(MapToString)(0);
#endif
	PKV_(unused_coda)();
}
static void PKV_(unused_coda)(void) { PKV_(unused_map)(); }

/* Un-define all macros. */
#undef MAP_NAME
#undef MAP_TYPE
/* Undocumented; allows nestled inclusion so long as: {CAT_}, {CAT}, {PCAT},
 {PCAT_} conform, and {T}, {K}, and {E}, are not used. */
#ifdef MAP_SUBTYPE /* <-- sub */
#undef MAP_SUBTYPE
#else /* sub --><-- !sub */
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#endif /* !sub --> */
#undef K
#undef V
#undef KV_
#undef PKV_
#undef MAP_KEY
#undef MAP_VALUE
#ifdef MAP_KEY_IN_VALUE /* <-- kiv */
#undef MAP_KEY_IN_VALUE
#endif /* kiv --> */
#undef MAP_HASH
#undef MAP_IS_EQUAL
#ifdef MAP_TO_STRING /* <-- string */
#undef MAP_TO_STRING
#endif /* string --> */
#ifdef MAP_DEFAULT /* <-- def */
#undef MAP_DEFAULT
#endif /* def --> */
#ifdef MAP_STRICT_ANSI /* <-- ansi */
#undef MAP_STRICT_ANSI /* It's kind of weird, but it's consistent. */
#endif /* ansi --> */
#ifdef MAP_TEST /* <-- test */
#undef MAP_TEST
#endif /* test --> */
#ifdef MAP_NDEBUG /* <-- ndebug */
#undef MAP_NDEBUG
#undef NDEBUG
#endif /* ndebug --> */
