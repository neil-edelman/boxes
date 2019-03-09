/** 2016 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 {<T>StablePool} stores unordered {<T>} in a memory pool, which must be set
 using {POOL_TYPE}. References to the items in the pool remain valid as long as
 the item is not removed. As such, it is not guaranteed to be contiguous. When
 a resizing occurs, the new active block of memory is geometrically larger.
 When data is removed, one of two things happens: if it is in the active block,
 it goes into an internal free list for re-use; if it is not, it is simply
 unused until all the items of that block are marked for removal, then it is
 freed.

 {StablePool} is not designed for iteration, reordering, or even counting, but
 instead to provide a fairly contiguous space, but stable, for more complicated
 structures to store data. There is no way to shrink the active block in
 memory, just exapand it.

 In development, we mostly saw two distinct cases: where the data was
 non-referential and simple. In this case, {C++} might use {vector}; this has
 developed into {Pool} in {CBoxes}. On the other hand, pointers to data
 in a more complicated structure means the position is not important, but the
 pointer is. With contiguity, we had a system of registering migrating
 functions on all structures which pointed-to data; this was sometimes
 error-prone and difficult to understand. The {<T>StablePool} type resembles
 {Boost C++} non-standard container {stable_vector}, where the data is at a
 constant address so references to that data will not be changed. It's probably
 not as fast, but less complex, thus far more robust.

 {<T>StablePool} is not synchronised. The preprocessor macros are all undefined
 at the end of the file for convenience.

 @param POOL_NAME, POOL_TYPE
 The name that literally becomes {<T>}, and a valid type associated therewith,
 accessible to the compiler at the time of inclusion; should be conformant to
 naming and to the maximum available length of identifiers. Must each be
 present before including.

 @param POOL_TO_STRING
 Optional print function implementing {<T>ToString}; makes available
 \see{<T>PoolToString}. Usually this is only used for debugging.

 @param POOL_TEST
 Unit testing framework using {<T>PoolTest}, included in a separate header,
 {../test/PoolTest.h}. Must be defined equal to a (random) filler function,
 satisfying {<T>Action}. If {NDEBUG} is not defined, turns on {assert} private
 function integrity testing. Requires {POOL_TO_STRING}.

 @title		StablePool.h
 @std		C89
 @author	Neil
 @version	2019-03 Split {Pool} from {Array}; allowed fragmentation in former.
 @since		2018-04 Merged {Stack} into {Pool} again.
			2018-03 Why have an extra level of indirection?
			2018-02 Errno instead of custom errors.
			2017-12 Introduced {POOL_PARENT} for type-safety.
			2017-11 Forked {Stack} from {Pool}.
			2017-10 Replaced {PoolIsEmpty} by {PoolElement}, much more useful.
			2017-10 Renamed Pool; made migrate automatic.
			2017-07 Made migrate simpler.
			2017-05 Split {List} from {Pool}; much simpler.
			2017-01 Almost-redundant functions simplified.
			2016-11 Multi-index.
			2016-08 Permute. */



#include <stddef.h>	/* ptrdiff_t offset_of */
#include <stdlib.h>	/* malloc free */
#include <assert.h>	/* assert */
#include <string.h>	/* memcpy (memmove strerror strcpy memcmp in PoolTest.h) */
#include <errno.h>	/* errno */
#ifdef POOL_TO_STRING /* <-- print */
#include <stdio.h>	/* snprintf */
#endif /* print --> */



/* Check defines. */
#ifndef POOL_NAME /* <-- error */
#error Generic POOL_NAME undefined.
#endif /* error --> */
#ifndef POOL_TYPE /* <-- error */
#error Generic POOL_TYPE undefined.
#endif /* --> */
#if defined(POOL_TEST) && !defined(POOL_TO_STRING) /* <-- error */
#error POOL_TEST requires POOL_TO_STRING.
#endif /* error --> */
#if !defined(POOL_TEST) && !defined(NDEBUG) /* <-- ndebug */
#define POOL_NDEBUG
#define NDEBUG
#endif /* ndebug --> */



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
#ifdef A
#undef A
#endif
#ifdef S
#undef S
#endif
#ifdef T
#undef T
#endif
#ifdef T_
#undef T_
#endif
#ifdef PT_
#undef PT_
#endif
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)
#define T_(thing) CAT(POOL_NAME, thing)
#define PT_(thing) PCAT(pool, PCAT(POOL_NAME, thing))
#define T_NAME QUOTE(POOL_NAME)

/* Troubles with this line? check to ensure that {POOL_TYPE} is a valid type,
 whose definition is placed above {#include "Pool.h"}. */
typedef POOL_TYPE PT_(Type);
#define T PT_(Type)



#ifdef POOL_TO_STRING /* <-- string */
/** Responsible for turning {<T>} (the first argument) into a 12 {char}
 null-terminated output string (the second.) Used for {POOL_TO_STRING}. */
typedef void (*PT_(ToString))(const T *, char (*const)[12]);
/* Check that {POOL_TO_STRING} is a function implementing {<PT>ToString}, whose
 definition is placed above {#include "Pool.h"}. */
static const PT_(ToString) PT_(to_string) = (POOL_TO_STRING);
#endif /* string --> */



/* Pool previous, next, in the free list of the largest block. The contract for
 this is {0,0} -> no nodes removed, otherwise, it's a circular list. This
 allows us to test whether it's removed by prev or next. The additional blocks
 have this unused, since they never have blocks added. */
struct PT_(X) {
	struct PT_(X) *prev, *next;
};

/* Pool nodes containing the data and prev, next deleted in the free list. */
struct PT_(Node) {
	T data;
	struct PT_(X) x;
};

/* Information about each block. Each block will have {capacity <PT>Node}'s
 after, {block + 1}. */
struct PT_(Block) {
	struct PT_(Block) *smaller;
	size_t capacity, size;
};

/** The pool. To instantiate, see \see{<T>Pool}. */
struct T_(Pool);
struct T_(Pool) {
	struct PT_(Block) *largest; /* We want all items to go in here. */
	size_t next_capacity;       /* Fibonacci */
	struct PT_(X) removed;      /* These are of the largest block. */
};



/** Private: {container_of}. */
static struct PT_(Node) *PT_(data_upcast)(T *const data) {
	return (struct PT_(Node) *)(void *)
		((char *)data - offsetof(struct PT_(Node), data));
}

/** Private: {container_of}. */
static const struct PT_(Node) *PT_(data_const_upcast)(const T *const data) {
	return (const struct PT_(Node) *)(const void *)
		((const char *)data - offsetof(struct PT_(Node), data));
}

/** Private: {container_of}. */
static struct PT_(Node) *PT_(x_upcast)(struct PT_(X) *const x) {
	return (struct PT_(Node) *)(void *)
		((char *)x - offsetof(struct PT_(Node), x));
}

/** Private: block to array. */
static struct PT_(Node) *PT_(block_array)(struct PT_(Block) *const b) {
	return (struct PT_(Node) *)(void *)(b + 1);
}

/** Ensures capacity of the largest block, ignoring removed elements.
 @return Success; otherwise, {errno} may be set.
 @throws ERANGE: Tried allocating more then can fit in {size_t}.
 @throws {malloc} errors: {IEEE Std 1003.1-2001}. */
static int PT_(reserve)(struct T_(Pool) *const pool, const size_t min) {
	size_t c0, c1;
	struct PT_(Block) *block;
	struct PT_(Node) *nodes;
	const size_t max_size = ((size_t)-1 - sizeof *block) / sizeof *nodes;
	assert(pool && !pool->removed.prev && !pool->removed.next);
	assert(!pool->largest
		|| (pool->largest->capacity > pool->next_capacity
		&& pool->next_capacity <= max_size)
		|| (pool->largest->capacity == pool->next_capacity) == max_size);
	if(pool->largest && pool->largest->capacity >= min) return 1;
	if(min > max_size) return errno = ERANGE, 0;
	if(!pool->largest) {
		c0 = 8, c1 = 13;
	} else {
		c0 = pool->largest->capacity, c1 = pool->next_capacity;
	}
	while(c0 < min) { /* min < max_size; this c0 ^= c1 ^= c0 ^= c1 += c0. */
		size_t temp = c0 + c1;
		if(temp > max_size || temp < c1) temp = max_size; /* Unlikely? */
		c0 = c1, c1 = temp;
	}
	if(!(block = malloc(sizeof *block + c0 * sizeof *nodes))) return 0;
	/* nodes = (struct PT_(Node) *)(block + 1); */
	block->smaller = (pool->largest) ? pool->largest->smaller : 0;
	block->capacity = c0;
	block->size = 0;
	pool->largest = block;
	pool->next_capacity = c1;
	pool->removed.prev = pool->removed.next = 0;
	return 1;
}

/** We are very lazy and we just enqueue the removed so that later data can
 overwrite it.
 @param n: Must be a valid index. */
static void PT_(enqueue_removed)(struct T_(Pool) *const pool,
	struct PT_(Node) *const node) {
	assert(pool && pool->largest && node
		&& node >= PT_(block_array)(pool->largest)
		&& node < PT_(block_array)(pool->largest) + pool->largest->capacity
		&& !node->x.prev && !node->x.next
		&& !pool->removed.prev == !pool->removed.next);
	node->x.prev = &pool->removed;
	if(!pool->removed.prev) { /* The first. */
		node->x.next = &pool->removed;
		pool->removed.next = &node->x;
	} else { /* Stick it on the end. */
		node->x.next = pool->removed.prev;
		pool->removed.prev->next = &node->x;
	}
	pool->removed.prev = &node->x;
}

/** Dequeues a removed node, or if the queue is empty, returns null. */
static struct PT_(Node) *PT_(dequeue_removed)(struct T_(Pool) *const pool) {
	struct PT_(X) *x;
	assert(pool && !pool->removed.next == !pool->removed.prev);
	/* No nodes removed. */
	if(!(x = pool->removed.next)) return 0;
	if((pool->removed.next = x->next) == &pool->removed) {
		pool->removed.prev = pool->removed.next = 0; /* The last element. */
	} else {
		x->next->prev = &pool->removed; /* Remove this element. */
	}
	/* Remove nodes internal pointers from the free list and return. */
	x->prev = x->next = 0;
	return PT_(x_upcast)(x);
}

/** Gets rid of the removed node at the tail of the list. Each remove has
 potentially one delete in the worst case, it just gets amortized a bit. */
static void PT_(trim_removed)(struct T_(Pool) *const pool) {
	struct PT_(Node) *node;
	struct PT_(Block) *const block = pool->largest;
	struct PT_(Node) *const nodes = PT_(block_array)(block);
	assert(pool && block);
	while(block->size
		&& (node = nodes + block->size - 1)->x.prev) {
		assert(node->x.next);
		if(node->x.prev == node->x.next) {
			/* There's only one: we don't want removed pointing to itself. */
			pool->removed.prev = pool->removed.next = 0;
		} else {
			node->x.prev->next = node->x.next;
			node->x.next->prev = node->x.prev;
		}
		/* node->x.prev = node->x.next = 0; Meh, clear doesn't either. */
		block->size--;
	}
}

/** Zeros {pool}. */
static void PT_(pool)(struct T_(Pool) *const pool) {
	assert(pool);
	pool->largest       = 0;
	pool->next_capacity = 0;
	pool->removed.prev  = 0;
	pool->removed.next  = 0;
}

/** Destructor for {pool}. All the {pool} contents are erased and should not be
 accessed anymore; after, the {pool} takes no memory.
 @param pool: If null, does nothing.
 @order \Theta(blocks)
 @allow */
static void T_(Pool_)(struct T_(Pool) *const pool) {
	struct PT_(Block) *block, *next;
	if(!pool) return;
	for(block = pool->largest; block; block = next)
		next = block->smaller, free(block);
	PT_(pool)(pool);
}

/** Initialises {pool} to be empty and zero. If it is {static} data then it
 is initialised by default and one doesn't have to call this.
 @order \Theta(1)
 @allow */
static void T_(Pool)(struct T_(Pool) *const pool) {
	if(!pool) return;
	PT_(pool)(pool);
}

/* Find what block it's in. I believe the expected value is {O(ln(n))} because
 the blocks get smaller exponentially. */
static struct PT_(Block) *PT_(find_block)(const struct T_(Pool) *const pool,
	const struct PT_(Node) *const node) {
	struct PT_(Block) *b;
	struct PT_(Node) *nodeblock;
	assert(pool && node);
	for(b = pool->largest; b; b = b->smaller) {
		nodeblock = (struct PT_(Node) *)(b + 1);
		if(node >= nodeblock && node < nodeblock + b->capacity) break;
	}
	return b;
}

/** Removes {data} from {pool}.
 @param pool, data: If null, returns false.
 @return Success, otherwise {errno} will be set for valid input.
 @throws EDOM: {data} is corrupted or not part of {pool}.
 @order amortised O(1) or O(ln(blocks))
 @allow */
static int T_(PoolRemove)(struct T_(Pool) *const pool, T *const data) {
	struct PT_(Node) *node, *nodes;
	struct PT_(Block) *block, **prev;
	if(!pool || !data) return 0;
	node = PT_(data_upcast)(data);
	/* Removed already or not part of the container. */
	if(node->x.next || !(block = PT_(find_block)(pool, node)))
		return errno = EDOM, 0;
	assert(!node->x.prev && block->size);
	if(block == pool->largest) { /* The largest block has a free list. */
		PT_(enqueue_removed)(pool, node);
		if((size_t)(node - nodes) >= block->size - 1) PT_(trim_removed)(pool);
	} else if(!--block->size) { /* The other blocks get a reference counter. */
		*prev = block->smaller;
		free(block);
	}
	return 1;
}

/** Removes all from {pool}, but leaves the active memory alone; if one wants
 to remove memory, see \see{Pool_}.
 @param pool: If null, does nothing.
 @order \Theta(1)
 @allow */
static void T_(PoolClear)(struct T_(Pool) *const pool) {
	struct PT_(Block) *block, *next;
	if(!pool || !pool->largest) return;
	(block = pool->largest)->size = 0;
	pool->removed.prev = pool->removed.next = 0;
	for(block = block->smaller; block; block = next)
		next = block->smaller, free(block);
}

#if 0
/** Private: is {idx} a valid index for {pool}?
 @order \Theta(1) */
static struct PT_(Node) *PT_(valid_index)(const struct T_(Pool) *const pool,
	const size_t idx) {
	assert(pool);
	if(idx >= pool->size) return 0;
	{
		struct PT_(Node) *const node = pool->nodes + idx;
		return node->x.prev == pool_void ? node : 0;
	}
}

/** Gets an existing element by index. Causing something to be added to the
 {<T>Pool} may invalidate this pointer.
 @param pool: If null, returns null.
 @param idx: Index.
 @return If failed, returns a null pointer.
 @order \Theta(1)
 @allow */
static T *T_(PoolGet)(const struct T_(Pool) *const pool, const size_t idx) {
	if(!pool) return 0;
	{
		struct PT_(Node) *const node = PT_(valid_index)(pool, idx);
		return node ? &node->data : 0;
	}
}

/** Gets an index given {data}.
 @param data: If the element is not part of the {Pool}, behaviour is undefined.
 @return An index.
 @order \Theta(1)
 @fixme Untested.
 @allow */
static size_t T_(PoolIndex)(const struct T_(Pool) *const pool,
	const T *const data) {
	return PT_(data_const_upcast)(data) - pool->nodes;
}

/** @param pool: If null, returns null.
 @return One element from the pool or null if the pool is empty. If
 {POOL_STACK} was specified, this will be the last element added, otherwise, it
 may not be, but it is deterministic. Causing something to be added to the
 {pool} may invalidate this pointer.
 @order \Theta(1)
 @fixme Untested.
 @allow */
static T *T_(PoolPeek)(const struct T_(Pool) *const pool) {
	if(!pool || !pool->size) return 0;
	return &pool->nodes[pool->size - 1].data;
}

/** The same value as \see{<T>PoolPeek}.
 @param pool: If null, returns null.
 @return Value from the the top of the {pool} that is removed or null if the
 stack is empty. Causing something to be added to the {pool} may invalidate
 this pointer.
 @order \Theta(1) (amortized if not {POOL_STACK})
 @allow */
static T *T_(PoolPop)(struct T_(Pool) *const pool) {
	T *data;
	if(!pool || !pool->size) return 0;
	data = &pool->nodes[--pool->size].data;
	PT_(trim_removed)(pool);
	return data;
}

/** Provides a way to iterate through the {pool}. If {<T> = <S>}, it is safe to
 add using {PoolUpdateNew} with the return value as {update}. If {POOL_STACK}
 is not defined, it is safe to remove an element,
 @param pool: If null, returns null. If {prev} is not from this {pool} and not
 null, returns null.
 @param prev: Set it to null to start the iteration.
 @return A pointer to the next element or null if there are no more.
 @order \Theta(1) (or O(pool space that has been deleted) if not {POOL_STACK})
 @allow */
static T *T_(PoolNext)(const struct T_(Pool) *const pool, T *const prev) {
	if(!pool) return 0;
	{
		struct PT_(Node) *node;
		size_t idx;
		const size_t size = pool->size;
		if(!prev) {
			node = pool->nodes;
			idx = 0;
		} else {
			node = PT_(data_upcast)(prev) + 1;
			idx = (size_t)(node - pool->nodes);
		}
		while(idx < size) {
			if(node->x.prev == pool_void) return &node->data;
			node++, idx++;
		}
	}
	return 0;
}
#endif

/** Gets an uninitialised new element.
 @param pool: If is null, returns null.
 @return A new, un-initialised, element, or null and {errno} may be set.
 @throws ERANGE: Tried allocating more then can fit in {size_t} objects.
 @throws {malloc} errors: {IEEE Std 1003.1-2001}.
 @order amortised O(1)
 @allow */
static T *T_(PoolNew)(struct T_(Pool) *const pool) {
	struct PT_(Node) *node;
	size_t size;
	if(!pool) return 0;
	if((node = PT_(dequeue_removed)(pool))) return &node->data;
	size = pool->largest ? pool->largest->size : 0;
	if(!PT_(reserve)(pool, size + 1)) return 0;
	assert(pool->largest);
	node = PT_(block_array)(pool->largest) + pool->largest->size++;
	node->x.prev = node->x.next = 0;
	return &node->data;
}

/** Iterates though {pool} from the bottom and calls {action} on all the
 elements. The topology of the list can not change while in this function.
 @param stack, action: If null, does nothing.
 @order O({size} \times {action})
 @fixme Untested.
 @fixme Sequence interface.
 @allow */
static void T_(PoolForEach)(struct T_(Pool) *const pool,
	const PT_(Action) action) {
	struct PT_(Node) *a, *end;
	if(!pool || !action) return;
	for(a = pool->nodes, end = a + pool->size; a < end; a++) {
#ifndef POOL_STACK /* <-- !stack */
		if(a->x.prev != pool_void) continue;
#endif /* !stack --> */
		action(&a->data);
	}
}

#ifdef POOL_TO_STRING /* <-- print */

#ifndef POOL_PRINT_THINGS /* <-- once inside translation unit */
#define POOL_PRINT_THINGS

static const char *const pool_cat_start     = "[";
static const char *const pool_cat_end       = "]";
static const char *const pool_cat_alter_end = "...]";
static const char *const pool_cat_sep       = ", ";
static const char *const pool_cat_star      = "*";
static const char *const pool_cat_null      = "null";

struct Pool_SuperCat {
	char *print, *cursor;
	size_t left;
	int is_truncated;
};
static void pool_super_cat_init(struct Pool_SuperCat *const cat,
	char *const print, const size_t print_size) {
	cat->print = cat->cursor = print;
	cat->left = print_size;
	cat->is_truncated = 0;
	print[0] = '\0';
}
static void pool_super_cat(struct Pool_SuperCat *const cat,
	const char *const append) {
	size_t lu_took; int took;
	if(cat->is_truncated) return;
	took = sprintf(cat->cursor, "%.*s", (int)cat->left, append);
	if(took < 0)  { cat->is_truncated = -1; return; } /*implementation defined*/
	if(took == 0) { return; }
	if((lu_took = (size_t)took) >= cat->left)
		cat->is_truncated = -1, lu_took = cat->left - 1;
	cat->cursor += lu_took, cat->left -= lu_took;
}
#endif /* once --> */

/** Can print 4 things at once before it overwrites. One must pool
 {POOL_TO_STRING} to a function implementing {<T>ToString} to get this
 functionality.
 @return Prints {pool} in a static buffer.
 @order \Theta(1); it has a 255 character limit; every element takes some of it.
 @fixme ToString interface.
 @allow */
static const char *T_(PoolToString)(const struct T_(Pool) *const pool) {
	static char buffer[4][256];
	static unsigned buffer_i;
	struct Pool_SuperCat cat;
	int is_first = 1;
	char scratch[12];
	size_t i;
	assert(strlen(pool_cat_alter_end) >= strlen(pool_cat_end));
	assert(sizeof buffer > strlen(pool_cat_alter_end));
	pool_super_cat_init(&cat, buffer[buffer_i],
		sizeof *buffer / sizeof **buffer - strlen(pool_cat_alter_end));
	buffer_i++, buffer_i &= 3;
	if(!pool) {
		pool_super_cat(&cat, pool_cat_null);
		return cat.print;
	}
	pool_super_cat(&cat, pool_cat_start);
	for(i = 0; i < pool->size; i++) {
#ifndef POOL_STACK /* <-- !stack */
		if(pool->nodes[i].x.prev != pool_void) continue;
#endif /* !stack --> */
		if(!is_first) pool_super_cat(&cat, pool_cat_sep); else is_first = 0;
		PT_(to_string)(&pool->nodes[i].data, &scratch),
		scratch[sizeof scratch - 1] = '\0';
		pool_super_cat(&cat, scratch);
		if(cat.is_truncated) break;
	}
	sprintf(cat.cursor, "%s",
		cat.is_truncated ? pool_cat_alter_end : pool_cat_end);
	return cat.print; /* Static buffer. */
}

#endif /* print --> */

#ifdef POOL_TEST /* <-- test */
#include "../test/TestPool.h" /* Need this file if one is going to run tests. */
#endif /* test --> */

/* Prototype. */
static void PT_(unused_coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 \url{ http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code } */
static void PT_(unused_set)(void) {
	T_(Pool_)(0);
#ifdef POOL_MIGRATE_ALL /* <-- all */
	T_(Pool)(0, 0, 0);
#else /* all --><-- !all */
	T_(Pool)(0);
#endif /* !all --> */
#ifdef POOL_STACK /* <-- stack */
	T_(PoolSize)(0);
#else /* stack --><-- !stack */
	T_(PoolRemove)(0, 0);
#endif /* !stack --> */
	T_(PoolClear)(0);
	T_(PoolGet)(0, 0);
	T_(PoolIndex)(0, 0);
	T_(PoolPeek)(0);
	T_(PoolPop)(0);
	T_(PoolNext)(0, 0);
	T_(PoolNew)(0);
	T_(PoolUpdateNew)(0, 0);
	T_(PoolForEach)(0, 0);
#ifdef POOL_TO_STRING
	T_(PoolToString)(0);
#endif
	PT_(unused_coda)();
}
/** {clang}'s pre-processor is not fooled if you have one function. */
static void PT_(unused_coda)(void) { PT_(unused_set)(); }



/* Un-define all macros. */
#undef POOL_NAME
#undef POOL_TYPE
/* Undocumented; allows nestled inclusion so long as: {CAT_}, {CAT}, {PCAT},
 {PCAT_} conform, and {T}, {S}, and {A}, are not used. */
#ifdef POOL_SUBTYPE /* <-- sub */
#undef POOL_SUBTYPE
#else /* sub --><-- !sub */
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#endif /* !sub --> */
#undef T
#undef T_
#undef PT_
#undef S
#ifdef A
#undef A
#endif
#ifdef POOL_MIGRATE_UPDATE
#undef POOL_MIGRATE_UPDATE
#endif
#ifdef POOL_TO_STRING
#undef POOL_TO_STRING
#endif
#ifdef POOL_TEST
#undef POOL_TEST
#endif
#ifdef POOL_NDEBUG
#undef POOL_NDEBUG
#undef NDEBUG
#endif
