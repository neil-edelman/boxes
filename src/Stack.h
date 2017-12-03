/** 2016 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 {<T>Stack} is a dynamic array that stores unordered {<T>} in a stack; that is,
 the most basic variable array. Indices will remain the same throughout the
 lifetime of the data, but expanding the data may change the pointers. You
 cannot shrink the capacity of this data type, only cause it to grow. Resizing
 incurs amortised cost, done though a Fibonacci sequence. {<T>Stack} is not
 synchronised. The preprocessor macros are all undefined at the end of the file
 for convenience when including multiple {<T>Stack} types in the same file.

 @param STACK_NAME
 This literally becomes {<T>}. As it's used in function names, this should
 comply with naming rules and be unique; required.

 @param STACK_TYPE
 The type associated with {<T>}. Has to be a valid type, accessible to the
 compiler at the time of inclusion; required.

 @param STACK_MIGRATE
 If set, the constructor has two extra arguments that allow it to be part of a
 larger data structure without referencing the {<T>Stack}.

 @param STACK_TO_STRING
 Optional print function implementing {<T>ToString}; makes available
 \see{<T>StackToString}.

 @param STACK_DEBUG
 Prints information to {stderr}. Requires {STACK_TO_STRING}.

 @param STACK_TEST
 Unit testing framework using {<T>StackTest}, included in a separate
 header, {../test/StackTest.h}. Must be defined equal to a (random) filler
 function, satisfying {<T>Action}. If {NDEBUG} is not defined, turns on
 {assert} private function integrity testing. Requires {STACK_TO_STRING}.

 @title		Stack.h
 @std		C89/90
 @author	Neil
 @version	2017-11 Added STACK_MIGRATE.
 @since		2017-11 Forked from Pool. */



#include <stddef.h>	/* ptrdiff_t */
#include <stdlib.h>	/* malloc free qsort */
#include <assert.h>	/* assert */
#include <string.h>	/* memcpy (memmove strerror strcpy memcmp in StackTest.h) */
#ifdef STACK_TO_STRING /* <-- print */
#include <stdio.h>	/* snprintf */
#endif /* print --> */
#include <errno.h>	/* errno */
#ifdef STACK_DEBUG
#include <stdarg.h>	/* for print */
#endif /* calls --> */



/* unused macro */
#ifdef UNUSED
#undef UNUSED
#endif
#define UNUSED(a) ((void)(a))



/* check defines */
#ifndef STACK_NAME
#error Stack generic STACK_NAME undefined.
#endif
#ifndef STACK_TYPE
#error Stack generic STACK_TYPE undefined.
#endif
#if (defined(STACK_DEBUG) || defined(STACK_TEST)) && !defined(STACK_TO_STRING)
#error Stack: STACK_DEBUG and STACK_TEST require STACK_TO_STRING.
#endif
#if !defined(STACK_TEST) && !defined(NDEBUG)
#define STACK_NDEBUG
#define NDEBUG
#endif



/* After this block, the preprocessor replaces T with STACK_TYPE, T_(X) with
 STACK_NAMEX, PRIVATE_T_(X) with STACK_U_NAME_X, and T_NAME with the string
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
#define T_(thing) CAT(STACK_NAME, thing)
#define PRIVATE_T_(thing) PCAT(pool, PCAT(STACK_NAME, thing))
#define T_NAME QUOTE(STACK_NAME)

/* Troubles with this line? check to ensure that STACK_TYPE is a valid type,
 whose definition is placed above {#include "Stack.h"}. */
typedef STACK_TYPE PRIVATE_T_(Type);
#define T PRIVATE_T_(Type)





/* constants across multiple includes in the same translation unit */
#ifndef STACK_H /* <-- STACK_H */
#define STACK_H

static const size_t stack_fibonacci6 = 8;
static const size_t stack_fibonacci7 = 13;

/* designated initializers are C99; this is safe because C has rules for enum
 default initialisers */
enum StackError {
	STACK_NO_ERROR,
	STACK_ERRNO,
	STACK_PARAMETER,
	STACK_OUT_OF_BOUNDS,
	STACK_OVERFLOW
};
static const char *const stack_error_explination[] = {
	/*[STACK_NO_ERROR]      =*/ "no error",
	/*[STACK_ERRNO]         =*/ 0, /* <- get errno */
	/*[STACK_PARAMETER]     =*/ "parameter out-of-range",
	/*[STACK_OUT_OF_BOUNDS] =*/ "out-of-bounds",
	/*[STACK_OVERFLOW]      =*/ "overflow"
};

/* global for constructor allocation errors */
static enum StackError stack_global_error;
static int             stack_global_errno_copy;

#endif /* STACK_H --> */



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



/** Operates by side-effects only. Used for {STACK_TEST}. */
typedef void (*T_(Action))(T *const element);

/** Given to \see{<T>StackMigrateEach} by the migrate function of another
 datatype. */
typedef void (*T_(StackMigrateElement))(T *const element,
	const struct Migrate *const migrate);

#ifdef STACK_TO_STRING /* <-- string */

/** Responsible for turning {<T>} (the first argument) into a 12 {char}
 null-terminated output string (the second.) Used for {STACK_TO_STRING}. */
typedef void (*T_(ToString))(const T *, char (*const)[12]);

/* Check that {STACK_TO_STRING} is a function implementing {<T>ToString}. */
static const T_(ToString) PRIVATE_T_(to_string) = (STACK_TO_STRING);

#endif /* string --> */



/** The stack. To instantiate, see \see{<T>Stack}. */
struct T_(Stack);
struct T_(Stack) {
	T *array;
	size_t capacity[2]; /* Fibonacci, [0] is the capacity, [1] is next */
	size_t size;
	enum StackError error; /* errors defined by enum StackError */
	int errno_copy; /* copy of errno when when error == E_ERRNO */
#ifdef STACK_MIGRATE
	Migrate migrate; /* called to update on resizing */
	void *parent; /* migrate parameter */
#endif
};



/** Debug messages from stack functions; turn on using {STACK_DEBUG}. */
static void PRIVATE_T_(debug)(struct T_(Stack) *const this,
	const char *const fn, const char *const fmt, ...) {
#ifdef STACK_DEBUG
	/* \url{ http://c-faq.com/varargs/vprintf.html } */
	va_list argp;
	fprintf(stderr, "Stack<" T_NAME ">#%p.%s: ", (void *)this, fn);
	va_start(argp, fmt);
	vfprintf(stderr, fmt, argp);
	va_end(argp);
#else
	UNUSED(this); UNUSED(fn); UNUSED(fmt);
#endif
}

/** Ensures capacity.
 @return Success.
 @throws STACK_OVERFLOW, STACK_ERRNO */
static int PRIVATE_T_(reserve)(struct T_(Stack) *const this,
	const size_t min_capacity) {
	size_t c0, c1;
	T *array;
	const size_t max_size = (size_t)(-1) / sizeof *array;
	assert(this);
	assert(this->size <= this->capacity[0]);
	assert(this->capacity[0] <= this->capacity[1]);
	if(this->capacity[0] >= min_capacity) return 1;
	if(max_size < min_capacity) return this->error = STACK_OVERFLOW, 0; 
	c0 = this->capacity[0];
	c1 = this->capacity[1];
	while(c0 < min_capacity) {
		c0 ^= c1, c1 ^= c0, c0 ^= c1, c1 += c0;
		if(c1 <= c0 || c1 > max_size) c1 = max_size;
	}
	if(!(array = realloc(this->array, c0 * sizeof *this->array)))
		return this->error = STACK_ERRNO, this->errno_copy = errno, 0;
	PRIVATE_T_(debug)(this, "reserve", "array#%p[%lu] -> #%p[%lu].\n",
		(void *)this->array, (unsigned long)this->capacity[0], (void *)array,
		(unsigned long)c0);
#ifdef STACK_MIGRATE
	/* Migrate parent class. Violates pedantic strict-ANSI? Subverts
	 type-safety? However, it is so convenient for the caller not to have to
	 worry about moving memory blocks. */
	if(this->array != array && this->migrate) {
		struct Migrate migrate;
		migrate.begin = this->array;
		migrate.end   = (const char *)this->array + this->size * sizeof *array;
		migrate.delta = (const char *)array - (const char *)this->array;
		PRIVATE_T_(debug)(this, "reserve", "calling migrate.\n");
		assert(this->parent);
		this->migrate(this->parent, &migrate);
	}
#endif
	this->array = array;
	this->capacity[0] = c0;
	this->capacity[1] = c1;
	return 1;
}

/** Destructor for {Stack}. Make sure that the stack's contents will not be
 accessed anymore.
 @param thisp: A reference to the object that is to be deleted; it will be stack
 to null. If it is already null or it points to null, doesn't do anything.
 @order \Theta(1)
 @allow */
static void T_(Stack_)(struct T_(Stack) **const thisp) {
	struct T_(Stack) *this;
	if(!thisp || !(this = *thisp)) return;
	PRIVATE_T_(debug)(this, "Delete", "erasing.\n");
	free(this->array);
	free(this);
	*thisp = 0;
}

/** Private constructor called from either \see{<T>Stack}. */
static struct T_(Stack) *PRIVATE_T_(stack)(void) {
	struct T_(Stack) *this;
	if(!(this = malloc(sizeof *this))) {
		stack_global_error = STACK_ERRNO;
		stack_global_errno_copy = errno;
		return 0;
	}
	this->array        = 0;
	this->capacity[0]  = stack_fibonacci6;
	this->capacity[1]  = stack_fibonacci7;
	this->size         = 0;
	this->error        = STACK_NO_ERROR;
	this->errno_copy   = 0;
	if(!(this->array = malloc(this->capacity[0] * sizeof *this->array))) {
		T_(Stack_)(&this);
		stack_global_error = STACK_ERRNO;
		stack_global_errno_copy = errno;
		return 0;
	}
	PRIVATE_T_(debug)(this, "New", "capacity %d.\n", this->capacity[0]);
	return this;
}

#ifdef STACK_MIGRATE /* <-- migrate */
/** Constructs an empty {Stack} with capacity Fibonacci6, which is 8. This
 is the constructor if STACK_MIGRATE is specifed.
 @param migrate, parent: Can be both null.
 @return A new {Stack}.
 @throws STACK_PARAMETER, STACK_ERRNO: Use {StackError(0)} to get the error.
 @order \Theta(1)
 @fixme Untested.
 @allow */
static struct T_(Stack) *T_(Stack)(const Migrate migrate, void *const parent) {
	struct T_(Stack) *this;
	if(!migrate ^ !parent) {
		pool_global_error = POOL_PARAMETER;
		pool_global_errno_copy = 0;
		return 0;
	}
	if(!(this = PRIVATE_T_(stack)())) return 0;
	this->migrate = migrate;
	this->parent = parent;
	return this;
}
#else /* migrate --><-- !migrate */
/** Constructs an empty {Stack} with capacity Fibonacci6, which is 8.
 @return A new {Stack}.
 @throws STACK_ERRNO: Use {StackError(0)} to get the error.
 @order \Theta(1)
 @allow */
static struct T_(Stack) *T_(Stack)(void) {
	return PRIVATE_T_(stack)();
}
#endif /* migrate --> */

/** See what's the error if something goes wrong. Resets the error.
 @return The last error string.
 @order \Theta(1)
 @allow */
static const char *T_(StackGetError)(struct T_(Stack) *const this) {
	const char *str;
	enum StackError *perr;
	int *perrno;
	perr   = this ? &this->error      : &stack_global_error;
	perrno = this ? &this->errno_copy : &stack_global_errno_copy;
	if(!(str = stack_error_explination[*perr])) str = strerror(*perrno);
	*perr = 0;
	return str;
}

/** @return The current size of the stack.
 @order \Theta(1)
 @allow */
static size_t T_(StackGetSize)(const struct T_(Stack) *const this) {
	if(!this) return 0;
	return this->size;
}

/** Gets an existing element by index. Causing something to be added to the
 {Stack} may invalidate this pointer.
 @param this: If {this} is null, returns null.
 @param idx: Index.
 @return If failed, returns a null pointer and the error condition will be set.
 @throws STACK_OUT_OF_BOUNDS
 @order \Theta(1)
 @allow */
static T *T_(StackGetElement)(struct T_(Stack) *const this, const size_t idx) {
	if(!this) return 0;
	if(idx >= this->size) { this->error = STACK_OUT_OF_BOUNDS; return 0; }
	return this->array + idx;
}

/** @return The last value to be added or null if the stack is empty. The
 pointer is valid until the stack gets bigger.
 @order \Theta(1)
 @allow */
static T *T_(StackPeek)(const struct T_(Stack) *const this) {
	if(!this || !this->size) return 0;
	return this->array + this->size - 1;
}

/** Decreases the size of the stack. The pointer is valid until the stack gets
 bigger.
 @return One value from the stack or null if the stack is empty.
 @order \Theta(1)
 @allow */
static T *T_(StackPop)(struct T_(Stack) *const this) {
	if(!this || !this->size) return 0;
	return this->array + --this->size;
}

/** Gets an index given an element. If the element is not part of the {Stack},
 behaviour is undefined.
 @order \Theta(1)
 @allow */
static size_t T_(StackGetIndex)(struct T_(Stack) *const this,
	const T *const element) {
	return element - this->array;
}

/** Increases the capacity of this Stack to ensure that it can hold at least
 the number of elements specified by the {min_capacity}.
 @param this: If {this} is null, returns false.
 @return True if the capacity increase was viable; otherwise the stack is not
 touched and the error condition is stack.
 @throws STACK_ERRNO, STACK_OVERFLOW
 @order \Omega(1), O({capacity})
 @allow */
static int T_(StackReserve)(struct T_(Stack) *const this,
	const size_t min_capacity) {
	if(!this) return 0;
	if(!PRIVATE_T_(reserve)(this, min_capacity)) return 0;
	PRIVATE_T_(debug)(this, "Reserve","stack stack size to %u to contain %u.\n",
		this->capacity[0], min_capacity);
	return 1;
}

/** Gets an uninitialised new element at the end of the stack. May move the
 stack to a new memory location to fit the new size.
 @param this: If {this} is null, returns null.
 @return If failed, returns a null pointer and the error condition will be set.
 @throws STACK_OVERFLOW, STACK_ERRNO
 @order amortised O(1)
 @allow */
static T *T_(StackNew)(struct T_(Stack) *const this) {
	T *elem;
	if(!this) return 0;
	if(!PRIVATE_T_(reserve)(this, this->size + 1)) return 0;
	elem = this->array + this->size++;
	PRIVATE_T_(debug)(this, "New", "added.\n");
	return elem;
}

/** Removes all data from {this}.
 @order \Theta(1)
 @allow */
static void T_(StackClear)(struct T_(Stack) *const this) {
	if(!this) return;
	this->size = 0;
	PRIVATE_T_(debug)(this, "Clear", "cleared.\n");
}

/** Iterates though the {Stack} and calls {action} on all the elements.
 @throws STACK_PARAMETER
 @order O({size}) O({action})
 @fixme Untested.
 @allow */
static void T_(StackForEach)(struct T_(Stack) *const this,
	const T_(Action) action) {
	size_t i = 0;
	if(!this) return;
	if(!action) { this->error = STACK_PARAMETER; return; }
	while(i < this->size) action(this->array + i++);
}

/** Use when the stack has pointers to another stack in the {Migrate} function
 of the other datatype.
 @param handler: Has the responsibility of calling the other data type's
 migrate pointer function on all pointers affected by the {realloc}.
 @param migrate: Should only be called in a {Migrate} function; pass the
 {migrate} parameter.
 @order O({greatest size})
 @fixme Untested.
 @allow */
static void T_(StackMigrateEach)(struct T_(Stack) *const this,
	const T_(StackMigrateElement) handler, const struct Migrate *const migrate){
	size_t i;
	T *e;
	if(!this) return;
	if(!handler || !migrate) { this->error = STACK_PARAMETER; return; }
	for(i = 0; i < this->size; i++) {
		e = this->array + i;
		handler(e, migrate);
	}
}

/** Use this inside the function that is passed to the (generally other's)
 migrate function. Allows pointers to the pool to be updated. It doesn't affect
 pointers not in the {realloc}ed region.
 @order O(1)
 @fixme Untested.
 @allow */
static void T_(MigratePointer)(T **const node_ptr,
	const struct Migrate *const migrate) {
	const void *ptr;
	if(!node_ptr
	   || !(ptr = *node_ptr)
	   || ptr < migrate->begin
	   || ptr >= migrate->end) return;
	*(char **)node_ptr += migrate->delta;
}

#ifdef STACK_TO_STRING /* <-- print */

#ifndef STACK_PRINT_THINGS /* <-- once inside translation unit */
#define STACK_PRINT_THINGS

static const char *const stack_cat_start     = "[ ";
static const char *const stack_cat_end       = " ]";
static const char *const stack_cat_alter_end = "...]";
static const char *const stack_cat_sep       = ", ";
static const char *const stack_cat_star      = "*";
static const char *const stack_cat_null      = "null";

struct Stack_SuperCat {
	char *print, *cursor;
	size_t left;
	int is_truncated;
};
static void stack_super_cat_init(struct Stack_SuperCat *const cat,
	char *const print, const size_t print_size) {
	cat->print = cat->cursor = print;
	cat->left = print_size;
	cat->is_truncated = 0;
	print[0] = '\0';
}
static void stack_super_cat(struct Stack_SuperCat *const cat,
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

/** Can print 4 things at once before it overwrites. One must stack
 {STACK_TO_STRING} to a function implementing {<T>ToString} to get this
 functionality.
 @return Prints {this} in a static buffer.
 @order \Theta(1); it has a 255 character limit; every element takes some of it.
 @allow */
static const char *T_(StackToString)(const struct T_(Stack) *const this) {
	static char buffer[4][256];
	static unsigned buffer_i;
	struct Stack_SuperCat cat;
	int is_first = 1;
	char scratch[12];
	size_t i;
	assert(strlen(stack_cat_alter_end) >= strlen(stack_cat_end));
	assert(sizeof buffer > strlen(stack_cat_alter_end));
	stack_super_cat_init(&cat, buffer[buffer_i],
		sizeof *buffer / sizeof **buffer - strlen(stack_cat_alter_end));
	buffer_i++, buffer_i &= 3;
	if(!this) {
		stack_super_cat(&cat, stack_cat_null);
		return cat.print;
	}
	stack_super_cat(&cat, stack_cat_start);
	for(i = 0; i < this->size; i++) {
		if(!is_first) stack_super_cat(&cat, stack_cat_sep); else is_first = 0;
		PRIVATE_T_(to_string)(this->array + i, &scratch),
		scratch[sizeof scratch - 1] = '\0';
		stack_super_cat(&cat, scratch);
		if(cat.is_truncated) break;
	}
	sprintf(cat.cursor, "%s",
		cat.is_truncated ? stack_cat_alter_end : stack_cat_end);
	return cat.print; /* static buffer */
}

#endif /* print --> */

#ifdef STACK_TEST /* <-- test */
#include "../test/TestStack.h" /* need this file if one is going to run tests */
#endif /* test --> */

/* prototype */
static void PRIVATE_T_(unused_coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 \url{ http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code } */
static void PRIVATE_T_(unused_set)(void) {
	T_(Stack_)(0);
#ifdef STACK_MIGRATE
	T_(Stack)(0, 0);
#else
	T_(Stack)();
#endif	
	T_(StackGetError)(0);
	T_(StackGetSize)(0);
	T_(StackGetElement)(0, 0);
	T_(StackPeek)(0);
	T_(StackPop)(0);
	T_(StackGetIndex)(0, 0);
	T_(StackReserve)(0, 0);
	T_(StackNew)(0);
	T_(StackClear)(0);
	T_(StackForEach)(0, 0);
	T_(StackMigrateEach)(0, 0, 0);
	T_(MigratePointer)(0, 0);
#ifdef STACK_TO_STRING
	T_(StackToString)(0);
#endif
	PRIVATE_T_(unused_coda)();
}
/** {clang}'s pre-processor is not fooled if you have one function. */
static void PRIVATE_T_(unused_coda)(void) { PRIVATE_T_(unused_set)(); }



/* un-define all macros */
#undef STACK_NAME
#undef STACK_TYPE
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
#ifdef STACK_TO_STRING
#undef STACK_TO_STRING
#endif
#ifdef STACK_DEBUG
#undef STACK_DEBUG
#endif
#ifdef STACK_NDEBUG
#undef STACK_NDEBUG
#undef NDEBUG
#endif
#ifdef STACK_TEST
#undef STACK_TEST
#endif
