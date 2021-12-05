/** 2016 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 {<T>Pool} is a dynamic array that stores unordered {<T>} in a memory pool,
 which must be set using {POOL_TYPE}. You cannot shrink the capacity of this
 data type, only cause it to grow. Resizing incurs amortised cost, done though
 a Fibonacci sequence. {<T>Pool} is not synchronised. The preprocessor macros
 are all undefined at the end of the file for convenience.

 @param POOL_NAME, POOL_TYPE
 The name that literally becomes {<T>}, and a valid type associated therewith,
 accessible to the compiler at the time of inclusion; should be conformant to
 naming and to the maximum available length of identifiers. Must each be
 present before including.

 @param POOL_STACK
 Removing an element is normally done in a free list internal to the pool's
 elements. With this defined, there is no such list; the data will be packed
 and one can only remove items by \see{<T>PoolPop}.

 @param POOL_MIGRATE_EACH
 Optional function implementing {<PT>Migrate}. Indices will remain the same
 throughout the lifetime of the data, but pointers may change on {realloc}
 expansion. This definition will call {POOL_MIGRATE_EACH} with all {<T>} in
 {<T>Pool}. Use when your data is self-referential, like a linked-list.

 @param POOL_MIGRATE_ALL
 Optional type {<A>}. When one may have pointers to the data that is contained
 in the {Pool} outside the data that can be accessed by the pool. It adds an
 element to the constructor, {<PT>MigrateAll migrate_all}, as well as it's
 constant parameter, {<A> all}. This usually is the parent of an agglomeration
 that includes and has references into the pool. This has the responsibility to
 call \see{<T>MigratePointer} or some migrate function for all references.

 @param POOL_MIGRATE_UPDATE
 Optional type association with {<S>}. {<S>} should be a super-type of {<T>}.
 If not set, {<S>} is {<T>}. Used in \see{<T>PoolUpdateNew}.

 @param POOL_TO_STRING
 Optional print function implementing {<T>ToString}; makes available
 \see{<T>PoolToString}.

 @param POOL_TEST
 Unit testing framework using {<T>PoolTest}, included in a separate header,
 {../test/PoolTest.h}. Must be defined equal to a (random) filler function,
 satisfying {<T>Action}. If {NDEBUG} is not defined, turns on {assert} private
 function integrity testing. Requires {POOL_TO_STRING}.

 @title		Pool.h
 @std		C89
 @author	Neil
 @version	2018-04 Merged {Stack} into {Pool} again;
			augh ifdefs, but too much duplicate code.
 @since		2018-03 Why have an extra level of indirection?
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
#include <stdlib.h>	/* realloc free */
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



/* Constants across multiple includes in the same translation unit. */
#ifndef POOL_H /* <-- POOL_H */
#define POOL_H
static const size_t pool_fibonacci6 = 8;
static const size_t pool_fibonacci7 = 13;
/* Is a node that is not part of the deleted list; a different kind of null. */
static const size_t pool_void       = (size_t)-1;
/* Is a node on the edge of the deleted list. */
static const size_t pool_null       = (size_t)-2;
/* Maximum size; smaller then any special values. */
static const size_t pool_max        = (size_t)-3;
/* Removed offset queue. */
struct PoolX { size_t prev, next; };
#endif /* POOL_H --> */

/* One time in the same translation unit. */
#ifndef MIGRATE /* <-- migrate */
#define MIGRATE
/** Contains information about a {realloc}. */
struct Migrate;
struct Migrate {
	const void *begin, *end; /* Old pointers. */
	ptrdiff_t delta;
};
#endif /* migrate --> */



/** This is the migrate function for {<T>}. */
typedef void (*PT_(Migrate))(T *const data,
	const struct Migrate *const migrate);
#ifdef POOL_MIGRATE_EACH /* <-- each */
/* Check that {POOL_MIGRATE_EACH} is a function implementing {<PT>Migrate},
 whose definition is placed above {#include "Pool.h"}. */
static const PT_(Migrate) PT_(migrate_each) = (POOL_MIGRATE_EACH);
#endif /* each --> */

#ifdef POOL_MIGRATE_ALL /* <-- all */
/* Troubles with this line? check to ensure that {POOL_MIGRATE_ALL} is a
 valid type, whose definition is placed above {#include "Pool.h"}. */
typedef POOL_MIGRATE_ALL PT_(MigrateAllType);
#define A PT_(MigrateAllType)
/** Function call on {realloc} if {POOL_MIGRATE_ALL} is defined. */
typedef void (*PT_(MigrateAll))(A *const all,
	const struct Migrate *const migrate);
#endif /* all --> */

#ifdef POOL_MIGRATE_UPDATE /* <-- update */
/* Troubles with this line? check to ensure that {POOL_MIGRATE_UPDATE} is a
 valid type, whose definition is placed above {#include "Pool.h"}. */
typedef POOL_MIGRATE_UPDATE PT_(MigrateUpdateType);
#define S PT_(MigrateUpdateType)
#else /* update --><-- !update */
#define S PT_(Type)
#endif /* !update --> */

#ifdef POOL_TO_STRING /* <-- string */
/** Responsible for turning {<T>} (the first argument) into a 12 {char}
 null-terminated output string (the second.) Used for {POOL_TO_STRING}. */
typedef void (*PT_(ToString))(const T *, char (*const)[12]);
/* Check that {POOL_TO_STRING} is a function implementing {<PT>ToString}, whose
 definition is placed above {#include "Pool.h"}. */
static const PT_(ToString) PT_(to_string) = (POOL_TO_STRING);
#endif /* string --> */

/* Operates by side-effects only. */
typedef void (*PT_(Action))(T *const data);



/* Pool nodes containing the data. */
struct PT_(Node) {
	T data;
#ifndef POOL_STACK /* <-- !stack */
	struct PoolX x;
#endif /* !stack --> */
};

/** The pool. To instantiate, see \see{<T>Pool}. */
struct T_(Pool);
struct T_(Pool) {
	struct PT_(Node) *nodes;
	/* {nodes} -> {capacity} -> {c[0] < c[1] || c[0] == c[1] == max_size}.
	 Fibonacci, [0] is the capacity, [1] is next. */
	size_t capacity[2];
	/* {nodes} ? {size <= capacity[0]} : {size == 0}. Including removed. */
	size_t size;
#ifndef POOL_STACK /* <-- !stack */
	struct PoolX removed;
#endif /* !stack --> */
#ifdef POOL_MIGRATE_ALL /* <-- all */
	PT_(MigrateAll) migrate_all; /* Called to update on resizing. */
	A *all; /* Migrate parameter. */
#endif /* all --> */
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

/** Ensures capacity.
 @return Success; otherwise, {errno} may be set.
 @throws ERANGE: Tried allocating more then can fit in {size_t}.
 @throws {realloc} errors: {IEEE Std 1003.1-2001}. */
static int PT_(reserve)(struct T_(Pool) *const pool,
	const size_t min_capacity, S **const update_ptr) {
	size_t c0, c1;
	struct PT_(Node) *nodes;
	const size_t max_size = pool_max / sizeof *nodes;
	assert(pool && pool->size <= pool->capacity[0]
		&& pool->capacity[0] <= pool->capacity[1]
		&& pool->capacity[1] <= pool_max);
#ifndef POOL_STACK /* <-- !stack -- spaces that we could have placed it. */
	assert(!pool->nodes || (pool->removed.next == pool->removed.prev
		&& pool->removed.next == pool_null));
#endif /* !stack --> */
	if(pool->capacity[0] >= min_capacity) return 1;
	if(min_capacity > max_size) return errno = ERANGE, 0;
	if(!pool->nodes) {
		c0 = pool_fibonacci6;
		c1 = pool_fibonacci7;
	} else {
		c0 = pool->capacity[0];
		c1 = pool->capacity[1];
	}
	while(c0 < min_capacity) {
		c0 ^= c1, c1 ^= c0, c0 ^= c1, c1 += c0;
		if(c1 > max_size || c1 <= c0) c1 = max_size;
	}
	if(!(nodes = realloc(pool->nodes, c0 * sizeof *pool->nodes))) return 0;
	if(pool->nodes != nodes) {
		/* Migrate data; violates pedantic strict-ANSI? */
		struct Migrate migrate;
		migrate.begin = pool->nodes;
		migrate.end   = (const char *)pool->nodes + pool->size * sizeof *nodes;
		migrate.delta = (const char *)nodes - (const char *)pool->nodes;
#ifdef POOL_MIGRATE_EACH /* <-- each: Self-referential data. */
		{
			struct PT_(Node) *e, *end;
			for(e = nodes, end = e + pool->size; e < end; e++) {
#ifndef POOL_STACK /* <-- !stack: not packed data */
				if(e->x.prev != pool_void) continue; /* It's on removed list. */
				assert(e->x.next == pool_void);
#endif /* !stack --> */
				PT_(migrate_each)(&e->data, &migrate);
			}
		}
#endif /* each --> */
#ifdef POOL_MIGRATE_ALL /* <-- all: random references. */
		if(pool->migrate_all) {
			assert(pool->all);
			pool->migrate_all(pool->all, &migrate);
		}
#endif /* all --> */
		if(update_ptr) {
			const void *const u = *update_ptr;
			if(u >= migrate.begin && u < migrate.end)
				*(char **const)update_ptr += migrate.delta;
		}
	}
#ifndef POOL_STACK /* <-- !stack -- initialise linked-list in case static. */
	if(!pool->nodes) pool->removed.next = pool->removed.prev = pool_null;
#endif /* !stack --> */
	pool->nodes = nodes;
	pool->capacity[0] = c0;
	pool->capacity[1] = c1;
	return 1;
}

#ifndef POOL_STACK /* <-- !stack */

/** We are very lazy and we just enqueue the removed so that later data can
 overwrite it.
 @param n: Must be a valid index.
 @fixme Change the order of the data to fill the start first. Tricky, but cache
 performance will probably go up? Probably impossible to do in {O(1)}. */
static void PT_(enqueue_removed)(struct T_(Pool) *const pool, const size_t n) {
	struct PT_(Node) *const node = pool->nodes + n;
	assert(pool && n < pool->size);
	/* Cannot be part of the removed pool already. */
	assert(node->x.prev == pool_void && node->x.next == pool_void);
	if((node->x.prev = pool->removed.prev) == pool_null) {
		/* The first {<PT>Node} removed. */
		assert(pool->removed.next == pool_null);
		pool->removed.prev = pool->removed.next = n;
	} else {
		/* Stick it on the end. */
		struct PT_(Node) *const last = pool->nodes + pool->removed.prev;
		assert(last->x.next == pool_null);
		last->x.next = pool->removed.prev = n;
	}
	node->x.next = pool_null;
}

/** Dequeues a removed node, or if the queue is empty, returns null. */
static struct PT_(Node) *PT_(dequeue_removed)(struct T_(Pool) *const pool) {
	struct PT_(Node) *node;
	size_t n;
	assert(pool &&
		(pool->removed.next == pool_null) == (pool->removed.prev == pool_null));
	/* No nodes removed. */
	if(!pool->nodes || (n = pool->removed.next) == pool_null) return 0;
	node = pool->nodes + n;
	assert(node->x.prev == pool_null && node->x.next != pool_void);
	if((pool->removed.next = node->x.next) == pool_null) {
		/* The last element. */
		pool->removed.prev = pool->removed.next = pool_null;
	} else {
		struct PT_(Node) *const next = pool->nodes + node->x.next;
		assert(node->x.next < pool->size && next->x.prev == n);
		next->x.prev = pool_null;
	}
	node->x.prev = node->x.next = pool_void;
	return node;
}

/** Gets rid of the removed node at the tail of the list. Each remove has
 potentially one delete in the worst case, it just gets amotized a bit. */
static void PT_(trim_removed)(struct T_(Pool) *const pool) {
	struct PT_(Node) *node, *prev, *next;
	size_t n;
	assert(pool);
	while(pool->size
		&& (node = pool->nodes + (n = pool->size - 1))->x.prev != pool_void) {
		assert(node->x.next != pool_void);
		if(node->x.prev == pool_null) { /* First. */
			assert(pool->removed.next == n);
			pool->removed.next = node->x.next;
		} else {
			assert(node->x.prev < pool->size);
			prev = pool->nodes + node->x.prev;
			prev->x.next = node->x.next;
		}
		if(node->x.next == pool_null) { /* Last. */
			assert(pool->removed.prev == n);
			pool->removed.prev = node->x.prev;
		} else {
			assert(node->x.next < pool->size);
			next = pool->nodes + node->x.next;
			next->x.prev = node->x.prev;
		}
		pool->size--;
	}
}

#endif /* !stack --> */

/** Zeros {pool} except for {POOL_MIGRATE_ALL} which is initialised in the
 containing function, and not {POOL_STACK}, which is initialised in
 \see{<PT>_reserve}. */
static void PT_(pool)(struct T_(Pool) *const pool) {
	assert(pool);
	pool->nodes        = 0;
	pool->capacity[0]  = 0;
	pool->capacity[1]  = 0;
	pool->size         = 0;
}

/** Destructor for {pool}. All the {pool} contents should not be accessed
 anymore and the {pool} takes no memory.
 @param pool: If null, does nothing.
 @order \Theta(1)
 @allow */
static void T_(Pool_)(struct T_(Pool) *const pool) {
	if(!pool) return;
	free(pool->nodes);
	PT_(pool)(pool);
}

#ifdef POOL_MIGRATE_ALL /* <-- all */
/** Initialises {pool} to be empty and have a migrate function with a
 parameter. This is the constructor if {POOL_MIGRATE_ALL} is specified.
 @param pool: If null, does nothing.
 @param migrate_all: The general {<PT>MigrateAll} function.
 @param all: The general migrate parameter.
 @order \Theta(1)
 @allow */
static void T_(Pool)(struct T_(Pool) *const pool,
	const PT_(MigrateAll) migrate_all, A *const all) {
	if(!pool) return;
	PT_(pool)(pool);
	pool->migrate_all = migrate_all;
	pool->all         = all;
}
#else /* all --><-- !all */
/** Initialises {pool} to be empty. This is the constructor is
 {POOL_MIGRATE_ALL} is not specified. If it is {static} data then it is
 initialised by default and one doesn't have to call this.
 @order \Theta(1)
 @allow */
static void T_(Pool)(struct T_(Pool) *const pool) {
	if(!pool) return;
	PT_(pool)(pool);
}
#endif /* all --> */

#ifdef POOL_STACK /* <-- stack */
/** If {POOL_STACK} is specified, otherwise it doesn't keep track of the size.
 @param pool: If null, returns zero.
 @return The current size of the stack.
 @order \Theta(1)
 @allow */
static size_t T_(PoolSize)(const struct T_(Pool) *const pool) {
	if(!pool) return 0;
	return pool->size;
}
#endif /* stack --> */

#ifndef POOL_STACK /* <-- !stack */
/** Removes {data} from {pool}. Only present if {POOL_STACK} is not defined.
 @param pool, data: If null, returns false.
 @return Success, otherwise {errno} will be set for valid input.
 @throws EDOM: {data} is not part of {pool}.
 @order amortised O(1)
 @allow */
static int T_(PoolRemove)(struct T_(Pool) *const pool, T *const data) {
	struct PT_(Node) *node;
	size_t n;
	if(!pool || !data) return 0;
	node = PT_(data_upcast)(data);
	n = node - pool->nodes;
	if(node < pool->nodes || n >= pool->size || node->x.prev != pool_void)
		return errno = EDOM, 0;
		PT_(enqueue_removed)(pool, n);
	if(n >= pool->size - 1) PT_(trim_removed)(pool);
	return 1;
}
#endif /* !stack --> */

/** Removes all from {pool}, but leaves the {pool} memory alone; if one wants
 to remove memory, see \see{Pool_}.
 @param pool: If null, does nothing.
 @order \Theta(1)
 @allow */
static void T_(PoolClear)(struct T_(Pool) *const pool) {
	if(!pool) return;
	pool->size = 0;
#ifndef POOL_STACK /* <-- !stack */
	pool->removed.prev = pool->removed.next = pool_null;
#endif /* !stack --> */
}

/** Private: is {idx} a valid index for {pool}?
 @order \Theta(1) */
static struct PT_(Node) *PT_(valid_index)(const struct T_(Pool) *const pool,
	const size_t idx) {
	assert(pool);
	if(idx >= pool->size) return 0;
#ifdef POOL_STACK /* <-- stack */
	return pool->nodes + idx;
#else /* stack --><-- !stack */
	{
		struct PT_(Node) *const node = pool->nodes + idx;
		return node->x.prev == pool_void ? node : 0;
	}
#endif /* !stack --> */
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
#ifndef POOL_STACK /* <-- stack */
	PT_(trim_removed)(pool);
#endif /* stack --> */
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
#ifdef POOL_STACK /* <-- stack */
		if(idx < size) return &node->data;
#else /* stack --><-- !stack */
		while(idx < size) {
			if(node->x.prev == pool_void) return &node->data;
			node++, idx++;
		}
#endif /* !stack */
	}
	return 0;
}

/** Called from \see{<T>PoolNew} and \see{<T>PoolUpdateNew}. */
static struct PT_(Node) *PT_(new)(struct T_(Pool) *const pool,
	S **const update_ptr) {
	struct PT_(Node) *node;
	assert(pool);
#ifndef POOL_STACK /* <-- !stack */
	if((node = PT_(dequeue_removed)(pool))) return node;
#endif /* !stack --> */
	/* ERANGE, ENOMEM? */
	if(!PT_(reserve)(pool, pool->size + 1, update_ptr)) return 0;
	node = pool->nodes + pool->size++;
#ifndef POOL_STACK /* <-- !stack */
	node->x.prev = node->x.next = pool_void;
#endif /* !stack --> */
	return node;
}

/** Gets an uninitialised new element. May move the {Pool} to a new memory
 location to fit the new size.
 @param pool: If is null, returns null.
 @return A new, un-initialised, element, or null and {errno} may be set.
 @throws ERANGE: Tried allocating more then can fit in {size_t} objects.
 @throws {realloc} errors: {IEEE Std 1003.1-2001}.
 @order amortised O(1)
 @allow */
static T *T_(PoolNew)(struct T_(Pool) *const pool) {
	struct PT_(Node) *node;
	if(!pool || !(node = PT_(new)(pool, 0))) return 0;
	return &node->data;
}

/** Gets an uninitialised new element and updates the {update_ptr} if it is
 within the memory region that was changed. For example, when iterating a
 pointer and new element is needed that could change the pointer.
 @param pool: If null, returns null.
 @param update_ptr: Pointer to update on memory move.
 @return A new, un-initialised, element, or null and {errno} may be set.
 @throws ERANGE: Tried allocating more then can fit in {size_t}.
 @throws {realloc} errors: {IEEE Std 1003.1-2001}.
 @order amortised O(1)
 @fixme Untested.
 @allow */
static T *T_(PoolUpdateNew)(struct T_(Pool) *const pool, S **const update_ptr) {
	struct PT_(Node) *node;
	if(!pool || !(node = PT_(new)(pool, update_ptr))) return 0;
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

/** Use when the pool has pointers to another memory move structure in the
 {MigrateAll} function of the other data type.
 @param pool: If null, does nothing.
 @param handler: If null, does nothing, otherwise has the responsibility of
 calling the other data type's migrate pointer function on all pointers
 affected by the {realloc}.
 @param migrate: If null, does nothing. Should only be called in a {Migrate}
 function; pass the {migrate} parameter.
 @order O({greatest size})
 @fixme Untested.
 @fixme Migrate interface.
 @allow */
static void T_(PoolMigrateEach)(struct T_(Pool) *const pool,
	const PT_(Migrate) handler, const struct Migrate *const migrate) {
	struct PT_(Node) *node, *end;
	if(!pool || !migrate || !handler) return;
	for(node = pool->nodes, end = node + pool->size; node < end; node++) {
#ifndef POOL_STACK /* <-- !stack */
		if(node->x.prev != pool_void) continue;
#endif /* !stack --> */
		handler(&node->data, migrate);
	}
}

/** Passed a {migrate} parameter, allows pointers to the pool to be updated. It
 doesn't affect pointers not in the {realloc}ed region.
 @order \Omega(1)
 @fixme Untested.
 @fixme Migrate interface.
 @allow */
static void T_(PoolMigratePointer)(T **const data_ptr,
	const struct Migrate *const migrate) {
	const void *ptr;
	if(!data_ptr
		|| !(ptr = *data_ptr)
		|| ptr < migrate->begin
		|| ptr >= migrate->end) return;
	*(char **)data_ptr += migrate->delta;
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
	T_(PoolMigrateEach)(0, 0, 0);
	T_(PoolMigratePointer)(0, 0);
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
#ifdef POOL_STACK
#undef POOL_STACK
#endif
#ifdef POOL_MIGRATE_EACH
#undef POOL_MIGRATE_EACH
#endif
#ifdef POOL_MIGRATE_ALL
#undef POOL_MIGRATE_ALL
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
