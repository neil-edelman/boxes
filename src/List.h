/** @license 2017 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Parameterised Doubly-Linked Closed List

 ![Example of <Animal>List](../web/list.png)

 <tag:<N>List> is a list of <tag:<N>ListNode>; it may be supplied a total-order
 function, `LIST_COMPARE` <typedef:<PN>Compare>.

 Internally, `<N>ListNode` is a doubly-linked node with sentinels residing in
 `<N>List`. It only provides an order, but `<N>ListNode` may be enclosed in
 another `struct` as needed.

 `<N>Link` is not synchronised. The parameters are `#define` preprocessor
 macros, and are all undefined at the end of the file for convenience. To stop
 assertions, use `#define NDEBUG` before inclusion of `assert.h`, (which is
 used in this file.)

 @param[LIST_NAME]
 `<N>` that satisfies `C` naming conventions when mangled; required. `<PN>` is
 private, whose names are prefixed in a manner to avoid collisions; any should
 be re-defined prior to use elsewhere.

 @param[LIST_COMPARE]
 Optional total-order function satisfying <typedef:<PN>Compare>.

 @param[LIST_TO_STRING]
 Optional print function implementing <typedef:<PN>ToString>; makes available
 <fn:<N>ListToString>.

 @param[LIST_TEST]
 Unit testing framework <fn:<N>ListTest>, included in a separate header,
 <../test/TestList.h>. Must be defined equal to a random filler function,
 satisfying <typedef:<PN>Action>. Requires `LIST_TO_STRING` and not `NDEBUG`.

 @std C89
 @cf [Array](https://github.com/neil-edelman/Array)
 @cf [Orcish](https://github.com/neil-edelman/Orcish)
 @cf [Pool](https://github.com/neil-edelman/Pool)
 @cf [Set](https://github.com/neil-edelman/Set) */

#include <assert.h>
#ifdef LIST_TO_STRING /* <!-- string */
#include <string.h> /* strlen */
#endif /* string --> */

/* Check defines. */
#ifndef LIST_NAME
#error Generic LIST_NAME undefined.
#endif
#if defined(LIST_TEST) && !defined(LIST_TO_STRING)
#error LIST_TEST requires LIST_TO_STRING.
#endif

/* Generics using the preprocessor;
 <http://stackoverflow.com/questions/16522341/pseudo-generics-in-c>. */
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
#ifdef N_
#undef N_
#endif
#ifdef PN_
#undef PN_
#endif
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#define N_(thing) CAT(LIST_NAME, thing)
#define PN_(thing) PCAT(list, PCAT(LIST_NAME, thing)) /* "Private." */


/** Storage of this structure is the responsibility of the caller. One can only
 be in one list at a time; adding to another list while in a list destroys the
 integrity of the original list, see <fn:<N>ListRemove>.

 ![States.](../web/node-states.png) */
struct N_(ListNode);
struct N_(ListNode) { struct N_(ListNode) *prev, *next; };

/** Serves as head and tail for linked-list of <tag:<N>ListNode>. Use
 <fn:<N>ListClear> to initialise the list. Because this list is closed; that
 is, given a valid pointer to an element, one can determine all others, null
 values are not allowed and it is _not_ the same as `{0}`.

 ![States.](../web/states.png) */
struct N_(List);
struct N_(List) {
	/* These are sentinels such that `head.prev` and `tail.next` are always and
	 the only ones to be null. Must be packed this way for
	 <fn:<PN>self_correct>. */
	struct N_(ListNode) head, tail;
};

#ifdef LIST_TO_STRING /* <!-- string */
/** Responsible for turning <tag:<N>ListNode> (the first argument) into a
 maximum 11-`char` string (the second.) Defined when `LIST_TO_STRING`. */
typedef void (*PN_(ToString))(const struct N_(ListNode) *, char (*)[12]);
/* Check that `LIST_TO_STRING` is a function implementing
 <typedef:<PN>ToString>. */
static const PN_(ToString) PN_(to_string) = (LIST_TO_STRING);
#endif /* string --> */

/** Operates by side-effects on the link. */
typedef void (*PN_(Action))(struct N_(ListNode) *);
/** Returns (Non-zero) true or (zero) false when given a link. */
typedef int (*PN_(Predicate))(const struct N_(ListNode) *);



/** Private: clears and initialises `list`. */
static void PN_(clear)(struct N_(List) *const list) {
	assert(list);
	list->head.prev = list->tail.next = 0;
	list->head.next = &list->tail;
	list->tail.prev = &list->head;
}

/** Private: `add` before `anchor`. */
static void PN_(add_before)(struct N_(ListNode) *const anchor,
	struct N_(ListNode) *const add) {
	assert(anchor && add && anchor != add && anchor->prev);
	add->prev = anchor->prev;
	add->next = anchor;
	anchor->prev->next = add;
	anchor->prev = add;
}

/** Private: `add` after `anchor`. */
static void PN_(add_after)(struct N_(ListNode) *const anchor,
	struct N_(ListNode) *const add) {
	assert(anchor && add && anchor != add && anchor->next);
	add->prev = anchor;
	add->next = anchor->next;
	anchor->next->prev = add;
	anchor->next = add;
}

/** Private: remove `node`. */
static void PN_(remove)(struct N_(ListNode) *const node) {
	assert(node && node->prev && node->next);
	node->prev->next = node->next;
	node->next->prev = node->prev;
	node->prev = node->next = 0;
}

/** Private: cats all `from` in front of `node`, (don't make `node` `head`);
 `from` will be empty after. Careful that `node` is not in `from` because that
 will just erase the list.
 @order \Theta(1) */
static void PN_(move)(struct N_(List) *const from,
	struct N_(ListNode) *const node) {
	assert(node && from && node->prev &&
		!from->head.prev && from->head.next
		&& from->tail.prev && !from->tail.next);
	from->head.next->prev = node->prev;
	node->prev->next = from->head.next;
	from->tail.prev->next = node;
	node->prev = from->tail.prev;
	from->head.next = &from->tail;
	from->tail.prev = &from->head;
}

/** Private: when the actual `list` but not the data changes locations. */
static void PN_(self_correct)(struct N_(List) *const list) {
	if(list->head.next == list->tail.prev + 1) {
		list->head.next = &list->tail;
		list->tail.prev = &list->head;
	} else {
		list->head.next->prev = &list->head;
		list->tail.prev->next = &list->tail;
	}
}


/** Clears and removes all values from `list`, thereby initialising it. All
 previous values are un-associated.
 @param[list] if null, does nothing.
 @order \Theta(1)
 @allow */
static void N_(ListClear)(struct N_(List) *const list) {
	if(list) PN_(clear)(list);
}

/** @param[list] If null, returns null.
 @return A pointer to the first element of `list`, if it exists.
 @order \Theta(1)
 @allow */
static struct N_(ListNode) *N_(ListFirst)(const struct N_(List) *const list) {
	struct N_(ListNode) *link;
	if(!list) return 0;
	link = list->head.next, assert(link);
	return link->next ? link : 0;
}

/** @param[list] If null, returns null.
 @return A pointer to the last element of `list`, if it exists.
 @order \Theta(1)
 @allow */
static struct N_(ListNode) *N_(ListLast)(const struct N_(List) *const list) {
	struct N_(ListNode) *link;
	if(!list) return 0;
	link = list->tail.prev, assert(link);
	return link->prev ? link : 0;
}

/** @param[link] If null, returns null, otherwise must be part of a list.
 @return The previous element. When `link` is the first element, returns null.
 @order \Theta(1)
 @allow */
static struct N_(ListNode) *N_(ListPrevious)(struct N_(ListNode) *link) {
	if(!link) return 0;
	link = link->prev;
	return link && link->prev ? link : 0;
}

/** @param[link] If null, returns null, otherwise must be part of a list.
 @return The next element. When `link` is the last element, returns null.
 @order \Theta(1)
 @allow */
static struct N_(ListNode) *N_(ListNext)(struct N_(ListNode) *link) {
	if(!link) return 0;
	link = link->next;
	return link && link->next ? link : 0;
}

/** Adds `add` to the beginning of `list`.
 @param[list, add] If null, does nothing.
 @param[add] Should not associated to any list.
 @order \Theta(1)
 @allow */
static void N_(ListUnshift)(struct N_(List) *const list,
	struct N_(ListNode) *const add) {
	if(!list || !add) return;
	PN_(add_after)(&list->head, add);
}

/** Adds `add` to the end of `list`.
 @param[list, add] If null, does nothing.
 @param[add] Should not associated to any list.
 @order \Theta(1)
 @allow */
static void N_(ListPush)(struct N_(List) *const list,
	struct N_(ListNode) *const add) {
	if(!list || !add) return;
	PN_(add_before)(&list->tail, add);
}

/** Adds `add` immediately before `anchor`.
 @param[anchor, add] If null, does nothing.
 @param[anchor] Must be part of a list.
 @param[add] Should not be part of any list.
 @order \Theta(1)
 @allow */
static void N_(ListAddBefore)(struct N_(ListNode) *const anchor,
	struct N_(ListNode) *const add) {
	if(!anchor || !add) return;
	PN_(add_before)(anchor, add);
}

/** Adds `add` immediately after `anchor`.
 @param[anchor, add] If null, does nothing.
 @param[anchor] Must be part of a list.
 @param[add] Should not be part of any list.
 @order \Theta(1)
 @allow */
static void N_(ListAddAfter)(struct N_(ListNode) *const anchor,
	struct N_(ListNode) *const add) {
	if(!anchor || !add) return;
	PN_(add_after)(anchor, add);
}

/** Un-associates `link` from the list; consequently, the `link` is free to add
 to another list. Removing an element that was not added to a list results in
 undefined behaviour.
 @param[link] If null, does nothing.
 @order \Theta(1)
 @allow */
static void N_(ListRemove)(struct N_(ListNode) *const link) {
	if(!link) return;
	PN_(remove)(link);
}

/** Un-associates the first element of `list`.
 @param[list] If null, returns null.
 @return The erstwhile first element or null if the list was empty.
 @order \Theta(1)
 @allow */
static struct N_(ListNode) *N_(ListShift)(struct N_(List) *const list) {
	struct N_(ListNode) *node;
	if(!list) return 0;
	if(!(node = list->head.next)->next) return 0;
	PN_(remove)(node);
	return node;
}

/** Un-associates the last element of `list`.
 @param[list] If null, returns null.
 @return The erstwhile last element or null if the list was empty.
 @order \Theta(1)
 @allow */
static struct N_(ListNode) *N_(ListPop)(struct N_(List) *const list) {
	struct N_(ListNode) *node;
	if(!list) return 0;
	if(!(node = list->tail.prev)->prev) return 0;
	PN_(remove)(node);
	return node;
}

/** Moves the elements `from` onto `to` at the end.
 @param[from] If null, it does nothing, otherwise this list will be empty on
 return.
 @param[to] If null, then it removes elements from `from`.
 @order \Theta(1)
 @allow */
static void N_(ListTo)(struct N_(List) *const from, struct N_(List) *const to) {
	if(!from || from == to) return;
	if(!to) { PN_(clear)(from); return; }
	PN_(move)(from, &to->tail);
}

/** Moves the elements `from` immediately before `anchor`.
 @param[anchor, from] If null, does nothing.
 @param[anchor] Must be part of a valid list that is not `from`.
 @param[from] This list will be empty on return.
 @order \Theta(1)
 @allow */
static void N_(ListToBefore)(struct N_(List) *const from,
	struct N_(ListNode) *const anchor) {
	if(!from || !anchor) return;
	PN_(move)(from, anchor);
}

/** Moves all elements `from` onto `to` at the end if `predicate` is true.
 @param[from, predicate] If null, does nothing.
 @param[to] If null, then it removes elements.
 @order \Theta(|`from`|) \times \O(`predicate`)
 @allow */
static void N_(ListToIf)(struct N_(List) *const from,
	struct N_(List) *const to, const PN_(Predicate) predicate) {
	struct N_(ListNode) *link, *next_link;
	if(!from || from == to || !predicate) return;
	for(link = from->head.next; (next_link = link->next); link = next_link) {
		if(!predicate(link)) continue;
		PN_(remove)(link);
		if(to) PN_(add_before)(&to->tail, link);
	}
}

/** Performs `action` for each element in `list` in order. `action` can be to
 delete the element.
 @param[list, action] If null, does nothing.
 @order \Theta(|`list`|) \times O(`action`)
 @allow */
static void N_(ListForEach)(struct N_(List) *const list,
	const PN_(Action) action) {
	struct N_(ListNode) *x, *next_x;
	if(!list || !action) return;
	for(x = list->head.next; (next_x = x->next); x = next_x)
		action(x);
}

/** Iterates through `list` and calls `predicate` until it returns true.
 @param[list, predicate] If null, returns null.
 @return The first `predicate` that returned true, or, if the statement is
 false on all, null.
 @order \O(|`list`|) \times \O(`predicate`)
 @allow */
static struct N_(ListNode) *N_(ListAny)(const struct N_(List) *const list,
	const PN_(Predicate) predicate) {
	struct N_(ListNode) *link, *next_link;
	if(!list || !predicate) return 0;
	for(link = list->head.next; (next_link = link->next); link = next_link)
		if(predicate(link)) return link;
	return 0;
}

/** Usually <tag:<N>List> doesn't change memory locations, but when it does,
 this corrects `list`'s two ends, (not the nodes, which must be fixed.) Note
 that the two ends become invalid even when it's empty.
 @param[list] If null, does nothing.
 @order \Theta(1)
 @allow */
static void N_(ListSelfCorrect)(struct N_(List) *const list) {
	if(!list) return;
	PN_(self_correct)(list);
}

#ifdef LIST_COMPARE /* <!-- comp: allowing multiple comparison functions with a
 parameter would have been more general, but then the user has to keep track of
all the comparison functions and pass the right one, and 99% of the time, it's
only one. */

/** Returns less then, equal to, or greater then zero, forming an equivalence
 relation between `a` as compared to `b`. Defined when `LIST_COMPARE`. */
typedef int (*PN_(Compare))(const struct N_(ListNode) *a,
	const struct N_(ListNode) *b);
/* Check that `LIST_COMPARE` is a function implementing
 <typedef:<PN>Compare>. */
static const PN_(Compare) PN_(compare) = (LIST_COMPARE);

/* Constants across multiple includes in the same translation unit. */
#ifndef LIST_H /* <!-- h */
#define LIST_H
/* <fn:<PN>boolean> operations bit-vector; dummy `LO_` ensures closed. */
enum ListOperation {
	LO_SUBTRACTION_AB = 1,
	LO_SUBTRACTION_BA = 2,
	LO_A,
	LO_INTERSECTION   = 4,
	LO_B, LO_C, LO_D,
	LO_DEFAULT_A      = 8,
	LO_E, LO_F, LO_G, LO_H, LO_I, LO_J, LO_K,
	LO_DEFAULT_B      = 16,
	LO_L, LO_M, LO_N, LO_O, LO_P, LO_Q, LO_R, LO_S,
	LO_T, LO_U, LO_V, LO_W, LO_X, LO_Y, LO_Z
};
#endif /* h --> */

/** Local duplicates from `from` onto the back of `to`.
 @order \O(|`from`|) */
static void PN_(duplicates)(struct N_(List) *const from,
	struct N_(List) *const to) {
	struct N_(ListNode) *a = from->head.next, *b, *temp;
	assert(from);
	if(!(b = a->next)) return;
	while(b->next) {
		if(PN_(compare)(a, b)) {
			a = b, b = b->next;
		} else {
			temp = b, b = b->next;
			PN_(remove)(temp);
			if(to) PN_(add_before)(&to->tail, temp);
		}
	}
}

/** Private: `alist` `mask` `blist` -> `result`. Prefers `a` to `b` when equal.
 @order \O(|`a`| + |`b`|) */
static void PN_(boolean)(struct N_(List) *const alist,
	struct N_(List) *const blist, const enum ListOperation mask,
	struct N_(List) *const result) {
	struct N_(ListNode) *a = alist ? alist->head.next : 0,
		*b = blist ? blist->head.next : 0, *temp;
	int comp;
	assert((!result || (result != alist && result != blist))
		&& (!alist || (alist != blist)));
	if(a && b) {
		while(a->next && b->next) {
			comp = PN_(compare)(a, b);
			if(comp < 0) {
				temp = a, a = a->next;
				if(mask & LO_SUBTRACTION_AB) {
					PN_(remove)(temp);
					if(result) PN_(add_before)(&result->tail, temp);
				}
			} else if(comp > 0) {
				temp = b, b = b->next;
				if(mask & LO_SUBTRACTION_BA) {
					PN_(remove)(temp);
					if(result) PN_(add_before)(&result->tail, temp);
				}
			} else {
				temp = a, a = a->next, b = b->next;
				if(mask & LO_INTERSECTION) {
					PN_(remove)(temp);
					if(result) PN_(add_before)(&result->tail, temp);
				}
			}
		}
	}
	if(a && mask & LO_DEFAULT_A) {
		while((temp = a, a = a->next)) {
			PN_(remove)(temp);
			if(result) PN_(add_before)(&result->tail, temp);
		}
	}
	if(b && mask & LO_DEFAULT_B) {
		while((temp = b, b = b->next)) {
			PN_(remove)(temp);
			if(result) PN_(add_before)(&result->tail, temp);
		}
	}
}

/** Private: merges `lb` into `la`; on equal elements, places `la` first.
 @order \O(|`alist`| + |`blist`|). */
static void PN_(merge)(struct N_(List) *const la, struct N_(List) *const lb) {
	struct N_(ListNode) *cur, *a, *b;
	assert(la && lb);
	/* `blist` empty -- that was easy. */
	if(!(b = lb->head.next)->next) return;
	/* `alist` empty -- `O(1)` <fn:<PN>move> is more efficient. */
	if(!(a = la->head.next)->next)
		{ PN_(move)(lb, &la->tail); return; }
	/* Merge */
	for(cur = &la->head; ; ) {
		if(PN_(compare)(a, b) < 0) {
			a->prev = cur, cur = cur->next = a;
			if(!(a = a->next)->next) {
				b->prev = cur, cur->next = b;
				lb->tail.prev->next = &la->tail;
				la->tail.prev = lb->tail.prev;
				break;
			}
		} else {
			b->prev = cur, cur = cur->next = b;
			if(!(b = b->next)->next) { a->prev = cur, cur->next = a; break; }
		}
	}
	lb->head.next = &lb->tail, lb->tail.prev = &lb->head;
}

/* A run is a sequence of values in the array that is weakly increasing. */
struct PN_(Run) { struct N_(ListNode) *head, *tail; size_t size; };
/* Store the maximum capacity for the indexing with {size_t}. (Much more then
 we need, in most cases.) \${
 \> range(runs) = Sum_{k=0}^runs 2^{runs-k} - 1
 \>             = 2^{runs+1} - 2
 \> 2^bits      = 2 (r^runs - 1)
 \> runs        = log(2^{bits-1} + 1) / log 2
 \> runs       <= 2^{bits - 1}, 2^{bits + 1} > 0} */
struct PN_(Runs) {
	struct PN_(Run) run[(sizeof(size_t) << 3) - 1];
	size_t run_no;
};

/** Inserts the first element from the larger of two sorted `r`, then merges
 the rest. */
static void PN_(merge_runs)(struct PN_(Runs) *const r) {
	struct PN_(Run) *const run_a = r->run + r->run_no - 2;
	struct PN_(Run) *const run_b = run_a + 1;
	struct N_(ListNode) *a = run_a->tail, *b = run_b->head, *chosen;
	assert(r->run_no >= 2);
	/* In the absence of any real information, assume that the elements farther
	 in the list are generally more apt to be at the back, _viz_, adaptive. */
	if(run_a->size <= run_b->size) {
		struct N_(ListNode) *prev_chosen;
		/* Run `a` is smaller: downwards insert `b.head` followed by upwards
		 merge. Insert the first element of `b` downwards into `a`. */
		for( ; ; ) {
			if(PN_(compare)(a, b) <= 0) { chosen = a; a = a->next; break; }
			if(!a->prev) { run_a->head = run_b->head; chosen = b; b = b->next;
				break; }
			a = a->prev;
		}
		/* Merge upwards; while the lists are interleaved. */
		while(chosen->next) {
			prev_chosen = chosen;
			if(PN_(compare)(a, b) > 0) chosen = b, b = b->next;
			else chosen = a, a = a->next;
			prev_chosen->next = chosen;
			chosen->prev = prev_chosen;
		}
		/* Splice the one list left. */
		if(!a) b->prev = chosen, chosen->next = b, run_a->tail = run_b->tail;
		else a->prev = chosen, chosen->next = a;
	} else {
		struct N_(ListNode) *next_chosen;
		int is_a_tail = 0;
		/* Run `b` is smaller; upwards insert followed by downwards merge.
		 Insert the last element of `a` upwards into `b`. */
		for( ; ; ) {
			if(PN_(compare)(a, b) <= 0) { chosen = b; b = b->prev; break; }
			/* Here, `a > b`. */
			if(!b->next) { is_a_tail = -1; chosen = a; a = a->prev; break; }
			b = b->next;
		}
		if(!is_a_tail) run_a->tail = run_b->tail;
		/* Merge downwards, while the lists are interleaved. */
		while(chosen->prev) {
			next_chosen = chosen;
			if(PN_(compare)(a, b) > 0) chosen = a, a = a->prev;
			else chosen = b, b = b->prev;
			next_chosen->prev = chosen;
			chosen->next = next_chosen;
		}
		/* Splice the one list left. */
		if(!a) b->next = chosen, chosen->prev = b, run_a->head = run_b->head;
		else a->next = chosen, chosen->prev = a;
	}
	run_a->size += run_b->size;
	r->run_no--;
}

/** Natural merge sorts `list`.  It hasn't been optimised and it's kind of
 experimental. */
static void PN_(natural)(struct N_(List) *const list) {
	/* fixme: This is half-a-KB; use recursion properly. */
	struct PN_(Runs) runs;
	struct PN_(Run) *new_run;
	/* Part of the state machine for classifying points. */
	enum { UNSURE, INCREASING, DECREASING } mono;
	/* The data that we are sorting. */
	struct N_(ListNode) *a, *b, *c, *first_iso_a;
	/* `run_count` is different from `runs.run_no` in that it only increases;
	 only used for calculating the path up the tree. */
	size_t run_count, rc;
	/* The value of the comparison. */
	int comp;
	assert(list);
	/* Needs an element. */
	a = list->head.next, assert(a);
	if(!(b = a->next)) return;
	/* Reset the state machine and output to just `a` in the first run. */
	mono = UNSURE;
	runs.run_no = 1;
	new_run = runs.run + 0, run_count = (size_t)1;
	new_run->size = 1;
	first_iso_a = new_run->head = new_run->tail = a;
	/* While `a` and `b` are elements, (that are consecutive.) */
	for(c = b->next; c; a = b, b = c, c = c->next) {
		comp = PN_(compare)(a, b);
		/* State machine that considers runs in both directions -- in practice,
		 slightly slower than only considering increasing runs on most cases. */
		if(comp < 0) { /* `a < b`, increasing -- good. */
			if(mono != DECREASING) { /* If decreasing, inflection. */
				mono = INCREASING;
				new_run->size++;
				continue;
			}
		} else if(comp > 0) { /* Decreasing; reverse preserving stability. */
			if(mono != INCREASING) { /* If increasing, inflection. */
				mono = DECREASING;
				b->next = first_iso_a;
				first_iso_a->prev = b;
				new_run->head = first_iso_a = b;
				new_run->size++;
				continue;
			}
			new_run->tail = a; /* Terminating an increasing sequence. */
		} else { /* `a == b`. */
			if(mono == DECREASING) { /* Extend. */
				struct N_(ListNode) *const a_next = a->next;
				b->next = a_next;
				a_next->prev = b;
				a->next = b;
				b->prev = a;
			} else { /* Monotone or weakly increasing. */
				new_run->tail = b;
			}
			new_run->size++;
			continue;
		}
		/* Head and tail don't necessarily correspond to the first and last. */
		new_run->head->prev = new_run->tail->next = 0;
		/* Greedy merge: keeps space to `O(log n)` instead of `O(n)`. */
		for(rc = run_count; !(rc & 1) && runs.run_no >= 2; rc >>= 1)
			PN_(merge_runs)(&runs);
		/* Reset the state machine and output to just `b` at the next run. */
		mono = UNSURE;
		assert(runs.run_no < sizeof(runs.run) / sizeof(*runs.run));
		new_run = runs.run + runs.run_no++, run_count++;
		new_run->size = 1;
		new_run->head = new_run->tail = first_iso_a = b;
	}
	/* Terminating the last increasing sequence. */
	if(mono == INCREASING) new_run->tail = a;
	new_run->tail->next = new_run->head->prev = 0;
	/* Clean up the rest; when only one, propagate `list_runs[0]` to head. */
	while(runs.run_no > 1) PN_(merge_runs)(&runs);
	runs.run[0].head->prev = &list->head;
	runs.run[0].tail->next = &list->tail;
	list->head.next = runs.run[0].head;
	list->tail.prev = runs.run[0].tail;
}


/** Performs a stable, adaptive sort of `list` according to `compare`. Requires
 `LIST_COMPARE`. <Peters 2002, Timsort>, _via_ <McIlroy 1993, Optimistic>, does
 long merges by galloping, but we don't have random access to the data because
 we are in a linked-list; this does natural merge sort.
 @param[list] If null, does nothing.
 @order \Omega(|`list`|), \O(|`list`| log |`list`|)
 @allow */
static void N_(ListSort)(struct N_(List) *const list) {
	if(list) PN_(natural)(list);
}

/** Merges from `from` into `list` according to `compare`. If the elements are
 sorted in both lists, (see <fn:<N>ListSort>,) then the elements of `list` will
 be sorted, too. Requires `LIST_COMPARE`.
 @param[list] If null, then it removes elements.
 @param[from] If null, does nothing, otherwise this list will be empty on
 return.
 @order \O(|`list`| + |`from`|)
 @allow */
static void N_(ListMerge)(struct N_(List) *const list,
	struct N_(List) *const from) {
	if(!from || from == list) return;
	if(!list) { PN_(clear)(from); return; }
	PN_(merge)(list, from);
}

/** Compares `alist` to `blist` as sequences. Requires `LIST_COMPARE`.
 @return The first `LIST_COMPARE` that is not equal to zero, or 0 if they are
 equal. Null is considered as before everything else; two null pointers are
 considered equal.
 @implements <typedef:<PN>Compare> as `<<PN>List>Compare`
 @order \Theta(min(|`alist`|, |`blist`|))
 @allow */
static int N_(ListCompare)(const struct N_(List) *const alist,
	const struct N_(List) *const blist) {
	struct N_(ListNode) *a, *b;
	int diff;
	/* Null counts as `-\infty`. */
	if(!alist) {
		return blist ? -1 : 0;
	} else if(!blist) {
		return 1;
	}
	/* Compare element by element. */
	for(a = alist->head.next, b = blist->head.next; ; a = a->next, b = b->next)
	{
		if(!a->next) {
			return b->next ? -1 : 0;
		} else if(!b->next) {
			return 1;
		} else if((diff = PN_(compare)(a, b))) {
			return diff;
		}
	}
}

/** Moves all local-duplicates of `from` to the end of `to`. Requires
 `LIST_COMPARE`. All parameters must be unique or can be null.

 For example, if `from` is `(A, B, B, A)`, it would concatenate `(B)` to `to`
 and leave `(A, B, A)` in `from`. If one <fn:<N>ListSort> `from` first,
 `(A, A, B, B)`, the global duplicates will be transferred, `(A, B)`.
 @order O(|`from`|)
 @allow */
static void N_(ListDuplicatesTo)(struct N_(List) *const from,
	struct N_(List) *const to) {
	if(!from) return;
	PN_(duplicates)(from, to);
}

/** Subtracts `a` from `b`, as sequential sorted individual elements, and moves
 it to `result`. All elements are removed from `a`. Requires `LIST_COMPARE`.
 All parameters must be unique or can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, a:D)` would be moved to `result`.
 @order \O(|`a`| + |`b`|)
 @allow */
static void N_(ListSubtractionTo)(struct N_(List) *const a,
	struct N_(List) *const b, struct N_(List) *const result) {
	PN_(boolean)(a, b, LO_SUBTRACTION_AB | LO_DEFAULT_A, result);
}

/** Moves the union of `a` and `b` as sequential sorted individual elements to
 `result`. Equal elements are moved from `a`. Requires `LIST_COMPARE`. All
 parameters must be unique or can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, a:B, b:C, a:D)` would be moved to `result`.
 @order \O(|`a`| + |`b`|)
 @allow */
static void N_(ListUnionTo)(struct N_(List) *const a, struct N_(List) *const b,
	struct N_(List) *const result) {
	PN_(boolean)(a, b, LO_SUBTRACTION_AB | LO_SUBTRACTION_BA | LO_INTERSECTION
		| LO_DEFAULT_A | LO_DEFAULT_B, result);
}

/** Moves the intersection of `a` and `b` as sequential sorted individual
 elements to `result`. Equal elements are moved from `a`. Requires
 `LIST_COMPARE`. All parameters must be unique or can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:B)` would be moved to `result`.
 @order \O(|`a`| + |`b`|)
 @allow */
static void N_(ListIntersectionTo)(struct N_(List) *const a,
	struct N_(List) *const b, struct N_(List) *const result) {
	PN_(boolean)(a, b, LO_INTERSECTION, result);
}

/** Moves `a` exclusive-or `b` as sequential sorted individual elements to
 `result`. Equal elements are moved from `a`. Requires `LIST_COMPARE`. All
 parameters must be unique or can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, b:C, a:D)` would be moved to `result`.
 @order O(|`a`| + |`b`|)
 @allow */
static void N_(ListXorTo)(struct N_(List) *const a, struct N_(List) *const b,
	struct N_(List) *const result) {
	PN_(boolean)(a, b, LO_SUBTRACTION_AB | LO_SUBTRACTION_BA | LO_DEFAULT_A
		| LO_DEFAULT_B, result);
}

#endif /* comp --> */

#ifdef LIST_TO_STRING /* <!-- string */
/** Can print 2 things at once before it overwrites. One must set
 `LIST_TO_STRING` to a function implementing <typedef:<PN>ToString> to get this
 functionality.
 @return Prints `list` in a static buffer.
 @order \Theta(1); it has a 1024 character limit; every element takes some.
 @allow */
static const char *N_(ListToString)(const struct N_(List) *const list) {
	static char buffers[2][1024];
	static size_t buffer_i;
	char *buffer = buffers[buffer_i++], *b = buffer;
	const size_t buffers_no = sizeof buffers / sizeof *buffers,
		buffer_size = sizeof *buffers / sizeof **buffers;
	const char space = ' ', start = '(', comma = ',', end = ')',
		*const ellipsis_end = ",…)", *const null = "null";
	const size_t ellipsis_end_len = strlen(ellipsis_end),
		null_len = strlen(null);
	struct N_(ListNode) *link;
	size_t i;
	int is_first = 1;
	/* fixme: I've lost track. */
	assert(!(buffers_no & (buffers_no - 1)) && ellipsis_end_len >= 2
		   && buffer_size >= 1 + 11 + ellipsis_end_len + 1
		   && buffer_size >= null_len + 1);
	/* Advance the buffer for next time. */
	buffer_i &= buffers_no - 1;
	/* Null set. */
	if(!list) { memcpy(b, null, null_len), b += null_len; goto terminate; }
	/* Otherwise */
	*b++ = start;
	for(link = N_(ListFirst)(list); link; link = N_(ListNext)(link)) {
		if(is_first) is_first = 0;
		else *b++ = comma, *b++ = space;
		PN_(to_string)(link, (char (*)[12])b);
		for(i = 0; *b != '\0' && i < 12; b++, i++);
		/* Greedy can not guarantee another; terminate by ellipsis. */
		if((size_t)(b - buffer) > buffer_size - 2 - 11 - ellipsis_end_len - 1)
			goto ellipsis;
	}
	*b++ = end;
	goto terminate;
ellipsis:
	memcpy(b, ellipsis_end, ellipsis_end_len), b += ellipsis_end_len;
terminate:
	*b++ = '\0';
	assert(b <= buffer + buffer_size);
	return buffer;
}
#endif /* string --> */

#ifdef LIST_TEST /* <!-- test: need this file. */
#include "../test/TestList.h" /** \include */
#endif /* test --> */

static void PN_(unused_coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 <http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code> */
static void PN_(unused_set)(void) {
	N_(ListClear)(0);
	N_(ListFirst)(0);
	N_(ListLast)(0);
	N_(ListPrevious)(0);
	N_(ListNext)(0);
	N_(ListUnshift)(0, 0);
	N_(ListPush)(0, 0);
	N_(ListAddBefore)(0, 0);
	N_(ListAddAfter)(0, 0);
	N_(ListRemove)(0);
	N_(ListShift)(0);
	N_(ListPop)(0);
	N_(ListTo)(0, 0);
	N_(ListToIf)(0, 0, 0);
	N_(ListToBefore)(0, 0);
	N_(ListForEach)(0, 0);
	N_(ListAny)(0, 0);
	N_(ListSelfCorrect)(0);
#ifdef LIST_COMPARE /* <!-- comp */
	N_(ListSort)(0);
	N_(ListMerge)(0, 0);
	N_(ListCompare)(0, 0);
	N_(ListSubtractionTo)(0, 0, 0);
	N_(ListUnionTo)(0, 0, 0);
	N_(ListIntersectionTo)(0, 0, 0);
	N_(ListXorTo)(0, 0, 0);
	N_(ListDuplicatesTo)(0, 0);
#endif /* comp --> */
#ifdef LIST_TO_STRING /* <!-- string */
	N_(ListToString)(0);
#endif /* string --> */
	PN_(unused_coda)();
}
static void PN_(unused_coda)(void) { PN_(unused_set)(); }

/* Un-define all macros. Undocumented: allows nestled inclusion in other .h so
 long as `CAT`, _etc_, are the same meaning and `E_`, _etc_, are not
 clobbered. */
#ifdef LIST_SUBTYPE /* <!-- sub */
#undef LIST_SUBTYPE
#else /* sub --><!-- !sub */
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#endif /* !sub --> */
#undef N_
#undef PN_
#undef LIST_NAME
#ifdef LIST_COMPARE /* <!-- !compare */
#undef LIST_COMPARE
#endif /* !compare --> */
#ifdef LIST_TO_STRING /* <!-- string */
#undef LIST_TO_STRING
#endif /* string --> */
#ifdef LIST_TEST /* <!-- test */
#undef LIST_TEST
#endif /* test --> */
