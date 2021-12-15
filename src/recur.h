/* @license 2021 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Recur trait

 Included by `list.h`.

 @param[RC_]
 A one-argument macro producing a name that is responsible for the name of the
 functions. The caller is responsible for undefining `RC_`.

 @param[COMPARE_NAME, COMPARE_IS_EQUAL, COMPARE]
 Optional unique name that satisfies `C` naming conventions when mangled,
 and a function implementing, for `COMPARE_IS_EQUAL`,
 <typedef:<PRC>bipredicate_fn> that establishes an equivalence relation, or
 for `COMPARE`, <typedef:<PRC>compare_fn> that establishes a total
 order. There can be multiple comparable functions, but only one can omit
 `COMPARE_NAME`.

 @std C89 */

#if !defined(RC_) || !(defined(LIST_IS_EQUAL) ^ defined(LIST_COMPARE)) \
	|| !defined(L_) || !defined(PL_)
#error Unexpected preprocessor symbols.
#endif

#ifndef RECUR_H /* <!-- idempotent */
#define RECUR_H
#if defined(PRC_)
#error Unexpected defines.
#endif
#define PRC_(n) LIST_CAT(recur, RC_(n))
/* <fn:<PRC>boolean> operations bit-vector; dummy `RECUR*` ensures closed. */
enum recur_operation {
	RECUR_SUBTRACTION_AB = 1,
	RECUR_SUBTRACTION_BA = 2,  RECURA,
	RECUR_INTERSECTION   = 4,  RECURB, RECURC, RECURD,
	RECUR_DEFAULT_A      = 8,  RECURE, RECURF, RECURG, RECURH, RECURI, RECURJ,
		RECURK,
	RECUR_DEFAULT_B      = 16, RECURL, RECURM, RECURN, RECURO, RECURP, RECURQ,
		RECURR, RECURS, RECURT, RECURU, RECURV, RECURW, RECURX, RECURY, RECURZ
};
#endif /* idempotent --> */

/** Returns a boolean given two read-only <typedef:<L>listlink>. */
typedef int (*PRC_(bipredicate_fn))(const struct L_(listlink) *,
	const struct L_(listlink) *);

#ifdef LIST_COMPARE /* <!-- compare */

/** Three-way comparison on a totally order set of <typedef:<L>listlink>;
 returns an integer value less then, equal to, greater then zero, if
 `a < b`, `a == b`, `a > b`, respectively. */
typedef int (*PRC_(compare_fn))(const struct L_(listlink) *a,
	const struct L_(listlink) *b);

/* Check that `LIST_COMPARE` is a function implementing
 <typedef:<PRC>compare_fn>. */
static const PRC_(compare_fn) PRC_(compare) = (LIST_COMPARE);

/** Lexicographically compares `alist` to `blist`. Null values are before
 everything.
 @return `a < b`: negative; `a == b`: zero; `a > b`: positive.
 @implements <typedef:<PL>compare_fn> (one can `qsort` an array of lists, as
 long as one calls <fn:<L>list_self_correct> on it's elements)
 @order \Theta(min(|`alist`|, |`blist`|)) @allow */
static int RC_(compare)(const struct L_(list) *const alist,
	const struct L_(list) *const blist) {
	struct L_(listlink) *a, *b;
	int diff;
	/* Null counts as `-\infty`. */
	if(!alist) {
		return blist ? -1 : 0;
	} else if(!blist) {
		return 1;
	}
	/* Compare element by element. */
	for(a = alist->u.flat.next, b = blist->u.flat.next; ;
		a = a->next, b = b->next) {
		if(!a->next) {
			return b->next ? -1 : 0;
		} else if(!b->next) {
			return 1;
		} else if((diff = PRC_(compare)(a, b))) {
			return diff;
		}
	}
}

/** Merges `from` into `to`, preferring elements from `to` go in the front.
 @order \O(|`from`| + |`to`|). */
static void RC_(merge)(struct L_(list) *const to, struct L_(list) *const from) {
	struct L_(listlink) *head, **x = &head, *prev = &to->u.as_head.head, *t, *f;
	assert(to && to->u.flat.next && to->u.flat.prev
		&& from && from->u.flat.next && from->u.flat.prev && from != to);
	/* Empty. */
	if(!(f = from->u.flat.next)->next) return;
	if(!(t = to->u.flat.next)->next)
		{ PL_(move)(from, &to->u.as_tail.tail); return; }
	/* Exclude sentinel. */
	from->u.flat.prev->next = to->u.flat.prev->next = 0;
	/* Merge. */
	for( ; ; ) {
		if(PRC_(compare)(t, f) <= 0) {
			t->prev = prev, prev = *x = t, x = &t->next;
			if(!(t = t->next)) { *x = f; goto from_left; }
		} else {
			f->prev = prev, prev = *x = f, x = &f->next;
			if(!(f = f->next)) { *x = t; break; }
		}
	}
	if(0) {
from_left:
		f->prev = prev;
		/* Switch sentinels. */
		f = from->u.flat.prev;
		to->u.flat.prev = f;
		f->next = &from->u.as_tail.tail;
	} else {
		t->prev = prev;
	}
	/* Erase `from`. */
	from->u.flat.next = &from->u.as_tail.tail;
	from->u.flat.prev = &from->u.as_head.head;
}

/** Merges the two top runs referenced by `head_ptr` in stack form. */
static void PRC_(merge_runs)(struct L_(listlink) **const head_ptr) {
	struct L_(listlink) *head = *head_ptr, **x = &head, *b = head, *a = b->prev,
		*const prev = a->prev;
	assert(head_ptr && a && b);
	for( ; ; ) {
		if(PRC_(compare)(a, b) <= 0) {
			*x = a, x = &a->next;
			if(!(a = a->next)) { *x = b; break; }
		} else {
			*x = b, x = &b->next;
			if(!(b = b->next)) { *x = a; break; }
		}
	}
	head->prev = prev, *head_ptr = head; /* `prev` is the previous run. */
}

/** The list form of `list` is restored from `head` in stack form with two
 runs. */
static void PRC_(merge_final)(struct L_(list) *const list,
	struct L_(listlink) *head) {
	struct L_(listlink) *prev = 0, **x = &list->u.flat.next,
		*b = head, *a = head->prev;
	assert(list && b && a && !a->prev);
	for( ; ; ) {
		if(PRC_(compare)(a, b) <= 0) {
			a->prev = prev, prev = *x = a, x = &a->next;
			if(!(a = a->next)) { a = *x = b; break; }
		} else {
			b->prev = prev, prev = *x = b, x = &b->next;
			if(!(b = b->next)) { *x = a; break; }
		}
	}
	do; while(a->prev = prev, prev = a, a = a->next);
	prev->next = &list->u.as_tail.tail, list->u.flat.prev = prev;
	/* Not empty. */
	assert(list->u.flat.next && list->u.flat.next != &list->u.as_tail.tail);
	list->u.flat.next->prev = &list->u.as_head.head;
}

/** Natural merge sort `list`; the requirement for \O(\log |`list`|) space is
 satisfied by converting it to a singly-linked with `prev` as a stack of
 increasing lists, which are merged. */
static void PRC_(sort)(struct L_(list) *const list) {
	/* Add `[-1,0,1]`: unique identifier for nine weakly-ordered transitions. */
	enum { DEC = 1, EQ = 4, INC = 7 };
	int mono = EQ, cmp;
	struct L_(listlink) *a, *b, *c, *dec_iso = /* Unused. */0;
	struct { size_t count; struct L_(listlink) *head, *prev; } run;
	/* Closed sentinel list. */
	assert(list
		&& list->u.flat.next && !list->u.flat.zero && list->u.flat.prev);
	if(a = list->u.flat.next, !(b = a->next)) return; /* Empty. */
	/* Identify runs of monotonicity until `b` sentinel. */
	run.count = 0, run.prev = 0, run.head = a;
	for(c = b->next; c; a = b, b = c, c = c->next) {
		cmp = PRC_(compare)(b, a);
		switch(mono + (0 < cmp) - (cmp < 0)) {
			/* Valley and mountain inflection. */
		case INC - 1: a->next = 0; /* _Sic_. */
		case DEC + 1: break;
			/* Decreasing more and levelled off from decreasing. */
		case DEC - 1: b->next = dec_iso; dec_iso = run.head = b; continue;
		case DEC + 0: b->next = a->next; a->next = b; continue;
			/* Turning down and up. */
		case EQ  - 1: a->next = 0; b->next = run.head; dec_iso = run.head = b;
			mono = DEC; continue;
		case EQ  + 1: mono = INC; continue;
		case EQ  + 0: /* Same. _Sic_. */
		case INC + 0: /* Levelled off from increasing. _Sic_. */
		case INC + 1: continue; /* Increasing more. */
		}
		/* Binary carry sequence, <https://oeis.org/A007814>, one delayed so
		 always room for final merge. */
		if(run.count) {
			size_t rc;
			for(rc = run.count - 1; rc & 1; rc >>= 1)
				PRC_(merge_runs)(&run.prev);
		}
		/* Add to runs, advance; `b` becomes `a` forthwith. */
		run.head->prev = run.prev, run.prev = run.head, run.count++;
		run.head = b, mono = EQ;
	}
	/* Last run; go into an accepting state. */
	if(mono != DEC) {
		if(!run.count) return; /* Sorted already. */
		a->next = 0; /* Last one of increasing or equal. */
	} else { /* Decreasing. */
		assert(dec_iso);
		run.head = dec_iso;
		if(!run.count) { /* Restore the pointers without having two runs. */
			list->u.flat.next = dec_iso, dec_iso->prev = &list->u.as_head.head;
			for(a = dec_iso, b = a->next; b; a = b, b = b->next) b->prev = a;
			list->u.flat.prev = a, a->next = &list->u.as_tail.tail;
			return; /* Reverse sorted; now good as well. */
		}
	}
	assert(run.count);
	/* (Actually slower to merge the last one eagerly. So do nothing.) */
	run.head->prev = run.prev, run.count++;
	/* Merge leftovers from the other direction, saving one for final. */
	while(run.head->prev->prev) PRC_(merge_runs)(&run.head);
	PRC_(merge_final)(list, run.head);
}

/** Performs a stable, adaptive sort of `list` according to `compare`.
 @order \Omega(|`list`|), \O(|`list`| log |`list`|) @allow */
static void RC_(sort)(struct L_(list) *const list) { PRC_(sort)(list); }

/** Private: `alist` `mask` `blist` -> `result`. Prefers `a` to `b` when equal.
 Either could be null.
 @order \O(|`a`| + |`b`|) */
static void PRC_(boolean)(struct L_(list) *const alist,
	struct L_(list) *const blist,
	const enum recur_operation mask, struct L_(list) *const result) {
	struct L_(listlink) *temp,
		*a = alist ? alist->u.flat.next : 0,
		*b = blist ? blist->u.flat.next : 0;
	int comp;
	assert((!result || (result != alist && result != blist))
		&& (!alist || (alist != blist)));
	if(a && b) {
		while(a->next && b->next) {
			comp = PRC_(compare)(a, b);
			if(comp < 0) {
				temp = a, a = a->next;
				if(mask & RECUR_SUBTRACTION_AB) {
					PL_(remove)(temp);
					if(result) PL_(push)(result, temp);
				}
			} else if(comp > 0) {
				temp = b, b = b->next;
				if(mask & RECUR_SUBTRACTION_BA) {
					PL_(remove)(temp);
					if(result) PL_(push)(result, temp);
				}
			} else {
				temp = a, a = a->next, b = b->next;
				if(mask & RECUR_INTERSECTION) {
					PL_(remove)(temp);
					if(result) PL_(push)(result, temp);
				}
			}
		}
	}
	if(a && mask & RECUR_DEFAULT_A) {
		while((temp = a, a = a->next)) {
			PL_(remove)(temp);
			if(result) PL_(push)(result, temp);
		}
	}
	if(b && mask & RECUR_DEFAULT_B) {
		while((temp = b, b = b->next)) {
			PL_(remove)(temp);
			if(result) PL_(push)(result, temp);
		}
	}
}

/** Subtracts `a` from `b`, as sequential sorted individual elements, and moves
 it to `result`. All elements are removed from `a`. All parameters must be
 unique or can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, a:D)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static void RC_(subtraction_to)(struct L_(list) *const a,
	struct L_(list) *const b, struct L_(list) *const result) {
	PRC_(boolean)(a, b, RECUR_SUBTRACTION_AB | RECUR_DEFAULT_A, result);
}

/** Moves the union of `a` and `b` as sequential sorted individual elements to
 `result`. Equal elements are moved from `a`. All parameters must be unique or
 can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, a:B, b:C, a:D)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static void RC_(union_to)(struct L_(list) *const a,
	struct L_(list) *const b, struct L_(list) *const result) {
	PRC_(boolean)(a, b, RECUR_SUBTRACTION_AB | RECUR_SUBTRACTION_BA
		| RECUR_INTERSECTION | RECUR_DEFAULT_A | RECUR_DEFAULT_B, result);
}

/** Moves the intersection of `a` and `b` as sequential sorted individual
 elements to `result`. Equal elements are moved from `a`. All parameters must
 be unique or can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:B)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static void RC_(intersection_to)(struct L_(list) *const a,
	struct L_(list) *const b, struct L_(list) *const result) {
	PRC_(boolean)(a, b, RECUR_INTERSECTION, result);
}

/** Moves `a` exclusive-or `b` as sequential sorted individual elements to
 `result`. Equal elements are moved from `a`. All parameters must be unique or
 can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, b:C, a:D)` would be moved to `result`.
 @order O(|`a`| + |`b`|) @allow */
static void RC_(xor_to)(struct L_(list) *const a, struct L_(list) *const b,
	struct L_(list) *const result) {
	PRC_(boolean)(a, b, RECUR_SUBTRACTION_AB | RECUR_SUBTRACTION_BA
		| RECUR_DEFAULT_A | RECUR_DEFAULT_B, result);
}

/** !compare(`a`, `b`) == equals(`a`, `b`).
 @implements <typedef:<PRC>bipredicate_fn> */
static int PRC_(is_equal)(const struct L_(listlink) *const a,
	const struct L_(listlink) *const b) { return !PRC_(compare)(a, b); }

#else /* compare --><!-- is equal */

/* Check that `LIST_IS_EQUAL` is a function implementing
 <typedef:<PRC>bipredicate_fn>. */
static const PRC_(bipredicate_fn) PRC_(is_equal) = (LIST_IS_EQUAL);

#endif /* is equal --> */

/** @return If `lista` piecewise equals `listb`, which both can be null.
 @order \O(min(|`lista`|, |`listb`|)) @allow */
static int RC_(is_equal)(const struct L_(list) *const lista,
	const struct L_(list) *const listb) {
	const struct L_(listlink) *a, *b;
	if(!lista) return !listb;
	if(!listb) return 0;
	for(a = lista->u.flat.next, b = listb->u.flat.next; ;
		a = a->next, b = b->next) {
		if(!a->next) return !b->next;
		if(!b->next) return 0;
		if(!PRC_(is_equal)(a, b)) return 0;
	}
}

/** Moves all local-duplicates of `from` to the end of `to`.

 For example, if `from` is `(A, B, B, A)`, it would concatenate the second
 `(B)` to `to` and leave `(A, B, A)` in `from`. If one <fn:<L>list_sort> `from`
 first, `(A, A, B, B)`, the global duplicates will be transferred, `(A, B)`.
 @order \O(|`from`|) @allow */
static void RC_(duplicates_to)(struct L_(list) *const from,
	struct L_(list) *const to) {
	struct L_(listlink) *a = from->u.flat.next, *b, *temp;
	assert(from);
	if(!(b = a->next)) return;
	while(b->next) {
		if(!PRC_(is_equal)(a, b)) {
			a = b, b = b->next;
		} else {
			temp = b, b = b->next;
			PL_(remove)(temp);
			if(to) PL_(push)(to, temp);
		}
	}
}

static void PRC_(unused_recur_coda)(void);
static void PRC_(unused_recur)(void) {
#ifdef LIST_COMPARE /* <!-- compare */
	RC_(compare)(0, 0); RC_(merge)(0, 0); RC_(sort)(0);
	RC_(subtraction_to)(0, 0, 0); RC_(union_to)(0, 0, 0);
	RC_(intersection_to)(0, 0, 0); RC_(xor_to)(0, 0, 0);
#endif /* compare --> */
	RC_(is_equal)(0, 0); RC_(duplicates_to)(0, 0);
	PRC_(unused_recur_coda)();
}
static void PRC_(unused_recur_coda)(void) { PRC_(unused_recur)(); }

#ifdef BOX_COMPARE
#undef BOX_COMPARE
#endif
#ifdef BOX_IS_EQUAL
#undef BOX_IS_EQUAL
#endif
#ifdef BOX_COMPARE_NAME
#undef BOX_COMPARE_NAME
#endif
