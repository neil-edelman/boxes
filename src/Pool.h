/** 2016 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 {<T>Pool} is a dynamic array that stores unordered {<T>}, which must be set
 using {POOL_TYPE}. Removing an element is done lazily through a linked-list
 internal to the pool; as such, indices will remain the same throughout the
 lifetime of the data. You cannot shrink the size of this data type, only cause
 it to grow. Resizing incurs amortised cost, done though a Fibonacci sequence.
 {<T>Pool} is not synchronised. The preprocessor macros are all undefined at
 the end of the file for convenience when including multiple pool types in the
 same file.

 @param POOL_NAME
 This literally becomes {<T>}. As it's used in function names, this should
 comply with naming rules and be unique; required.

 @param POOL_TYPE
 The type associated with {<T>}. Has to be a valid type, accessible to the
 compiler at the time of inclusion; required.

 @param POOL_TO_STRING
 Optional print function implementing {<T>ToString}; makes available
 \see{<T>PoolToString}.

 @param POOL_DEBUG
 Prints information to {stderr}. Requires {POOL_TO_STRING}.

 @param POOL_TEST
 Unit testing framework using {<T>PoolTest}, included in a separate header,
 {../test/PoolTest.h}. Must be defined equal to a (random) filler function,
 satisfying {<T>Action}. If {NDEBUG} is not defined, turns on {assert} private
 function integrity testing. Requires {POOL_TO_STRING}.

 @title		Pool.h
 @std		C89/90
 @author	Neil
 @version	2017-10 Replaced {PoolIsEmpty} by {PoolElement}, much more useful.
 @since		2017-10 Renamed Pool; made migrate automatic.
			2017-07 Made migrate simpler.
			2017-05 Split {List} from {Pool}; much simpler.
			2017-01 Almost-redundant functions simplified.
			2016-11 Multi-index.
			2016-08 Permute. */



#include <stddef.h>	/* ptrdiff_t */
#include <stdlib.h>	/* malloc free qsort */
#include <assert.h>	/* assert */
#include <string.h>	/* memcpy (memmove strerror strcpy memcmp in PoolTest.h) */
#ifdef POOL_TO_STRING /* <-- print */
#include <stdio.h>	/* snprintf */
#endif /* print --> */
#include <errno.h>	/* errno */
#ifdef POOL_DEBUG
#include <stdarg.h>	/* for print */
#endif /* calls --> */



/* unused macro */
#ifdef UNUSED
#undef UNUSED
#endif
#define UNUSED(a) ((void)(a))



/* check defines */
#ifndef POOL_NAME
#error Pool generic POOL_NAME undefined.
#endif
#ifndef POOL_TYPE
#error Pool generic POOL_TYPE undefined.
#endif
#if (defined(POOL_DEBUG) || defined(POOL_TEST)) && !defined(POOL_TO_STRING)
#error Pool: POOL_DEBUG and POOL_TEST require POOL_TO_STRING.
#endif
#if !defined(POOL_TEST) && !defined(NDEBUG)
#define POOL_NDEBUG
#define NDEBUG
#endif



/* After this block, the preprocessor replaces T with POOL_TYPE, T_(X) with
 POOL_NAMEX, PRIVATE_T_(X) with POOL_U_NAME_X, and T_NAME with the string
 version. http://stackoverflow.com/questions/16522341/pseudo-generics-in-c */
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
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)
#define T_(thing) CAT(POOL_NAME, thing)
#define PRIVATE_T_(thing) PCAT(pool, PCAT(POOL_NAME, thing))
#define T_NAME QUOTE(POOL_NAME)

/* Troubles with this line? check to ensure that POOL_TYPE is a valid type,
 whose definition is placed above {#include "Pool.h"}. */
typedef POOL_TYPE PRIVATE_T_(Type);
#define T PRIVATE_T_(Type)





/* constants across multiple includes in the same translation unit */
#ifndef POOL_H /* <-- POOL_H */
#define POOL_H

static const size_t pool_fibonacci6 = 8;
static const size_t pool_fibonacci7 = 13;
static const size_t pool_not_part   = (size_t)-1;
/** Used as a null pointer with indices; {Pool} will not allow the size to be
 this big. */
static const size_t pool_null       = (size_t)-2;

/* designated initializers are C99; this is safe because C has rules for enum
 default initialisers */
enum PoolError {
	POOL_NO_ERROR,
	POOL_ERRNO,
	POOL_PARAMETER,
	POOL_OUT_OF_BOUNDS,
	POOL_OVERFLOW
};
static const char *const pool_error_explination[] = {
	/*[POOL_NO_ERROR]      =*/ "no error",
	/*[POOL_ERRNO]         =*/ 0, /* <- get errno */
	/*[POOL_PARAMETER]     =*/ "parameter out-of-range",
	/*[POOL_OUT_OF_BOUNDS] =*/ "out-of-bounds",
	/*[POOL_OVERFLOW]      =*/ "overflow"
};

/* global for constructor allocation errors */
static enum PoolError pool_global_error;
static int            pool_global_errno_copy;

#endif /* POOL_H --> */



#ifndef MIGRATE /* <-- migrate */
#define MIGRATE
/** Contains information about a {realloc}. */
struct Migrate;
struct Migrate {
	const void *begin, *end; /* old pointers */
	ptrdiff_t delta;
};
/** Function call on {realloc}. */
typedef void (*Migrate)(void *const parent,
	const struct Migrate *const migrate);
#endif /* migrate --> */



/** Operates by side-effects only. Used for {POOL_TEST}. */
typedef void (*T_(Action))(T *const element);

/** Given to \see{<T>PoolMigrateEach} by the migrate function of another
 {Pool}. */
typedef void (*T_(PoolMigrateElement))(T *const element,
	const struct Migrate *const migrate);

#ifdef POOL_TO_STRING /* <-- string */

/** Responsible for turning {<T>} (the first argument) into a 12 {char}
 null-terminated output string (the second.) Used for {POOL_TO_STRING}. */
typedef void (*T_(ToString))(const T *, char (*const)[12]);

/* Check that {POOL_TO_STRING} is a function implementing {<T>ToString}. */
static const T_(ToString) PRIVATE_T_(to_string) = (POOL_TO_STRING);

#endif /* string --> */



/* Pool element. */
struct PRIVATE_T_(Element) {
	T data; /* has to be the first element for convenience */
	size_t prev, next; /* removed offset queue */
};

/** The pool. To instantiate, see \see{<T>Pool}. */
struct T_(Pool);
struct T_(Pool) {
	struct PRIVATE_T_(Element) *array;
	size_t capacity[2]; /* Fibonacci, [0] is the capacity, [1] is next */
	size_t size; /* including removed */
	size_t head, tail; /* removed queue */
	enum PoolError error; /* errors defined by enum PoolError */
	int errno_copy; /* copy of errno when when error == E_ERRNO */
	Migrate migrate; /* called to update on resizing */
	void *parent; /* migrate parameter */
};



/** Debug messages from pool functions; turn on using {POOL_DEBUG}. */
static void PRIVATE_T_(debug)(struct T_(Pool) *const this,
	const char *const fn, const char *const fmt, ...) {
#ifdef POOL_DEBUG
	/* \url{ http://c-faq.com/varargs/vprintf.html } */
	va_list argp;
	fprintf(stderr, "Pool<" T_NAME ">#%p.%s: ", (void *)this, fn);
	va_start(argp, fmt);
	vfprintf(stderr, fmt, argp);
	va_end(argp);
#else
	UNUSED(this); UNUSED(fn); UNUSED(fmt);
#endif
}

/** Ensures capacity.
 @return Success.
 @throws POOL_OVERFLOW, POOL_ERRNO */
static int PRIVATE_T_(reserve)(struct T_(Pool) *const this,
	const size_t min_capacity) {
	size_t c0, c1;
	struct PRIVATE_T_(Element) *array;
	const size_t max_size = (pool_null - 1) / sizeof *array;
	assert(this);
	assert(this->size <= this->capacity[0]);
	assert(this->capacity[0] <= this->capacity[1]);
	assert(this->capacity[1] < pool_null);
	assert(pool_null < pool_not_part);
	if(this->capacity[0] >= min_capacity) return 1;
	if(max_size < min_capacity) return this->error = POOL_OVERFLOW, 0; 
	c0 = this->capacity[0];
	c1 = this->capacity[1];
	while(c0 < min_capacity) {
		c0 ^= c1, c1 ^= c0, c0 ^= c1, c1 += c0;
		if(c1 <= c0 || c1 > max_size) c1 = max_size;
	}
	if(!(array = realloc(this->array, c0 * sizeof *this->array)))
		return this->error = POOL_ERRNO, this->errno_copy = errno, 0;
	PRIVATE_T_(debug)(this, "reserve", "array#%p[%lu] -> #%p[%lu].\n",
		(void *)this->array, (unsigned long)this->capacity[0], (void *)array,
		(unsigned long)c0);
	/* Migrate parent class. Violates pedantic strict-ANSI? Subverts
	 type-safety? However, it is so convenient for the caller not to have to
	 worry about moving memory blocks. */
	if(this->array != array && this->migrate) {
		struct Migrate migrate;
		migrate.begin = this->array;
		migrate.end   = (const char *)this->array + this->size * sizeof *array;
		migrate.delta = (const char *)array - (const char *)this->array;
		PRIVATE_T_(debug)(this, "reserve", "calling migrate.\n");
		this->migrate(this->parent, &migrate);
	}
	this->array = array;
	this->capacity[0] = c0;
	this->capacity[1] = c1;
	return 1;
}

/** We are very lazy and we just enqueue the removed for later elements.
 @param idx: Must be a valid index. */
static void PRIVATE_T_(enqueue_removed)(struct T_(Pool) *const this,
	const size_t e) {
	struct PRIVATE_T_(Element) *elem;
	assert(this);
	assert(e < this->size);
	elem = this->array + e;
	/* cannot be part of the removed pool already */
	assert(elem->prev == pool_not_part);
	assert(elem->next == pool_not_part);
	if((elem->prev = this->tail) == pool_null) {
		assert(this->head == pool_null);
		this->head = this->tail = e;
	} else {
		struct PRIVATE_T_(Element) *const last = this->array + this->tail;
		assert(last->next == pool_null);
		last->next = this->tail = e;
	}
	elem->next = pool_null;
}

/** Dequeues a removed element, or if the queue is empty, returns null. */
static struct PRIVATE_T_(Element) *PRIVATE_T_(dequeue_removed)(
	struct T_(Pool) *const this) {
	struct PRIVATE_T_(Element) *elem;
	size_t e;
	assert(this);
	assert((this->head == pool_null) == (this->tail == pool_null));
	if((e = this->head) == pool_null) return 0;
	elem = this->array + e;
	assert(elem->prev == pool_null);
	assert(elem->next != pool_not_part);
	if((this->head = elem->next) == pool_null) {
		this->head = this->tail = pool_null;
	} else {
		struct PRIVATE_T_(Element) *const next = this->array + elem->next;
		assert(elem->next < this->size);
		next->prev = pool_null;
	}
	elem->prev = elem->next = pool_not_part;
	return elem;
}

/** Gets rid of the removed elements at the tail of the list. Each remove has
 potentially one delete in the worst case, it just gets differed a bit. */
static void PRIVATE_T_(trim_removed)(struct T_(Pool) *const this) {
	struct PRIVATE_T_(Element) *elem, *prev, *next;
	size_t e;
	assert(this);
	while(this->size
		&& (elem = this->array + (e = this->size - 1))->prev != pool_not_part) {
		if(elem->prev == pool_null) {
			assert(this->head == e), this->head = elem->next;
		} else {
			assert(elem->prev < this->size), prev = this->array + elem->prev;
			prev->next = elem->next;
		}
		if(elem->next == pool_null) {
			assert(this->tail == e), this->tail = elem->prev;
		} else {
			assert(elem->next < this->size), next = this->array + elem->next;
			next->prev = elem->prev;
		}
		this->size--;
	}
}

/** Destructor for Pool. Make sure that the pool's contents will not be
 accessed anymore.
 @param thisp: A reference to the object that is to be deleted; it will be pool
 to null. If it is already null or it points to null, doesn't do anything.
 @order \Theta(1)
 @allow */
static void T_(Pool_)(struct T_(Pool) **const thisp) {
	struct T_(Pool) *this;
	if(!thisp || !(this = *thisp)) return;
	PRIVATE_T_(debug)(this, "Delete", "erasing.\n");
	free(this->array);
	free(this);
	*thisp = 0;
}

/** Constructs an empty {Pool} with capacity Fibonacci6, which is 8.
 @param migrate: The ADT parent's {Migrate} function.
 @param parent: The parent itself; to have multiple parents, implement an
 intermediary {Migrate} function that takes multiple values; required if
 {migrate} is specified.
 @return A new {Pool} for the polymorphic variable {parent}.
 @throws POOL_PARAMETER, POOL_ERRNO: Use {PoolError(0)} to get the error.
 @order \Theta(1)
 @allow */
static struct T_(Pool) *T_(Pool)(const Migrate migrate, void *const parent) {
	struct T_(Pool) *this;
	if(!migrate ^ !parent) {
		pool_global_error = POOL_PARAMETER;
		pool_global_errno_copy = 0;
		return 0;
	}
	if(!(this = malloc(sizeof(struct T_(Pool))))) {
		pool_global_error = POOL_ERRNO;
		pool_global_errno_copy = errno;
		return 0;
	}
	this->array        = 0;
	this->capacity[0]  = pool_fibonacci6;
	this->capacity[1]  = pool_fibonacci7;
	this->size         = 0;
	this->head = this->tail = pool_null;
	this->error        = POOL_NO_ERROR;
	this->errno_copy   = 0;
	this->migrate      = migrate;
	this->parent       = parent;
	if(!(this->array = malloc(this->capacity[0] * sizeof *this->array))) {
		T_(Pool_)(&this);
		pool_global_error = POOL_ERRNO;
		pool_global_errno_copy = errno;
		return 0;
	}
	PRIVATE_T_(debug)(this, "New", "capacity %d.\n", this->capacity[0]);
	return this;
}

/** See what's the error if something goes wrong. Resets the error.
 @return The last error string.
 @order \Theta(1)
 @allow */
static const char *T_(PoolGetError)(struct T_(Pool) *const this) {
	const char *str;
	enum PoolError *perr;
	int *perrno;
	perr   = this ? &this->error      : &pool_global_error;
	perrno = this ? &this->errno_copy : &pool_global_errno_copy;
	if(!(str = pool_error_explination[*perr])) str = strerror(*perrno);
	*perr = 0;
	return str;
}

/** @return	One value from the pool or null if the pool is empty. It selects
 the position in the memory which is farthest from the start of the buffer
 deterministically. Generally, you can't select which element you want, but if
 the pool has been treated like a stack, this is peek.
 @order \Theta(1)
 @fixme Untested.
 @allow */
static T *T_(PoolElement)(const struct T_(Pool) *const this) {
	if(!this || !this->size) return 0;
	return &this->array[this->size - 1].data;
}

/** Is {idx} a valid index for {this}.
 @order \Theta(1)
 @allow */
static int T_(PoolIsElement)(struct T_(Pool) *const this, const size_t idx) {
	struct PRIVATE_T_(Element) *elem;
	if(!this) return 0;
	if(idx >= this->size
		|| (elem = this->array + idx, elem->prev != pool_not_part))
		return 0;
	return 1;
}

/** Gets an existing element by index. Causing something to be added to the
 {Pool} may invalidate this pointer.
 @param this: If {this} is null, returns null.
 @param idx: Index.
 @return If failed, returns a null pointer and the error condition will be set.
 @throws POOL_OUT_OF_BOUNDS
 @order \Theta(1)
 @allow */
static T *T_(PoolGetElement)(struct T_(Pool) *const this, const size_t idx) {
	struct PRIVATE_T_(Element) *elem;
	if(!this) return 0;
	if(idx >= this->size
		|| (elem = this->array + idx, elem->prev != pool_not_part))
		{ this->error = POOL_OUT_OF_BOUNDS; return 0; }
	return &elem->data;
}

/** Gets an index given an element. If the element is not part of the {Pool},
 behaviour is undefined.
 @order \Theta(1)
 @fixme Untested.
 @allow */
static size_t T_(PoolGetIndex)(struct T_(Pool) *const this,
	const T *const element) {
	return (const struct PRIVATE_T_(Element) *)(const void *)element
		- this->array;
}

/** Increases the capacity of this Pool to ensure that it can hold at least
 the number of elements specified by the {min_capacity}.
 @param this: If {this} is null, returns false.
 @return True if the capacity increase was viable; otherwise the pool is not
 touched and the error condition is pool.
 @throws POOL_ERRNO, POOL_OVERFLOW
 @order \Omega(1), O({capacity})
 @allow */
static int T_(PoolReserve)(struct T_(Pool) *const this,
	const size_t min_capacity) {
	if(!this) return 0;
	if(!PRIVATE_T_(reserve)(this, min_capacity)) return 0;
	PRIVATE_T_(debug)(this, "Reserve","pool pool size to %u to contain %u.\n",
		this->capacity[0], min_capacity);
	return 1;
}

/** Gets an uninitialised new element.
 @param this: If {this} is null, returns null.
 @return If failed, returns a null pointer and the error condition will be set.
 @throws POOL_OVERFLOW, POOL_ERRNO
 @order amortised O(1)
 @allow */
static T *T_(PoolNew)(struct T_(Pool) *const this) {
	struct PRIVATE_T_(Element) *elem;
	if(!this) return 0;
	if(!(elem = PRIVATE_T_(dequeue_removed)(this))) {
		if(!PRIVATE_T_(reserve)(this, this->size + 1)) return 0;
		elem = this->array + this->size++;
		elem->prev = elem->next = pool_not_part;
	}
	PRIVATE_T_(debug)(this, "New", "added.\n");
	return &elem->data;
}

/** Removes an element associated with {data} from {this}.
 @param this: If {this} is null, returns false.
 @return Success.
 @throws POOL_OUT_OF_BOUNDS
 @order amortised O(1)
 @allow */
static int T_(PoolRemove)(struct T_(Pool) *const this, T *const data) {
	struct PRIVATE_T_(Element) *elem;
	size_t e;
	if(!this || !data) return 0;
	elem = (struct PRIVATE_T_(Element) *)(void *)data;
	if(elem < this->array
		|| this->array + this->size <= elem
		|| elem->prev != pool_not_part)
		return this->error = POOL_OUT_OF_BOUNDS, 0;
	e = elem - this->array;
	PRIVATE_T_(enqueue_removed)(this, e);
	if(e >= this->size - 1) PRIVATE_T_(trim_removed)(this);
	PRIVATE_T_(debug)(this, "Remove", "removing %lu.\n", (unsigned long)e);
	return 1;
}

/** Removes all data from {this}.
 @order \Theta(1)
 @allow */
static void T_(PoolClear)(struct T_(Pool) *const this) {
	if(!this) return;
	this->size = 0;
	this->head = this->tail = pool_null;
	PRIVATE_T_(debug)(this, "Clear", "cleared.\n");
}

/** Use when the pool has pointers to another pool in the {Migrate} function of
 the other pool (passed when creating the other pool.)
 @param handler: Has the responsibility of calling \see{<T>PoolMigratePointer}
 on all pointers affected by the {realloc}.
 @param migrate: Should only be called in a {Migrate} function; pass the
 {migrate} parameter.
 @order O({greatest size})
 @fixme Untested.
 @allow */
static void T_(PoolMigrateEach)(struct T_(Pool) *const this,
	const T_(PoolMigrateElement) handler, const struct Migrate *const migrate) {
	size_t i;
	struct PRIVATE_T_(Element) *e;
	if(!this) return;
	if(!migrate || !handler) { this->error = POOL_PARAMETER; return; }
	for(i = 0; i < this->size; i++) {
		e = this->array + i;
		if(e->prev != pool_not_part) continue;
		handler(&e->data, migrate);
	}
}

/** Use this inside the function that is passed to \see{<T>PoolMigrateEach}.
 Allows pointers to the pool to be updated. It doesn't affect pointers not in
 the {realloc}ed region.
 @order O(1)
 @fixme Untested.
 @allow */
static void T_(PoolMigratePointer)(T **const node_ptr,
	const struct Migrate *const migrate) {
	const void *ptr;
	if(!node_ptr
		|| !(ptr = *node_ptr)
		|| ptr < migrate->begin
		|| ptr >= migrate->end) return;
	*(char **)node_ptr += migrate->delta;
}

#ifdef POOL_TO_STRING /* <-- print */

#ifndef POOL_PRINT_THINGS /* <-- once inside translation unit */
#define POOL_PRINT_THINGS

static const char *const pool_cat_start     = "[ ";
static const char *const pool_cat_end       = " ]";
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
 @return Prints {this} in a static buffer.
 @order \Theta(1); it has a 255 character limit; every element takes some of it.
 @allow */
static const char *T_(PoolToString)(const struct T_(Pool) *const this) {
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
	if(!this) {
		pool_super_cat(&cat, pool_cat_null);
		return cat.print;
	}
	pool_super_cat(&cat, pool_cat_start);
	for(i = 0; i < this->size; i++) {
		if(this->array[i].prev != pool_not_part) continue;
		if(!is_first) pool_super_cat(&cat, pool_cat_sep); else is_first = 0;
		PRIVATE_T_(to_string)(&this->array[i].data, &scratch),
		scratch[sizeof scratch - 1] = '\0';
		pool_super_cat(&cat, scratch);
		if(cat.is_truncated) break;
	}
	sprintf(cat.cursor, "%s",
		cat.is_truncated ? pool_cat_alter_end : pool_cat_end);
	return cat.print; /* static buffer */
}

#endif /* print --> */

#ifdef POOL_TEST /* <-- test */
#include "../test/TestPool.h" /* need this file if one is going to run tests */
#endif /* test --> */

/* prototype */
static void PRIVATE_T_(unused_coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 \url{ http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code } */
static void PRIVATE_T_(unused_set)(void) {
	T_(Pool_)(0);
	T_(Pool)(0, 0);
	T_(PoolGetError)(0);
	T_(PoolElement)(0);
	T_(PoolIsElement)(0, (size_t)0);
	T_(PoolGetElement)(0, (size_t)0);
	T_(PoolGetIndex)(0, 0);
	T_(PoolReserve)(0, (size_t)0);
	T_(PoolNew)(0);
	T_(PoolRemove)(0, 0);
	T_(PoolClear)(0);
	T_(PoolMigrateEach)(0, 0, 0);
	T_(PoolMigratePointer)(0, 0);
#ifdef POOL_TO_STRING
	T_(PoolToString)(0);
#endif
	PRIVATE_T_(unused_coda)();
}
/** {clang}'s pre-processor is not fooled if you have one function. */
static void PRIVATE_T_(unused_coda)(void) { PRIVATE_T_(unused_set)(); }



/* un-define all macros */
#undef POOL_NAME
#undef POOL_TYPE
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#undef T
#undef T_
#undef PRIVATE_T_
#undef T_NAME
#undef QUOTE
#undef QUOTE_
#ifdef POOL_TO_STRING
#undef POOL_TO_STRING
#endif
#ifdef POOL_DEBUG
#undef POOL_DEBUG
#endif
#ifdef POOL_NDEBUG
#undef POOL_NDEBUG
#undef NDEBUG
#endif
#ifdef POOL_TEST
#undef POOL_TEST
#endif
