/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Hash Set

 ![Example of <String>Set.](../web/set.png)

 <tag:<E>set> is a collection of elements of <tag:<E>set_node> that doesn't
 allow duplication; it must be supplied an equality function, `SET_IS_EQUAL`
 <typedef:<PE>is_equal_fn>, and a hash function, `SET_HASH`
 <typedef:<PE>hash_fn>.

 Internally, it is a separately chained hash table with a maximum load factor
 of `ln 2`, power-of-two resizes, with buckets as a forward linked list of
 <tag:<E>set_node>. This offers some independence of sets from set elements.
 It can be expanded to an associative array by enclosing the `<E>set_node` in
 another `struct`.

 @param[SET_NAME, SET_TYPE]
 `<E>` that satisfies `C` naming conventions when mangled and a valid
 <typedef:<PE>type> associated therewith; required. `<PE>` is private, whose
 names are prefixed in a manner to avoid collisions; any should be re-defined
 prior to use elsewhere.

 @param[SET_HASH]
 A function satisfying <typedef:<PE>hash_fn>; required.

 @param[SET_IS_EQUAL]
 A function satisfying <typedef:<PE>is_equal_fn>; required.

 @param[SET_POINTER]
 Usually <typedef:<PE>mtype> in the same as <typedef:<PE>type> for simple
 `SET_TYPE`, but this flag makes `<PE>mtype` be a pointer-to-`<PE>type`. This
 affects <typedef:<PE>hash_fn>, <typedef:<PE>is_equal_fn>, and <fn:<E>set_get>,
 making them accept a pointer-to-const-`<E>` instead of a copy of `<E>`.

 @param[SET_UINT]
 This is <typedef:<PE>uint> and defaults to `unsigned int`; use when
 <typedef:<PE>hash_fn> is a specific hash length.

 @param[SET_NO_CACHE]
 Calculates the hash every time and discards it; should be used when the hash
 calculation is trivial to avoid storing duplicate <typedef:<PE>uint> _per_
 datum, (in rare cases.)

 @param[SET_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[SET_TO_STRING]
 To string trait contained in <to_string.h>; `<S>` that satisfies `C` naming
 conventions when mangled and function implementing <typedef:<PS>to_string_fn>.
 There can be multiple to string traits, but only one can omit
 `SET_TO_STRING_NAME`.

 @param[SET_TEST]
 To string trait contained in <../test/set_test.h>; optional unit testing
 framework using `assert`. Can only be defined once _per_ set. Must be defined
 equal to a (random) filler function, satisfying <typedef:<PE>action_fn>.
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
#define SET_TO_STRING_TRAIT 1
#else
#define SET_TO_STRING_TRAIT 0
#endif
#define SET_TRAITS SET_TO_STRING_TRAIT
#if SET_TRAITS > 1
#error Only one trait per include is allowed; use SET_EXPECT_TRAIT.
#endif
#if SET_TRAITS != 0 && (!defined(E_) || !defined(CAT) || !defined(CAT_))
#error E_ or CAT_? not yet defined; use SET_EXPECT_TRAIT.
#endif
#if (SET_TRAITS == 0) && defined(SET_TEST)
#error SET_TEST must be defined in SET_TO_STRING trait.
#endif
#if defined(SET_TO_STRING_NAME) && !defined(SET_TO_STRING)
#error SET_TO_STRING_NAME requires SET_TO_STRING.
#endif


#if SET_TRAITS == 0 /* <!-- base code */


/* <Kernighan and Ritchie, 1988, p. 231>. */
#if defined(E_) || defined(PE_) \
	|| (defined(SET_SUBTYPE) ^ (defined(CAT) || defined(CAT_)))
#error Unexpected P?E_ or CAT_?; possible stray SET_EXPECT_TRAIT?
#endif
#ifndef SET_SUBTYPE /* <!-- !sub-type */
#define CAT_(x, y) x ## _ ## y
#define CAT(x, y) CAT_(x, y)
#endif /* !sub-type --> */
#define E_(thing) CAT(SET_NAME, thing)
#define PE_(thing) CAT(set, CAT(SET_NAME, thing))
#ifndef SET_UINT
#define SET_UINT unsigned
#endif

/** Valid unsigned integer type used for hash values. The hash map will
 saturate at `min(((ln 2)/2) \cdot range(<PE>uint), (1/8) \cdot range(size_t))`,
 at which point no new buckets can be added and the load factor will increase
 over the maximum. */
typedef SET_UINT PE_(uint);

/** Valid tag type defined by `SET_TYPE`. Included in <tag:<E>set_node>. */
typedef SET_TYPE PE_(type);
#ifdef SET_POINTER /* <!-- !raw */
/** `SET_POINTER` modifies `<PE>mtype` to be a pointer, otherwise it's
 the same as <typedef:<PE>type>. */
typedef const PE_(type)* PE_(mtype);
#else /* !raw --><!-- raw */
typedef PE_(type) PE_(mtype);
#endif /* raw --> */

/** A map from <typedef:<PE>mtype> onto <typedef:<PE>uint>. Should be as close
 as possible to a discrete uniform distribution while using all argument for
 maximum performance. */
typedef PE_(uint) (*PE_(hash_fn))(const PE_(mtype));
/* Check that `SET_HASH` is a function implementing <typedef:<PE>hash_fn>. */
static const PE_(hash_fn) PE_(hash) = (SET_HASH);

/** Equivalence relation between <typedef:<PE>mtype> that satisfies
 `<PE>is_equal_fn(a, b) -> <PE>hash_fn(a) == <PE>hash_fn(b)`. */
typedef int (*PE_(is_equal_fn))(const PE_(mtype) a, const PE_(mtype) b);
/* Check that `SET_IS_EQUAL` is a function implementing
 <typedef:<PE>is_equal_fn>. */
static const PE_(is_equal_fn) PE_(equal) = (SET_IS_EQUAL);

/** A di-predicate; returns true if the `replace` replaces the `original`. */
typedef int (*PE_(replace_fn))(PE_(type) *original, PE_(type) *replace);

/** Used in <fn:<E>set_policy_put> when `replace` is null; `original` and
 `replace` are ignored. @implements `<PE>replace_fn` */
static int PE_(false)(PE_(type) *original, PE_(type) *replace)
	{ (void)(original); (void)(replace); return 0; }

/** Contains <typedef:<PE>type> as the first element `key`, along with data
 internal to the set. Storage of the `<E>set_node` structure is the
 responsibility of the caller. */
struct E_(set_node);
struct E_(set_node) {
	PE_(type) key; /* This should be next, but offsetof key in set_node 0. */
	struct E_(set_node) *next;
#ifndef SET_NO_CACHE /* <!-- cache */
	PE_(uint) hash;
#endif /* cache --> */
};

/* Singly-linked list head for `buckets`. Not really needed, but
 double-pointers are confusing. */
struct PE_(bucket) { struct E_(set_node) *first; };

/** An `<E>set` of `size`. To initialise, see <fn:<E>set>, `SET_IDLE`, `{0}`
 (`C99`,) or being `static`.

 ![States.](../web/states.png) */
struct E_(set);
struct E_(set) {
	struct PE_(bucket) *buckets; /* An array of 1 << log_capacity (>3) or 0. */
	size_t size;
	unsigned log_capacity;
};
#ifndef SET_IDLE /* <!-- !zero */
#define SET_IDLE { 0, 0, 0 }
#endif /* !zero --> */

#ifdef SET_POINTER /* <!-- !raw */
/** @return `element`. */
static const PE_(type) *PE_(pointer)(const PE_(type) *const element)
	{ return element; }
#else /* !raw --><!-- raw */
/** @return Re-de-reference `element`. */
static PE_(type) PE_(pointer)(const PE_(type) *const element)
	{ return *element; }
#endif /* raw --> */

/** Gets the hash of `element`, which should be consistant. */
static PE_(uint) PE_(get_hash)(struct E_(set_node) *element) {
	assert(element);
#ifdef SET_NO_CACHE /* <!-- !cache */
	return PE_(hash)(PE_(pointer)(&element->key));
#else /* !cache --><!-- cache */
	return element->hash;
#endif /* cache --> */
}

/** Retrieves a bucket from `set` given the `hash`. Only call this function if
 non-empty. May be invalidated upon a call to <fn:<PE>reserve>.
 @return Given a `hash`, compute the bucket. */
static struct PE_(bucket) *PE_(get_bucket)(struct E_(set) *const set,
	const PE_(uint) hash) {
	assert(set && set->buckets);
	return set->buckets + (hash & ((1 << set->log_capacity) - 1));
}

/** Linear search for `data` in `bucket`.
 @param[hash] Must match the hash of `data`.
 @return The link that points to the `data` or null. */
static struct E_(set_node) **PE_(bucket_to)(struct PE_(bucket) *const bucket,
	const PE_(uint) hash, const PE_(mtype) data) {
	struct E_(set_node) **to_x, *x;
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

/** Ensures `min_capacity` (`\times ln^-1 2`) of the buckets of `set`.
 @param[min_capacity] If zero, does nothing.
 @return Success; otherwise, `errno` will be set. @throws[ERANGE] Tried
 allocating more then can fit in `size_t` or `realloc` doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html).
 @throws[realloc] */
static int PE_(reserve)(struct E_(set) *const set, const size_t min_capacity) {
	struct PE_(bucket) *buckets, *b, *b_end, *new_b;
	struct E_(set_node) **to_x, *x;
	const unsigned log_c0 = set->log_capacity,
		log_limit = sizeof(PE_(uint)) * 8 - 1;
	unsigned log_c1;
	PE_(uint) c0 = (PE_(uint))(1 << log_c0), c1, mask;
	size_t no_buckets;
	/* One did set `<PE>uint` to an unsigned type, right? */
	assert(set && c0 && log_c0 <= log_limit && (log_c0 >= 3 || !log_c0)
		&& (PE_(uint))-1 > 0);
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
		PE_(uint) hash;
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

/** General put, `element` in `set` and returns the collided element, if any,
 as long as `replace` is null or returns true. If `replace` returns false,
 returns `element`. */
static struct E_(set_node) *PE_(put)(struct E_(set) *const set,
	struct E_(set_node) *const element, const PE_(replace_fn) replace) {
	struct PE_(bucket) *bucket;
	struct E_(set_node) **to_x = 0, *x = 0;
	PE_(uint) hash;
	assert(set && element);
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
	/* Didn't <fn:<PE>reserve>, now one can't tell error except `errno`. */
	if(!PE_(reserve)(set, set->size + 1)) return 0;
	bucket = PE_(get_bucket)(set, hash);
	set->size++;
add_element:
	element->next = bucket->first, bucket->first = element;
	return x;
}

/** Initialises `set` to idle. @order \Theta(1) @allow */
static void E_(set)(struct E_(set) *const set)
	{ assert(set); set->buckets = 0; set->log_capacity = 0; set->size = 0; }

/** Destroys `set` and returns it to idle. @allow */
static void E_(set_)(struct E_(set) *const set)
	{ assert(set), free(set->buckets), E_(set)(set); }

/** Clears and removes all entries from `set`. The capacity and memory of the
 hash table is preserved, but all previous values are un-associated. The load
 factor will be less until it reaches it's previous size.
 @param[set] If null, does nothing. @order \Theta(`set.buckets`) @allow */
static void E_(set_clear)(struct E_(set) *const set) {
	struct PE_(bucket) *b, *b_end;
	assert(set);
	if(!set->log_capacity) return;
	for(b = set->buckets, b_end = b + (1 << set->log_capacity); b < b_end; b++)
		b->first = 0;
	set->size = 0;
}

/** @return The value in `set` which <typedef:<PE>is_equal_fn> `SET_IS_EQUAL`
 `data`, or, if no such value exists, null.
 @param[set] If null, returns null. @order Average \O(1), (hash distributes
 elements uniformly); worst \O(n). @allow */
static struct E_(set_node) *E_(set_get)(struct E_(set) *const set,
	const PE_(mtype) data) {
	struct E_(set_node) **to_x;
	PE_(uint) hash;
	if(!set || !set->buckets) return 0;
	hash = PE_(hash)(data);
	to_x = PE_(bucket_to)(PE_(get_bucket)(set, hash), hash, data);
	return to_x ? *to_x : 0;
}

/** Reserve at least `reserve`, divided by the maximum load factor, space in
 the buckets of `set`. @return Success.
 @throws[ERANGE] `reserve` plus the size would take a bigger number then could
 fit in a `size_t`. @throws[realloc] @allow */
static int E_(set_reserve)(struct E_(set) *const set, const size_t reserve)
	{ return set ? reserve > (size_t)-1 - set->size ? (errno = ERANGE, 0) :
	PE_(reserve)(set, set->size + reserve) : 0; }

/** Puts the `element` in `set`.
 @param[set, element] If null, returns null. @param[element] Should not be of a
 set because the integrity of that set will be compromised.
 @return Any ejected element or null. (An ejected element has
 <typedef:<PE>is_equal_fn> `SET_IS_EQUAL` the `element`.)
 @throws[realloc, ERANGE] There was an error with a re-sizing. Successfully
 calling <fn:<E>set_reserve> with at least one before ensures that this does
 not happen. @order Average amortised \O(1), (hash distributes elements
 uniformly); worst \O(n). @allow */
static struct E_(set_node) *E_(set_put)(struct E_(set) *const set,
	struct E_(set_node) *const element) { return PE_(put)(set, element, 0); }

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
 Successfully calling <fn:<E>set_reserve> with at least one before ensures that
 this does not happen. @order Average amortised \O(1), (hash distributes
 elements uniformly); worst \O(n). @allow */
static struct E_(set_node) *E_(set_policy_put)(struct E_(set) *const set,
	struct E_(set_node) *const element, const PE_(replace_fn) replace)
	{ return PE_(put)(set, element, replace ? replace : &PE_(false)); }

/** Removes an element `data` from `set`.
 @return Successfully ejected element or null. This element is free to be put
 into another set or modify it's hash values. @order Average \O(1), (hash
 distributes elements uniformly); worst \O(n). @allow */
static struct E_(set_node) *E_(set_remove)(struct E_(set) *const set,
	const PE_(mtype) data) {
	PE_(uint) hash;
	struct E_(set_node) **to_x, *x;
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

/** Contains all iteration parameters. */
struct PE_(iterator);
struct PE_(iterator)
	{ const struct E_(set) *set; size_t b; struct E_(set_node) *e; };

/** Loads `set` into `it`. @implements begin */
static void PE_(begin)(struct PE_(iterator) *const it,
	const struct E_(set) *const set)
	{ assert(it && set), it->set = set, it->b = 0, it->e = 0; }

/** Advances `it`. @implements next */
static const PE_(type) *PE_(next)(struct PE_(iterator) *const it) {
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

#if defined(ITERATE) || defined(ITERATE_BOX) || defined(ITERATE_TYPE) \
	|| defined(ITERATE_BEGIN) || defined(ITERATE_NEXT)
#error Unexpected ITERATE*.
#endif

#define ITERATE struct PE_(iterator)
#define ITERATE_BOX struct E_(set)
#define ITERATE_TYPE PE_(type)
#define ITERATE_BEGIN PE_(begin)
#define ITERATE_NEXT PE_(next)

static void PE_(unused_base_coda)(void);
static void PE_(unused_base)(void) {
	E_(set)(0); E_(set_)(0); E_(set_clear)(0); E_(set_get)(0, 0);
	E_(set_reserve)(0, 0); E_(set_put)(0, 0);  E_(set_policy_put)(0, 0, 0);
	E_(set_remove)(0, 0); PE_(begin)(0, 0); PE_(next)(0);
	PE_(unused_base_coda)();
}
static void PE_(unused_base_coda)(void) { PE_(unused_base)(); }


#elif defined(SET_TO_STRING) /* base code --><!-- to string trait */


#ifdef SET_TO_STRING_NAME /* <!-- name */
#define S_(thing) CAT(E_(set), CAT(SET_TO_STRING_NAME, thing))
#else /* name --><!-- !name */
#define S_(thing) CAT(E_(set), thing)
#endif /* !name --> */
#define TO_STRING SET_TO_STRING
#define TO_STRING_LEFT '{'
#define TO_STRING_RIGHT '}'
#include "to_string.h" /** \include */

#if !defined(SET_TEST_BASE) && defined(SET_TEST) /* <!-- test */
#define SET_TEST_BASE /* Only one instance of base tests. */
#include "../test/test_set.h" /** \include */
#endif /* test --> */

#undef S_
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
#undef PCAT
#undef PCAT_
#else /* !sub-type --><!-- sub-type */
#undef SET_SUBTYPE
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
#undef ITERATE
#undef ITERATE_BOX
#undef ITERATE_TYPE
#undef ITERATE_BEGIN
#undef ITERATE_NEXT
#endif /* !trait --> */

#undef SET_TO_STRING_TRAIT
#undef SET_TRAITS
