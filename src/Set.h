/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Hash Set

 ![Example of <String>Set.](../web/set.png)

 <tag:<E>Set> is a collection of elements of <tag:<E>SetElement> that doesn't
 allow duplication; it must be supplied an equality function, `SET_IS_EQUAL`
 <typedef:<PE>IsEqual>, and a hash function, `SET_HASH` <typedef:<PE>Hash>.

 Internally, it is a separately chained hash table with a maximum load factor
 of `ln 2`, and power-of-two resizes, with buckets as a forward linked list of
 <tag:<E>SetElement>. This offers some independence of sets from set elements,
 but cache performance is left up to the caller. It can be expanded to a hash
 map or associative array by enclosing the `<E>SetElement` in another `struct`,
 as appropriate. While in a set, the elements should not change in a way that
 affects their hash values.

 `<E>Set` is not synchronised. Errors are returned with `errno`. The parameters
 are `#define` preprocessor macros, and are all undefined at the end of the
 file for convenience. `assert.h` is used; to stop assertions, use
 `#define NDEBUG` before inclusion.

 @param[SET_NAME, SET_TYPE]
 `<E>` that satisfies `C` naming conventions when mangled and a valid
 <typedef:<PE>Type> associated therewith; required. `<PE>` is private, whose
 names are prefixed in a manner to avoid collisions; any should be re-defined
 prior to use elsewhere.

 @param[SET_HASH]
 A function satisfying <typedef:<PE>Hash>; required.

 @param[SET_IS_EQUAL]
 A function satisfying <typedef:<PE>IsEqual>; required.

 @param[SET_POINTER]
 Usually <typedef:<PE>MType> in the same as <typedef:<PE>Type> for simple
 `SET_TYPE`, but this flag makes `<PE>MType` be a pointer-to-`<PE>Type`. This
 affects <typedef:<PE>Hash>, <typedef:<PE>IsEqual>, and <fn:<E>SetGet>, making
 them accept a pointer-to-const-`<E>` instead of a copy of `<E>`.

 @param[SET_UINT]
 This is <typedef:<PE>UInt> and defaults to `unsigned int`; use when
 <typedef:<PE>Hash> is a specific hash length.

 @param[SET_NO_CACHE]
 Calculates the hash every time and discards it; should be used when the hash
 calculation is trivial to avoid storing duplicate <typedef:<PE>UInt> _per_
 datum, (in rare cases.)

 @param[SET_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[SET_TO_STRING]
 To string trait contained in <ToString.h>; `<A>` that satisfies `C` naming
 conventions when mangled and function implementing <typedef:<PE>ToString>.
 There can be multiple to string traits, but only one can omit
 `SET_TO_STRING_NAME`.

 @param[SET_TEST]
 To string trait contained in <../test/SetTest.h>; optional unit testing
 framework using `assert`. Can only be defined once _per_ `Set`. Must be
 defined equal to a (random) filler function, satisfying <typedef:<PE>Action>.
 Output will be shown with the to string trait in which it's defined; provides
 tests for the base code and all later traits.

 @std C89
 @cf [Array](https://github.com/neil-edelman/Array)
 @cf [Heap](https://github.com/neil-edelman/Heap)
 @cf [List](https://github.com/neil-edelman/List)
 @cf [Orcish](https://github.com/neil-edelman/Orcish)
 @cf [Pool](https://github.com/neil-edelman/Pool)
 @cf [Trie](https://github.com/neil-edelman/Trie) */

#include <stdlib.h> /* realloc free */
#include <assert.h>	/* assert */
#include <errno.h>  /* errno */


#ifndef SET_NAME
#error Name SET_NAME undefined.
#endif
#ifndef SET_TYPE
#error Tag type SET_TYPE undefined.
#endif
#ifndef SET_IS_EQUAL
#error Function SET_IS_EQUAL undefined.
#endif
#ifndef SET_HASH
#error Function SET_HASH undefined.
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
#if (SET_TRAITS == 0) && defined(SET_TEST)
#error SET_TEST must be defined in SET_TO_STRING trait.
#endif
#if defined(SET_TO_STRING_NAME) && !defined(SET_TO_STRING)
#error SET_TO_STRING_NAME requires SET_TO_STRING.
#endif


#if SET_TRAITS == 0 /* <!-- base code */


/* <Kernighan and Ritchie, 1988, p. 231>. */
#if defined(E_) || defined(PE_)
#error P?E_? cannot be defined; possible stray SET_EXPECT_TRAIT?
#endif
#ifndef SET_CHILD /* <!-- !sub-type */
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#elif !defined(CAT) || !defined(PCAT) /* !sub-type --><!-- !cat */
#error SET_CHILD defined but CAT is not.
#endif /* !cat --> */
#define E_(thing) CAT(SET_NAME, thing)
#define PE_(thing) PCAT(set, PCAT(SET_NAME, thing))

#ifndef SET_UINT
#define SET_UINT unsigned
#endif

/** Valid unsigned integer type used for hash values. The hash map will
 saturate at `min(((ln 2)/2) \cdot range(<PE>UInt), (1/8) \cdot range(size_t))`,
 at which point no new buckets can be added and the load factor will increase
 over the maximum. */
typedef SET_UINT PE_(UInt);

/** Valid tag type defined by `SET_TYPE`. Included in <tag:<E>SetElement>. */
typedef SET_TYPE PE_(Type);
#ifdef SET_POINTER /* <!-- !raw */
/** `SET_POINTER` modifies `<PE>MType` to be a pointer, otherwise it's
 the same as <typedef:<PE>Type>. */
typedef const PE_(Type)* PE_(MType);
#else /* !raw --><!-- raw */
typedef PE_(Type) PE_(MType);
#endif /* raw --> */

/** A map from <typedef:<PE>MType> onto <typedef:<PE>UInt>. Should be as close
 as possible to a discrete uniform distribution while being surjective on bits
 of the argument for maximum performance. */
typedef PE_(UInt) (*PE_(Hash))(const PE_(MType));
/* Check that `SET_HASH` is a function implementing <typedef:<PE>Hash>. */
static const PE_(Hash) PE_(hash) = (SET_HASH);

/** Equivalence relation between <typedef:<PE>MType> that satisfies
 `<PE>IsEqual(a, b) -> <PE>Hash(a) == <PE>Hash(b)`. */
typedef int (*PE_(IsEqual))(const PE_(MType) a, const PE_(MType) b);
/* Check that `SET_IS_EQUAL` is a function implementing
 <typedef:<PE>IsEqual>. */
static const PE_(IsEqual) PE_(equal) = (SET_IS_EQUAL);

/** A di-predicate; returns true if the `replace` replaces the `original`; used
 in <fn:<E>SetPolicyPut>. */
typedef int (*PE_(Replace))(PE_(Type) *original, PE_(Type) *replace);

/** Used in <fn:<E>SetPolicyPut> when `replace` is null; `original` and
 `replace` are ignored. @implements <PE>Replace */
static int PE_(false)(PE_(Type) *original, PE_(Type) *replace)
	{ (void)(original); (void)(replace); return 0; }

/** Operates by side-effects. Used for `SET_TEST`. */
typedef void (*PE_(Action))(PE_(Type) *);

/** Responsible for turning the first argument into a 12-`char` null-terminated
 output string. Used for `SET_TO_STRING`. */
typedef void (*PE_(ToString))(const PE_(Type) *, char (*)[12]);

/** Contains <typedef:<PE>Type> as the first element `key`, along with data
 internal to the set. Storage of the `<E>SetElement` structure is the
 responsibility of the caller. */
struct E_(SetElement);
struct E_(SetElement) {
	PE_(Type) key;
	struct E_(SetElement) *next;
#ifndef SET_NO_CACHE /* <!-- cache */
	PE_(UInt) hash;
#endif /* cache --> */
};

/* Singly-linked list head for `buckets`. Not really needed, but
 double-pointers are confusing. */
struct PE_(Bucket) { struct E_(SetElement) *first; };

/** An `<E>Set` of `size`. To initialise, see <fn:<E>Set>, `SET_IDLE`, `{0}`
 (`C99`,) or being `static`.

 ![States.](../web/states.png) */
struct E_(Set);
struct E_(Set) {
	struct PE_(Bucket) *buckets; /* An array of 1 << log_capacity (>3) or 0. */
	size_t size;
	unsigned log_capacity;
};
#ifndef SET_IDLE /* <!-- !zero */
#define SET_IDLE { 0, 0, 0 }
#endif /* !zero --> */

/** Contains all iteration parameters in one. */
struct PE_(Iterator); struct PE_(Iterator)
	{ const struct E_(Set) *set; size_t b; struct E_(SetElement) *e; };

/** Initialises `set` to idle. */
static void PE_(set)(struct E_(Set) *const set)
	{ assert(set); set->buckets = 0; set->log_capacity = 0; set->size = 0; }

/** Destroys `set` and returns it to idle. */
static void PE_(set_)(struct E_(Set) *const set)
	{ assert(set), free(set->buckets), PE_(set)(set); }

#ifdef SET_POINTER /* <!-- !raw */
/** @return `element`. */
static const PE_(Type) *PE_(pointer)(const PE_(Type) *const element)
	{ return element; }
#else /* !raw --><!-- raw */
/** @return Re-de-reference `element`. */
static PE_(Type) PE_(pointer)(const PE_(Type) *const element)
	{ return *element; }
#endif /* raw --> */

/** Gets the hash of `element`, which should be consistant. */
static PE_(UInt) PE_(get_hash)(struct E_(SetElement) *element) {
	assert(element);
#ifdef SET_NO_CACHE /* <!-- !cache */
	return PE_(hash)(PE_(pointer)(&element->key));
#else /* !cache --><!-- cache */
	return element->hash;
#endif /* cache --> */
}

/** Retrieves a bucket from `set` given the `hash`. Only call this function if
 non-empty. May be invalidated upon a call to <fn:<PE>grow>.
 @return Given a `hash`, compute the bucket. */
static struct PE_(Bucket) *PE_(get_bucket)(struct E_(Set) *const set,
	const PE_(UInt) hash) {
	assert(set && set->buckets);
	return set->buckets + (hash & ((1 << set->log_capacity) - 1));
}

/** Linear search for `data` in `bucket`.
 @param[hash] Must match the hash of `data`.
 @return The link that points to the `data` or null. */
static struct E_(SetElement) **PE_(bucket_to)(struct PE_(Bucket) *const bucket,
	const PE_(UInt) hash, const PE_(MType) data) {
	struct E_(SetElement) **to_x, *x;
	assert(bucket);
	for(to_x = &bucket->first; (x = *to_x); to_x = &x->next) {
#ifndef SET_NO_CACHE /* <!-- cache: a quick out. */
		if(hash != x->hash) continue;
#endif /* cache --> */
		if(PE_(equal)(data, PE_(pointer)(&x->key))) return to_x;
	}
#ifdef SET_NO_CACHE /* <!-- !cache */
	(void)(hash);
#endif /* cache --> */
	return 0;
}

/** Ensures `min_capacity` (`\times ln 2`) of `a`.
 @param[min_capacity] If zero, does nothing.
 @return Success; otherwise, `errno` will be set. @throws[ERANGE] Tried
 allocating more then can fit in `size_t` or `realloc` doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html).
 @throws[realloc] */
static int PE_(reserve)(struct E_(Set) *const set, const size_t size) {
	struct PE_(Bucket) *buckets, *b, *b_end, *new_b;
	struct E_(SetElement) **to_x, *x;
	const unsigned log_c0 = set->log_capacity,
		log_limit = sizeof(PE_(UInt)) * 8 - 1;
	unsigned log_c1;
	PE_(UInt) c0 = 1 << log_c0, c1, mask;
	size_t no_buckets;
	/* One did set `<PE>UInt` to an unsigned type, right? */
	assert(set && c0 && log_c0 <= log_limit && (log_c0 >= 3 || !log_c0)
		&& (PE_(UInt))-1 > 0);
	/* `SIZE_MAX` min 65535 (`C99`) -> 5041 but typically much larger _st_ it
	 becomes saturated while the load factor increases. */
	if(size > (size_t)-1 / 13) return 1; /* <- Saturation `1/8 * SIZE_MAX`. */
	/* Load factor `0.693147180559945309417232121458176568 ~= 9/13`.
	 Starting bucket number is a power of 2 in `[8, 1 << log_limit]`. */
	if((no_buckets = size * 13 / 9) > 1u << log_limit) {
		log_c1 = log_limit;
		c1 = 1 << log_limit;
	} else {
		if(log_c0 < 3) log_c1 = 3u,     c1 = 8u;
		else           log_c1 = log_c0, c1 = c0;
		while(c1 < no_buckets) log_c1++, c1 <<= 1;
	}
	/* It's under the critical load factor; don't need to do anything. */
	if(log_c0 == log_c1) return 1;
	/* Everything else is amortised. Allocate new space for an expansion. */
	if(!(buckets = realloc(set->buckets, sizeof *buckets * c1)))
		{ if(!errno) errno = ERANGE; return 0; }
	set->buckets = buckets;
	set->log_capacity = log_c1;
	/* The mask needs domain `c0 \in [1, max]`, but we want 0 for loops. */
	mask = (c1 - 1) ^ (c0 - 1), assert(mask);
	if(c0 == 1) c0 = 0, assert(!c0 || c0 >= 8);
	/* Initialize second part (new) lists to contain no elements. */
	for(b = buckets + c0, b_end = buckets + c1; b < b_end; b++) b->first = 0;
	/* Expectation value of rehashing an entry is half. */
	for(b = buckets, b_end = buckets + c0; b < b_end; b++) {
		PE_(UInt) hash;
		assert(!((b - buckets) & mask));
		to_x = &b->first;
		while(*to_x) {
			hash = PE_(get_hash)((x = *to_x));
			if(!(hash & mask)) { to_x = &x->next; continue; }
			*to_x = x->next; /* Remove. */
			new_b = PE_(get_bucket)(set, hash);
			x->next = new_b->first, new_b->first = x;
		}
	}
	return 1;
}

/** Most general put function that every put function calls. Puts `element` in
 `set` and returns the collided element, if any, as long as `replace` is null
 or returns true. If `replace` returns false, returns `element`. */
static struct E_(SetElement) *PE_(put)(struct E_(Set) *const set,
	struct E_(SetElement) *const element, const PE_(Replace) replace) {
	struct PE_(Bucket) *bucket;
	struct E_(SetElement) **to_x = 0, *x = 0;
	PE_(UInt) hash;
	if(!set || !element) return 0;
	hash = PE_(hash)(PE_(pointer)(&element->key));
#ifndef SET_NO_CACHE /* <!-- cache */
	element->hash = hash;
#endif /* cache --> */
	if(!set->buckets) goto grow_table;
	/* Delete any duplicate. */
	bucket = PE_(get_bucket)(set, hash);
	if(!(to_x = PE_(bucket_to)(bucket, hash, PE_(pointer)(&element->key))))
		goto grow_table;
	x = *to_x;
	if(replace && !replace(&x->key, &element->key)) return element;
	*to_x = x->next, x->next = 0;
	goto add_element;
grow_table:
	assert(set->size + 1 > set->size);
	/* Didn't <fn:<E>SetReserve>, now one can't tell error except `errno`. */
	if(!PE_(reserve)(set, set->size + 1)) return 0;
	bucket = PE_(get_bucket)(set, hash);
	set->size++;
add_element:
	element->next = bucket->first, bucket->first = element;
	return x;
}

#ifndef SET_CHILD /* <!-- !sub-type */

/** Initialises `set` to be take no memory and be in an idle state. Calling
 this on an active set will cause memory leaks.
 @param[set] If null, does nothing. @order \Theta(1) @allow */
static void E_(Set)(struct E_(Set) *const set) { if(set) PE_(set)(set); }

/** Destructor for active `set`. After, it takes no memory and is in an idle
 state. If idle, does nothing. @allow */
static void E_(Set_)(struct E_(Set) *const set) { if(set) PE_(set_)(set); }

/** Clears and removes all entries from `set`. The capacity and memory of the
 hash table is preserved, but all previous values are un-associated. The load
 factor will be less until it reaches it's previous size.
 @param[set] If null, does nothing. @order \Theta(`set.buckets`) @allow */
static void E_(SetClear)(struct E_(Set) *const set) {
	struct PE_(Bucket) *b, *b_end;
	if(!set || !set->log_capacity) return;
	for(b = set->buckets, b_end = b + (1 << set->log_capacity); b < b_end; b++)
		b->first = 0;
	set->size = 0;
}

/** @return The value in `set` which <typedef:<PE>IsEqual> `SET_IS_EQUAL`
 `data`, or, if no such value exists, null.
 @param[set] If null, returns null. @order Average \O(1), (hash distributes
 elements uniformly); worst \O(n). @allow */
static struct E_(SetElement) *E_(SetGet)(struct E_(Set) *const set,
	const PE_(MType) data) {
	struct E_(SetElement) **to_x;
	PE_(UInt) hash;
	if(!set || !set->buckets) return 0;
	hash = PE_(hash)(data);
	to_x = PE_(bucket_to)(PE_(get_bucket)(set, hash), hash, data);
	return to_x ? *to_x : 0;
}

/** Reserve at least `reserve`, divided by the maximum load factor, space in
 the buckets of `set`. @return Success.
 @throws[ERANGE] `reserve` plus the size would take a bigger number then could
 fit in a `size_t`. @throws[realloc] @allow */
static int E_(SetReserve)(struct E_(Set) *const set, const size_t reserve)
	{ return set ? reserve > (size_t)-1 - set->size ? (errno = ERANGE, 0) :
	PE_(reserve)(set, set->size + reserve) : 0; }

/** Puts the `element` in `set`.
 @param[set, element] If null, returns null. @param[element] Should not be of a
 set because the integrity of that set will be compromised.
 @return Any ejected element or null. (An ejected element has
 <typedef:<PE>IsEqual> `SET_IS_EQUAL` the `element`.)
 @throws[realloc, ERANGE] There was an error with a re-sizing. Successfully
 calling <fn:<E>SetReserve> with at least one before ensures that this does not
 happen. @order Average amortised \O(1), (hash distributes elements uniformly);
 worst \O(n). @allow */
static struct E_(SetElement) *E_(SetPut)(struct E_(Set) *const set,
	struct E_(SetElement) *const element) { return PE_(put)(set, element, 0); }

/** Puts the `element` in `set` only if the entry is absent or if calling
 `replace` returns true.
 @param[set, element] If null, returns null.
 @param[element] Should not be of a set because the integrity of that set will
 be compromised.
 @param[replace] Called on collision and only replaces it if the function
 returns true. If null, doesn't do any replacement on collision.
 @return Any ejected element or null. On collision, if `replace` returns false
 or `replace` is null, returns `element` and leaves the other element in the
 set. @throws[realloc, ERANGE] There was an error with a re-sizing.
 Successfully calling <fn:<E>SetReserve> with at least one before ensures that
 this does not happen. @order Average amortised \O(1), (hash distributes
 elements uniformly); worst \O(n). @allow */
static struct E_(SetElement) *E_(SetPolicyPut)(struct E_(Set) *const set,
	struct E_(SetElement) *const element, const PE_(Replace) replace)
	{ return PE_(put)(set, element, replace ? replace : &PE_(false)); }

/** Removes an element `data` from `set`.
 @return Successfully ejected element or null. This element is free to be put
 into another set or modify it's hash values. @order Average \O(1), (hash
 distributes elements uniformly); worst \O(n). @allow */
static struct E_(SetElement) *E_(SetRemove)(struct E_(Set) *const set,
	const PE_(MType) data) {
	PE_(UInt) hash;
	struct E_(SetElement) **to_x, *x;
	if(!set || !set->buckets) return 0;
	hash = PE_(hash)(data);
	if(!(to_x = PE_(bucket_to)(PE_(get_bucket)(set, hash), hash, data)))
		return 0;
	x = *to_x;
	*to_x = x->next;
	assert(set->size);
	set->size--;
	return x;
}

#endif /* !sub-type --> */

static void PE_(unused_base_coda)(void);
static void PE_(unused_base)(void) {
	PE_(false)(0, 0); PE_(set)(0), PE_(set_)(0); PE_(put)(0, 0, 0);
#ifndef SET_CHILD /* <!-- !sub-type */
	E_(Set)(0); E_(Set_)(0); E_(SetClear)(0); E_(SetGet)(0, 0);
	E_(SetReserve)(0, 0); E_(SetPut)(0, 0); E_(SetPolicyPut)(0, 0, 0);
	E_(SetRemove)(0, 0);
#endif /* !sub-type --> */
	PE_(unused_base_coda)();
}
static void PE_(unused_base_coda)(void) { PE_(unused_base)(); }


#elif defined(SET_TO_STRING) /* base code --><!-- to string trait */


#if !defined(E_) || !defined(PE_) || !defined(CAT) \
	|| !defined(CAT_) || !defined(PCAT) || !defined(PCAT_)
#error P?E_? or P?CAT_? not yet defined in to string trait; include set?
#endif

#include <string.h> /* strlen memcpy */


#ifdef SET_TO_STRING_NAME /* <!-- name */
#define PEA_(thing) PCAT(PE_(thing), SET_TO_STRING_NAME)
#define E_A_(thing1, thing2) CAT(E_(thing1), CAT(SET_TO_STRING_NAME, thing2))
#else /* name --><!-- !name */
#define PEA_(thing) PCAT(PE_(thing), anonymous)
#define E_A_(thing1, thing2) CAT(E_(thing1), thing2)
#endif /* !name --> */

/* Check that `SET_TO_STRING` is a function implementing
 <typedef:<PE>ToString>. */
static const PE_(ToString) PEA_(to_str12) = (SET_TO_STRING);

/** Writes `it` to `str` and advances or returns false.
 @implements <AI>NextToString */
static int PEA_(next_to_str12)(struct PE_(Iterator) *const it,
	char (*const str)[12]) {
	const size_t b_end = 1 << it->set->log_capacity;
	assert(it && it->set && str);
	if(!it->set->buckets) return 0;
	while(it->b < b_end) {
		if(!it->e) it->e = it->set->buckets[it->b].first;
		else it->e = it->e->next;
		if(it->e) goto next;
		it->b++;
	}
	it->set = 0, it->b = 0, it->e = 0;
	return 0;
next:
	PEA_(to_str12)(&it->e->key, (char (*)[12])str);
	return 1;
}

/** @return If `it` contains not-null. */
static int PEA_(is_valid)(const struct PE_(Iterator) *const it)
	{ assert(it); return !!it->set; }

#define AI_ PEA_
#define TO_STRING_ITERATOR struct PE_(Iterator)
#define TO_STRING_NEXT &PEA_(next_to_str12)
#define TO_STRING_IS_VALID &PEA_(is_valid)
#include "ToString.h"

/** @return Prints `set`. */
static const char *PEA_(to_string)(const struct E_(Set) *const set) {
	struct PE_(Iterator) it = { 0, 0, 0 };
	it.set = set; /* Can be null. */
	return PEA_(iterator_to_string)(&it, '{', '}'); /* In ToString. */
}

#ifndef SET_CHILD /* <!-- !sub-type */

/** @return Print the contents of `a` in a static string buffer with the
 limitations of `ToString.h`. @order \Theta(1) @allow */
static const char *E_A_(Set, ToString)(const struct E_(Set) *const set)
	{ return PEA_(to_string)(set); /* Can be null. */ }

#endif /* !sub-type --> */

static void PEA_(unused_to_string_coda)(void);
static void PEA_(unused_to_string)(void) {
	PEA_(to_string)(0);
#ifndef SET_CHILD /* <!-- !sub-type */
	E_A_(Set, ToString)(0);
#endif /* !sub-type --> */
	PEA_(unused_to_string_coda)();
}
static void PEA_(unused_to_string_coda)(void) { PEA_(unused_to_string)(); }

#if !defined(SET_TEST_BASE) && defined(SET_TEST) /* <!-- test */
#define SET_TEST_BASE /* Only one instance of base tests. */
#include "../test/TestSet.h" /** \include */
#endif /* test --> */

#undef PEA_
#undef E_A_
#undef SET_TO_STRING
#ifdef SET_TO_STRING_NAME
#undef SET_TO_STRING_NAME
#endif


#endif /* traits --> */


#ifdef SET_EXPECT_TRAIT /* <!-- trait */
#undef SET_EXPECT_TRAIT
#else /* trait --><!-- !trait */
#ifndef SET_CHILD /* <!-- !sub-type */
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#else /* !sub-type --><!-- sub-type */
#undef SET_CHILD
#endif /* sub-type --> */
#undef E_
#undef PE_
#undef SET_NAME
#undef SET_TYPE
#undef SET_UINT
#undef SET_HASH
#undef SET_IS_EQUAL
#ifdef SET_TO_STRING
#undef SET_TO_STRING
#endif
#ifdef SET_POINTER
#undef SET_POINTER
#endif
#ifdef SET_NO_CACHE
#undef SET_NO_CACHE
#endif
#ifdef SET_TEST
#undef SET_TEST
#endif
#ifdef SET_TEST_BASE
#undef SET_TEST_BASE
#endif
#endif /* !trait --> */

#undef SET_TO_STRING_TRAIT
#undef SET_TRAITS
