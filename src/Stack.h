/** 2016 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 {<T>Stack} is a dynamic array that stores unordered {<T>} in a stack, which
 must be set using {STACK_TYPE}. This is the most basic variable array; the
 array is packed, with no other extraneous information. Use when one needs a
 non-polymorphic simply accessable dynamic memory store. Indices will remain
 the same throughout the lifetime of the data, but expanding the data may
 change the pointers. You cannot shrink the capacity of this data type, only
 cause it to grow. Resizing incurs amortised cost, done though a Fibonacci
 sequence.

 {<T>Stack} is not synchronised. The preprocessor macros are all undefined at
 the end of the file for convenience.

 @param STACK_NAME
 This literally becomes {<T>}. As it's used in function names, this should
 comply with naming rules and be unique; required.

 @param STACK_TYPE
 The type associated with {<T>}. Has to be a valid type, accessible to the
 compiler at the time of inclusion; required.

 @param STACK_MIGRATE_EACH
 Optional function implementing {<PT>Migrate}. On memory move, this definition
 will call {STACK_MIGRATE_EACH} with all {<T>} in {<T>Pool}. Use when your data
 is self-referential, like a linked-list.

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
 @std		C89
 @author	Neil
 @version	2018-03 Why have an extra level of indirection? Not like Pool.
 @since		2018-02 Made it like POOL.
			2017-12 Changed STACK_PARENT for type-safety.
			2017-11 Added STACK_PARENT.
			2017-11 Forked from Pool.
 @fixme		Have initial setting. Check to make sure all the objects accept
 array = 0; make array=0 the default state for simplicity. */



#include <stddef.h>	/* ptrdiff_t */
#include <stdlib.h>	/* realloc free */
#include <assert.h>	/* assert */
#include <string.h>	/* memcpy (memmove strerror strcpy memcmp in StackTest.h) */
#ifdef STACK_TO_STRING /* <-- print */
#include <stdio.h>	/* snprintf */
#endif /* print --> */
#include <errno.h>	/* errno */
#ifdef STACK_DEBUG
#include <stdarg.h>	/* for debug print */
#endif /* calls --> */



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
 STACK_NAMEX, PT_(X) with STACK_U_NAME_X, and T_NAME with the string
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
#define T_(thing) CAT(STACK_NAME, thing)
#define PT_(thing) PCAT(stack, PCAT(STACK_NAME, thing))
#define T_NAME QUOTE(STACK_NAME)

/* Troubles with this line? check to ensure that {STACK_TYPE} is a valid type,
 whose definition is placed above {#include "Stack.h"}. */
typedef STACK_TYPE PT_(Type);
#define T PT_(Type)



/* Constants across multiple includes in the same translation unit. */
#ifndef STACK_H /* <-- STACK_H */
#define STACK_H
static const size_t stack_fibonacci6 = 8;
static const size_t stack_fibonacci7 = 13;
#endif /* STACK_H --> */

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



/** This is the migrate function for {<T>}. This definition is about the
 {STACK_TYPE} type, that is, it is without the prefix {Stack}; to avoid
 namespace collisions, this is private, meaning the name is mangled. If you
 want this definition, re-declare it. */
typedef void (*PT_(Migrate))(T *const data,
	const struct Migrate *const migrate);
#ifdef STACK_MIGRATE_EACH /* <-- each */
/* Check that {STACK_MIGRATE_EACH} is a function implementing {<PT>Migrate},
 whose definition is placed above {#include "Stack.h"}. */
static const PT_(Migrate) PT_(migrate_each) = (STACK_MIGRATE_EACH);
#endif /* each --> */

/** Operates by side-effects only. This definition is about the {STACK_NAME}
 type, that is, it is without the prefix {Stack}; to avoid namespace
 collisions, this is private, meaning the name is mangled. If you want this
 definition, re-declare it as {<T>Action}. */
typedef void (*PT_(Action))(T *const element);

/** Operates by side-effects only. */
typedef void (*PT_(BiAction))(T *const element, void *const);

#ifdef STACK_TO_STRING /* <-- string */
/** Responsible for turning {<T>} (the first argument) into a 12 {char}
 null-terminated output string (the second.) Used for {STACK_TO_STRING}. */
typedef void (*PT_(ToString))(const T *, char (*const)[12]);
/* Check that {STACK_TO_STRING} is a function implementing {<T>ToString}. */
static const PT_(ToString) PT_(to_string) = (STACK_TO_STRING);
#endif /* string --> */



/** The stack. To instantiate, see \see{<T>Stack}. */
struct T_(Stack);
struct T_(Stack) {
	T *array;
	/* {array} -> {capacity} -> {c[0] < c[1] || c[0] == c[1] == max_size}.
	 Fibonacci, [0] is the capacity, [1] is next. */
	size_t capacity[2];
	/* {nodes} ? {size <= capacity[0]} : {size == 0}. */
	size_t size;
};



/** Debug messages from stack functions; turn on using {STACK_DEBUG}. */
static void PT_(debug)(struct T_(Stack) *const stack,
	const char *const fn, const char *const fmt, ...) {
#ifdef STACK_DEBUG
	/* \url{ http://c-faq.com/varargs/vprintf.html } */
	va_list argp;
	fprintf(stderr, "Stack<" T_NAME ">#%p.%s: ", (void *)stack, fn);
	va_start(argp, fmt);
	vfprintf(stderr, fmt, argp);
	va_end(argp);
#else
	(void)(stack), (void)(fn), (void)(fmt);
#endif
}

/** Ensures capacity.
 @return Success.
 @throws ERANGE: Tried allocating more then can fit in {size_t}.
 @throws {realloc} errors: {IEEE Std 1003.1-2001}. */
static int PT_(reserve)(struct T_(Stack) *const stack,
	const size_t min_capacity, T **const update_ptr) {
	size_t c0, c1;
	T *array;
	const size_t max_size = (size_t)(-1) / sizeof *array;
	assert(stack && stack->size <= stack->capacity[0]
		&& stack->capacity[0] <= stack->capacity[1]);
	if(stack->capacity[0] >= min_capacity) return 1;
	if(max_size < min_capacity) return errno = ERANGE, 0;
	if(!stack->array) {
		c0 = stack_fibonacci6;
		c1 = stack_fibonacci7;
	} else {
		c0 = stack->capacity[0];
		c1 = stack->capacity[1];
	}
	while(c0 < min_capacity) {
		c0 ^= c1, c1 ^= c0, c0 ^= c1, c1 += c0;
		if(c1 > max_size || c1 <= c0) c1 = max_size;
	}
	if(!(array = realloc(stack->array, c0 * sizeof *stack->array))) return 0;
	PT_(debug)(stack, "reserve", "array#%p[%lu] -> #%p[%lu].\n",
		(void *)stack->array, (unsigned long)stack->capacity[0], (void *)array,
		(unsigned long)c0);
	if(stack->array != array) {
		/* Migrate parent class. Violates pedantic strict-ANSI? */
		struct Migrate migrate;
		migrate.begin = stack->array;
		migrate.end   = (const char *)stack->array + stack->size*sizeof *array;
		migrate.delta = (const char *)array - (const char *)stack->array;
#ifdef STACK_MIGRATE_EACH /* <-- each */
		{
			T *a, *end;
			for(a = stack->array, end = a + stack->size; a < end; a++)
				PT_(migrate_each)(a, &migrate);
		}
#endif /* each --> */
		if(update_ptr) {
			const void *const u = *update_ptr;
			if(u >= migrate.begin && u < migrate.end)
				*(char **const)update_ptr += migrate.delta;
		}
	}
	stack->array = array;
	stack->capacity[0] = c0;
	stack->capacity[1] = c1;
	return 1;
}

/** Initialises {stack} to be empty and take no memory. */
static void PT_(init)(struct T_(Stack) *const stack) {
	assert(stack);
	stack->array        = 0;
	stack->capacity[0]  = 0;
	stack->capacity[1]  = 0;
	stack->size         = 0;
}

/** Destructor for {Stack}.
 @param stack: If null or empty, does nothing.
 @order \Theta(1)
 @allow */
static void T_(Stack_)(struct T_(Stack) *const stack) {
	if(!stack) return;
	PT_(debug)(stack, "Delete", "erasing.\n");
	free(stack->array);
	PT_(init)(stack);
}

/** Initialises {stack} to an empty {Stack}.
 @param stack: If null, does nothing.
 @order \Theta(1)
 @allow */
static void T_(Stack)(struct T_(Stack) *const stack) {
	if(!stack) return;
	PT_(init)(stack);
	PT_(debug)(stack, "New", "capacity %d.\n", stack->capacity[0]);
}

/** @param stack: If null, returns zero.
 @return The current size of the stack.
 @order \Theta(1)
 @allow */
static size_t T_(StackSize)(const struct T_(Stack) *const stack) {
	if(!stack) return 0;
	return stack->size;
}

/** Gets an existing element by index. Causing something to be added to the
 {Stack} may invalidate this pointer.
 @param stack: If {stack} is null, returns null.
 @param idx: Index.
 @return The element, otherwise {errno} will be set if {stack} is non-null.
 @throws EDOM: {idx} out of bounds.
 @order \Theta(1)
 @allow */
static T *T_(StackElement)(struct T_(Stack) *const stack, const size_t idx) {
	if(!stack) return 0;
	if(idx >= stack->size) { errno = EDOM; return 0; }
	return stack->array + idx;
}

/** Gets an index given an {element}.
 @param stack: If it is not a {Stack} or null, behaviour is undefined.
 @param element: If the element is not part of the {Stack}, behaviour is
 undefined.
 @return An index.
 @order \Theta(1)
 @allow */
static size_t T_(StackIndex)(struct T_(Stack) *const stack,
	const T *const element) {
	return element - stack->array;
}

/** @param stack: If {stack} is null, returns null.
 @return The last value to be added or null if the stack is empty. The pointer
 is valid until the stack gets bigger.
 @order \Theta(1)
 @allow */
static T *T_(StackPeek)(const struct T_(Stack) *const stack) {
	if(!stack || !stack->size) return 0;
	return stack->array + stack->size - 1;
}

/** Decreases the size of the stack.
 @return Value from the the top of the stack that is removed or null if the
 stack is empty. The pointer is valid until the stack gets bigger, and you may
 need to duplicate it for permanent storage.
 @order \Theta(1)
 @allow */
static T *T_(StackPop)(struct T_(Stack) *const stack) {
	if(!stack || !stack->size) return 0;
	return stack->array + --stack->size;
}

/** Provides a way to iterate through the stack.
 @param stack: If null, returns null.
 @param prev: Set it to null to start the iteration.
 @return A pointer to the next element or null if there are no more. If you add
 to the stack, the pointer becomes invalid.
 @order \Theta(1)
 @allow */
static T *T_(StackNext)(struct T_(Stack) *const stack, T *const prev) {
	if(!stack || !stack->size) return 0;
	if(!prev) return stack->array;
	if((size_t)(prev - stack->array) + 1 >= stack->size) return 0;
	return prev + 1;
}

/** Increases the capacity of this Stack to ensure that it can hold at least
 the number of elements specified by the {min_capacity}.
 @param stack: If {stack} is null, returns false.
 @return True if the capacity increase was viable; otherwise the stack is not
 touched and {errno} may be set.
 @throws ERANGE: Tried allocating more then can fit in {size_t} objects.
 @throws {realloc} errors: {IEEE Std 1003.1-2001}.
 @order \Omega(1), O({capacity})
 @allow */
static int T_(StackReserve)(struct T_(Stack) *const stack,
	const size_t min_capacity) {
	if(!stack) return 0;
	if(!PT_(reserve)(stack, min_capacity, 0)) return 0; /* ERANGE, ENOMEM? */
	PT_(debug)(stack, "Reserve", "stack stack size to %u to contain %u.\n",
		stack->capacity[0], min_capacity);
	return 1;
}

/** Gets an uninitialised new element at the end of the {Stack}. May move the
 {Stack} to a new memory location to fit the new size.
 @param stack: If {stack} is null, returns null.
 @return A new, un-initialised, element, or null and {errno} may be set.
 @throws ERANGE: Tried allocating more then can fit in {size_t} objects.
 @throws {realloc} errors: {IEEE Std 1003.1-2001}.
 @order amortised O(1)
 @allow */
static T *T_(StackNew)(struct T_(Stack) *const stack) {
	T *elem;
	if(!stack) return 0;
	if(sizeof(T) == 1 && stack->size == (size_t)-1) { errno = ERANGE; return 0;}
	if(!PT_(reserve)(stack, stack->size + 1, 0)) return 0; /* ERANGE, ENOMEM? */
	elem = stack->array + stack->size++;
	PT_(debug)(stack, "New", "added.\n");
	return elem;
}

/** Gets an uninitialised new element and updates the {update_ptr} if it is
 within the memory region that was changed.
 @param stack: If {stack} is null, returns null.
 @param update_ptr: Pointer to update on memory move.
 @return A new, un-initialised, element, or null and {errno} may be set.
 @throws ERANGE: Tried allocating more then can fit in {size_t}.
 @throws {realloc} errors: {IEEE Std 1003.1-2001}.
 @order amortised O(1)
 @fixme Untested.
 @allow */
static T *T_(StackUpdateNew)(struct T_(Stack) *const stack,
	T **const update_ptr) {
	if(!stack) return 0;
	if(!PT_(reserve)(stack, stack->size + 1, update_ptr))
		return 0; /* ERANGE, ENOMEM? */
	PT_(debug)(stack, "New", "added.\n");
	return stack->array + stack->size++;
}

/** Removes all data from {stack}. Leaves the stack memory alone; if one wants
 to remove memory, see \see{Stack_}.
 @param stack: If null, does nothing.
 @order \Theta(1)
 @allow */
static void T_(StackClear)(struct T_(Stack) *const stack) {
	if(!stack) return;
	stack->size = 0;
	PT_(debug)(stack, "Clear", "cleared.\n");
}

/** Iterates though the {Stack} from the bottom and calls {action} on all the
 elements.
 @param stack, action: If null, does nothing.
 @order O({size} \times {action})
 @fixme Untested.
 @fixme Sequence interface.
 @allow */
static void T_(StackForEach)(struct T_(Stack) *const stack,
	const PT_(Action) action) {
	T *a, *end;
	if(!stack || !action) return;
	for(a = stack->array, end = a + stack->size; a < end; a++) action(a);
}

/** Iterates though the {Stack} from the bottom and calls {biaction} on all the
 elements with {param} as the second element.
 @param stack, action: If null, does nothing.
 @order O({size} \times {action})
 @fixme Untested.
 @fixme BiSequence interface.
 @allow */
static void T_(StackBiForEach)(struct T_(Stack) *const stack,
	const PT_(BiAction) biaction, void *const param) {
	T *a, *head;
	if(!stack || !biaction) return;
	for(head = stack->array, a = head + stack->size - 1; a >= head; a--)
		biaction(a, param);
}

/** Use when the stack has pointers to another memory move structure in the
 {MigrateAll} function of the other data type.
 @param stack: If null, does nothing.
 @param handler: If null, does nothing, otherwise has the responsibility of
 calling the other data type's migrate pointer function on all pointers
 affected by the {realloc}.
 @param migrate: If null, does nothing. Should only be called in a {Migrate}
 function; pass the {migrate} parameter.
 @order O({size})
 @fixme Untested.
 @fixme Migrate interface.
 @allow */
static void T_(StackMigrateEach)(struct T_(Stack) *const stack,
	const PT_(Migrate) handler, const struct Migrate *const migrate) {
	T *a, *end;
	if(!stack || !migrate || !handler) return;
	for(a = stack->array, end = a + stack->size; a < end; a++)
		handler(a, migrate);
}

/** Passed a {migrate} parameter, allows pointers to the stack to be updated.
 It doesn't affect pointers not in the {realloc}ed region.
 @order \Omega(1)
 @fixme Untested.
 @fixme Migrate interface.
 @allow */
static void T_(StackMigratePointer)(T **const data_ptr,
	const struct Migrate *const migrate) {
	const void *ptr;
	if(!data_ptr
	   || !(ptr = *data_ptr)
	   || ptr < migrate->begin
	   || ptr >= migrate->end) return;
	*(char **)data_ptr += migrate->delta;
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
 @return Prints {stack} in a static buffer.
 @order \Theta(1); it has a 255 character limit; every element takes some of it.
 @fixme ToString interface?
 @allow */
static const char *T_(StackToString)(const struct T_(Stack) *const stack) {
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
	if(!stack) {
		stack_super_cat(&cat, stack_cat_null);
		return cat.print;
	}
	stack_super_cat(&cat, stack_cat_start);
	for(i = 0; i < stack->size; i++) {
		if(!is_first) stack_super_cat(&cat, stack_cat_sep); else is_first = 0;
		PT_(to_string)(stack->array + i, &scratch),
		scratch[sizeof scratch - 1] = '\0';
		stack_super_cat(&cat, scratch);
		if(cat.is_truncated) break;
	}
	sprintf(cat.cursor, "%s",
		cat.is_truncated ? stack_cat_alter_end : stack_cat_end);
	return cat.print; /* Static buffer. */
}

#endif /* print --> */

#ifdef STACK_TEST /* <-- test */
#include "../test/TestStack.h" /* need this file if one is going to run tests */
#endif /* test --> */

/* Prototype. */
static void PT_(unused_coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 \url{ http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code } */
static void PT_(unused_set)(void) {
	T_(Stack_)(0);
	T_(Stack)(0);
	T_(StackSize)(0);
	T_(StackElement)(0, 0);
	T_(StackIndex)(0, 0);
	T_(StackPeek)(0);
	T_(StackPop)(0);
	T_(StackNext)(0, 0);
	T_(StackReserve)(0, 0);
	T_(StackNew)(0);
	T_(StackUpdateNew)(0, 0);
	T_(StackClear)(0);
	T_(StackForEach)(0, 0);
	T_(StackBiForEach)(0, 0, 0);
	T_(StackMigrateEach)(0, 0, 0);
	T_(StackMigratePointer)(0, 0);
#ifdef STACK_TO_STRING
	T_(StackToString)(0);
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
#undef STACK_NAME
#undef STACK_TYPE
#ifdef STACK_MIGRATE_EACH
#undef STACK_MIGRATE_EACH
#endif
#ifdef STACK_TO_STRING
#undef STACK_TO_STRING
#endif
#ifdef STACK_DEBUG
#undef STACK_DEBUG
#endif
#ifdef STACK_TEST
#undef STACK_TEST
#endif
#ifdef STACK_NDEBUG
#undef STACK_NDEBUG
#undef NDEBUG
#endif
