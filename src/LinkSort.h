/** Copyright 2016 Neil Edelman, distributed under the terms of the MIT License;
 < https://opensource.org/licenses/MIT >.

 Generic sort of doubly linked-List. You must #define the required constants
 before including this file; they are undefined at the end of the this file for
 convenience when including multiple link types.

 @param LINK_NAME Name
 NameLinkSort() will be generated, only 1 word; required;
 @param LINK_TYPE struct Name
 because in C, a type does not have to be a single word; required;
 @param LINK_PREV prev
 @param LINK_NEXT next
 within the structure LINK_TYPE, previous and next fields; required. Does not
 do checks that they are valid;
 @param LINK_COMPARATOR LINK_NAMEComparator
 a comparator function taking two pointers to LINK_TYPE and returning an int;
 required,
 @param LINK_STATIC_EXTRA_STORAGE
 this keeps O(log n) space needed for greedy natural merge sort in a static
 global parameter so you don't have to create it repeatedly (probably
 1520 Bytes on a 64 Bit machine, tests have shown that this is negligible.)
 However, if you call this exact NameLinkSort() concurrently with this
 parameter on, it will result in undefined behaviour.

 @author	Neil
 @version	1.0; 2016-11
 @since		1.0; 2016-11 */

#include <stddef.h>	/* size_t */

/* check defines */

#ifndef LINK_NAME
#error Link generic LINK_NAME undefined.
#endif

#ifndef LINK_TYPE
#error Link generic LINK_TYPE undefined.
#endif

#ifndef LINK_PREV
#error Link generic LINK_PREV undefined.
#endif

#ifndef LINK_NEXT
#error Link generic LINK_NEXT undefined.
#endif

#ifndef LINK_COMPARATOR
#error Link generic LINK_COMPARATOR undefined.
#endif

/* After this block, the preprocessor replaces T with LINK_TYPE, T_(X) with
 LINK_NAMEX, PRIVATE_T_(X) with _LINK_NAME_X, and T_NAME with the string
 version.
 <p>
 http://c-faq.com/decl/namespace.html "You may use identifiers consisting of an
 underscore followed by a digit or lower case letter at function, block, or
 prototype scope." So PRIVATE_T_, which makes use of PCAT_, is intended for
 prototype scope.
 <p>
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
#ifdef PRIVATE_T_
#undef PRIVATE_T_
#endif
#ifdef T_NAME
#undef T_NAME
#endif
#ifdef QUOTE
#undef QUOTE
#endif
#ifdef QUOTE_
#undef QUOTE_
#endif
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT(x, y) PCAT_(x, y)
#define PCAT_(x, y) _ ## x ## _ ## y
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)
#define T_(thing) CAT(LINK_NAME, thing)
#define PRIVATE_T_(thing) PCAT(LINK_NAME, thing)
#define T_NAME QUOTE(LINK_NAME)
typedef LINK_TYPE PRIVATE_T_(Type);
#define T PRIVATE_T_(Type)

/* typedef a comparison function */
typedef int (*T_(LinkCompare))(const T *, const T *);

/* global comparison function */
static const T_(LinkCompare) PRIVATE_T_(compare) = LINK_COMPARATOR;

/* A run is a temporary sequence of values in the array that is weakly
 increasing. */
struct PRIVATE_T_(LinkRun) {
	T *head, *tail;
	size_t size;
};

/* Store the maximum capacity for the indexing with size_t. (Overkill, really.)
 range(runs) = Sum_{k=0}^runs 2^{runs-k} - 1
             = 2^{runs+1} - 2
 2^bits      = 2 (r^runs - 1)
 runs        = log(2^{bits-1} + 1) / log 2
 runs       <= 2^{bits - 1}, 2^{bits + 1} > 0 */
struct PRIVATE_T_(LinkRuns) {
	struct PRIVATE_T_(LinkRun) run[(sizeof(size_t) << 3) - 1];
	size_t run_no;
}
#ifdef LINK_STATIC_EXTRA_STORAGE
PRIVATE_T_(runs)
#endif
;

/* generated prototypes */
static void T_(LinkSort)(T **const);

/** Inserts the first element from the larger of two sorted runs, then merges
 the rest. \cite{Peters2002Timsort}, via \cite{McIlroy1993Optimistic}, does
 long merges by galloping, but we don't have random access to the data. In
 practice, this is 2% slower on randomly distributed keys when the linked-list
 size is over 500 000; randomly distributed keys have high insertion times that
 to well in standard merging. However, it's (potentially much) faster when the
 keys have structure: observed, [-2%, 500%].
 <p>
 Assumes array contains at least 2 elements and there are at least two runs. */
static void PRIVATE_T_(natural_merge)(struct PRIVATE_T_(LinkRuns) *const runs) {
	struct PRIVATE_T_(LinkRun) *const run_a = runs->run + runs->run_no - 2;
	struct PRIVATE_T_(LinkRun) *const run_b = run_a + 1;
	T *a = run_a->tail, *b = run_b->head;
	T *chosen;

	if(run_a->size <= run_b->size) {
		T *prev_chosen;

		/* run a is smaller: downwards insert b.head followed by upwards
		 merge */

		/* insert the first element of b downwards into a */
		for( ; ; ) {
			if(PRIVATE_T_(compare)(a, b) <= 0) {
				chosen = a;
				a = a->LINK_NEXT;
				break;
			}
			if(!a->LINK_PREV) {
				run_a->head = run_b->head;
				chosen = b;
				b = b->LINK_NEXT;
				break;
			}
			a = a->LINK_PREV;
		}

		/* merge upwards, while the lists are interleaved */
		while(chosen->LINK_NEXT) {
			prev_chosen = chosen;
			if(PRIVATE_T_(compare)(a, b) > 0) {
				chosen = b;
				b = b->LINK_NEXT;
			} else {
				chosen = a;
				a = a->LINK_NEXT;
			}
			prev_chosen->LINK_NEXT = chosen;
			chosen->LINK_PREV = prev_chosen;
		}

		/* splice the one list left */
		if(!a) {
			b->LINK_PREV = chosen;
			chosen->LINK_NEXT = b;
			run_a->tail = run_b->tail;
		} else {
			a->LINK_PREV = chosen;
			chosen->LINK_NEXT = a;
		}

	} else {
		T *next_chosen;
		int is_a_tail = 0;

		/* run b is smaller; upwards insert followed by downwards merge */

		/* insert the last element of a upwards into b */
		for( ; ; ) {
			if(PRIVATE_T_(compare)(a, b) <= 0) {
				chosen = b;
				b = b->LINK_PREV;
				break;
			}
			/* here, a > b */
			if(!b->LINK_NEXT) {
				is_a_tail = -1;
				chosen = a;
				a = a->LINK_PREV;
				break;
			}
			b = b->LINK_NEXT;
		}
		if(!is_a_tail) run_a->tail = run_b->tail;

		/* merge downwards, while the lists are interleaved */
		while(chosen->LINK_PREV) {
			next_chosen = chosen;
			if(PRIVATE_T_(compare)(a, b) > 0) {
				chosen = a;
				a = a->LINK_PREV;
			} else {
				chosen = b;
				b = b->LINK_PREV;
			}
			next_chosen->LINK_PREV = chosen;
			chosen->LINK_NEXT = next_chosen;
		}

		/* splice the one list left */
		if(!a) {
			b->LINK_NEXT = chosen;
			chosen->LINK_PREV = b;
			run_a->head = run_b->head;
		} else {
			a->LINK_NEXT = chosen;
			chosen->LINK_PREV = a;
		}

	}

	run_a->size += run_b->size;
	runs->run_no--;

}

/** Greedy natural insertion-merge sort on doubly-linked lists.
 @param phead	A pointer to the head of the list, which is a pointer to
 				LIST_TYPE; the head of the list will, in general, change,
 				unless it's the smallest item. */
static void T_(LinkSort)(T **const phead) {
#ifndef LINK_STATIC_EXTRA_STORAGE
	/* allocate enough space on the stack for the maximum possible runs */
	struct PRIVATE_T_(LinkRuns) PRIVATE_T_(runs);
#endif
	/* simulate call-by-reference */
	T *head;
	/* new_run is an index into link_runs, a temporary sorting structure;
	 head is first smallest, tail is last largest */
	struct PRIVATE_T_(LinkRun) *new_run;
	/* part of the state machine for classifying points wrt their neighbours */
	enum { UNSURE, INCREASING, DECREASING } mono;
	/* the data that we are sorting */
	T *a, *b, *c, *first_iso_a;
	/* run_count is different from link_runs.run_no in that it only increases;
	 only used for calculating the path up the tree */
	size_t run_count, rc;
	/* the value of the comparison */
	int comp;

	if(!phead || !(head = *phead)) return; /* ensure we have an 'a' */

	/* reset the state machine and output to just 'a' in the first run */
	mono = UNSURE;
	PRIVATE_T_(runs).run_no = 1;
	new_run = PRIVATE_T_(runs).run + 0, run_count = 1;
	new_run->size = 1;
	a = first_iso_a = new_run->head = new_run->tail = head;

	for(b = a->LINK_NEXT; b; a = b, b = c) {

		/* b.next can be modified, and we always want the iteration original */
		c = b->LINK_NEXT;

		comp = PRIVATE_T_(compare)(a, b);

		/* state machine that considers runs in both directions -- in practice,
		 slightly slower than only considering increasing runs on most cases;
		 however, I would hate to see my code replaced with one line; reverse
		 order is 15 times faster, but it's not likely */
		if(comp < 0) { /* a < b, increasing -- good */
			if(mono != DECREASING) { /* if decreasing, inflection */
				mono = INCREASING;
				new_run->size++;
				continue;
			}
		} else if(comp > 0) { /* decreasing; reverse preserving stability */
			if(mono != INCREASING) { /* if increasing, inflection */
				mono = DECREASING;
				b->LINK_NEXT = first_iso_a;
				first_iso_a->LINK_PREV = b;
				new_run->head = first_iso_a = b;
				new_run->size++;
				continue;
			}
			new_run->tail = a; /* terminating an increasing sequence */
		} else { /* a == b */
			if(mono == DECREASING) { /* extend */
				T *const a_next = a->LINK_NEXT;
				b->LINK_NEXT = a_next;
				a_next->LINK_PREV = b;
				a->LINK_NEXT = b;
				b->LINK_PREV = a;
			} else { /* weakly increasing */
				new_run->tail = b;
			}
			new_run->size++;
			continue;
		}
		/* head and tail don't necessarily correspond to the first and last */
		new_run->head->LINK_PREV = new_run->tail->LINK_NEXT = 0;

		/* greedy merge: keeps space to O(log n) instead of O(n) */
		for(rc = run_count; !(rc & 1) && PRIVATE_T_(runs).run_no >= 2; rc >>= 1)
			PRIVATE_T_(natural_merge)(&PRIVATE_T_(runs));
		/* reset the state machine and output to just 'b' at the next run */
		mono = UNSURE;
		new_run = PRIVATE_T_(runs).run + PRIVATE_T_(runs).run_no++, run_count++;
		new_run->size = 1;
		new_run->head = new_run->tail = first_iso_a = b;
	}

	/* terminating the last increasing sequence */
	if(mono == INCREASING) new_run->tail = a;
	new_run->tail->LINK_NEXT = new_run->head->LINK_PREV = 0;

	/* clean up the rest; when only one run, propagate link_runs[0] to head */
	while(PRIVATE_T_(runs).run_no > 1)
		PRIVATE_T_(natural_merge)(&PRIVATE_T_(runs));
	*phead = PRIVATE_T_(runs).run[0].head;
}

#undef LINK_NAME
#undef LINK_TYPE
#undef LINK_PREV
#undef LINK_NEXT
#undef LINK_COMPARATOR
#undef LINK_STATIC_EXTRA_STORAGE
