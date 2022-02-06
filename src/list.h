/** @license 2017 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Source <src/list.h>; examples <test/test_list.c>.

 @subtitle Doubly-linked component

 ![Example of a stochastic skip-list.](../web/list.png)

 In parlance of <Thareja 2014, Structures>, <tag:<L>list> is a circular
 header, or sentinel, to a doubly-linked list of <tag:<L>listlink>. This allows
 it to benefit from being closed structure, such that with with a pointer to
 any element, it is possible to extract the entire list.

 @param[LIST_NAME]
 `<L>` that satisfies `C` naming conventions when mangled; required. `<PL>` is
 private, whose names are prefixed in a manner to avoid collisions.

 @param[LIST_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[LIST_COMPARE_NAME, LIST_COMPARE, LIST_IS_EQUAL]
 Compare trait contained in <src/list_coda.h>. An optional mangled name for
 uniqueness and a function implementing either <typedef:<PLC>compare_fn> or
 <typedef:<PLC>bipredicate_fn>.

 @param[LIST_TO_STRING_NAME, LIST_TO_STRING]
 To string trait contained in <src/to_string.h>. An optional mangled name for
 uniqueness and function implementing <typedef:<PSZ>to_string_fn>.

 @std C89 */

#ifndef LIST_NAME
#error Name LIST_NAME undefined.
#endif
#if defined(LIST_TO_STRING_NAME) || defined(LIST_TO_STRING) /* <!-- str */
#define LIST_TO_STRING_TRAIT 1
#else /* str --><!-- !str */
#define LIST_TO_STRING_TRAIT 0
#endif /* !str --> */
#if defined(LIST_COMPARE_NAME) || defined(LIST_COMPARE) \
	|| defined(LIST_IS_EQUAL) /* <!-- comp */
#define LIST_COMPARE_TRAIT 1
#else /* comp --><!-- !comp */
#define LIST_COMPARE_TRAIT 0
#endif /* !comp --> */
#define LIST_TRAITS LIST_TO_STRING_TRAIT + LIST_COMPARE_TRAIT
#if LIST_TRAITS > 1
#error Only one trait per include is allowed; use LIST_EXPECT_TRAIT.
#endif
#if defined(LIST_TO_STRING_NAME) && !defined(LIST_TO_STRING)
#error LIST_TO_STRING_NAME requires LIST_TO_STRING.
#endif
#if defined(LIST_COMPARE_NAME) \
	&& (!(!defined(LIST_COMPARE) ^ !defined(LIST_IS_EQUAL)))
#error LIST_COMPARE_NAME requires LIST_COMPARE or LIST_IS_EQUAL not both.
#endif

#ifndef LIST_H /* <!-- idempotent */
#define LIST_H
#include <assert.h>
#if defined(LIST_CAT_) || defined(LIST_CAT) || defined(L_) || defined(PL_)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define LIST_CAT_(n, m) n ## _ ## m
#define LIST_CAT(n, m) LIST_CAT_(n, m)
#define L_(n) LIST_CAT(LIST_NAME, n)
#define PL_(n) LIST_CAT(list, L_(n))
#endif /* idempotent --> */


#if LIST_TRAITS == 0 /* <!-- base code */


/** Storage of this structure is the responsibility of the caller, who must
 provide a stable pointer while it's in the list. Generally, one encloses this
 in a host `struct` or `union`. Multiple independent lists can be in the same
 host, however one link can can only be a part of one list at a time. Adding a
 link to a second list destroys the integrity of the original list, as does
 moving a pointer, (specifically, arrays that might increase in size.)

 ![States.](../web/node-states.png) */
struct L_(listlink) { struct L_(listlink) *next, *prev; };

/** Serves as head and tail for linked-list of <tag:<L>listlink>. Use
 <fn:<L>list_clear> to initialize the list. Because this list is closed; that
 is, given a valid pointer to an element, one can determine all others, null
 values are not allowed and it is _not_ the same as `{0}`. In a valid list,
 `as_head.head.tail`, `as_tail.tail.head`, and `flat.zero`, refer to the same
 sentinel element, and it's always the only one null. If the address changes,
 one must call <fn:<L>list_self_correct>.

 ![States.](../web/states.png) */
struct L_(list) {
	union {
		struct { struct L_(listlink) head, *part_of_tail; } as_head;
		struct { struct L_(listlink) *part_of_head, tail; } as_tail;
		struct { struct L_(listlink) *next, *zero, *prev; } flat;
	} u;
};

/** Operates by side-effects on the node. */
typedef void (*PL_(action_fn))(struct L_(listlink) *);

/** Returns (Non-zero) true or (zero) false when given a node. */
typedef int (*PL_(predicate_fn))(const struct L_(listlink) *);

/** Cats all `from` in front of `after`; `from` will be empty after.
 Careful that `after` is not in `from` because that will just erase the list.
 @order \Theta(1) */
static void PL_(move)(struct L_(list) *const from,
	struct L_(listlink) *const after) {
	assert(from && from->u.flat.next && !from->u.flat.zero && from->u.flat.prev
		&& after && after->prev);
	from->u.flat.next->prev = after->prev;
	after->prev->next = from->u.as_head.head.next;
	from->u.flat.prev->next = after;
	after->prev = from->u.as_tail.tail.prev;
	from->u.flat.next = &from->u.as_tail.tail;
	from->u.flat.prev = &from->u.as_head.head;
}

/** @return A pointer to the first element of `list`, if it exists.
 @order \Theta(1) @allow */
static struct L_(listlink) *L_(list_head)(const struct L_(list) *const list) {
	struct L_(listlink) *link;
	assert(list);
	link = list->u.flat.next, assert(link);
	return link->next ? link : 0;
}

/** @return A pointer to the last element of `list`, if it exists.
 @order \Theta(1) @allow */
static struct L_(listlink) *L_(list_tail)(const struct L_(list) *const list) {
	struct L_(listlink) *link;
	assert(list);
	link = list->u.flat.prev, assert(link);
	return link->prev ? link : 0;
}

/** @return The previous element. When `link` is the first element, returns
 null. @order \Theta(1) @allow */
static struct L_(listlink) *L_(list_previous)(struct L_(listlink) *link) {
	assert(link && link->prev);
	link = link->prev;
	return link->prev ? link : 0;
}

/** @return The next element. When `link` is the last element, returns null.
 @order \Theta(1) @allow */
static struct L_(listlink) *L_(list_next)(struct L_(listlink) *link) {
	assert(link && link->next);
	link = link->next;
	return link->next ? link : 0;
}

/** Clear `list`. */
static void PL_(clear)(struct L_(list) *const list) {
	list->u.flat.next = &list->u.as_tail.tail;
	list->u.flat.zero = 0;
	list->u.flat.prev = &list->u.as_head.head;
}

/** Clears and initializes `list`. @order \Theta(1) @allow */
static void L_(list_clear)(struct L_(list) *const list)
	{ assert(list); PL_(clear)(list); }

/** `add` before `anchor`. @order \Theta(1) */
static void PL_(add_before)(struct L_(listlink) *const anchor,
	struct L_(listlink) *const add) {
	add->prev = anchor->prev;
	add->next = anchor;
	anchor->prev->next = add;
	anchor->prev = add;
}

/** `add` before `anchor`. @order \Theta(1) @allow */
static void L_(list_add_before)(struct L_(listlink) *const anchor,
	struct L_(listlink) *const add) {
	assert(anchor && add && anchor != add && anchor->prev);
	PL_(add_before)(anchor, add);
}

/** `add` after `anchor`. @order \Theta(1) */
static void PL_(add_after)(struct L_(listlink) *const anchor,
	struct L_(listlink) *const add) {
	add->prev = anchor;
	add->next = anchor->next;
	anchor->next->prev = add;
	anchor->next = add;
}

/** `add` after `anchor`. @order \Theta(1) @allow */
static void L_(list_add_after)(struct L_(listlink) *const anchor,
	struct L_(listlink) *const add) {
	assert(anchor && add && anchor != add && anchor->next);
	PL_(add_after)(anchor, add);
}

/** Adds `add` to the end of `list`. @order \Theta(1) */
static void PL_(push)(struct L_(list) *const list,
	struct L_(listlink) *const add)
	{ PL_(add_before)(&list->u.as_tail.tail, add); }

/** Adds `add` to the end of `list`. @order \Theta(1) @allow */
static void L_(list_push)(struct L_(list) *const list,
	struct L_(listlink) *const add)
	{ assert(list && add), PL_(push)(list, add); }

/** Adds `add` to the beginning of `list`. @order \Theta(1) @allow */
static void L_(list_unshift)(struct L_(list) *const list,
	struct L_(listlink) *const add)
	{ assert(list && add), PL_(add_after)(&list->u.as_head.head, add); }

/** Remove `node`. @order \Theta(1) */
static void PL_(remove)(struct L_(listlink) *const node) {
	node->prev->next = node->next;
	node->next->prev = node->prev;
	node->prev = node->next = 0;
}

/** Remove `node`. @order \Theta(1) @allow */
static void L_(list_remove)(struct L_(listlink) *const node)
	{ assert(node && node->prev && node->next), PL_(remove)(node); }

/** Removes the first element of `list` and returns it, if any.
 @order \Theta(1) @allow */
static struct L_(listlink) *L_(list_shift)(struct L_(list) *const list) {
	struct L_(listlink) *node;
	assert(list && list->u.flat.next);
	if(!(node = list->u.flat.next)->next) return 0;
	L_(list_remove)(node);
	return node;
}

/** Removes the last element of `list` and returns it, if any.
 @order \Theta(1) @allow */
static struct L_(listlink) *L_(list_pop)(struct L_(list) *const list) {
	struct L_(listlink) *node;
	assert(list && list->u.flat.prev);
	if(!(node = list->u.flat.prev)->prev) return 0;
	L_(list_remove)(node);
	return node;
}

/** Moves the elements `from` onto `to` at the end.
 @param[to] If null, then it removes elements from `from`.
 @order \Theta(1) @allow */
static void L_(list_to)(struct L_(list) *const from,
	struct L_(list) *const to) {
	assert(from && from != to);
	if(!to) { PL_(clear)(from); return; }
	PL_(move)(from, &to->u.as_tail.tail);
}

/** Moves the elements `from` immediately before `anchor`, which can not be in
 the same list. @order \Theta(1) @allow */
static void L_(list_to_before)(struct L_(list) *const from,
	struct L_(listlink) *const anchor) {
	assert(from && anchor);
	PL_(move)(from, anchor);
}

/** Moves all elements `from` onto `to` at the tail if `predicate` is true.
 They ca'n't be the same list.
 @param[to] If null, then it removes elements.
 @order \Theta(|`from`|) \times \O(`predicate`) @allow */
static void L_(list_to_if)(struct L_(list) *const from,
	struct L_(list) *const to, const PL_(predicate_fn) predicate) {
	struct L_(listlink) *link, *next_link;
	assert(from && from != to && predicate);
	for(link = from->u.flat.next; next_link = link->next; link = next_link) {
		if(!predicate(link)) continue;
		L_(list_remove)(link);
		if(to) L_(list_add_before)(&to->u.as_tail.tail, link);
	}
}

/** Performs `action` for each element in `list` in order.
 @param[action] It makes a double of the next node, so it can be to delete the
 element and even assign it's values null.
 @order \Theta(|`list`|) \times O(`action`) @allow */
static void L_(list_for_each)(struct L_(list) *const list,
	const PL_(action_fn) action) {
	struct L_(listlink) *x, *next_x;
	assert(list && action);
	for(x = list->u.flat.next; next_x = x->next; x = next_x) action(x);
}

/** Iterates through `list` and calls `predicate` until it returns true.
 @return The first `predicate` that returned true, or, if the statement is
 false on all, null.
 @order \O(|`list`|) \times \O(`predicate`) @allow */
static struct L_(listlink) *L_(list_any)(const struct L_(list) *const list,
	const PL_(predicate_fn) predicate) {
	struct L_(listlink) *link, *next_link;
	assert(list && predicate);
	for(link = list->u.flat.next; next_link = link->next; link = next_link)
		if(predicate(link)) return link;
	return 0;
}

/** Corrects `list` ends to compensate for memory relocation of the list
 itself. Because the `list` is part of the links, this will invalidate all
 other copies. @order \Theta(1) @allow */
static void L_(list_self_correct)(struct L_(list) *const list) {
	assert(list && !list->u.flat.zero);
	if(!list->u.flat.next->next) { /* Empty. */
		assert(!list->u.flat.prev->prev);
		list->u.flat.next = &list->u.as_tail.tail;
		list->u.flat.prev = &list->u.as_head.head;
	} else { /* Non-empty. */
		list->u.flat.prev->next = &list->u.as_tail.tail;
		list->u.flat.next->prev = &list->u.as_head.head;
	}
}

/* <!-- iterate interface */

/** Contains all iteration parameters. (Since this is a permutation, the
 iteration is defined by none other then itself. Used for traits.) */
struct PL_(iterator) { struct L_(listlink) *link; };

/** Loads `list` into `it`. @implements begin */
static void PL_(begin)(struct PL_(iterator) *const it,
	const struct L_(list) *const list)
	{ assert(it && list), it->link = L_(list_head)(list); }

/** Advances `it`. @implements next */
static const struct L_(listlink) *PL_(next)(struct PL_(iterator) *const it) {
	struct L_(listlink) *here = it->link;
	assert(it);
	if(!here) return 0;
	it->link = L_(list_next)(it->link);
	return here;
}

/* iterate --> */

/* <!-- box (multiple traits) */
#define BOX_ PL_
#define BOX_CONTAINER struct L_(list)
#define BOX_CONTENTS struct L_(listlink)

#ifdef LIST_TEST /* <!-- test */
/* Forward-declare. */
static void (*PL_(to_string))(const struct L_(listlink) *, char (*)[12]);
static const char *(*PL_(list_to_string))(const struct L_(list) *);
#include "../test/test_list.h"
#endif /* test --> */

static void PL_(unused_base_coda)(void);
static void PL_(unused_base)(void) {
	L_(list_head)(0); L_(list_tail)(0); L_(list_previous)(0); L_(list_next)(0);
	L_(list_clear)(0); L_(list_add_before)(0, 0); L_(list_add_after)(0, 0);
	L_(list_unshift)(0, 0); L_(list_push)(0, 0); L_(list_remove)(0);
	L_(list_shift)(0); L_(list_pop)(0); L_(list_to)(0, 0);
	L_(list_to_before)(0, 0); L_(list_to_if)(0, 0, 0); L_(list_for_each)(0, 0);
	L_(list_any)(0, 0); L_(list_self_correct)(0);
	PL_(begin)(0, 0); PL_(next)(0); PL_(unused_base_coda)();
}
static void PL_(unused_base_coda)(void) { PL_(unused_base)(); }


#elif defined(LIST_TO_STRING) /* base code --><!-- to string trait */


#ifdef LIST_TO_STRING_NAME /* <!-- name */
#define SZ_(n) LIST_CAT(L_(list), LIST_CAT(LIST_TO_STRING_NAME, n))
#else /* name --><!-- !name */
#define SZ_(n) LIST_CAT(L_(list), n)
#endif /* !name --> */
#define TO_STRING LIST_TO_STRING
#include "to_string.h" /** \include */
#ifdef LIST_TEST /* <!-- expect: greedy satisfy forward-declared. */
#undef LIST_TEST
static PSZ_(to_string_fn) PL_(to_string) = PSZ_(to_string);
static const char *(*PL_(list_to_string))(const struct L_(list) *)
	= &SZ_(to_string);
#endif /* expect --> */
#undef SZ_
#undef LIST_TO_STRING
#ifdef LIST_TO_STRING_NAME
#undef LIST_TO_STRING_NAME
#endif


#else /* to string trait --><!-- compare trait */


#ifdef LIST_COMPARE_NAME /* <!-- name */
#define LC_(n) LIST_CAT(L_(list), LIST_CAT(LIST_COMPARE_NAME, n))
#else /* name --><!-- !name */
#define LC_(n) LIST_CAT(L_(list), n)
#endif /* !name --> */
#include "list_coda.h" /** \include */
#ifdef LIST_TEST /* <!-- test: this detects and outputs compare test. */
#include "../test/test_list.h"
#endif /* test --> */
#undef LC_
#ifdef LIST_COMPARE_NAME
#undef LIST_COMPARE_NAME
#endif
#ifdef LIST_COMPARE
#undef LIST_COMPARE
#endif
#ifdef LIST_IS_EQUAL
#undef LIST_IS_EQUAL
#endif


#endif /* traits --> */


#ifdef LIST_EXPECT_TRAIT /* <!-- trait */
#undef LIST_EXPECT_TRAIT
#else /* trait --><!-- !trait */
#ifdef LIST_TEST
#error No LIST_TO_STRING traits defined for LIST_TEST.
#endif
#undef LIST_NAME
#undef BOX_
#undef BOX_CONTAINER
#undef BOX_CONTENTS
/* box (multiple traits) --> */
#endif /* !trait --> */
#undef LIST_TO_STRING_TRAIT
#undef LIST_COMPARE_TRAIT
#undef LIST_TRAITS
