/** 2018 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 {<E>Map} is a general associative array abstract data type, (hash table, hash
 map, hashmap, dictionary,) of which a subset, specified by {MAP_KEY},
 {K \in E} is used as a deterministic unique key. The key is then hashed to a
 {uint32_t} and placed in a bucket implemented as a power-of-two dynamically
 allocated array of lists for easy rehashing. Adding an element with the same
 {K}, according to {MAP_IS_EQUAL}, causes the old data to be ejected, see
 \see{<E>MapPut}. Depends on {List.h} being in the same directory as {Map.h}.
 This requires the storage of {<E>MapNode} or structures therewith.

 Use when {<E>} contains the key within the data. If the data can be separated,
 into keys and values, see {<K,V>Entry}, which is a subclass requiring fewer
 parameters (if you will.)

 @param MAP_NAME
 A unique (among {Map}) name associated with {<E>} that satisfies {C} naming
 rules when mangled; required.

 @param MAP_TYPE
 This type becomes {<E>}. To have a linked hash map, one can specify a
 {<T>ListNode}; required.

 @param MAP_KEY
 The type associated with {<K>}, a constant (possibly pointer-to) sub-type of
 {<K> \in <E>}. One should not specify {const} for the outer type. Should be a
 primitive type; required.

 @param MAP_TYPE_TO_KEY
 A function satisfying \see{<PT>TypeKey}; required.

 @param MAP_IS_EQUAL
 A function satisfying \see{<PT>IsEqual}; required.

 @param MAP_HASH
 A function satisfying \see{<PT>Hash}; required.

 @param MAP_TO_STRING
 Optional print function implementing {<E>ToString}; makes available
 \see{<E>MapToString}.

 @param MAP_TEST
 Unit testing framework, included in a separate header, {../test/MapTest.h}.
 Must be defined equal to a (random) filler function, satisfying {<PT>Action}.
 If {NDEBUG} is not defined, turns on {assert} private function integrity
 testing. Requires {MAP_TO_STRING}.

 @fixme MAP_DEFAULT -- instead of null so one can do, {<E>MapGet(key)->value}.

 @title		Map
 @author	Neil
 @std		C89/90 + C99 stdint.h:uint32_t
 @version	2018-03 */



#include <stddef.h>	/* offsetof */
#include <stdlib.h> /* malloc free */
#include <assert.h>	/* assert */
#include <stdio.h>  /* perror fprintf */
#include <errno.h>  /* errno */
#include <stdint.h> /* uint32_t uintptr_t (C99) */



/* Check defines. */
#ifndef MAP_NAME
#error Map generic MAP_NAME undefined.
#endif
#ifndef MAP_TYPE
#error Map generic MAP_TYPE undefined.
#endif
#ifndef MAP_KEY
#error Map generic MAP_KEY undefined.
#endif
#ifndef MAP_TYPE_TO_KEY
#error Map generic MAP_TYPE_TO_KEY undefined.
#endif
#ifndef MAP_IS_EQUAL
#error Map generic MAP_IS_EQUAL undefined.
#endif
#ifndef MAP_HASH
#error Map generic MAP_HASH undefined.
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
#ifdef E
#undef E
#endif
#ifdef E_
#undef E_
#endif
#ifdef PE_
#undef PE_
#endif
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#define E_(thing) CAT(MAP_NAME, thing)
#define PE_(thing) PCAT(map, PCAT(MAP_NAME, thing)) /* {private <E>}. */

/* Troubles with this line? check to ensure that {MAP_TYPE} is a valid type,
 whose definition is placed above {#include "Map.h"}. */
typedef MAP_TYPE PE_(Entry);
#define E PE_(Entry)

/* Troubles with this line? check to ensure that {MAP_KEY} is a valid type,
 whose definition is placed above {#include "Map.h"}. */
typedef MAP_KEY PE_(Key);
#define K PE_(Key)

/* This relies on {List.h} which must be in the same directory.
 Defines {<PE>EntryList} and {<PE>EntryListNode}. */
#define LIST_NAME PE_(Entry)
#define LIST_TYPE PE_(Entry)
#ifdef MAP_TO_STRING
#define LIST_TO_STRING MAP_TO_STRING
#endif
#define LIST_SUBTYPE
#include "List.h"



/** Storage of this structure is the responsibility of the caller. The {<E>} is
 stored in the element {node.data}. */
struct E_(MapNode);
struct E_(MapNode) {
	struct PE_(EntryLink) node;
	uint32_t hash;
};



/* Constants across multiple includes in the same translation unit. */
#ifndef MAP_H /* <-- MAP_H */
#define MAP_H
/** Perform a 32 bit Fowler/Noll/Vo FNV-1a hash on a string, edited slightly.
 \url{http://www.isthe.com/chongo/tech/comp/fnv/}. Provided for convenience
 when using constant string keys, {ie}, {#define MAP_KEY const char *},
 {#define MAP_HASH &map_fnv_32a_str}.
 @allow */
static uint32_t map_fnv_32a_str(const char *str) {
	union IntChar { uint32_t integer; unsigned char character[4]; };
	const unsigned char *s = (const unsigned char *)str;
	/* 32 bit FNV-1 and FNV-1a non-zero initial basis, FNV1_32A_INIT */
	uint32_t hval = (uint32_t)0x811c9dc5;
	union IntChar intchar = { 0 };
	const union IntChar *ic;
	/* Strings are pointers that must have at least 4 byte alignment (x86.) */
	assert(((uintptr_t)(const void *)s & 3) == 0);
	/* Take multiples of 4 bytes; FNV magic prime {FNV_32_PRIME 0x01000193}. */
	while(ic = (const union IntChar *)(const void *)s,
		*s++ && *s++ && *s++ && *s++) {
		hval ^= ic->integer;
		hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
	}
	/* Left-over. The null terminator does need to go in; it's initialised. */
	switch(s - &ic->character[1]) {
		case 0:	return hval; /* The string is a multiple of 4 in length. */
		case 1: intchar.character[0] = ic->character[0];
		case 2: intchar.character[1] = ic->character[1];
		case 3: intchar.character[2] = ic->character[2];
	}
	hval ^= intchar.integer;
	hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
	return hval;
}
#endif /* MAP_H */



/** Provided {<E>}, returns {<K> \in <E>}. */
typedef K (*PE_(EntryKey))(const E *const);
/* Check that {MAP_TYPE_TO_KEY} is a function implementing {<PT>TypeKey}. */
static const PE_(EntryKey) PE_(item_key) = (MAP_TYPE_TO_KEY);

/** A constant equivalence relation between {<K>} that satisfies
 {<PT>IsEqual(a, b) -> <PT>Hash(a) == <PT>Hash(b)}. */
typedef int (*PE_(IsEqual))(const K, const K);
/* Check that {MAP_IS_EQUAL} is a function implementing {<PT>IsEqual}. */
static const PE_(IsEqual) PE_(is_equal) = (MAP_IS_EQUAL);

/** A map from {<K>} onto {uint32_t}; should be as close as possible to a
 discrete uniform distribution while satisfying the above for optimum
 performance. */
typedef uint32_t (*PE_(Hash))(const K);
/* Check that {MAP_HASH} is a function implementing {<PT>Hash}. */
static const PE_(Hash) PE_(hash) = (MAP_HASH);

#ifdef MAP_TO_STRING /* <-- string */
/** Responsible for turning {<E>} (the first argument) into a 12 {char}
 null-terminated output string (the second.) */
typedef void (*PE_(ToString))(const E *, char (*const)[12]);
/* Check that {MAP_TO_STRING} is a function implementing {<E>ToString}. */
static const PE_(ToString) PE_(to_string) = (MAP_TO_STRING);
#endif /* string --> */

#ifdef MAP_TEST /* <-- test */
/** Used for {MAP_TEST}. */
typedef void (*PE_(Action))(E *const);
#endif /* test --> */



/** A {Map} or dictionary. To instantiate, \see{<E>Map}. */
struct E_(Map);
struct E_(Map) {
	struct PE_(EntryList) *bins;
	size_t entries;
	unsigned log_bins;
};



/** Private: {container_of}. */
static struct E_(MapNode) *PE_(node_holds_item)(E *const entry) {
	return (struct E_(MapNode) *)(void *)
		((char *)entry - offsetof(struct E_(MapNode), node)
		- offsetof(struct PE_(EntryLink), data));
}

/** @return An {<E>List} associated to the {key}, given the {map}. */
static struct PE_(EntryList) *PE_(get_bin)(struct E_(Map) *const map,
	const uint32_t hash) {
	assert(map && map->log_bins > 1 && map->log_bins < 32);
	return map->bins + (hash & ((1 << map->log_bins) - 1));
}

/** Private: Grow the table until the capacity of the buckets (bins) is the
 optimal {ln 2 = 0.69}.
 @param size: How many entries are there going to be; {size > 1}.
 @return Success; otherwise, {errno} may be set.
 @throws ERANGE: Tried allocating more then can fit in {size_t}.
 @throws ENOMEM?: {IEEE Std 1003.1-2001}; C standard does not say.
 @order O(1) amortized */
static int PE_(grow)(struct E_(Map) *const map, const size_t entries) {
	struct PE_(EntryList) *bins, *b, *end, *new_bin;
	E *i, *next_i;
	const float one_ln2 = 1.44269504088896340735992468100f;
	const size_t max_bins = (uint32_t)-1 & ~((uint32_t)-1 >> 1);
	uint32_t items_ln2, c0, c1, mask, hash;
	int is_moved;
	unsigned log_bins;
	/*char a[12];*/

	assert(map && map->log_bins < sizeof c0 * 8);
	/* {c0} is the current number of bins. (*except zero, see below.) */
	c0 = 1 << (uint32_t)(log_bins = map->log_bins);
	if(entries > max_bins
		|| (items_ln2 = (uint32_t)((float)entries * one_ln2)) > max_bins) {
		c1 = (uint32_t)max_bins;
	} else {
		/* Under load factor; this is probably the case. */
		if(items_ln2 < c0) return 1;
		assert(entries > 1);
		c1 = c0;
		/* Expand the space to {c1}. */
		do { assert(c1<<1 > c1), c1 <<= 1, log_bins++; } while(items_ln2 > c1);
	}
	/* The domain is \${ (0, max] } but we want 0. */
	mask = (c1 - 1) ^ (c0 - 1), assert(mask && log_bins);
	if(c0 == 1) c0 = 0;
	/* Allocate new space. */
	if(!(bins = realloc(map->bins, c1 * sizeof *map->bins))) return 0;
	is_moved      = (bins != map->bins);
	map->bins     = bins;
	map->log_bins = log_bins;			
	/* Initialize the new lists to contain no elements. */
	for(b = bins + c0, end = bins + c1; b < end; b++) PE_(EntryListClear)(b);
	/* Rehash some entries into the new lists; if hash is normally distrubuted,
	 expectation value on the ratio of rehashed entries:
	 \${ 1 / (log_bins - map->log_bins) }. */
	for(b = bins, end = bins + c0; b < end; b++) {
		if(is_moved) PE_(EntryListSelfCorrect(b));
		for(i = PE_(EntryListFirst)(b); i; i = next_i) {
			next_i = PE_(EntryListNext(i));
			/* Skip the ones that go nowhere. */
			if(!((hash = PE_(node_holds_item)(i)->hash) & mask)) continue;
			/* Rehash to the higher bins. */
			new_bin = PE_(get_bin)(map, hash);
			assert(new_bin >= end);
			PE_(EntryListRemove)(i);
			PE_(EntryListPush)(new_bin, i);
			/*PE_(to_string)(i, &a);
			printf("Entry %s with hash%u rehashed to %u -> %u.\n", a, PE_(node_holds_item)(i)->hash, (unsigned)(b - bins), (unsigned)(new_bin - bins));*/
		}
	}
	map->bins     = bins;
	map->log_bins = log_bins;			
	return 1;
}

/** Destructor for {Map}.
 @order \Theta(1)
 @allow */
static void E_(Map_)(struct E_(Map) **const pmap) {
	struct E_(Map) *map;
	if(!pmap || !(map = *pmap)) return;
	free(map->bins);
	free(map);
	*pmap = 0;
}

/** Constructs an empty {Map} with the capacity of 16.
 @order O(1)
 @allow */
static struct E_(Map) *E_(Map)(void) {
	struct E_(Map) *map;
	if(!(map = malloc(sizeof *map))) return 0;
	map->bins     = 0;
	map->entries  = 0;
	map->log_bins = 0;
	if(!PE_(grow)(map, 10)) E_(Map_)(&map);
	return map;
}

/** Clears and removes all entries from {map}. All previous values are
 un-associated.
 @param map: if null, does nothing.
 @order \Theta({map.bins})
 @allow */
static void E_(MapClear)(struct E_(Map) *const map) {
	struct PE_(EntryList) *i, *end;
	if(!map) return;
	for(i = map->bins, end = i + (1 << map->log_bins); i < end; i++)
		PE_(EntryListClear)(i);
	map->entries = 0;
}

/** @return The number of entries in the map. */
static size_t E_(MapSize)(const struct E_(Map) *const map) {
	if(!map) return 0;
	return map->entries;
}

/** Does the {entry} not match the {key}? Go forward in the list.
 @implements <E>BiPredicate */
static int PE_(is_item)(E *const entry, void *const pkey_void) {
	const K key_test = *(K *)pkey_void;
	const K key_item = PE_(item_key)(entry);
	assert(entry && pkey_void);
	return !PE_(is_equal)(key_test, key_item);
}

/* Java: V getOrDefault(Object key, V defaultValue) Returns the value to which
 the specified key is mapped, or defaultValue if this map contains no mapping
 for the key. @fixme E'd like to merge this into {<E>MapGet} as a {#define}. */

/** @return The entry with the specified {key} or null if it didn't find it.
 @order Average \O(1), Worst \O(n)
 @allow */
static E *E_(MapGet)(struct E_(Map) *const map, K key) {
	if(!map) return 0;
	return PE_(EntryListBiAll)(PE_(get_bin)(map, PE_(hash)(key)),
		&PE_(is_item), (void *)&key);
}

/** Replaces the specified entry, or, if there is no entry with the specified
 key, puts the {entry} in {map}. {entry} must be within an un-itialised
 {<E>MapNode}; if {entry} is already in a map, it will cause the other map to be
 in an invalid state.
 @return The entry that was replaced or null if {entry} was the first.
 @order Average \O(1), Worst \O(n)
 @allow */
static E *E_(MapPut)(struct E_(Map) *const map, E *const entry) {
	E *replaced;
	struct PE_(EntryList) *bin;
	K key;
	if(!map || !entry) return 0;
	/* We may need to grow. Not an error, just a warning that the hash table
	 could not accommodate the load factor. What should we do? We are probably
	 screwed anyway. */
	if(!PE_(grow)(map, map->entries + 1)) perror("Map::grow");
	/* Get the key. */
	key = PE_(item_key)(entry);
	/* Cache the hash and get the bin. */
	bin = PE_(get_bin)(map, PE_(node_holds_item)(entry)->hash = PE_(hash)(key));
	if((replaced = PE_(EntryListBiAll)(bin, &PE_(is_item),
		(void *)&key))) {
		/* Replace key. */
		PE_(EntryListRemove)(replaced);
	} else {
		/* New key. */
		map->entries++;
	}
	PE_(EntryListPush)(bin, entry);
	return replaced;
}

/** @fixme <E>MapRehash(map, node) */
/*V 	remove(Object key)
Removes the mapping for the specified key from this map if present.*/
/** @fixme <E>MapRemove(node) */
/* From Java8
 V 	merge(K key, V value, BiFunction<? super V,? super V,? extends V> remappingFunction)
 If the specified key is not already associated with a value or is associated with null, associates it with the given non-null value.
 void 	putAll(Map<? extends K,? extends V> m)
 Copies all of the mappings from the specified map to this map. */


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
 {MAP_TO_STRING} to a function implementing {<E>ToString} to get this
 functionality.
 @return Prints {map} in a static buffer.
 @order \Theta(1); it has a 1024 character limit; every element takes some of
 it.
 @allow */
static const char *E_(MapToString)(const struct E_(Map) *const map) {
	static char buffer[2][1024];
	static unsigned buffer_i;
	struct Map_SuperCat cat;
	struct PE_(EntryList) *b, *end;
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
	for(b = map->bins, end = b + (1 << map->log_bins); b < end; b++) {
		map_super_cat(&cat, PE_(EntryListToString)(b));
		map_super_cat(&cat, "\n");
		if(cat.is_truncated) break;
	}
	sprintf(cat.cursor, "%s",
		cat.is_truncated ? map_cat_alter_end : map_cat_end);
	return cat.print; /* Static buffer. */
}

#endif /* print --> */

#ifdef MAP_TEST /* <-- test */
#include "../test/TestMap.h" /* need this file if one is going to run tests */
#endif /* test --> */

static void PE_(unused_coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 \url{ http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code } */
static void PE_(unused_map)(void) {
	E_(Map_)(0);
	E_(Map)();
	E_(MapClear)(0);
	E_(MapSize)(0);
	E_(MapGet)(0, 0);
	E_(MapPut)(0, 0);
#ifdef MAP_TO_STRING
	E_(MapToString)(0);
#endif
	PE_(unused_coda)();
}
static void PE_(unused_coda)(void) { PE_(unused_map)(); }

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
#undef E
#undef E_
#undef PE_
#undef MAP_KEY
#undef MAP_TYPE_TO_KEY
#undef MAP_IS_EQUAL
#undef MAP_HASH
#ifdef MAP_TO_STRING
#undef MAP_TO_STRING
#endif
#ifdef MAP_TEST
#undef MAP_TEST
#endif
#ifdef MAP_NDEBUG
#undef MAP_NDEBUG
#undef NDEBUG
#endif
