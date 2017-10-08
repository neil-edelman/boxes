/** 2016 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 {<T>Store} is a dynamic array that stores unordered {<T>}, which must be set
 using {STORE_TYPE}. When using polymorphic data types, providing a contiguous
 array for each concrete type is better for cache performance. Removing an
 element is done lazily through a queue internal to the store; as such, indices
 will remain the same throughout the lifetime of the data. You cannot shrink
 the size of this data type, only cause it to grow. Resizing incurs amortised
 cost, done though a Fibonacci sequence. {<T>Store} is not synchronised. The
 preprocessor macros are all undefined at the end of the file for convenience
 when including multiple store types in the same file.

 @param STORE_NAME
 This literally becomes {<T>}. As it's used in function names, this should
 comply with naming rules and be unique; required.

 @param STORE_TYPE
 The type associated with {<T>}. Has to be a valid type, accessible to the
 compiler at the time of inclusion; required.

 @param STORE_TO_STRING
 Optional print function implementing {<T>ToString}; makes available
 \see{<T>StoreToString}.

 @param STORE_DEBUG
 Prints information to {stderr}. Requires {STORE_TO_STRING}.

 @param STORE_TEST
 Unit testing framework using {<T>StoreTest}, included in a separate header,
 {../test/StoreTest.h}. Must be defined equal to a (random) filler function,
 satisfying {<T>Action}. If {NDEBUG} is not defined, turns on {assert} private
 function integrity testing. Requires {STORE_TO_STRING}.

 @title		Store.h
 @std		C89/90
 @author	Neil
 @version	1.4; 2017-07 made migrate simpler
 @since		1.3; 2017-05 split {List} from {Store}; much simpler
			1.2; 2017-01 almost-redundant functions simplified
			1.1; 2016-11 multi-index
			1.0; 2016-08 permute */



#include <stddef.h>	/* ptrdiff_t */
#include <stdlib.h>	/* malloc free qsort */
#include <assert.h>	/* assert */
#include <string.h>	/* memcpy (memmove strerror strcpy memcmp in StoreTest.h) */
#ifdef STORE_TO_STRING /* <-- print */
#include <stdio.h>	/* snprintf */
#endif /* print --> */
#include <errno.h>	/* errno */
#ifdef STORE_DEBUG
#include <stdarg.h>	/* for print */
#endif /* calls --> */



/* unused macro */
#ifdef UNUSED
#undef UNUSED
#endif
#define UNUSED(a) ((void)(a))



/* check defines */
#ifndef STORE_NAME
#error Store generic STORE_NAME undefined.
#endif
#ifndef STORE_TYPE
#error Store generic STORE_TYPE undefined.
#endif
#if (defined(STORE_DEBUG) || defined(STORE_TEST)) && !defined(STORE_TO_STRING)
#error Store: STORE_DEBUG and STORE_TEST require STORE_TO_STRING.
#endif
#if !defined(STORE_TEST) && !defined(NDEBUG)
#define STORE_NDEBUG
#define NDEBUG
#endif



/* After this block, the preprocessor replaces T with STORE_TYPE, T_(X) with
 STORE_NAMEX, PRIVATE_T_(X) with STORE_U_NAME_X, and T_NAME with the string
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
#define T_(thing) CAT(STORE_NAME, thing)
#define PRIVATE_T_(thing) PCAT(store, PCAT(STORE_NAME, thing))
#define T_NAME QUOTE(STORE_NAME)

/* Troubles with this line? check to ensure that STORE_TYPE is a valid type,
 whose definition is placed above {#include "Store.h"}. */
typedef STORE_TYPE PRIVATE_T_(Type);
#define T PRIVATE_T_(Type)





/* constants across multiple includes in the same translation unit */
#ifndef STORE_H /* <-- STORE_H */
#define STORE_H

static const size_t store_fibonacci6 = 8;
static const size_t store_fibonacci7 = 13;
static const size_t store_not_part   = (size_t)-1;
/** Used as a null pointer with indices; {Store} will not allow the size to be
 this big. */
static const size_t store_null       = (size_t)-2;

/* designated initializers are C99; this is safe because C has rules for enum
 default initialisers */
enum StoreError {
	STORE_NO_ERROR,
	STORE_ERRNO,
	STORE_PARAMETER,
	STORE_OUT_OF_BOUNDS,
	STORE_OVERFLOW
};
static const char *const store_error_explination[] = {
	/*[STORE_NO_ERROR]      =*/ "no error",
	/*[STORE_ERRNO]         =*/ 0, /* <- get errno */
	/*[STORE_PARAMETER]     =*/ "parameter out-of-range",
	/*[STORE_OUT_OF_BOUNDS] =*/ "out-of-bounds",
	/*[STORE_OVERFLOW]      =*/ "overflow"
};

/* global for constructor allocation errors */
static enum StoreError store_global_error;
static int             store_global_errno_copy;

#endif /* STORE_H --> */



/** Operates by side-effects only. */
typedef void (*T_(Action))(T *const element);

/** Takes along a param. */
typedef void (*T_(BiAction))(T *const, void *const);

/** Returns (non-zero) true or (zero) false. */
typedef int  (*T_(Predicate))(T *const element);

/** Passed {T} and a user-defined pointer value, returns (non-zero) true or
 (zero) false. */
typedef int (*T_(BiPredicate))(T *const, void *const);

#ifdef STORE_TO_STRING /* <-- string */

/** Responsible for turning {<T>} (the first argument) into a 12 {char}
 null-terminated output string (the second.) */
typedef void (*T_(ToString))(const T *, char (*const)[12]);

/* Check that {STORE_TO_STRING} is a function implementing {<T>ToString}. */
static const T_(ToString) PRIVATE_T_(to_string) = (STORE_TO_STRING);

#endif /* string --> */

#ifndef MIGRATE /* <-- migrate */
#define MIGRATE
/** Contains information about a {realloc}. */
struct Migrate;
struct Migrate {
	const void *begin, *end; /* old pointers */
	ptrdiff_t delta;
};
/** Calls on {realloc}. */
typedef void (*Migrate)(const void *parent,
	const struct Migrate *const migrate);
#endif /* migrate --> */




/* Store element. */
struct PRIVATE_T_(Element) {
	T data; /* has to be the first element for convenience */
	size_t prev, next; /* removed offset queue */
};

/** The store. To instantiate, see \see{<T>Store}. */
struct T_(Store);
struct T_(Store) {
	struct PRIVATE_T_(Element) *array;
	size_t capacity[2]; /* Fibonacci, [0] is the capacity, [1] is next */
	size_t size; /* including removed */
	size_t head, tail; /* removed queue */
	enum StoreError error; /* errors defined by enum StoreError */
	int errno_copy; /* copy of errno when when error == E_ERRNO */
	Migrate migrate; /* called to update on resizing */
	void *parent; /* what to call migrate on */
};



/** Debug messages from store functions; turn on using {STORE_DEBUG}. */
static void PRIVATE_T_(debug)(struct T_(Store) *const this,
	const char *const fn, const char *const fmt, ...) {
#ifdef STORE_DEBUG
	/* \url{ http://c-faq.com/varargs/vprintf.html } */
	va_list argp;
	fprintf(stderr, "Store<" T_NAME ">#%p.%s: ", (void *)this, fn);
	va_start(argp, fmt);
	vfprintf(stderr, fmt, argp);
	va_end(argp);
#else
	UNUSED(this); UNUSED(fn); UNUSED(fmt);
#endif
}

/** Ensures capacity.
 @return Success.
 @throws STORE_OVERFLOW, STORE_ERRNO */
static int PRIVATE_T_(reserve)(struct T_(Store) *const this,
	const size_t min_capacity) {
	size_t c0, c1;
	struct PRIVATE_T_(Element) *array;
	const size_t max_size = (store_null - 1) / sizeof *array;
	assert(this);
	assert(this->size <= this->capacity[0]);
	assert(this->capacity[0] <= this->capacity[1]);
	assert(this->capacity[1] < store_null);
	assert(store_null < store_not_part);
	if(this->capacity[0] >= min_capacity) return 1;
	if(max_size < min_capacity) return this->error = STORE_OVERFLOW, 0; 
	c0 = this->capacity[0];
	c1 = this->capacity[1];
	while(c0 < min_capacity) {
		c0 ^= c1, c1 ^= c0, c0 ^= c1, c1 += c0;
		if(c1 <= c0 || c1 > max_size) c1 = max_size;
	}
	if(!(array = realloc(this->array, c0 * sizeof *this->array)))
		return this->error = STORE_ERRNO, this->errno_copy = errno, 0;
	PRIVATE_T_(debug)(this, "reserve", "array#%p[%lu] -> #%p[%lu].\n",
		(void *)this->array, (unsigned long)this->capacity[0], (void *)array,
		(unsigned long)c0);
	/* Migrate parent class. This is _ugly_ af, and ensures
	 in-interoperablility. I think it violates pedantic strict-ANSI. It
	 subverts type-safety. It doesn't allow moving of temporary pointers. It is
	 awful. However, it is so convenient for the caller not to have to worry
	 about moving memory blocks. */
	if(this->array != array) {
		struct Migrate m;
		m.begin = (const char *)this->array;
		m.end   = (const char *)this->array + this->size * sizeof *array;
		m.delta = (const char *)array - (const char *)this->array;
		PRIVATE_T_(debug)(this, "reserve", "calling migrate.\n");
		this->migrate(this->parent, &m);
	}
	this->array = array;
	this->capacity[0] = c0;
	this->capacity[1] = c1;
	return 1;
}

/** We are very lazy and we just enqueue the removed for later elements.
 @param idx: Must be a valid index. */
static void PRIVATE_T_(enqueue_removed)(struct T_(Store) *const this,
	const size_t e) {
	struct PRIVATE_T_(Element) *elem;
	assert(this);
	assert(e < this->size);
	elem = this->array + e;
	/* cannot be part of the removed store already */
	assert(elem->prev == store_not_part);
	assert(elem->next == store_not_part);
	if((elem->prev = this->tail) == store_null) {
		assert(this->head == store_null);
		this->head = this->tail = e;
	} else {
		struct PRIVATE_T_(Element) *const last = this->array + this->tail;
		assert(last->next == store_null);
		last->next = this->tail = e;
	}
	elem->next = store_null;
}

/** Dequeues a removed element, or if the queue is empty, returns null. */
static struct PRIVATE_T_(Element) *PRIVATE_T_(dequeue_removed)(
	struct T_(Store) *const this) {
	struct PRIVATE_T_(Element) *elem;
	size_t e;
	assert(this);
	assert((this->head == store_null) == (this->tail == store_null));
	if((e = this->head) == store_null) return 0;
	elem = this->array + e;
	assert(elem->prev == store_null);
	assert(elem->next != store_not_part);
	if((this->head = elem->next) == store_null) {
		this->head = this->tail = store_null;
	} else {
		struct PRIVATE_T_(Element) *const next = this->array + elem->next;
		assert(elem->next < this->size);
		next->prev = store_null;
	}
	elem->prev = elem->next = store_not_part;
	return elem;
}

/** Gets rid of the removed elements at the tail of the list. Each remove has
 potentially one delete in the worst case, it just gets differed a bit. */
static void PRIVATE_T_(trim_removed)(struct T_(Store) *const this) {
	struct PRIVATE_T_(Element) *elem, *prev, *next;
	size_t e;
	assert(this);
	while(this->size
		&& (elem = this->array + (e = this->size - 1))->prev != store_not_part) {
		if(elem->prev == store_null) {
			assert(this->head == e), this->head = elem->next;
		} else {
			assert(elem->prev < this->size), prev = this->array + elem->prev;
			prev->next = elem->next;
		}
		if(elem->next == store_null) {
			assert(this->tail == e), this->tail = elem->prev;
		} else {
			assert(elem->next < this->size), next = this->array + elem->next;
			next->prev = elem->prev;
		}
		this->size--;
	}
}

/** Destructor for Store. Make sure that the store's contents will not be
 accessed anymore.
 @param thisp: A reference to the object that is to be deleted; it will be store
 to null. If it is already null or it points to null, doesn't do anything.
 @order \Theta(1)
 @allow */
static void T_(Store_)(struct T_(Store) **const thisp) {
	struct T_(Store) *this;
	if(!thisp || !(this = *thisp)) return;
	PRIVATE_T_(debug)(this, "Delete", "erasing.\n");
	free(this->array);
	free(this);
	*thisp = 0;
}

/** Constructs an empty Store with capacity Fibonacci6, which is 8.
 @param migrate: The ADT parent's {Migrate} function.
 @param parent: The parent itself. You can not have multiple parents. You
 cannot change a parent. If you need this flexibility, create a new store.
 @return A new Store for the polymorphic variable {parent}.
 @throws STORE_PARAMETER, STORE_ERRNO: Use {StoreError(0)} to get the error.
 @order \Theta(1)
 @allow */
static struct T_(Store) *T_(Store)(const Migrate migrate, void *const parent) {
	struct T_(Store) *this;
	if(!migrate || !parent) {
		store_global_error = STORE_PARAMETER;
		store_global_errno_copy = 0;
		return 0;
	}
	if(!(this = malloc(sizeof(struct T_(Store))))) {
		store_global_error = STORE_ERRNO;
		store_global_errno_copy = errno;
		return 0;
	}
	this->array        = 0;
	this->capacity[0]  = store_fibonacci6;
	this->capacity[1]  = store_fibonacci7;
	this->size         = 0;
	this->head = this->tail = store_null;
	this->error        = STORE_NO_ERROR;
	this->errno_copy   = 0;
	this->migrate      = migrate;
	this->parent       = parent;
	if(!(this->array = malloc(this->capacity[0] * sizeof *this->array))) {
		T_(Store_)(&this);
		store_global_error = STORE_ERRNO;
		store_global_errno_copy = errno;
		return 0;
	}
	PRIVATE_T_(debug)(this, "New", "capacity %d.\n", this->capacity[0]);
	return this;
}

/** See what's the error if something goes wrong. Resets the error.
 @return The last error string.
 @order \Theta(1)
 @allow */
static const char *T_(StoreGetError)(struct T_(Store) *const this) {
	const char *str;
	enum StoreError *perr;
	int *perrno;
	perr   = this ? &this->error      : &store_global_error;
	perrno = this ? &this->errno_copy : &store_global_errno_copy;
	if(!(str = store_error_explination[*perr])) str = strerror(*perrno);
	*perr = 0;
	return str;
}

/** @return	Is the store empty?
 @param this: If {this} is null, returns true.
 @order \Theta(1)
 @allow */
static size_t T_(StoreIsEmpty)(const struct T_(Store) *const this) {
	if(!this) return 1;
	return this->size ? 0 : 1;
}

/** Is {idx} a valid index for {this}.
 @order \Theta(1)
 @allow */
static int T_(StoreIsElement)(struct T_(Store) *const this, const size_t idx) {
	struct PRIVATE_T_(Element) *elem;
	if(!this) return 0;
	if(idx >= this->size
		|| (elem = this->array + idx, elem->prev != store_not_part))
		return 0;
	return 1;
}

/** Gets an existing element by index.
 @param this: If {this} is null, returns null.
 @param idx: Index.
 @return If failed, returns a null pointer and the error condition will be set.
 @throws STORE_OUT_OF_BOUNDS
 @order \Theta(1)
 @allow */
static T *T_(StoreGetElement)(struct T_(Store) *const this, const size_t idx) {
	struct PRIVATE_T_(Element) *elem;
	if(!this) return 0;
	if(idx >= this->size
	   || (elem = this->array + idx, elem->prev != store_not_part))
	{ this->error = STORE_OUT_OF_BOUNDS; return 0; }
	return &elem->data;
}

/** Increases the capacity of this Store to ensure that it can hold at least
 the number of elements specified by the {min_capacity}.
 @param this: If {this} is null, returns false.
 @return True if the capacity increase was viable; otherwise the store is not
 touched and the error condition is store.
 @throws STORE_ERRNO, STORE_OVERFLOW
 @order \Omega(1), O({capacity})
 @allow */
static int T_(StoreReserve)(struct T_(Store) *const this,
	const size_t min_capacity) {
	if(!this) return 0;
	if(!PRIVATE_T_(reserve)(this, min_capacity)) return 0;
	PRIVATE_T_(debug)(this, "Reserve","store store size to %u to contain %u.\n",
		this->capacity[0], min_capacity);
	return 1;
}

/** Gets an uninitialised new element.
 @param this: If {this} is null, returns null.
 @return If failed, returns a null pointer and the error condition will be set.
 @throws STORE_OVERFLOW, STORE_ERRNO
 @order amortised O(1)
 @allow */
static T *T_(StoreNew)(struct T_(Store) *const this) {
	struct PRIVATE_T_(Element) *elem;
	if(!this) return 0;
	if(!(elem = PRIVATE_T_(dequeue_removed)(this))) {
		if(!PRIVATE_T_(reserve)(this, this->size + 1)) return 0;
		elem = this->array + this->size++;
		elem->prev = elem->next = store_not_part;
	}
	PRIVATE_T_(debug)(this, "New", "added.\n");
	return &elem->data;
}

/** Removes an element associated with {data} from {this}.
 @param this: If {this} is null, returns false.
 @return Success.
 @throws STORE_OUT_OF_BOUNDS
 @order amortised O(1)
 @allow */
static int T_(StoreRemove)(struct T_(Store) *const this, T *const data) {
	struct PRIVATE_T_(Element) *elem;
	size_t e;
	if(!this || !data) return 0;
	elem = (struct PRIVATE_T_(Element) *)(void *)data;
	if(elem < this->array
		|| this->array + this->size <= elem
		|| elem->prev != store_not_part)
		return this->error = STORE_OUT_OF_BOUNDS, 0;
	e = elem - this->array;
	PRIVATE_T_(enqueue_removed)(this, e);
	if(e >= this->size - 1) PRIVATE_T_(trim_removed)(this);
	PRIVATE_T_(debug)(this, "Remove", "removing %lu.\n", (unsigned long)e);
	return 1;
}

/** Removes all data from {this}.
 @order \Theta(1)
 @allow */
static void T_(StoreClear)(struct T_(Store) *const this) {
	if(!this) return;
	this->size = 0;
	this->head = this->tail = store_null;
	PRIVATE_T_(debug)(this, "Clear", "cleared.\n");
}

#ifdef STORE_TO_STRING /* <-- print */

#ifndef STORE_PRINT_THINGS /* <-- once inside translation unit */
#define STORE_PRINT_THINGS

static const char *const store_cat_start     = "[ ";
static const char *const store_cat_end       = " ]";
static const char *const store_cat_alter_end = "...]";
static const char *const store_cat_sep       = ", ";
static const char *const store_cat_star      = "*";
static const char *const store_cat_null      = "null";

struct Store_SuperCat {
	char *print, *cursor;
	size_t left;
	int is_truncated;
};
static void store_super_cat_init(struct Store_SuperCat *const cat,
	char *const print, const size_t print_size) {
	cat->print = cat->cursor = print;
	cat->left = print_size;
	cat->is_truncated = 0;
	print[0] = '\0';
}
static void store_super_cat(struct Store_SuperCat *const cat,
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

/** Can print 4 things at once before it overwrites. One must store
 {STORE_TO_STRING} to a function implementing {<T>ToString} to get this
 functionality.
 @return Prints {this} in a static buffer.
 @order \Theta(1); it has a 255 character limit; every element takes some of it.
 @allow */
static const char *T_(StoreToString)(const struct T_(Store) *const this) {
	static char buffer[4][256];
	static unsigned buffer_i;
	struct Store_SuperCat cat;
	int is_first = 1;
	char scratch[12];
	size_t i;
	assert(strlen(store_cat_alter_end) >= strlen(store_cat_end));
	assert(sizeof buffer > strlen(store_cat_alter_end));
	store_super_cat_init(&cat, buffer[buffer_i],
		sizeof *buffer / sizeof **buffer - strlen(store_cat_alter_end));
	buffer_i++, buffer_i &= 3;
	if(!this) {
		store_super_cat(&cat, store_cat_null);
		return cat.print;
	}
	store_super_cat(&cat, store_cat_start);
	for(i = 0; i < this->size; i++) {
		if(this->array[i].prev != store_not_part) continue;
		if(!is_first) store_super_cat(&cat, store_cat_sep); else is_first = 0;
		PRIVATE_T_(to_string)(&this->array[i].data, &scratch),
		scratch[sizeof scratch - 1] = '\0';
		store_super_cat(&cat, scratch);
		if(cat.is_truncated) break;
	}
	sprintf(cat.cursor, "%s",
		cat.is_truncated ? store_cat_alter_end : store_cat_end);
	return cat.print; /* static buffer */
}

#endif /* print --> */

#ifdef STORE_TEST /* <-- test */
#include "../test/TestStore.h" /* need this file if one is going to run tests */
#endif /* test --> */

/* prototype */
static void PRIVATE_T_(unused_coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 \url{ http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code } */
static void PRIVATE_T_(unused_set)(void) {
	T_(Store_)(0);
	T_(Store)(0, 0);
	T_(StoreGetError)(0);
	T_(StoreIsEmpty)(0);
	T_(StoreIsElement)(0, (size_t)0);
	T_(StoreGetElement)(0, (size_t)0);
	T_(StoreReserve)(0, (size_t)0);
	T_(StoreNew)(0);
	T_(StoreRemove)(0, 0);
	T_(StoreClear)(0);
#ifdef STORE_TO_STRING
	T_(StoreToString)(0);
#endif
	PRIVATE_T_(unused_coda)();
}
/** {clang}'s pre-processor is not fooled if you have one function. */
static void PRIVATE_T_(unused_coda)(void) { PRIVATE_T_(unused_set)(); }



/* un-define all macros */
#undef STORE_NAME
#undef STORE_TYPE
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
#ifdef STORE_TO_STRING
#undef STORE_TO_STRING
#endif
#ifdef STORE_DEBUG
#undef STORE_DEBUG
#endif
#ifdef STORE_NDEBUG
#undef STORE_NDEBUG
#undef NDEBUG
#endif
#ifdef STORE_TEST
#undef STORE_TEST
#endif
