# Array.h #

2016 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or [ https://opensource.org/licenses/MIT ].

 _<T>Array_ is a dynamic array that stores unordered _<T>_, which must be set
 using _ARRAY_TYPE_. The capacity is greater then or equal to the size;
 resizing incurs amortised cost. You cannot shrink the capacity, only cause it
 to grow.

 _<T>Array_ is contiguous, and therefore unstable; that is, when adding new
 elements, it may change the memory location. Pointers to this memory become
 stale and unusable.

 _<T>Array_ is not synchronised. The parameters are preprocessor macros, and
 are all undefined at the end of the file for convenience.

parameter: ARRAY_NAME, ARRAY_TYPE -- The name that literally becomes _<T>_, and a valid type associated therewith,
 accessible to the compiler at the time of inclusion; should be conformant to
 naming and to the maximum available length of identifiers. Must each be
 present before including.

parameter: ARRAY_STACK -- Doesn't define `<T>ArrayRemove`, making it a stack. Not compatible with
 _ARRAY_TAIL_DELETE_.

parameter: ARRAY_TAIL_DELETE -- Instead of preserving order on removal, _O(n)_, this copies the tail element
 to the removed. One gives up order, but preserves contiguity in _O(1)_. Not
 compatible with _ARRAY_STACK_.

parameter: ARRAY_TO_STRING -- Optional print function implementing _<T>ToString_; makes available
 `<T>ArrayToString`.

parameter: ARRAY_TEST -- Unit testing framework using _<T>ArrayTest_, included in a separate header,
 _../test/ArrayTest.h_. Must be defined equal to a (random) filler function,
 satisfying _<T>Action_. If _NDEBUG_ is not defined, turns on _assert_ private
 function integrity testing. Requires _ARRAY_TO_STRING_.

minimum standard: C89

author: Neil

version: 2019-03 Renamed _Pool_ to _Array_. Took out migrate.

since: 2018-04 Merged _Stack_ into _Pool_ again to eliminate duplication;
			2018-03 Why have an extra level of indirection?
			2018-02 Errno instead of custom errors.
			2017-12 Introduced _POOL_PARENT_ for type-safety.
			2017-11 Forked _Stack_ from _Pool_.
			2017-10 Replaced _PoolIsEmpty_ by _PoolElement_, much more useful.
			2017-10 Renamed Pool; made migrate automatic.
			2017-07 Made migrate simpler.
			2017-05 Split _List_ from _Pool_; much simpler.
			2017-01 Almost-redundant functions simplified.
			2016-11 Multi-index.
			2016-08 Permute.



## Declarations ##

### typedef void (*<PT>ToString)(const T *, char (*const)[12]) ###

typedef void (*<PT>ToString)(const T *, char (*const)[12])

Responsible for turning _<T>_ (the first argument) into a 12 _char_
 null-terminated output string (the second.) Used for _ARRAY_TO_STRING_.


### struct <T>Array ###

struct <T>Array

The array. To instantiate, see `<T>Array`.




## Function Summary ##

_Return Type_	_Function Name_	_Argument List_
static void	<T>Array_	(struct <T>Array *const a)
static void	<T>Array	(struct <T>Array *const a)
static size_t	<T>ArraySize	(const struct <T>Array *const a)
static int	<T>ArrayRemove	(struct <T>Array *const a, T *const data)
static void	<T>ArrayClear	(struct <T>Array *const a)
static T *	<T>ArrayGet	(const struct <T>Array *const a, const size_t idx)
static size_t	<T>ArrayIndex	(const struct <T>Array *const a,
	const T *const data)
static T *	<T>ArrayPeek	(const struct <T>Array *const a)
static T *	<T>ArrayPop	(struct <T>Array *const a)
static T *	<T>ArrayNext	(const struct <T>Array *const a, T *const prev)
static T *	<T>ArrayNew	(struct <T>Array *const a)
static T *	<T>ArrayUpdateNew	(struct <T>Array *const a,
	T **const update_ptr)
static void	<T>ArrayForEach	(struct <T>Array *const a,
	const <PT>Action action)
static const char *	<T>ArrayToString	(const struct <T>Array *const a)



## Function Detail ##

### <T>Array_ ###

static void <T>Array_(struct <T>Array *const a)

Destructor for _a_. All the _a_ contents should not be accessed
 anymore and the _a_ takes no memory.

parameter: a -- If null, does nothing.

order: \Theta(1)



### <T>Array ###

static void <T>Array(struct <T>Array *const a)

Initialises _a_ to be empty. If it is _static_ data then it is
 initialised by default and one doesn't have to call this.

order: \Theta(1)



### <T>ArraySize ###

static size_t <T>ArraySize(const struct <T>Array *const a)



return: The size.

order: O(1)



### <T>ArrayRemove ###

static int <T>ArrayRemove(struct <T>Array *const a, T *const data)

Removes _data_ from _a_. Only if not _ARRAY_STACK_.

parameter: a, data -- If null, returns false.

parameter: data -- Will be removed; data will remain the same but be updated to the
 next element, (ARRAY_TAIL_DELETE causes the next element to be the tail,) or
 if this was the last element, the pointer will be past the end.

return: Success, otherwise _errno_ will be set for valid input.

throws: EDOM -- _data_ is not part of _a_.

order: Amortised O(1) if _ARRAY_FREE_LIST_ is defined, otherwise, O(n).

fixme: Test on stack.



### <T>ArrayClear ###

static void <T>ArrayClear(struct <T>Array *const a)

Removes all from _a_, but leaves the _a_ memory alone; if one wants
 to remove memory, see `Array_`.

parameter: a -- If null, does nothing.

order: \Theta(1)



### <T>ArrayGet ###

static T * <T>ArrayGet(const struct <T>Array *const a, const size_t idx)

Gets an existing element by index. Causing something to be added to the
 _<T>Array_ may invalidate this pointer, see `<T>ArrayUpdateNew`.

parameter: a -- If null, returns null.

parameter: idx -- Index.

return: If failed, returns a null pointer.

order: \Theta(1)



### <T>ArrayIndex ###

static size_t <T>ArrayIndex(const struct <T>Array *const a,
	const T *const data)

Gets an index given _data_.

parameter: data -- If the element is not part of the _Array_, behaviour is undefined.

return: An index.

order: \Theta(1)

fixme: Untested.



### <T>ArrayPeek ###

static T * <T>ArrayPeek(const struct <T>Array *const a)



parameter: a -- If null, returns null.

return: The last element or null if the a is empty. Causing something to be
 added to the _array_ may invalidate this pointer.

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



### <T>ArrayNext ###

static T * <T>ArrayNext(const struct <T>Array *const a, T *const prev)

Provides a way to iterate through the _a_. It is safe to add using
 `<T>ArrayUpdateNew` with the return value as _update_. Removing an element
 causes the pointer to go to the next element, if it exists.

parameter: a -- If null, returns null. If _prev_ is not from this _a_ and not
 null, returns null.

parameter: prev -- Set it to null to start the iteration.

return: A pointer to the next element or null if there are no more.

order: \Theta(1)



### <T>ArrayNew ###

static T * <T>ArrayNew(struct <T>Array *const a)

Gets an uninitialised new element. May move the _Array_ to a new memory
 location to fit the new size.

parameter: a -- If is null, returns null.

return: A new, un-initialised, element, or null and _errno_ may be set.

throws: ERANGE -- Tried allocating more then can fit in _size_t_ objects.

throws: _realloc_ errors -- _IEEE Std 1003.1-2001_.

order: amortised O(1)



### <T>ArrayUpdateNew ###

static T * <T>ArrayUpdateNew(struct <T>Array *const a,
	T **const update_ptr)

Gets an uninitialised new element and updates the _update_ptr_ if it is
 within the memory region that was changed to accomidate new space. For
 example, when iterating a pointer and new element is needed that could change
 the pointer.

parameter: a -- If null, returns null.

parameter: update_ptr -- Pointer to update on memory move.

return: A new, un-initialised, element, or null and _errno_ may be set.

throws: ERANGE -- Tried allocating more then can fit in _size_t_.

throws: _realloc_ errors -- _IEEE Std 1003.1-2001_.

order: amortised O(1)

fixme: Untested.



### <T>ArrayForEach ###

static void <T>ArrayForEach(struct <T>Array *const a,
	const <PT>Action action)

Iterates though _a_ from the bottom and calls _action_ on all the
 elements. The topology of the list can not change while in this function.
 That is, don't call `<T>ArrayNew`, `<T>ArrayRemove`, _etc_ in
 _action_.

parameter: stack, action -- If null, does nothing.

order: O(_size_ \times _action_)

fixme: Untested.

fixme: Sequence interface.



### <T>ArrayToString ###

static const char * <T>ArrayToString(const struct <T>Array *const a)

Can print 4 things at once before it overwrites. One must a
 _ARRAY_TO_STRING_ to a function implementing _<T>ToString_ to get this
 functionality.

return: Prints _a_ in a static buffer.

order: \Theta(1); it has a 255 character limit; every element takes some of it.

fixme: ToString interface.




