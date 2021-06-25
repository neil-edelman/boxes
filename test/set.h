/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Hash Set

 ![Example of <String>Set.](../web/set.png)

 <tag:<S>set> is a collection of elements of <tag:<S>set_node> that doesn't
 allow duplication; it must be supplied an equality function, `SET_IS_EQUAL`
 <typedef:<PS>is_equal_fn>, and a hash function, `SET_HASH`
 <typedef:<PS>hash_fn>.

 Internally, it is a separately chained hash table with a maximum load factor
 of `ln 2`, power-of-two resizes, with buckets as a forward linked list of
 <tag:<S>set_node>. This offers some independence of sets from set elements.
 It can be expanded to an associative array by enclosing the `<S>set_node` in
 another `struct`.

 @param[SET_NAME, SET_TYPE]
 `<S>` that satisfies `C` naming conventions when mangled and a valid
 <typedef:<PS>type> associated therewith; required. `<PS>` is private, whose
 names are prefixed in a manner to avoid collisions; any should be re-defined
 prior to use elsewhere.

 @param[SET_HASH]
 A function satisfying <typedef:<PS>hash_fn>; required.

 @param[SET_IS_EQUAL]
 A function satisfying <typedef:<PS>is_equal_fn>; required.

 @param[SET_POINTER]
 Usually <typedef:<PS>mtype> in the same as <typedef:<PS>type> for simple
 `SET_TYPE`, but this flag makes `<PS>mtype` be a pointer-to-`<PS>type`. This
 affects <typedef:<PS>hash_fn>, <typedef:<PS>is_equal_fn>, and <fn:<S>set_get>,
 making them accept a pointer-to-const-`<S>` instead of a copy of `<S>`.

 @param[SET_UINT]
 This is <typedef:<PS>uint> and defaults to `unsigned int`; use when
 <typedef:<PS>hash_fn> is a specific hash length.

 @param[SET_NO_CACHE]
 Calculates the hash every time and discards it; should be used when the hash
 calculation is trivial to avoid storing duplicate <typedef:<PS>uint> _per_
 datum, (in rare cases.)

 @param[SET_ITERATE]
 Satisfies the <iterate.h> interface for forwards iteration in original
 inclusion.

 @param[SET_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[SET_TO_STRING]
 To string trait contained in <to_string.h>; `<Z>` that satisfies `C` naming
 conventions when mangled and function implementing <typedef:<PZ>to_string_fn>.
 There can be multiple to string traits, but only one can omit
 `SET_TO_STRING_NAME`.

 @param[SET_TEST]
 To string trait contained in <../test/set_test.h>; optional unit testing
 framework using `assert`. Can only be defined once _per_ set. Must be defined
 equal to a (random) filler function, satisfying <typedef:<PS>action_fn>.
 Output will be shown with the to string trait in which it's defined; provides
 tests for the base code and all later traits.

 @std C89 */

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
#ifndef SET_ITERATE
#error SET_ITERATE must be defined for string trait.
#endif
#define SET_TO_STRING_TRAIT 1
#else
#define SET_TO_STRING_TRAIT 0
#endif
#define SET_TRAITS SET_TO_STRING_TRAIT
#if SET_TRAITS > 1
#error Only one trait per include is allowed; use SET_EXPECT_TRAIT.
#endif
#if SET_TRAITS != 0 && (!defined(S_) || !defined(CAT) || !defined(CAT_))
#error S_ or CAT_? not yet defined; use SET_EXPECT_TRAIT.
#endif
#if (SET_TRAITS == 0) && defined(SET_TEST)
#error SET_TEST must be defined in SET_TO_STRING trait.
#endif
#if defined(SET_TO_STRING_NAME) && !defined(SET_TO_STRING)
#error SET_TO_STRING_NAME requires SET_TO_STRING.
#endif


#if SET_TRAITS == 0 /* <!-- base code */


/* <Kernighan and Ritchie, 1988, p. 231>. */
#if defined(S_) || defined(PS_) \
	|| (defined(SET_SUBTYPE) ^ (defined(CAT) || defined(CAT_)))
#error Unexpected P?S_ or CAT_?; possible stray SET_EXPECT_TRAIT?
#endif
#ifndef SET_SUBTYPE /* <!-- !sub-type */
#define CAT_(x, y) x ## _ ## y
#define CAT(x, y) CAT_(x, y)
#endif /* !sub-type --> */
#define S_(thing) CAT(SET_NAME, thing)
#define PS_(thing) CAT(set, CAT(SET_NAME, thing))
#ifndef SET_UINT
#define SET_UINT unsigned
#endif

/** Valid unsigned integer type used for hash values. The hash map will
 saturate at `min(((ln 2)/2) \cdot range(<PS>uint), (1/8) \cdot range(size_t))`,
 at which point no new buckets can be added and the load factor will increase
 over the maximum. */
typedef SET_UINT PS_(uint);

/** Valid tag type defined by `SET_TYPE`. Included in <tag:<S>set_node>. */
typedef SET_TYPE PS_(type);
#ifdef SET_POINTER /* <!-- !raw */
/** `SET_POINTER` modifies `<PS>mtype` to be a pointer, otherwise it's
 the same as <typedef:<PS>type>. */
typedef const PS_(type)* PS_(mtype);
#else /* !raw --><!-- raw */
typedef PS_(type) PS_(mtype);
#endif /* raw --> */

/** A map from <typedef:<PS>mtype> onto <typedef:<PS>uint>. Should be as close
 as possible to a discrete uniform distribution while using all argument for
 maximum performance. */
typedef PS_(uint) (*PS_(hash_fn))(const PS_(mtype));
/* Check that `SET_HASH` is a function implementing <typedef:<PS>hash_fn>. */
static const PS_(hash_fn) PS_(hash) = (SET_HASH);

/** Equivalence relation between <typedef:<PS>mtype> that satisfies
 `<PS>is_equal_fn(a, b) -> <PS>hash_fn(a) == <PS>hash_fn(b)`. */
typedef int (*PS_(is_equal_fn))(const PS_(mtype) a, const PS_(mtype) b);
/* Check that `SET_IS_EQUAL` is a function implementing
 <typedef:<PS>is_equal_fn>. */
static const PS_(is_equal_fn) PS_(equal) = (SET_IS_EQUAL);

/** A di-predicate; returns true if the `replace` replaces the `original`. */
typedef int (*PS_(replace_fn))(PS_(type) *original, PS_(type) *replace);

/** Used in <fn:<S>set_policy_put> when `replace` is null; `original` and
 `replace` are ignored. @implements `<PS>replace_fn` */
static int PS_(false)(PS_(type) *original, PS_(type) *replace)
	{ (void)(original); (void)(replace); return 0; }

/** Contains <typedef:<PS>type> as the first element `key`, along with data
 internal to the set. Storage of the `<S>set_node` structure is the
 responsibility of the caller. */
struct S_(set_node);
struct S_(set_node) {
	PS_(type) key; /* This should be next, but offsetof key in set_node 0. */
	struct S_(set_node) *next;
#ifndef SET_NO_CACHE /* <!-- cache */
	PS_(uint) hash;
#endif /* cache --> */
};

/* Singly-linked list head for `buckets`. Not really needed, but
 double-pointers are confusing. */
struct PS_(bucket) { struct S_(set_node) *first; };

/** An `<S>set` of `size`. To initialise, see <fn:<S>set>, `SET_IDLE`, `{0}`
 (`C99`,) or being `static`.

 ![States.](../web/states.png) */
struct S_(set);
struct S_(set) {
	struct PS_(bucket) *buckets; /* An array of 1 << log_capacity (>3) or 0. */
	size_t size;
	unsigned log_capacity;
};
#ifndef SET_IDLE /* <!-- !zero */
#define SET_IDLE { 0, 0, 0 }
#endif /* !zero --> */

#ifdef SET_POINTER /* <!-- !raw */
/** @return `element`. */
static const PS_(type) *PS_(pointer)(const PS_(type) *const element)
	{ return element; }
#else /* !raw --><!-- raw */
/** @return Re-de-reference `element`. */
static PS_(type) PS_(pointer)(const PS_(type) *const element)
	{ return *element; }
#endif /* raw --> */

/** Gets the hash of `element`, which should be consistant. */
static PS_(uint) PS_(get_hash)(struct S_(set_node) *element) {
	assert(element);
#ifdef SET_NO_CACHE /* <!-- !cache */
	return PS_(hash)(PS_(pointer)(&element->key));
#else /* !cache --><!-- cache */
	return element->hash;
#endif /* cache --> */
}

/** Retrieves a bucket from `set` given the `hash`. Only call this function if
 non-empty. May be invalidated upon a call to <fn:<PS>reserve>.
 @return Given a `hash`, compute the bucket. */
static struct PS_(bucket) *PS_(get_bucket)(struct S_(set) *const set,
	const PS_(uint) hash) {
	assert(set && set->buckets);
	return set->buckets + (hash & ((1 << set->log_capacity) - 1));
}

/** Linear search for `data` in `bucket`.
 @param[hash] Must match the hash of `data`.
 @return The link that points to the `data` or null. */
static struct S_(set_node) **PS_(bucket_to)(struct PS_(bucket) *const bucket,
	const PS_(uint) hash, const PS_(mtype) data) {
	struct S_(set_node) **to_x, *x;
	assert(bucket);
	for(to_x = &bucket->first; (x = *to_x); to_x = &x->next) {
#ifndef SET_NO_CACHE /* <!-- cache: a quick out. */
		if(hash != x->hash) continue;
#endif /* cache --> */
		if(PS_(equal)(data, PS_(pointer)(&x->key))) return to_x;
	}
#ifdef SET_NO_CACHE /* <!-- !cache */
	(void)(hash);
#endif /* cache --> */
	return 0;
}

/** Ensures `min_capacity` (`\times ln^-1 2`) of the buckets of `set`.
 @param[min_capacity] If zero, does nothing.
 @return Success; otherwise, `errno` will be set. @throws[ERANGE] Tried
 allocating more then can fit in `size_t` or `realloc` doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html).
 @throws[realloc] */
static int PS_(reserve)(struct S_(set) *const set, const size_t min_capacity) {
	struct PS_(bucket) *buckets, *b, *b_end, *new_b;
	struct S_(set_node) **to_x, *x;
	const unsigned log_c0 = set->log_capacity,
		log_limit = sizeof(PS_(uint)) * 8 - 1;
	unsigned log_c1;
	PS_(uint) c0 = (PS_(uint))(1 << log_c0), c1, mask;
	size_t no_buckets;
	/* One did set `<PS>uint` to an unsigned type, right? */
	assert(set && c0 && log_c0 <= log_limit && (log_c0 >= 3 || !log_c0)
		&& (PS_(uint))-1 > 0);
	/* `SIZE_MAX` min 65535 (`C99`) -> 5041 but typically much larger _st_ it
	 becomes saturated while the load factor increases.
	 Saturation `1/8 * SIZE_MAX`, (which is not defined in `C90`.) */
	if(min_capacity > (size_t)-1 / 13) return 1;
	/* Load factor `ln 2 ~= 0.693 ~= 9/13`.
	 Starting bucket number is a power of 2 in `[8, 1 << log_limit]`. */
	if((no_buckets = min_capacity * 13 / 9) > 1u << log_limit) {
		log_c1 = log_limit;
		c1 = 1u << log_limit;
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
		PS_(uint) hash;
		assert(!((b - buckets) & mask));
		to_x = &b->first;
		while(*to_x) {
			hash = PS_(get_hash)((x = *to_x));
			if(!(hash & mask)) { to_x = &x->next; continue; }
			*to_x = x->next; /* Remove. */
			new_b = PS_(get_bucket)(set, hash);
			x->next = new_b->first, new_b->first = x;
		}
	}
	return 1;
}

/** General put, `element` in `set` and returns the collided element, if any,
 as long as `replace` is null or returns true. If `replace` returns false,
 returns `element`. */
static struct S_(set_node) *PS_(put)(struct S_(set) *const set,
	struct S_(set_node) *const element, const PS_(replace_fn) replace) {
	struct PS_(bucket) *bucket;
	struct S_(set_node) **to_x = 0, *x = 0;
	PS_(uint) hash;
	assert(set && element);
	hash = PS_(hash)(PS_(pointer)(&element->key));
#ifndef SET_NO_CACHE /* <!-- cache */
	element->hash = hash;
#endif /* cache --> */
	if(!set->buckets) goto grow_table;
	/* Delete any duplicate. */
	bucket = PS_(get_bucket)(set, hash);
	if(!(to_x = PS_(bucket_to)(bucket, hash, PS_(pointer)(&element->key))))
		goto grow_table;
	x = *to_x;
	if(replace && !replace(&x->key, &element->key)) return element;
	*to_x = x->next, x->next = 0;
	goto add_element;
grow_table:
	assert(set->size + 1 > set->size);
	/* Didn't <fn:<PS>reserve>, now one can't tell error except `errno`. */
	if(!PS_(reserve)(set, set->size + 1)) return 0;
	bucket = PS_(get_bucket)(set, hash);
	set->size++;
add_element:
	element->next = bucket->first, bucket->first = element;
	return x;
}

/** Initialises `set` to idle. @order \Theta(1) @allow */
static void S_(set)(struct S_(set) *const set)
	{ assert(set); set->buckets = 0; set->log_capacity = 0; set->size = 0; }

/** Destroys `set` and returns it to idle. @allow */
static void S_(set_)(struct S_(set) *const set)
	{ assert(set), free(set->buckets), S_(set)(set); }

/** Clears and removes all entries from `set`. The capacity and memory of the
 hash table is preserved, but all previous values are un-associated. The load
 factor will be less until it reaches it's previous size.
 @param[set] If null, does nothing. @order \Theta(`set.buckets`) @allow */
static void S_(set_clear)(struct S_(set) *const set) {
	struct PS_(bucket) *b, *b_end;
	assert(set);
	if(!set->log_capacity) return;
	for(b = set->buckets, b_end = b + (1 << set->log_capacity); b < b_end; b++)
		b->first = 0;
	set->size = 0;
}

/** @return The value in `set` which <typedef:<PS>is_equal_fn> `SET_IS_EQUAL`
 `data`, or, if no such value exists, null.
 @param[set] If null, returns null. @order Average \O(1), (hash distributes
 elements uniformly); worst \O(n). @allow */
static struct S_(set_node) *S_(set_get)(struct S_(set) *const set,
	const PS_(mtype) data) {
	struct S_(set_node) **to_x;
	PS_(uint) hash;
	if(!set || !set->buckets) return 0;
	hash = PS_(hash)(data);
	to_x = PS_(bucket_to)(PS_(get_bucket)(set, hash), hash, data);
	return to_x ? *to_x : 0;
}

/** Reserve at least `reserve`, divided by the maximum load factor, space in
 the buckets of `set`. @return Success.
 @throws[ERANGE] `reserve` plus the size would take a bigger number then could
 fit in a `size_t`. @throws[realloc] @allow */
static int S_(set_reserve)(struct S_(set) *const set, const size_t reserve)
	{ return set ? reserve > (size_t)-1 - set->size ? (errno = ERANGE, 0) :
	PS_(reserve)(set, set->size + reserve) : 0; }

/** Puts the `element` in `set`.
 @param[set, element] If null, returns null. @param[element] Should not be of a
 set because the integrity of that set will be compromised.
 @return Any ejected element or null. (An ejected element has
 <typedef:<PS>is_equal_fn> `SET_IS_EQUAL` the `element`.)
 @throws[realloc, ERANGE] There was an error with a re-sizing. Successfully
 calling <fn:<S>set_reserve> with at least one before ensures that this does
 not happen. @order Average amortised \O(1), (hash distributes elements
 uniformly); worst \O(n). @allow */
static struct S_(set_node) *S_(set_put)(struct S_(set) *const set,
	struct S_(set_node) *const element) { return PS_(put)(set, element, 0); }

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
 Successfully calling <fn:<S>set_reserve> with at least one before ensures that
 this does not happen. @order Average amortised \O(1), (hash distributes
 elements uniformly); worst \O(n). @allow */
static struct S_(set_node) *S_(set_policy_put)(struct S_(set) *const set,
	struct S_(set_node) *const element, const PS_(replace_fn) replace)
	{ return PS_(put)(set, element, replace ? replace : &PS_(false)); }

/** Removes an element `data` from `set`.
 @return Successfully ejected element or null. This element is free to be put
 into another set or modify it's hash values. @order Average \O(1), (hash
 distributes elements uniformly); worst \O(n). @allow */
static struct S_(set_node) *S_(set_remove)(struct S_(set) *const set,
	const PS_(mtype) data) {
	PS_(uint) hash;
	struct S_(set_node) **to_x, *x;
	if(!set || !set->buckets) return 0;
	hash = PS_(hash)(data);
	if(!(to_x = PS_(bucket_to)(PS_(get_bucket)(set, hash), hash, data)))
		return 0;
	x = *to_x;
	*to_x = x->next;
	assert(set->size);
	set->size--;
	return x;
}

#ifdef SET_ITERATE /* <!-- iterate */

/** Contains all iteration parameters. */
struct PS_(iterator);
struct PS_(iterator)
	{ const struct S_(set) *set; size_t b; struct S_(set_node) *e; };

/** Loads `set` into `it`. @implements begin */
static void PS_(begin)(struct PS_(iterator) *const it,
	const struct S_(set) *const set)
	{ assert(it && set), it->set = set, it->b = 0, it->e = 0; }

/** Advances `it`. @implements next */
static const PS_(type) *PS_(next)(struct PS_(iterator) *const it) {
	const size_t b_end = 1 << it->set->log_capacity;
	assert(it && it->set);
	if(!it->set->buckets) return 0;
	while(it->b < b_end) {
		if(!it->e) it->e = it->set->buckets[it->b].first;
		else it->e = it->e->next;
		if(it->e) return &it->e->key; /*???*/
		it->b++;
	}
	it->set = 0, it->b = 0, it->e = 0;
	return 0;
}

#define SET_FORWARD_(n) CAT(S_(set_forward), n)
#define I_ SET_FORWARD_
#define ITERATE struct PS_(iterator)
#define ITERATE_BOX struct S_(set)
#define ITERATE_TYPE PS_(type)
#define ITERATE_BEGIN PS_(begin)
#define ITERATE_NEXT PS_(next)
#include "iterate.h" /** \include */

#endif /* iterate --> */

static void PS_(unused_base_coda)(void);
static void PS_(unused_base)(void) {
	S_(set)(0); S_(set_)(0); S_(set_clear)(0); S_(set_get)(0, 0);
	S_(set_reserve)(0, 0); S_(set_put)(0, 0);  S_(set_policy_put)(0, 0, 0);
	S_(set_remove)(0, 0);
#ifdef ARRAY_ITERATE
	PS_(begin)(0, 0); PS_(next)(0);
#endif
	PS_(unused_base_coda)();
}
static void PS_(unused_base_coda)(void) { PS_(unused_base)(); }


#elif defined(SET_TO_STRING) /* base code --><!-- to string trait */


#define TO_STRING SET_TO_STRING
#define ZI_ SET_FORWARD_
#ifdef SET_TO_STRING_NAME /* <!-- name */
#define Z_(n) CAT(S_(set), CAT(SET_TO_STRING_NAME, n))
#else /* name --><!-- !name */
#define Z_(n) CAT(S_(set), n)
#endif /* !name --> */
#define TO_STRING_LEFT '{'
#define TO_STRING_RIGHT '}'
#include "to_string.h" /** \include */

#if !defined(SET_TEST_BASE) && defined(SET_TEST) /* <!-- test */
#define SET_TEST_BASE /* Only one instance of base tests. */
#include "../test/test_set.h" /** \include */
#endif /* test --> */

#undef Z_ /* From <to_string.h>. */
#undef SET_TO_STRING
#ifdef SET_TO_STRING_NAME
#undef SET_TO_STRING_NAME
#endif


#endif /* traits --> */


#ifdef SET_EXPECT_TRAIT /* <!-- trait */
#undef SET_EXPECT_TRAIT
#else /* trait --><!-- !trait */
#ifndef SET_SUBTYPE /* <!-- !sub-type */
#undef CAT
#undef CAT_
#else /* !sub-type --><!-- sub-type */
#undef SET_SUBTYPE
#endif /* sub-type --> */
#ifdef SET_ITERATE /* <!-- iter */
#undef SET_FORWARD_
#undef SET_ITERATE
#endif /* iter --> */
#undef S_
#undef PS_
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
#undef ITERATE
#undef ITERATE_BOX
#undef ITERATE_TYPE
#undef ITERATE_BEGIN
#undef ITERATE_NEXT
#endif /* !trait --> */
#undef SET_TO_STRING_TRAIT
#undef SET_TRAITS
