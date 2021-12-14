/* @license 2021 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Recur trait

 Requires a lot. Fixme.

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

#if !defined(BOX_) || !defined(BOX_CONTAINER) || !defined(BOX_CONTENTS) \
	|| !defined(RC_) || !defined(BOX_HEAD) || !defined(BOX_REMOVE) \
	|| !defined(BOX_PUSH) || !(defined(BOX_IS_EQUAL) ^ defined(BOX_COMPARE))
#error Unexpected preprocessor symbols.
#endif

#ifdef BOX_IS_EQUAL
#error BOX_IS_EQUAL not implemented.
#endif

#ifndef RECURRENT_H /* <!-- idempotent */
#define RECURRENT_H
#if defined(LIST_COMPARE_CAT_) || defined(LIST_COMPARE_CAT) || defined(PRC_)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define RECUR_CAT_(n, m) n ## _ ## m
#define RECUR_CAT(n, m) RECUR_CAT_(n, m)
#define PRC_(n) RECUR_CAT(recur, RC_(n))
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

/** <recur.h>: an alias to the box. */
typedef BOX_CONTAINER PRC_(box);

/** <recur.h>: an alias to the individual type contained in the box. */
typedef BOX_CONTENTS PRC_(type);

/** Operates by side-effects on <typedef:<PRC>type>. */
typedef void (*PRC_(action_fn))(PRC_(type) *);

/** Operates by side-effects on <typedef:<PRC>box> and <typedef:<PRC>type>. */
typedef void (*PRC_(mutator_fn))(PRC_(box) *, PRC_(type) *);

/** Chooses one out of box. */
typedef PRC_(type) *(*PRC_(choose_fn))(const PRC_(box) *);

/** Returns less then, equal to, or greater then zero, inducing an ordering
 between `a` and `b`. */
typedef int (*PRC_(compare_fn))(const PRC_(type) *a, const PRC_(type) *b);

/* Check that `LIST_HEAD` is a function implementing
 <typedef:<PRC>choose_fn>. */
static const PRC_(choose_fn) PRC_(head) = (BOX_HEAD);

/* Check that `LIST_REMOVE` is a function implementing
 <typedef:<PRC>compare_fn>. */
static const PRC_(action_fn) PRC_(remove) = (BOX_REMOVE);

/* Check that `LIST_REMOVE` is a function implementing
 <typedef:<PRC>compare_fn>. */
static const PRC_(mutator_fn) PRC_(push) = (BOX_PUSH);

/* Check that `LIST_COMPARE` is a function implementing
 <typedef:<PRC>compare_fn>. */
static const PRC_(compare_fn) PRC_(compare) = (BOX_COMPARE);

/** Returns a boolean given two read-only <typedef:<PCM>type>. */
/*typedef int (*PRC_(bipredicate_fn))(const PRC_(type) *, const PRC_(type) *);*/

/** Returns a boolean given two <typedef:<PCM>type>. */
/*typedef int (*PRC_(biaction_fn))(PRC_(type) *, PRC_(type) *);*/

/** Merges `from` into `to`, preferring elements from `to` go in the front.
 @order \O(|`from`| + |`to`|). @fixme Assumes list. */
static void RC_(merge)(PRC_(box) *const to, PRC_(box) *const from) {
	PRC_(type) *head, **x = &head, *prev = &to->u.as_head.head, *t, *f;
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
	from->u.flat.next = &from->u.as_tail.tail;
	from->u.flat.prev = &from->u.as_head.head;
}

/** Merges the two top runs referenced by `head_ptr` in stack form. */
static void PRC_(merge_runs)(PRC_(type) **const head_ptr) {
	PRC_(type) *head = *head_ptr, **x = &head, *b = head, *a = b->prev,
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
static void PRC_(merge_final)(PRC_(box) *const list, PRC_(type) *head) {
	PRC_(type) *prev = 0, **x = &list->u.flat.next, *b = head, *a = head->prev;
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
static void PRC_(sort)(PRC_(box) *const list) {
	/* Add `[-1,0,1]`: unique identifier for nine weakly-ordered transitions. */
	enum { DEC = 1, EQ = 4, INC = 7 };
	int mono = EQ, cmp;
	PRC_(type) *a, *b, *c, *dec_iso = /* Unused. */0;
	struct { size_t count; PRC_(type) *head, *prev; } run;
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
static void RC_(sort)(PRC_(box) *const list) { PRC_(sort)(list); }

/** Compares `alist` to `blist` as sequences.
 @return The first `LIST_COMPARE` that is not equal to zero, or 0 if they are
 equal. Null is considered as before everything else; two null pointers are
 considered equal. @implements <typedef:<PL>compare_fn>
 @order \Theta(min(|`alist`|, |`blist`|)) @allow */
static int PRC_(list_compare)(const PRC_(box) *const alist,
	const PRC_(box) *const blist) {
	PRC_(type) *a, *b;
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

/** Moves all local-duplicates of `from` to the end of `to`.

 For example, if `from` is `(A, B, B, A)`, it would concatenate `(B)` to `to`
 and leave `(A, B, A)` in `from`. If one <fn:<L>list_sort> `from` first,
 `(A, A, B, B)`, the global duplicates will be transferred, `(A, B)`.
 @order \O(|`from`|) @allow */
static void PRC_(duplicates_to)(PRC_(box) *const from,
	PRC_(box) *const to) {
	PRC_(type) *a = from->u.flat.next, *b, *temp;
	assert(from);
	if(!(b = a->next)) return;
	while(b->next) {
		if(PRC_(compare)(a, b)) {
			a = b, b = b->next;
		} else {
			temp = b, b = b->next;
			PRC_(remove)(temp);
			if(to) L_(list_add_before)(&to->u.as_tail.tail, temp);
		}
	}
}


/** Private: `alist` `mask` `blist` -> `result`. Prefers `a` to `b` when equal.
 @order \O(|`a`| + |`b`|) */
static void PRC_(boolean)(PRC_(box) *const abox, PRC_(box) *const bbox,
	const enum recur_operation mask, PRC_(box) *const result) {
	PRC_(type) *a = PRC_(head)(abox), *b = PRC_(head)(bbox), *temp;
	int comp;
	assert((!result || (result != abox && result != bbox))
		&& (!abox || (abox != bbox)));
	if(a && b) {
		while(a->next && b->next) {
			comp = PRC_(compare)(a, b);
			if(comp < 0) {
				temp = a, a = a->next;
				if(mask & RECUR_SUBTRACTION_AB) {
					PRC_(remove)(temp);
					if(result) PRC_(push)(result, temp);
				}
			} else if(comp > 0) {
				temp = b, b = b->next;
				if(mask & RECUR_SUBTRACTION_BA) {
					PRC_(remove)(temp);
					if(result) PRC_(push)(result, temp);
				}
			} else {
				temp = a, a = a->next, b = b->next;
				if(mask & RECUR_INTERSECTION) {
					PRC_(remove)(temp);
					if(result) PRC_(push)(result, temp);
				}
			}
		}
	}
	if(a && mask & RECUR_DEFAULT_A) {
		while((temp = a, a = a->next)) {
			PRC_(remove)(temp);
			if(result) PRC_(push)(result, temp);
		}
	}
	if(b && mask & RECUR_DEFAULT_B) {
		while((temp = b, b = b->next)) {
			PRC_(remove)(temp);
			if(result) PRC_(push)(result, temp);
		}
	}
}



/** Subtracts `a` from `b`, as sequential sorted individual elements, and moves
 it to `result`. All elements are removed from `a`. All parameters must be
 unique or can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, a:D)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static void PRC_(list_subtraction_to)(PRC_(box) *const a,
	PRC_(box) *const b, PRC_(box) *const result) {
	PRC_(boolean)(a, b, RECUR_SUBTRACTION_AB | RECUR_DEFAULT_A, result);
}

/** Moves the union of `a` and `b` as sequential sorted individual elements to
 `result`. Equal elements are moved from `a`. All parameters must be unique or
 can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, a:B, b:C, a:D)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static void PRC_(list_union_to)(PRC_(box) *const a,
	PRC_(box) *const b, PRC_(box) *const result) {
	PRC_(boolean)(a, b, RECUR_SUBTRACTION_AB | RECUR_SUBTRACTION_BA
		| RECUR_INTERSECTION | RECUR_DEFAULT_A | RECUR_DEFAULT_B, result);
}

/** Moves the intersection of `a` and `b` as sequential sorted individual
 elements to `result`. Equal elements are moved from `a`. All parameters must
 be unique or can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:B)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static void PRC_(list_intersection_to)(PRC_(box) *const a,
	PRC_(box) *const b, PRC_(box) *const result) {
	PRC_(boolean)(a, b, RECUR_INTERSECTION, result);
}

/** Moves `a` exclusive-or `b` as sequential sorted individual elements to
 `result`. Equal elements are moved from `a`. All parameters must be unique or
 can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, b:C, a:D)` would be moved to `result`.
 @order O(|`a`| + |`b`|) @allow */
static void PRC_(list_xor_to)(PRC_(box) *const a, PRC_(box) *const b,
	PRC_(box) *const result) {
	PRC_(boolean)(a, b, RECUR_SUBTRACTION_AB | RECUR_SUBTRACTION_BA
		| RECUR_DEFAULT_A | RECUR_DEFAULT_B, result);
}


#if 0

/* Array *************/

#ifdef BOX_COMPARE /* <!-- compare */

/** Three-way comparison on a totally order set of <typedef:<PCM>type>; returns
 an integer value less then, equal to, greater then zero, if
 `a < b`, `a == b`, `a > b`, respectively. */
typedef int (*PRC_(compare_fn))(const PRC_(type) *a, const PRC_(type) *b);

/* Check that `BOX_COMPARE` is a function implementing
 <typedef:<PCM>compare_fn>. */
static const PRC_(compare_fn) PRC_(compare) = (BOX_COMPARE);

/** Lexicographically compares <typedef:<PCM>box> `a` to `b`. Null values are
 before everything.
 @return `a < b`: negative; `a == b`: zero; `a > b`: positive.
 @order \O(`a.size`) @allow */
static int CM_(compare)(const PRC_(box) *const a, const PRC_(box) *const b) {
	PRC_(type) *ia, *ib, *end;
	int diff;
	/* Null counts as `-\infty`. */
	if(!a) return b ? -1 : 0;
	else if(!b) return 1;
	if(a->size > b->size) {
		for(ia = a->data, ib = b->data, end = ib + b->size; ib < end;
			ia++, ib++) if((diff = PRC_(compare)(ia, ib))) return diff;
		return 1;
	} else {
		for(ia = a->data, ib = b->data, end = ia + a->size; ia < end;
			ia++, ib++) if((diff = PRC_(compare)(ia, ib))) return diff;
		return -(a->size != b->size);
	}
}

/** !compare(`a`, `b`) == equals(`a`, `b`).
 @implements <typedef:<PCM>bipredicate_fn> */
static int PRC_(is_equal)(const PRC_(type) *const a, const PRC_(type) *const b)
	{ return !PRC_(compare)(a, b); }

#else /* compare --><!-- is equal */

/* Check that `BOX_IS_EQUAL` is a function implementing
 <typedef:<PCM>bipredicate_fn>. */
static const PRC__(bipredicate_fn) PRC__(is_equal) = (BOX_IS_EQUAL);

#endif /* is equal --> */

/** @return If <typedef:<PCM>box> `a` piecewise equals `b`, which both can be
 null. @order \O(`size`) @allow */
static int CM_(is_equal)(const PRC_(box) *const a, const PRC_(box) *const b) {
	const PRC_(type) *ia, *ib, *end;
	if(!a) return !b;
	if(!b || a->size != b->size) return 0;
	for(ia = a->data, ib = b->data, end = ia + a->size; ia < end; ia++, ib++)
		if(!PRC_(is_equal)(ia, ib)) return 0;
	return 1;
}

/** Removes consecutive duplicate elements in <typedef:<PCM>box> `a`.
 @param[merge] Controls surjection. Called with duplicate elements, if false
 `(x, y)->(x)`, if true `(x,y)->(y)`. More complex functions, `(x, y)->(x+y)`
 can be simulated by mixing the two in the value returned. Can be null: behaves
 like false. @order \O(`a.size` \times `merge`) @allow */
static void CM_(unique_merge)(PRC_(box) *const a, const PRC_(biaction_fn) merge) {
	size_t target, from, cursor, choice, next, move;
	const size_t last = a->size;
	int is_first, is_last;
	assert(a);
	for(target = from = cursor = 0; cursor < last; cursor += next) {
		/* Bijective `[from, cursor)` is moved lazily. */
		for(choice = 0, next = 1; cursor + next < last && PRC_(is_equal)(a->data
			+ cursor + choice, a->data + cursor + next); next++)
			if(merge && merge(a->data + choice, a->data + next)) choice = next;
		if(next == 1) continue;
		/* Must move injective `cursor + choice \in [cursor, cursor + next)`. */
		is_first = !choice;
		is_last  = (choice == next - 1);
		move = cursor - from + (size_t)is_first;
		memmove(a->data + target, a->data + from, sizeof *a->data * move),
		target += move;
		if(!is_first && !is_last) memcpy(a->data + target,
			a->data + cursor + choice, sizeof *a->data), target++;
		from = cursor + next - (size_t)is_last;
	}
	/* Last differed move. */
	move = last - from;
	memmove(a->data + target, a->data + from, sizeof *a->data * move),
	target += move, assert(a->size >= target);
	a->size = target;
}

/** Removes consecutive duplicate elements in <typedef:<PCM>box> `a`.
 @order \O(`a.size`) @allow */
static void CM_(unique)(PRC_(box) *const a) { CM_(unique_merge)(a, 0); }

#endif /* 0 */




static void PRC_(unused_recur_coda)(void);
static void PRC_(unused_recur)(void) {
	RC_(sort)(0);
#if 0
#ifdef BOX_COMPARE /* <!-- compare */
	L_(list_merge)(0, 0); L_(list_compare)(0, 0);
	L_(list_duplicates_to)(0, 0); L_(list_subtraction_to)(0, 0, 0);
	L_(list_union_to)(0, 0, 0); L_(list_intersection_to)(0, 0, 0);
	L_(list_xor_to)(0, 0, 0);*/
#endif /* compare --> */
#endif

#if 0 /* <!-- pasted from array */
#ifdef BOX_COMPARE /* <!-- compare */
	CM_(compare)(0, 0); CM_(lower_bound)(0, 0); CM_(upper_bound)(0, 0);
	CM_(insert_after)(0, 0); CM_(sort)(0); CM_(reverse)(0);
#endif /* compare --> */
	CM_(is_equal)(0, 0); CM_(unique_merge)(0, 0); CM_(unique)(0);
#endif /* pasted --> */

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
#undef BOX_HEAD
#undef BOX_REMOVE
#undef BOX_PUSH
