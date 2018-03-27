/** 2018 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 {<T>Measure} is an on-line statistics calculator based on
 \cite{Welford1962Note}. It stores {O(1)} information and is numerically stable
 up to the replicas that will fit in a {size_t}. As such, it should be used
 with values that are represented in a quasi-continuous domain.

 @param MEASURE_NAME
 A unique name associated with {<T>} that satisfies {C} naming rules when
 mangled; required.

 @param MEASURE_TYPE
 This type becomes {<T>}, a field; required. It should be a concrete value and
 not a pointer, such as {double} or {struct Vector}; required.

 @param MEASURE_ZERO
 A function satisfting \see{<PT>Action} setting the measurement to the additive
 identity of {<T>}, used by \see{<T>MeasureClear}; required.

 @param MEASURE_ADD
 A function satisfying \see{<PT>Operation} that satisfies addition on {<T>};
 required.

 @param MEASURE_MULTIPLY
 A function satisfying \see{<PT>Operation} that satisfies mutiplcation on
 {<T>}; required.

 @param MEASURE_TEST
 Unit testing framework, included in a separate header,
 {../test/MeasureTest.h}. Must be defined equal to a (random) filler function,
 satisfying \see{<PT>Action}. If {NDEBUG} is not defined, turns on {assert}
 function integrity testing.

 @title		Measure
 @author	Neil
 @std		C89/90
 @version	2018-03 */



#include <stddef.h>	/* offset_of */
#include <stdlib.h> /* malloc free */
#include <assert.h>	/* assert */
#include <stdio.h>  /* perror fprintf */
#include <errno.h>  /* errno */
#include <stdint.h> /* uint32_t uintptr_t (C99) */



/* Check defines. */
#ifndef MEASURE_NAME
#error Map generic MEASURE_NAME undefined.
#endif
#ifndef MEASURE_TYPE
#error Map generic MEASURE_TYPE undefined.
#endif
#ifndef MEASURE_ZERO
#error Map generic MEASURE_ZERO undefined.
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



/* After this block, the preprocessor replaces everything about the types.
 http://stackoverflow.com/questions/16522341/pseudo-generics-in-c */
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
#ifdef T
#undef T
#endif
#ifdef T_
#undef T_
#endif
#ifdef PT_
#undef PT_
#endif
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#define PT_(thing) PCAT(map, PCAT(MEASURE_NAME, thing))
#define T_(thing) CAT(MEASURE_NAME, thing)
#define PT_(thing) PCAT(map, PCAT(MEASURE_NAME, thing)) /* {private <T>}. */
/* Troubles with this line? check to ensure that {MEASURE_TYPE} is a valid
 type, whose definition is placed above {#include "Measure.h"}. */
typedef MEASURE_TYPE PT_(Measure);
#define T PT_(Measure)



/**  */
struct T_(Measure) {
	size_t count;
	T mean, ssdm;
};

/* Troubles with this line? check to ensure that {MAP_KEY} is a valid type,
 whose definition is placed above {#include "Map.h"}. */
typedef MAP_KEY PI_(Key);
#define K PI_(Key)

#if 0

/* \cite{Welford1962Note} */

int n = 0;
double dt_ms = 0.0, mean_ms = 0.0, delta_ms, ssdm = 0.0;
for(r = 0; r < replicas; r++) {
	dt_ms = experiment;
	n++;
	delta_ms = dt_ms - mean_ms;
	mean_ms += delta_ms / n;
	ssdm += delta_ms * (dt_ms - mean_ms);
}
fprintf(fp, "%u\t%f\t%f\n", elements, mean_ms, sqrt(ssdm / (n - 1)));

struct Stats { unsigned n; double mean, ssdm; } stats[10] = { { 0, 0, 0 } };
double x = hist[i];
double delta = x - stats.mean;
stats.mean += delta / ++stats.n;
stats.ssdm += delta * (x - stats.mean);

printf("%u\t%u\t%f\t%f\n", i, hist, stats.mean, sqrt(stats.ssdm / (stats.n - 1)));
printf("# %lu hashes, %u replicas\n", StringMapSize(map), replicas);
printf("%u\t%f\t%f\n", i, stats.mean, sqrt(stats.ssdm / (stats.n - 1)));
#endif



/** Provided {<I>}, returns {<K> \in <I>}. */
typedef K (*PI_(ItemKey))(const I *const);
/* Check that {MAP_TYPE_TO_KEY} is a function implementing {<PT>TypeKey}. */
static const PI_(ItemKey) PI_(item_key) = (MAP_TYPE_TO_KEY);

/** A constant equivalence relation between {<K>} that satisfies
 {<PT>IsEqual(a, b) -> <PT>Hash(a) == <PT>Hash(b)}. */
typedef int (*PI_(IsEqual))(const K, const K);
/* Check that {MAP_IS_EQUAL} is a function implementing {<PT>IsEqual}. */
static const PI_(IsEqual) PI_(is_equal) = (MAP_IS_EQUAL);

/** A map from {<K>} onto {int32_t}; should be as close as possible to a
 discrete uniform distribution while satisfying the above for optimum
 performance. */
typedef uint32_t (*PI_(Hash))(const K);
/* Check that {MAP_HASH} is a function implementing {<PT>Hash}. */
static const PI_(Hash) PI_(hash) = (MAP_HASH);

#ifdef MAP_TO_STRING /* <-- string */
/** Responsible for turning {<I>} (the first argument) into a 12 {char}
 null-terminated output string (the second.) */
typedef void (*PI_(ToString))(const I *, char (*const)[12]);
/* Check that {MAP_TO_STRING} is a function implementing {<I>ToString}. */
static const PI_(ToString) PI_(to_string) = (MAP_TO_STRING);
#endif /* string --> */

#ifdef MAP_TEST /* <-- test */
/** Used for {MAP_TEST}. */
typedef void (*PI_(Action))(I *const);
#endif /* test --> */



/** A {Map} or dictionary. To instatiate, \see{<I>Map}. */
struct I_(Map);
struct I_(Map) {
	struct PI_(ItemList) *bins;
	size_t items;
	unsigned log_bins;
};



/** Private: {container_of}. */
static struct I_(MapNode) *PI_(node_holds_item)(I *const item) {
	return (struct I_(MapNode) *)(void *)
		((char *)item - offsetof(struct I_(MapNode), node)
		- offsetof(struct PI_(ItemListNode), data));
}

/** @return An {<I>List} associated to the {key}, given the {map}. */
static struct PI_(ItemList) *PI_(get_bin)(struct I_(Map) *const map,
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
static int PI_(grow)(struct I_(Map) *const map, const size_t items) {
	struct PI_(ItemList) *bins, *b, *end, *new_bin;
	I *i, *next_i;
	const float one_ln2
		= 1.442695040888963407359924681001892137426645954152985934f;
	const size_t max_bins = (uint32_t)-1 & ~((uint32_t)-1 >> 1);
	uint32_t items_ln2, c0, c1, mask, hash;
	int is_moved;
	unsigned log_bins;
	char a[12];
	
	assert(map && map->log_bins < sizeof c0 * 8);
	/* {c0} is the current number of bins. (*except zero, see below.) */
	c0 = 1 << (uint32_t)(log_bins = map->log_bins);
	if(items > max_bins
		|| (items_ln2 = (uint32_t)((float)items * one_ln2)) > max_bins) {
		c1 = (uint32_t)max_bins;
	} else {
		/* Under load factor; this is probably the case. */
		if(items_ln2 < c0) return 1;
		assert(items > 1);
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
	for(b = bins + c0, end = bins + c1; b < end; b++) PI_(ItemListClear)(b);
	/* Rehash some entries into the new lists; if hash is normally distrubuted,
	 expectation value on the ratio of rehashed items:
	 \${ 1 / (log_bins - map->log_bins) }. */
	for(b = bins, end = bins + c0; b < end; b++) {
		if(is_moved) PI_(ItemListSelfCorrect(b));
		for(i = PI_(ItemListFirst)(b); i; i = next_i) {
			next_i = PI_(ItemListNext(i));
			/* Skip the ones that go nowhere. */
			if(!((hash = PI_(node_holds_item)(i)->hash) & mask)) continue;
			/* Rehash to the higher bins. */
			new_bin = PI_(get_bin)(map, hash);
			assert(new_bin >= end);
			PI_(ItemListRemove)(i);
			PI_(ItemListPush)(new_bin, i);
			PI_(to_string)(i, &a);
			/*printf("Item %s with hash%u rehashed to %u -> %u.\n", a, PI_(node_holds_item)(i)->hash, (unsigned)(b - bins), (unsigned)(new_bin - bins));*/
		}
	}
	map->bins     = bins;
	map->log_bins = log_bins;			
	return 1;
}

/** Destructor for {Map}.
 @order \Theta(1)
 @allow */
static void I_(Map_)(struct I_(Map) **const pmap) {
	struct I_(Map) *map;
	if(!pmap || !(map = *pmap)) return;
	free(map->bins);
	free(map);
	*pmap = 0;
}

/** Constructs an empty {Map} with the capacity of 16.
 @order O(1)
 @allow */
static struct I_(Map) *I_(Map)(void) {
	struct I_(Map) *map;
	if(!(map = malloc(sizeof *map))) return 0;
	map->bins     = 0;
	map->items    = 0;
	map->log_bins = 0;
	if(!PI_(grow)(map, 10)) I_(Map_)(&map);
	return map;
}

/** Clears and removes all values from {map}. All previous values are
 un-associated.
 @param map: if null, does nothing.
 @order \Theta({map.bins})
 @allow */
static void I_(MapClear)(struct I_(Map) *const map) {
	struct PI_(ItemList) *i, *end;
	if(!map) return;
	for(i = map->bins, end = i+map->items; i < end; i++) PI_(ItemListClear)(i);
}

/** Does the {item} not match the {key}? Go forward in the list.
 @implements <I>BiPredicate */
static int PI_(is_item)(I *const item, void *const pkey_void) {
	const K key_test = *(K *)pkey_void;
	const K key_item = PI_(item_key)(item);
	assert(item && pkey_void);
	return !PI_(is_equal)(key_test, key_item);
}

/** @return The item or null if it didn't find it.
 @order Average \O(1), Worst \O(n)
 @allow */
static I *I_(MapGet)(struct I_(Map) *const map, const K key) {
	if(!map) return 0;
	return PI_(ItemListBiShortCircuit)(PI_(get_bin)(map, PI_(hash)(key)),
		&PI_(is_item), (void *)&key);
}

/** Replaces the specified item in {map} with {item}. {item} must be within an
 un-itialised {<I>MapNode}; if {item} is already in a map, it will cause the
 other map to in an invalid state.
 @return The item that was replaced or null if {item} was the first.
 @order Average \O(1), Worst \O(n)
 @allow */
static I *I_(MapPut)(struct I_(Map) *const map, I *const item) {
	I *replaced;
	struct PI_(ItemList) *bin;
	K key;
	char a[12];
	if(!map || !item) return 0;
	/* Get the key. */
	key = PI_(item_key)(item);
	/* Cache the hash and get the bin. */
	bin = PI_(get_bin)(map, PI_(node_holds_item)(item)->hash = PI_(hash)(key));
	/* Replace key. */
	if((replaced = PI_(ItemListBiShortCircuit)(bin, &PI_(is_item),
		(void *)&key))) {
		PI_(ItemListRemove)(replaced);
	} else {
		/* Not an error, just a warning that the hash table wanted to get
		 bigger to accommodate the load factor. What should we do? We are
		 probably screwed anyway. */
		if(!PI_(grow)(map, map->items + 1)) perror("Map::grow");
		/* Refesh the bin because it may have chaged in {<PT>grow}. */
		bin = PI_(get_bin)(map, PI_(node_holds_item)(item)->hash);
		map->items++;
	}
	PI_(to_string)(item, &a);
	PI_(ItemListPush)(bin, item);
	return replaced;
}

/* @fixme Recompute key? */

/* From Java8
*** void 	clear()
 Removes all of the mappings from this map.
 V 	compute(K key, BiFunction<? super K,? super V,? extends V> remappingFunction)
 Attempts to compute a mapping for the specified key and its current mapped value (or null if there is no current mapping).
 V 	computeIfAbsent(K key, Function<? super K,? extends V> mappingFunction)
 If the specified key is not already associated with a value (or is mapped to null), attempts to compute its value using the given mapping function and enters it into this map unless null.
 V 	computeIfPresent(K key, BiFunction<? super K,? super V,? extends V> remappingFunction)
 If the value for the specified key is present and non-null, attempts to compute a new mapping given the key and its current mapped value.
*** V 	get(Object key)
 Returns the value to which the specified key is mapped, or null if this map contains no mapping for the key.
*** V 	getOrDefault(Object key, V defaultValue)
 Returns the value to which the specified key is mapped, or defaultValue if this map contains no mapping for the key.
 V 	get(Object key)
 Returns the value to which the specified key is mapped, or null if this map contains no mapping for the key.
 boolean 	isEmpty()
 Returns true if this map contains no key-value mappings.
 V 	merge(K key, V value, BiFunction<? super V,? super V,? extends V> remappingFunction)
 If the specified key is not already associated with a value or is associated with null, associates it with the given non-null value.
*** V 	put(K key, V value)
 Associates the specified value with the specified key in this map.
 void 	putAll(Map<? extends K,? extends V> m)
 Copies all of the mappings from the specified map to this map.
 V 	putIfAbsent(K key, V value)
 If the specified key is not already associated with a value (or is mapped to null) associates it with the given value and returns null, else returns the current value.
 V 	remove(Object key)
 Removes the mapping for the specified key from this map if present.
 boolean 	remove(Object key, Object value)
 Removes the entry for the specified key only if it is currently mapped to the specified value.
 V 	replace(K key, V value)
 Replaces the entry for the specified key only if it is currently mapped to some value.
 boolean 	replace(K key, V oldValue, V newValue)
 Replaces the entry for the specified key only if currently mapped to the specified value.
 void 	replaceAll(BiFunction<? super K,? super V,? extends V> function)
 Replaces each entry's value with the result of invoking the given function on that entry until all entries have been processed or the function throws an exception.
 int 	size()
 Returns the number of key-value mappings in this map.
 */


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
 {MAP_TO_STRING} to a function implementing {<I>ToString} to get this
 functionality.
 @return Prints {map} in a static buffer.
 @order \Theta(1); it has a 1024 character limit; every element takes some of
 it.
 @allow */
static const char *I_(MapToString)(const struct I_(Map) *const map) {
	static char buffer[2][1024];
	static unsigned buffer_i;
	struct Map_SuperCat cat;
	struct PI_(ItemList) *b, *end;
	/*int is_first = 1;
	char scratch[12];
	size_t i;*/
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
		map_super_cat(&cat, PI_(ItemListToString)(b));
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

static void PI_(unused_coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 \url{ http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code } */
static void PI_(unused_map)(void) {
	I_(Map_)(0);
	I_(Map)();
	I_(MapClear)(0);
	I_(MapGet)(0, 0);
	I_(MapPut)(0, 0);
#ifdef MAP_TO_STRING
	I_(MapToString)(0);
#endif
	PI_(unused_coda)();
}
static void PI_(unused_coda)(void) { PI_(unused_map)(); }

/* Un-define all macros. */
#undef MAP_NAME
#undef MAP_TYPE
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#undef K
#undef I
#undef I_
#undef PI_
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
