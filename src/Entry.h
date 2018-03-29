/** 2018 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 This, if you will, extends {<E>Map} to {<K,V>Map}. The entry for this map is
 {<K,V>Entry}, which requires storage of {<K,V>EntryMapList}. This is a more
 specialised version of {<E>Map} where the key and the value are separate,
 resembling more traditional associative arrays. Maps between the {uint32_t}
 hash of {<K>} and one bucket, implemented as a {<K,V>List}. Requires {Map.h},
 which requires {List.h}, in the same directory.

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
 Optional print function implementing \see{<KV>ToString}; makes available
 \see{<KV>EntryToString} and {<KV>EntryMapToString}.

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
#error Entry generic ENTRY_NAME undefined.
#endif
#ifndef ENTRY_KEY
#error Map generic ENTRY_KEY undefined.
#endif
#ifndef ENTRY_VALUE
#error Map generic ENTRY_VALUE undefined.
#endif
#ifndef ENTRY_CMP
#error Map generic ENTRY_CMP undefined.
#endif
#ifndef ENTRY_HASH
#error Map generic ENTRY_HASH undefined.
#endif
#if !defined(ENTRY_TEST) && !defined(NDEBUG)
#define ENTRY_NDEBUG
#define NDEBUG
#endif



/* After this block, the preprocessor replaces T with LIST_TYPE, T_(X) with
 LIST_NAMEX, PT_(X) with LIST_U_NAME_X, and T_NAME with the string
 version. http://stackoverflow.com/questions/16522341/pseudo-generics-in-c */
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
#ifdef KV_NAME
#undef KV_NAME
#endif
#ifdef QUOTE
#undef QUOTE
#endif
#ifdef QUOTE_
#undef QUOTE_
#endif
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)
#define KV_(thing) CAT(ENTRY_NAME, thing)
#define PKV PCAT(map, ENTRY_NAME) /* Used for list name. */
#define PKV_(thing) PCAT(map, PCAT(ENTRY_NAME, thing)) /* {private <T>}. */
#define KV_NAME QUOTE(ENTRY_NAME)

/* Troubles with this line? check to ensure that {ENTRY_KEY} is a valid type,
 whose definition is placed above {#include "Entry.h"}. */
typedef ENTRY_KEY PKV_(KeyType);
#define K PKV_(KeyType)

/* Troubles with this line? check to ensure that {ENTRY_VALUE} is a valid type,
 whose definition is placed above {#include "Entry.h"}. */
typedef ENTRY_VALUE PKV_(ValueType);
#define V PKV_(ValueType)



/* Constants across multiple includes in the same translation unit. */
#ifndef ENTRY_H /* <-- ENTRY_H */
#define ENTRY_H
#endif /* ENTRY_H */



/** An entry. */
struct KV_(Entry) {
	/*const*/ K key;
	V value;
};



static V *KV_(MapGetValue)(struct E_(Map) *const map, const K key) {
}

#ifdef ENTRY_TEST /* <-- test */
#include "../test/TestEntry.h"
#endif /* test --> */

/* Un-define all macros. */
#undef ENTRY_NAME
#undef ENTRY_KEY
#undef ENTRY_VALUE
#undef ENTRY_CMP
#undef ENTRY_HASH
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#undef K
#undef V
#undef KV_
#undef PKV
#undef PKV_
#undef KV_NAME
#undef QUOTE
#undef QUOTE_
#ifdef ENTRY_TEST
#undef ENTRY_TEST
#endif
#ifdef ENTRY_NDEBUG
#undef ENTRY_NDEBUG
#undef NDEBUG
#endif
