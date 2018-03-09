/** 2016 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 {<T>Pool} is a dynamic array that stores unordered {<T>}, which must be set
 using {POOL_TYPE}. Removing an element is done lazily through a linked-list
 internal to the pool; as such, indices will remain the same throughout the
 lifetime of the data. You cannot shrink the capacity of this data type, only
 cause it to grow. Resizing incurs amortised cost, done though a Fibonacci
 sequence. {<T>Pool} is not synchronised. The preprocessor macros are all
 undefined at the end of the file for convenience when including multiple pool
 types in the same file.

 @param POOL_NAME
 This literally becomes {<T>}. As it's used in function names, this should
 comply with naming rules and be unique; required.

 @param POOL_TYPE
 The type associated with {<T>}. Has to be a valid type, accessible to the
 compiler at the time of inclusion; required.

 @param POOL_PARENT
 Optional type association with {<P>}. If set, the constructor has two extra
 arguments that allow it to be part of a larger data structure without
 referencing the {<T>Pool} directly. Can be {void} to turn off type checking.

 @param POOL_UPDATE
 Optional type association with {<U>}. If set, the function
 \see{<T>PoolUpdateNew} becomes available, intended for a local iterator
 update on migrate.

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
 @std		C89
 @author	Neil
 @version	2018-02 Errno instead of custom errors.
 @since		2017-12 Introduced POOL_PARENT for type-safety.
			2017-10 Replaced {PoolIsEmpty} by {PoolElement}, much more useful.
			2017-10 Renamed Pool; made migrate automatic.
			2017-07 Made migrate simpler.
			2017-05 Split {List} from {Pool}; much simpler.
			2017-01 Almost-redundant functions simplified.
			2016-11 Multi-index.
			2016-08 Permute. */



#include <stddef.h>	/* ptrdiff_t */
#include <stdlib.h>	/* malloc realloc free qsort */
#include <assert.h>	/* assert */
#include <string.h>	/* memcpy (memmove strerror strcpy memcmp in PoolTest.h) */
#include <errno.h>	/* errno */
#ifdef POOL_TO_STRING /* <-- print */
#include <stdio.h>	/* snprintf */
#endif /* print --> */
#ifdef POOL_DEBUG /* <-- debug */
#include <stdarg.h>	/* for print debug */
#endif /* debug --> */



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
 POOL_NAMEX, PT_(X) with POOL_U_NAME_X, and T_NAME with the string
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
#ifdef P
#undef P
#endif
#ifdef U
#undef U
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
static const size_t pool_not_part   = (size_t)-1;
/** Used as a null pointer with indices; {Pool} will not allow the size to be
 this big. */
static const size_t pool_null       = (size_t)-2;
#endif /* POOL_H --> */

/* Also left in the same translation unit. */
#ifndef MIGRATE /* <-- migrate */
#define MIGRATE
/** Contains information about a {realloc}. */
struct Migrate;
struct Migrate {
	const void *begin, *end; /* Old pointers. */
	ptrdiff_t delta;
};
#endif /* migrate --> */



/** Given to the custom migrate function of another data type. Used only for
 {POOL_TEST}. This definition is about the {POOL_NAME} type, that is, it is
 without the prefix {Pool}; to avoid namespace collisions, this is private,
 meaning the name is mangled. If you want this definition, re-declare it as
 {<T>Migrate}. */
typedef void (*PT_(Migrate))(T *const element,
	const struct Migrate *const migrate);

#ifdef POOL_TEST /* <-- test */
/* Operates by side-effects only. */
typedef void (*PT_(Action))(T *const element);
#endif /* test --> */

#ifdef POOL_TO_STRING /* <-- string */
/** Responsible for turning {<T>} (the first argument) into a 12 {char}
 null-terminated output string (the second.) Used for {POOL_TO_STRING}. */
typedef void (*PT_(ToString))(const T *, char (*const)[12]);
/* Check that {POOL_TO_STRING} is a function implementing {<T>ToString}. */
static const PT_(ToString) PT_(to_string) = (POOL_TO_STRING);
#endif /* string --> */

#ifdef POOL_PARENT /* <-- parent */
/* Troubles with this line? check to ensure that {POOL_PARENT} is a valid type,
 whose definition is placed above {#include "Pool.h"}. */
typedef POOL_PARENT PT_(ParentType);
#define P PT_(ParentType)
/** Function call on {realloc}. */
typedef void (*PT_(MigrateParent))(P *const parent,
	const struct Migrate *const migrate);
#endif /* parent --> */

#ifdef POOL_UPDATE /* <-- update */
/* Troubles with this line? check to ensure that {POOL_UPDATE} is a valid type,
 whose definition is placed above {#include "Pool.h"}. */
typedef POOL_UPDATE PT_(UpdateType);
#define U PT_(UpdateType)
#endif /* update --> */



/* Pool element. */
struct PT_(Element) {
	T data; /* The data. @fixme Has the be first, lame. */
	size_t prev, next; /* Removed offset queue. */
};

/** The pool. To instantiate, see \see{<T>Pool}. */
struct T_(Pool);
struct T_(Pool) {
	struct PT_(Element) *array;
	size_t capacity[2]; /* Fibonacci, [0] is the capacity, [1] is next. */
	size_t size; /* Including removed. */
	size_t head, tail; /* Removed queue. */
#ifdef POOL_PARENT
	PT_(MigrateParent) migrate; /* Called to update on resizing. */
	P *parent; /* Migrate parameter. */
#endif
};



/** Debug messages from pool functions; turn on using {POOL_DEBUG}. */
static void PT_(debug)(struct T_(Pool) *const this,
	const char *const fn, const char *const fmt, ...) {
#ifdef POOL_DEBUG
	/* \url{ http://c-faq.com/varargs/vprintf.html } */
	va_list argp;
	fprintf(stderr, "Pool<" T_NAME ">#%p.%s: ", (void *)this, fn);
	va_start(argp, fmt);
	vfprintf(stderr, fmt, argp);
	va_end(argp);
#else
	(void)(this), (void)(fn), (void)(fmt);
#endif
}

/* * Ensures capacity.
 @return Success; otherwise, {errno} will be set.
 @throws ERANGE: Tried allocating more then can fit in {size_t}.
 @throws ENOMEM: Technically, whatever {realloc} sets it to, as this is
 {IEEE Std 1003.1-2001}. */
static int PT_(reserve)(struct T_(Pool) *const this,
	const size_t min_capacity
#ifdef POOL_UPDATE /* <-- update */
	, U **const update_ptr
#endif /* update --> */
	) {
	size_t c0, c1;
	struct PT_(Element) *array;
	const size_t max_size = (pool_null - 1) / sizeof *array;
	assert(this && this->size <= this->capacity[0]
		&& this->capacity[0] <= this->capacity[1]
		&& this->capacity[1] < pool_null && pool_null < pool_not_part);
	if(this->capacity[0] >= min_capacity) return 1;
	if(max_size < min_capacity) return errno = ERANGE, 0;
	c0 = this->capacity[0];
	c1 = this->capacity[1];
	while(c0 < min_capacity) {
		c0 ^= c1, c1 ^= c0, c0 ^= c1, c1 += c0;
		if(c1 <= c0 || c1 > max_size) c1 = max_size;
	}
	if(!(array = realloc(this->array, c0 * sizeof *this->array))) return 0;
	PT_(debug)(this, "reserve", "array#%p[%lu] -> #%p[%lu].\n",
		(void *)this->array, (unsigned long)this->capacity[0], (void *)array,
		(unsigned long)c0);
#if defined(POOL_PARENT) || defined(POOL_UPDATE) /* <-- migrate */
	if(this->array != array && this->migrate) {
		/* Migrate parent class. Violates pedantic strict-ANSI? */
		struct Migrate migrate;
		migrate.begin = this->array;
		migrate.end   = (const char *)this->array + this->size * sizeof *array;
		migrate.delta = (const char *)array - (const char *)this->array;
#ifdef POOL_PARENT /* <-- parent */
		PT_(debug)(this, "reserve", "calling migrate.\n");
		assert(this->parent);
		this->migrate(this->parent, &migrate);
#endif /* parent --> */
#ifdef POOL_UPDATE /* <-- update */
		if(update_ptr) {
			const void *const u = *update_ptr;
			if(u >= migrate.begin && u < migrate.end)
				*(char **)update_ptr += migrate.delta;
		}
#endif /* update --> */
	}
#endif /* migrate --> */
	this->array = array;
	this->capacity[0] = c0;
	this->capacity[1] = c1;
	return 1;
}

/** We are very lazy and we just enqueue the removed for later elements.
 @param idx: Must be a valid index. */
static void PT_(enqueue_removed)(struct T_(Pool) *const this,
	const size_t e) {
	struct PT_(Element) *elem;
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
		struct PT_(Element) *const last = this->array + this->tail;
		assert(last->next == pool_null);
		last->next = this->tail = e;
	}
	elem->next = pool_null;
}

/** Dequeues a removed element, or if the queue is empty, returns null. */
static struct PT_(Element) *PT_(dequeue_removed)(
	struct T_(Pool) *const this) {
	struct PT_(Element) *elem;
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
		struct PT_(Element) *const next = this->array + elem->next;
		assert(elem->next < this->size);
		next->prev = pool_null;
	}
	elem->prev = elem->next = pool_not_part;
	return elem;
}

/** Gets rid of the removed elements at the tail of the list. Each remove has
 potentially one delete in the worst case, it just gets differed a bit. */
static void PT_(trim_removed)(struct T_(Pool) *const this) {
	struct PT_(Element) *elem, *prev, *next;
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

/** Destructor for {Pool}. Make sure that the pool's contents will not be
 accessed anymore.
 @param thisp: A reference to the object that is to be deleted; it will be set
 to null. If it is already null or it points to null, doesn't do anything.
 @order \Theta(1)
 @allow */
static void T_(Pool_)(struct T_(Pool) **const thisp) {
	struct T_(Pool) *this;
	if(!thisp || !(this = *thisp)) return;
	PT_(debug)(this, "Delete", "erasing.\n");
	free(this->array);
	free(this);
	*thisp = 0;
}

/** Private constructor called from either \see{<T>Pool}.
 @throws ENOMEM: Technically, whatever {malloc} sets it to, as this is
 {IEEE Std 1003.1-2001}. */
static struct T_(Pool) *PT_(pool)(void) {
	struct T_(Pool) *this;
	if(!(this = malloc(sizeof *this))) return 0;
	this->array        = 0;
	this->capacity[0]  = pool_fibonacci6;
	this->capacity[1]  = pool_fibonacci7;
	this->size         = 0;
	this->head = this->tail = pool_null;
	if(!(this->array = malloc(this->capacity[0] * sizeof *this->array)))
		return 0;
	PT_(debug)(this, "New", "capacity %d.\n", this->capacity[0]);
	return this;
}

#ifdef POOL_PARENT /* <-- parent */
/** Constructs an empty {Pool} with capacity Fibonacci6, which is 8. This is
 the constructor if {POOL_PARENT} is specified.
 @param migrate: The parent's {Migrate} function.
 @param parent: The parent; to have multiple parents, implement an intermediary
 {Migrate} function that takes multiple values; required if {migrate} is
 specified.
 @return A new {Pool} or null and {errno} may be set.
 @throws ERANGE: If one and not the other arguments is null.
 @throws ENOMEM: Technically, whatever {malloc} sets it to, as this is
 {IEEE Std 1003.1-2001}.
 @order \Theta(1)
 @allow */
static struct T_(Pool) *T_(Pool)(const PT_(MigrateParent) migrate, P *const parent) {
	struct T_(Pool) *this;
	if(!migrate ^ !parent) { errno = ERANGE; return 0; }
	if(!(this = PT_(pool)())) return 0; /* ENOMEM? */
	this->migrate = migrate;
	this->parent  = parent;
	return this;
}
#else /* parent --><-- !parent */
/** Constructs an empty {Pool} with capacity Fibonacci6, which is 8.
 @return A new {Pool} or null and {errno} may be set.
 @throws ENOMEM: Technically, whatever {malloc} sets it to, as this is
 {IEEE Std 1003.1-2001}.
 @order \Theta(1)
 @allow */
static struct T_(Pool) *T_(Pool)(void) {
	return PT_(pool)(); /* ENOMEM? */
}
#endif /* parent --> */

/** @param this: If null, returns null.
 @return One value from the pool or null if the pool is empty. It selects
 the position deterministically.
 @order \Theta(1)
 @fixme Untested.
 @allow */
static T *T_(PoolElement)(const struct T_(Pool) *const this) {
	if(!this || !this->size) return 0;
	return &this->array[this->size - 1].data;
}

/** Is {idx} a valid index for {this}?
 @order \Theta(1)
 @allow */
static int T_(PoolIsElement)(struct T_(Pool) *const this, const size_t idx) {
	struct PT_(Element) *elem;
	if(!this) return 0;
	if(idx >= this->size
		|| (elem = this->array + idx, elem->prev != pool_not_part))
		return 0;
	return 1;
}

/** Is {data} still a valid element? Use when you have a pointer to an element,
 but you're not sure if it's been deleted. One can not use it on a {realloc}ed
 or not part of a {Pool} pointer. If you delete and add another one,
 {<T>}PoolIsValid may return true, but may not be the element that one expects.
 @order \Theta(1)
 @allow */
static int T_(PoolIsValid)(const T *const data) {
	const struct PT_(Element) *const elem
		= (const struct PT_(Element) *const)(const void *const)data;
	if(!elem || elem->prev != pool_not_part) return 0;
	return 1;
}

/** Gets an existing element by index. Causing something to be added to the
 {Pool} may invalidate this pointer because of a {realloc}.
 @param this: If {this} is null, returns null.
 @param idx: Index.
 @return If failed, returns a null pointer {errno} will be set.
 @throws EDOM: {idx} out of bounds.
 @order \Theta(1)
 @allow */
static T *T_(PoolGetElement)(struct T_(Pool) *const this, const size_t idx) {
	struct PT_(Element) *elem;
	if(!this) return 0;
	if(idx >= this->size
		|| (elem = this->array + idx, elem->prev != pool_not_part))
		{ errno = EDOM; return 0; }
	return &elem->data;
}

/** Gets an index given {element}.
 @param element: If the element is not part of the {Pool}, behaviour is
 undefined.
 @return An index.
 @order \Theta(1)
 @fixme Untested.
 @allow */
static size_t T_(PoolGetIndex)(struct T_(Pool) *const this,
	const T *const element) {
	return (const struct PT_(Element) *)(const void *)element
		- this->array;
}

/** Increases the capacity of this Pool to ensure that it can hold at least
 the number of elements specified by the {min_capacity}.
 @param this: If {this} is null, returns false.
 @return True if the capacity increase was viable; otherwise the pool is not
 touched and {errno} may be set.
 @throws ERANGE: Tried allocating more then can fit in {size_t} objects.
 @throws ENOMEM: Technically, whatever {realloc} sets it to, as this is
 {IEEE Std 1003.1-2001}.
 @order \Omega(1), O({capacity})
 @allow */
static int T_(PoolReserve)(struct T_(Pool) *const this,
	const size_t min_capacity) {
	if(!this) return 0;
	if(!PT_(reserve)(this, min_capacity
#ifdef POOL_UPDATE /* <-- update */
		, 0
#endif /* update --> */
		)) return 0; /* ERANGE, ENOMEM? */
	PT_(debug)(this, "Reserve", "pool pool size to %u to contain %u.\n",
		this->capacity[0], min_capacity);
	return 1;
}

/** Gets an uninitialised new element at the end of the {Pool}. May move the
 {Pool} to a new memory location to fit the new size.
 @param this: If {this} is null, returns null.
 @return A new, un-initialised, element, or null and {errno} may be set.
 @throws ERANGE: Tried allocating more then can fit in {size_t} objects.
 @throws ENOMEM: Technically, whatever {realloc} sets it to, as this is
 {IEEE Std 1003.1-2001}.
 @order amortised O(1)
 @allow */
static T *T_(PoolNew)(struct T_(Pool) *const this) {
	struct PT_(Element) *elem;
	if(!this) return 0;
	if(!(elem = PT_(dequeue_removed)(this))) {
		if(!PT_(reserve)(this, this->size + 1
#ifdef POOL_UPDATE /* <-- update */
			, 0
#endif /* update --> */
			)) return 0; /* ERANGE, ENOMEM? */
		elem = this->array + this->size++;
		elem->prev = elem->next = pool_not_part;
	}
	PT_(debug)(this, "New", "added.\n");
	return &elem->data;
}

#ifdef POOL_UPDATE /* <-- update */
/** Gets an uninitialised new element and updates the {update_ptr} if it is
 within the memory region that was changed. Must have {POOL_UPDATE} defined.
 @param this: If {this} is null, returns null.
 @param iterator_ptr: Pointer to update on migration.
 @return A new, un-initialised, element, or null and {errno} may be set.
 @throws ERANGE: Tried allocating more then can fit in {size_t}.
 @throws ENOMEM: Technically, whatever {realloc} sets it to, as this is
 {IEEE Std 1003.1-2001}.
 @order amortised O(1)
 @fixme Untested.
 @allow */
static T *T_(PoolUpdateNew)(struct T_(Pool) *const this,
	U **const update_ptr) {
	struct PT_(Element) *elem;
	if(!this) return 0;
	if(!(elem = PT_(dequeue_removed)(this))) {
		if(!PT_(reserve)(this, this->size + 1, update_ptr))
			return 0; /* ERANGE, ENOMEM? */
		elem = this->array + this->size++;
		elem->prev = elem->next = pool_not_part;
	}
	PT_(debug)(this, "New", "added.\n");
	return &elem->data;
}
#endif /* update --> */

/** Removes an element associated with {data} from {this}.
 @param this, data: If null, returns false.
 @return Success, otherwise {errno} will be set.
 @throws EDOM: {data} is not part of {list}.
 @order amortised O(1)
 @allow */
static int T_(PoolRemove)(struct T_(Pool) *const this, T *const data) {
	struct PT_(Element) *elem;
	size_t e;
	if(!this || !data) return 0;
	elem = (struct PT_(Element) *)(void *)data;
	e = elem - this->array;
	if(elem < this->array || e >= this->size || elem->prev != pool_not_part)
		return errno = EDOM, 0;
	PT_(enqueue_removed)(this, e);
	if(e >= this->size - 1) PT_(trim_removed)(this);
	PT_(debug)(this, "Remove", "removing %lu.\n", (unsigned long)e);
	return 1;
}

/** Removes all data from {this}.
 @param this: if null, does nothing.
 @order \Theta(1)
 @allow */
static void T_(PoolClear)(struct T_(Pool) *const this) {
	if(!this) return;
	this->size = 0;
	this->head = this->tail = pool_null;
	PT_(debug)(this, "Clear", "cleared.\n");
}

/** Use when the pool has pointers to another pool in the {Migrate} function of
 the other data type (passed when creating the other data type.)
 @param this: If null, does nothing.
 @param handler: If null, does nothing, otherwise has the responsibility of
 calling the other data type's migrate pointer function on all pointers
 affected by the {realloc}.
 @param migrate: If null, does nothing. Should only be called in a {Migrate}
 function; pass the {migrate} parameter.
 @order O({greatest size})
 @fixme Untested.
 @allow */
static void T_(PoolMigrateEach)(struct T_(Pool) *const this,
	const PT_(Migrate) handler, const struct Migrate *const migrate) {
	struct PT_(Element) *e, *end;
	if(!this || !migrate || !handler) return;
	for(e = this->array, end = e + this->size; e < end; e++)
		if(e->prev == pool_not_part) handler(&e->data, migrate);
}

/** Use this inside the function that is passed to the (generally other's)
 migrate function. Allows pointers to the pool to be updated. It doesn't affect
 pointers not in the {realloc}ed region.
 @order \Omega(1)
 @fixme Untested.
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
		PT_(to_string)(&this->array[i].data, &scratch),
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
#include "../test/TestPool.h" /* Need this file if one is going to run tests. */
#endif /* test --> */

/* Prototype. */
static void PT_(unused_coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 \url{ http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code } */
static void PT_(unused_set)(void) {
	T_(Pool_)(0);
#ifdef POOL_PARENT
	T_(Pool)(0, 0);
#else
	T_(Pool)();
#endif
	T_(PoolElement)(0);
	T_(PoolIsElement)(0, (size_t)0);
	T_(PoolIsValid)(0);
	T_(PoolGetElement)(0, (size_t)0);
	T_(PoolGetIndex)(0, 0);
	T_(PoolReserve)(0, (size_t)0);
	T_(PoolNew)(0);
#ifdef POOL_UPDATE /* <-- update */
	T_(PoolUpdateNew)(0, 0);
#endif /* update --> */
	T_(PoolRemove)(0, 0);
	T_(PoolClear)(0);
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
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#undef T
#undef T_
#undef PT_
#undef T_NAME
#undef QUOTE
#undef QUOTE_
#ifdef P
#undef P
#endif
#ifdef U
#undef U
#endif
#undef POOL_NAME
#undef POOL_TYPE
#ifdef POOL_PARENT
#undef POOL_PARENT
#endif
#ifdef POOL_UPDATE
#undef POOL_UPDATE
#endif
#ifdef POOL_TO_STRING
#undef POOL_TO_STRING
#endif
#ifdef POOL_DEBUG
#undef POOL_DEBUG
#endif
#ifdef POOL_TEST
#undef POOL_TEST
#endif
#ifdef POOL_NDEBUG
#undef POOL_NDEBUG
#undef NDEBUG
#endif
