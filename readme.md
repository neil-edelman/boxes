# Array.h #

2016 Neil Edelman, distributed under the terms of the MIT License; see
 [ https://opensource.org/licenses/MIT ].

![States](web/states.png)

 _<T>Array_ is a dynamic contiguous array that stores _<T>_, which must be set
 using _ARRAY_TYPE_. The capacity is greater then or equal to the size, and
 resizing incurs amortised cost. When adding new elements, the elements may
 change memory location to fit, it is therefore unstable; any pointers to this
 memory become stale and unusable.

 _<T>Array_ is not synchronised. Errors are returned with _errno_. The
 parameters are preprocessor macros, and are all undefined at the end of the
 file for convenience. _assert.h_ is included in this file; to stop the
 debug assertions, use _#define NDEBUG_ before inclusion.

parameter: ARRAY_NAME, ARRAY_TYPE -- The name that literally becomes _<T>_, and a valid type associated therewith,
 accessible to the compiler at the time of inclusion; should be conformant to
 naming and to the maximum available length of identifiers. Must each be
 present before including.

parameter: ARRAY_STACK -- Doesn't define removal functions except `<T>ArrayPop`, making it a stack.

parameter: ARRAY_TO_STRING -- Optional print function implementing _<T>ToString_; makes available
 `<T>ArrayToString`.

parameter: ARRAY_TEST -- Unit testing framework using _<T>ArrayTest_, included in a separate header,
 _../test/ArrayTest.h_. Must be defined equal to a (random) filler function,
 satisfying _<T>Action_. Requires _ARRAY_TO_STRING_ and not _NDEBUG_.

minimum standard: C89

author: Neil

version: 2019-05 Added `<T>ArrayReplace` and `<T>ArrayBuffer`.

since: 2018-03 Renamed _Pool_ to _Array_. Took out migrate.



## Declarations ##

### typedef void (*<PT>ToString)(const T *, char (*const)[12]) ###

typedef void (*<PT>ToString)(const T *, char (*const)[12])

Responsible for turning _<T>_ (the first argument) into a 12 _char_
 null-terminated output string (the second.) Used for _ARRAY_TO_STRING_.


### struct <T>Array ###

struct <T>Array

The array. Zeroed data is a valid state. To instantiate explicitly, see
 `<T>Array` or initialise it with `ARRAY_INIT` or `_0_` (C99.)




## Function Summary ##

| _Return Type_	| _Function Name_	| _Argument List_ |
| - | - | - |
| static void	| <T>Array_	| (struct <T>Array *const a) |
static void	<T>Array	(struct <T>Array *const a)
static size_t	<T>ArraySize	(const struct <T>Array *const a)
static int	<T>ArrayRemove	(struct <T>Array *const a, T *const data)
static int	<T>ArrayTailRemove	(struct <T>Array *const a, T *const data)
static void	<T>ArrayClear	(struct <T>Array *const a)
static T *	<T>ArrayGet	(const struct <T>Array *const a)
static T *	<T>ArrayEnd	(const struct <T>Array *const a)
static size_t	<T>ArrayIndex	(const struct <T>Array *const a,
	const T *const data)
static T *	<T>ArrayPeek	(const struct <T>Array *const a)
static T *	<T>ArrayPop	(struct <T>Array *const a)
static T *	<T>ArrayBack	(const struct <T>Array *const a, const T *const here)
static T *	<T>ArrayNext	(const struct <T>Array *const a, const T *const here)
static T *	<T>ArrayNew	(struct <T>Array *const a)
static T *	<T>ArrayUpdateNew	(struct <T>Array *const a,
	T **const update_ptr)
static T *	<T>ArrayBuffer	(struct <T>Array *const a, const size_t buffer)
static int	<T>ArrayAddSize	(struct <T>Array *const a, const size_t add)
static void	<T>ArrayForEach	(struct <T>Array *const a,
	const <PT>Action action)
static void	<T>ArrayIfEach	(struct <T>Array *const a,
	const <PT>Predicate predicate, const <PT>Action action)
static void	<T>ArrayKeepIf	(struct <T>Array *const a,
	const <PT>Predicate keep)
static void	<T>ArrayTrim	(struct <T>Array *const a,
	const <PT>Predicate predicate)
static int	<T>ArrayReplace	(struct <T>Array *const a, const T *anchor,
	const long range, const struct <T>Array *const b)
static int	<T>ArrayIndexReplace	(struct <T>Array *const a, const size_t i0,
	const size_t i1, const struct <T>Array *const b)
static const char *	<T>ArrayToString	(const struct <T>Array *const a)



## Function Detail ##

### <T>Array_ ###

static void <T>Array_(struct <T>Array *const a)

Destructor for _a_; returns an initialised _a_ to the empty state where it
 takes no memory.

parameter: a -- If null, does nothing.

order: \Theta(1)



### <T>Array ###

static void <T>Array(struct <T>Array *const a)

Initialises _a_ to be empty.

order: \Theta(1)



### <T>ArraySize ###

static size_t <T>ArraySize(const struct <T>Array *const a)



return: The size.

order: O(1)



### <T>ArrayRemove ###

static int <T>ArrayRemove(struct <T>Array *const a, T *const data)

Removes _data_ from _a_.

parameter: a, data -- If null, returns false.

parameter: data -- Will be removed; data will remain the same but be updated to the
 next element, or if this was the last element, the pointer will be past the
 end.

return: Success, otherwise _errno_ will be set for valid input.

throws: EDOM -- _data_ is not part of _a_.

order: O(n).



### <T>ArrayTailRemove ###

static int <T>ArrayTailRemove(struct <T>Array *const a, T *const data)

Removes _data_ from _a_ and replaces the spot it was in with the tail.

parameter: a, data -- If null, returns false.

parameter: data -- Will be removed; data will remain the same but be updated to the
 last element, or if this was the last element, the pointer will be past the
 end.

return: Success, otherwise _errno_ will be set for valid input.

throws: EDOM -- _data_ is not part of _a_.

order: O(1).



### <T>ArrayClear ###

static void <T>ArrayClear(struct <T>Array *const a)

Sets the size of _a_ to zero, effectively removing all the elements, but
 leaves the capacity alone, (the only thing that will free memory allocation
 is `<T>Array_`.)

parameter: a -- If null, does nothing.

order: \Theta(1)



### <T>ArrayGet ###

static T * <T>ArrayGet(const struct <T>Array *const a)

Causing something to be added to the _<T>Array_ may invalidate this
 pointer, see `<T>ArrayUpdateNew`.

parameter: a -- If null, returns null.

return: A pointer to the _a_'s data, indexable up to the _a_'s size.

order: \Theta(1)



### <T>ArrayEnd ###

static T * <T>ArrayEnd(const struct <T>Array *const a)

Causing something to be added to the _<T>Array_ may invalidate this
 pointer, see `<T>ArrayUpdateNew`.

parameter: a -- If null, returns null.

return: One past the end of the array; take care when dereferencing, as it is
 not part of the array.

order: \Theta(1)



### <T>ArrayIndex ###

static size_t <T>ArrayIndex(const struct <T>Array *const a,
	const T *const data)

Gets an index given _data_.

parameter: a -- Must be a valid object.

parameter: data -- If the element is not part of the _a_, behaviour is undefined.

return: An index.

order: \Theta(1)

fixme: Untested.



### <T>ArrayPeek ###

static T * <T>ArrayPeek(const struct <T>Array *const a)



parameter: a -- If null, returns null.

return: The last element or null if the a is empty. Causing something to be
 added to the _a_ may invalidate this pointer.

order: \Theta(1)

fixme: Untested.



### <T>ArrayPop ###

static T * <T>ArrayPop(struct <T>Array *const a)

The same value as `<T>ArrayPeek`.

parameter: a -- If null, returns null.

return: Value from the the top of the _a_ that is removed or null if the
 stack is empty. Causing something to be added to the _a_ may invalidate
 this pointer. See `<T>ArrayUpdateNew`.

order: \Theta(1)



### <T>ArrayBack ###

static T * <T>ArrayBack(const struct <T>Array *const a, const T *const here)

Iterate through _a_ backwards.

parameter: a -- The array; if null, returns null.

parameter: here -- Set it to null to get the last element, if it exists.

return: A pointer to the previous element or null if it does not exist.

order: \Theta(1)



### <T>ArrayNext ###

static T * <T>ArrayNext(const struct <T>Array *const a, const T *const here)

Iterate through _a_. It is safe to add using `<T>ArrayUpdateNew` with
 the return value as _update_. Removing an element causes the pointer to go to
 the next element, if it exists.

parameter: a -- The array; if null, returns null.

parameter: here -- Set it to null to get the first element, if it exists.

return: A pointer to the next element or null if there are no more.

order: \Theta(1)



### <T>ArrayNew ###

static T * <T>ArrayNew(struct <T>Array *const a)

Gets an uninitialised new element. May move the _a_ to a new memory
 location to fit the new size.

parameter: a -- If is null, returns null.

return: A new, un-initialised, element, or null and _errno_ may be set.

throws: ERANGE -- Tried allocating more then can fit in _size_t_ objects.

throws: _realloc_ errors -- _IEEE Std 1003.1-2001_.

order: Amortised O(1).



### <T>ArrayUpdateNew ###

static T * <T>ArrayUpdateNew(struct <T>Array *const a,
	T **const update_ptr)

Gets an uninitialised new element and updates the _update_ptr_ if it is
 within the memory region that was changed to accomodate new space. For
 example, when iterating a pointer and new element is needed that could change
 the pointer.

parameter: a -- If null, returns null.

parameter: update_ptr -- Pointer to update on memory move.

return: A new, un-initialised, element, or null and _errno_ may be set.

throws: ERANGE -- Tried allocating more then can fit in _size_t_.

throws: _realloc_ errors -- _IEEE Std 1003.1-2001_.

order: amortised O(1)

fixme: Untested.



### <T>ArrayBuffer ###

static T * <T>ArrayBuffer(struct <T>Array *const a, const size_t buffer)

Ensures that _a_ array is _buffer_ capacity beyond the elements in the
 array.

parameter: a -- If is null, returns null.

parameter: buffer -- If this is zero, returns null.

return: One past the end of the array, or null and _errno_ may be set.

throws: ERANGE

throws: realloc

order: Amortised O(_buffer_).

fixme: Test.



### <T>ArrayAddSize ###

static int <T>ArrayAddSize(struct <T>Array *const a, const size_t add)

Adds _add_ to the size in _a_.

return: Success.

throws: ERANGE -- The size added is greater than the capacity. To avoid this,
 call `<T>ArrayBuffer` before.

order: O(1)

fixme: Test.



### <T>ArrayForEach ###

static void <T>ArrayForEach(struct <T>Array *const a,
	const <PT>Action action)

Iterates though _a_ and calls _action_ on all the elements. The topology of
 the list can not change while in this function. That is, don't call
 `<T>ArrayNew`, `<T>ArrayRemove`, _etc_ in _action_.

parameter: a, action -- If null, does nothing.

order: O(_size_ \times _action_)

fixme: Untested.

fixme: Sequence interface.



### <T>ArrayIfEach ###

static void <T>ArrayIfEach(struct <T>Array *const a,
	const <PT>Predicate predicate, const <PT>Action action)

Iterates though _a_ and calls _action_ on all the elements for which
 _predicate_ returns true. The topology of the list can not change while in
 this function.

parameter: a, predicate, action -- If null, does nothing.

order: O(_size_ \times _action_)

fixme: Untested.

fixme: Sequence interface.



### <T>ArrayKeepIf ###

static void <T>ArrayKeepIf(struct <T>Array *const a,
	const <PT>Predicate keep)

For all elements of _a_, calls _keep_, and for each element, if the return
 value is false, lazy deletes that item.

parameter: a, keep -- If null, does nothing.

order: O(_size_)



### <T>ArrayTrim ###

static void <T>ArrayTrim(struct <T>Array *const a,
	const <PT>Predicate predicate)

Removes at either end of _a_ of things that _predicate_ returns true.

parameter: a, predicate -- If null, does nothing.

order: O(_size_)



### <T>ArrayReplace ###

static int <T>ArrayReplace(struct <T>Array *const a, const T *anchor,
	const long range, const struct <T>Array *const b)

In _a_, replaces the elements from _anchor_ up to _range_ with a copy of
 _b_.

parameter: a -- If null, returns zero.

parameter: anchor -- Beginning of the replaced value, inclusive. If null, appends to
 the end.

parameter: range -- How many replaced values; negative values are implicitly plus
 the length of the array; clamped at the minimum and maximum.

parameter: b -- The replacement array. If null, deletes without replacing.

return: Success.

throws: EDOM -- _a_ and _b_ are not null and the same.

throws: ERANGE -- _anchor_ is not null and not in _a_.

throws: ERANGE -- _range_ is greater then 65535 or smaller then -65534.

throws: ERANGE -- _b_ would cause the array to overflow.

throws: _realloc_.

order: \Theta(_b.size_) if the elements have the same size, otherwise,
 amortised O(_a.size_ + _b.size_).



### <T>ArrayIndexReplace ###

static int <T>ArrayIndexReplace(struct <T>Array *const a, const size_t i0,
	const size_t i1, const struct <T>Array *const b)

In _a_, replaces the elements from indices _i0_ (inclusive) to _i1_
 (exclusive) with a copy of _b_.

parameter: a -- If null, returns zero.

parameter: i0, i1 -- The replacement indices, _[i0, i1)_, such that
 _0 <= i0 <= i1 <= a.size_.

parameter: b -- The replacement array. If null, deletes without replacing.

return: Success.

throws: EDOM -- _a_ and _b_ are not null and the same.

throws: EDOM -- _i0_ or _i1_ are out-of-bounds or _i0 > i1_.

throws: ERANGE -- _b_ would cause the array to overflow.

throws: _realloc_.

order: \Theta(_b.size_) if the elements have the same size, otherwise,
 amortised O(_a.size_ + _b.size_).



### <T>ArrayToString ###

static const char * <T>ArrayToString(const struct <T>Array *const a)

Can print 4 things at once before it overwrites. One must a
 _ARRAY_TO_STRING_ to a function implementing _<T>ToString_ to get this
 functionality.

return: Prints _a_ in a static buffer.

order: \Theta(1); it has a 255 character limit; every element takes some of it.

fixme: ToString interface.




