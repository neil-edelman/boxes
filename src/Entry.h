/** 2018 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 This, if you will, extends {<E>Map} to {<K,V>Map}, and the entry for this map
 is {<K,V>Entry}, which requires storage of {<K,V>MapNode}; as such, it defines
 {<K,V>Entry}, {<K,V>Map}, and {<K,V>MapNode}. This is a more specialised
 version of {<E>Map} where the key and the value are separate, resembling more
 traditional associative arrays. Maps between the {uint32_t} hash of {<K>} and
 one bucket, implemented internally as a {<K,V>List}. Requires {Map.h}, which
 requires {List.h}, in the same directory. See {Map} for the rest of
 documentation on {Map}.

 @param ENTRY_NAME
 A unique name associated with {<K,V>} that satisfies {C} naming rules when
 mangled as {<KV>}; required.

 @param ENTRY_KEY
 This becomes {<K>}. A non-compound type, that is, it can be assigned. For
 example, {char *}, {int}, or {struct *}, but not, {struct}; required.
 (@fixme are all these restrictions necessary?)

 @param ENTRY_VALUE
 This becomes {<V>}; required.

 @param ENTRY_CMP
 A function satisfying \see{<PKV>Compare}. For example, if one's {ENTRY_KEY}
 where {char *}, this might be {&strcmp}; required.

 @param ENTRY_HASH
 A function satisfying \see{<PKV>Hash}. For example, if one's {ENTRY_KEY}
 where {char *}, this might be {&map_fnv_32a_str} supplied by {Map.h};
 required.

 @param ENTRY_TO_STRING
 Optional print function implementing \see{<KV>ToString}; requires {ENTRY_TEST}.

 @param ENTRY_TEST
 Unit testing framework, included in a separate header, {../test/EntryTest.h}.
 Must be defined equal to a (random) filler function, satisfying {<PKV>Action}.
 If {NDEBUG} is not defined, turns on {assert} private function integrity
 testing. Requires {MAP_TO_STRING}.

 @title		Entry
 @author	Neil
 @std		C89/90
 @version	2018-03 */



#include <assert.h>	/* assert */



/* Check defines. */
#ifndef ENTRY_NAME
#error Generic ENTRY_NAME undefined.
#endif
#ifndef ENTRY_KEY
#error Generic ENTRY_KEY undefined.
#endif
#ifndef ENTRY_VALUE
#error Generic ENTRY_VALUE undefined.
#endif
#ifndef ENTRY_CMP
#error Generic ENTRY_CMP undefined.
#endif
#ifndef ENTRY_HASH
#error Generic ENTRY_HASH undefined.
#endif
#if defined(ENTRY_TEST) ^ defined(ENTRY_TO_STRING)
#error ENTRY_TEST and ENTRY_TO_STRING are mutual.
#endif
#if !defined(ENTRY_TEST) && !defined(NDEBUG)
#define ENTRY_NDEBUG
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
#define KV_(thing) CAT(ENTRY_NAME, thing)
#define PKV PCAT(map, ENTRY_NAME) /* Used for list name. */
#define PKV_(thing) PCAT(map, PCAT(ENTRY_NAME, thing)) /* {private <T>}. */

/* Troubles with this line? check to ensure that {ENTRY_KEY} is a valid type,
 whose definition is placed above {#include "Entry.h"}. */
typedef ENTRY_KEY PKV_(KeyType);
#define K PKV_(KeyType)

/* Troubles with this line? check to ensure that {ENTRY_VALUE} is a valid type,
 whose definition is placed above {#include "Entry.h"}. */
typedef ENTRY_VALUE PKV_(ValueType);
#define V PKV_(ValueType)

#ifdef ENTRY_TO_STRING /* <-- string */
/** Just used for {ENTRY_TEST}. Responsible for turning {<KV>} (the first
 argument) into a 12 {char} null-terminated output string (the second.) */
typedef void (*PKV_(ToString))(const struct KV_(Entry) *, char (*const)[12]);
/* Check that {ENTRY_TO_STRING} is a function implementing {<PKV>ToString}. */
static const PKV_(ToString) PKV_(to_string) = (ENTRY_TO_STRING);
#endif /* string --> */

#ifdef ENTRY_TEST /* <-- test */
/** Just used for {ENTRY_TEST}. */
typedef void (*PKV_(Action))(struct KV_(Entry) *const);
#endif /* test --> */



/** An entry. Any changes to the {key} field and you must re-hash with
 {<P,K>MapRehash}. */
struct KV_(Entry) {
	K key;
	V value;
};



/** An inverted equivalence relation between keys, where 0 means they are
 equivalent. Done this way because {strcmp} is often used in this capacity. */
typedef int (*PKV_(EntryCmp))(const K, const K);
/* Check that {ENTRY_CMP} is a function implementing {<PKV>EntryCmp}. */
static const PKV_(EntryCmp) PKV_(cmp) = (ENTRY_CMP);

/** Private: get the key. */
static K PKV_(get_key)(const struct KV_(Entry) *const entry) {
	return entry->key;
}

/** Private: get equality between keys. */
static int PKV_(key_is_equal)(const K a, const K b) {
	return !PKV_(cmp)(a, b);
}

/* This relies on {Map.h} which must be in the same directory.
 Defines {<KV>Map} and {<KV>MapNode}. */
#define MAP_NAME ENTRY_NAME
#define MAP_TYPE struct KV_(Entry)
#define MAP_KEY PKV_(KeyType)
#define MAP_TYPE_TO_KEY &PKV_(get_key)
#define MAP_IS_EQUAL &PKV_(key_is_equal)
#define MAP_HASH ENTRY_HASH
#define MAP_SUBTYPE
#include "Map.h"
/* Messes with {K}; re-define it. */
#define K PKV_(KeyType)



/** Private: {container_of} */
static struct KV_(Entry) *PKV_(node_holds_value)(V *const v) {
	return (struct KV_(Entry) *)(void *)
		((char *)v - offsetof(struct KV_(Entry), value));
}

/** @param value: The value part of a key-value {<KV>Entry} pair.
 @return The {<KV>Entry} pair.
 @order \Omega(1)
 @allow */
static struct KV_(Entry) *KV_(EntryHoldsValue)(V *const value) {
	if(!value) return 0;
	return PKV_(node_holds_value)(value);
}

/** @return The value associated with the specified {key} or null if it didn't
 find it.
 @order Average \O(1), Worst \O(n)
 @allow */
static V *KV_(MapGetValue)(struct KV_(Map) *const map, const K key) {
	struct KV_(Entry) *kv;
	if(!map || (kv = KV_(MapGet)(map, key))) return 0;
	return &kv->value;
}



#ifdef ENTRY_TEST /* <-- test */
#include "../test/TestEntry.h"
#endif /* test --> */

static void PKV_(entry_unused_coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 \url{ http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code } */
static void PKV_(entry_unused_map)(void) {
	KV_(EntryHoldsValue)(0);
	KV_(MapGetValue)(0, 0);
	PKV_(entry_unused_coda)();
}
static void PKV_(entry_unused_coda)(void) { PKV_(entry_unused_map)(); }

/* Un-define all macros. */
#undef ENTRY_NAME
#undef ENTRY_KEY
#undef ENTRY_VALUE
#undef ENTRY_CMP
#undef ENTRY_HASH
/* Undocumented; allows nestled inclusion so long as: {CAT_}, {CAT}, {PCAT},
 {PCAT_} conform, and {T}, {K}, {V}, and {E}, are not used. */
#ifdef ENTRY_SUBTYPE /* <-- sub */
#undef ENTRY_SUBTYPE
#else /* sub --><-- !sub */
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#endif /* !sub --> */
#undef K
#undef V
#undef KV_
#undef PKV
#undef PKV_
#ifdef ENTRY_TEST
#undef ENTRY_TEST
#endif
#ifdef ENTRY_NDEBUG
#undef ENTRY_NDEBUG
#undef NDEBUG
#endif
