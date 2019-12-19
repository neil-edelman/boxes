# Array\.h #

## Parameterised Contiguous Dynamic Array \(Vector\) ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PT&gt;ToString](#user-content-typedef-c92c3b0f), [&lt;PT&gt;Action](#user-content-typedef-33725a81), [&lt;PT&gt;Predicate](#user-content-typedef-d7c73930)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;T&gt;Array](#user-content-tag-f128eca2)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of Array](web/array.png)

[&lt;T&gt;Array](#user-content-tag-f128eca2) is a dynamic array that stores contiguous `<T>`, which must be set using `ARRAY_TYPE`\. To ensure that the capacity is greater then or equal to the size, resizing may be necessary and incurs amortised cost\. When adding new elements, the elements may change memory location to fit\. It is therefore unstable; any pointers to this memory may become stale and unusable\.

`<T>Array` is not synchronised\. Errors are returned with `errno`\. The parameters are preprocessor macros, and are all undefined at the end of the file for convenience\. `assert.h` is included in this file; to stop the debug assertions, use `#define NDEBUG` before `assert.h`\.



 * Parameter: ARRAY\_NAME, ARRAY\_TYPE  
   `<T>` that satisfies `C` naming conventions when mangled and a valid tag \(type\) associated therewith; required\. `<PT>` is private, whose names are prefixed in a manner to avoid collisions; any should be re\-defined prior to use elsewhere\.
 * Parameter: ARRAY\_STACK  
   Doesn't define removal functions except [&lt;T&gt;ArrayPop](#user-content-fn-c32fdd31), making it a stack\.
 * Parameter: ARRAY\_TO\_STRING  
   Optional print function implementing [&lt;PT&gt;ToString](#user-content-typedef-c92c3b0f); makes available [&lt;T&gt;ArrayToString](#user-content-fn-e365d362)\.
 * Parameter: ARRAY\_TEST  
   Unit testing framework [&lt;T&gt;ArrayTest](#user-content-fn-8737e8e2), included in a separate header, [\.\./test/ArrayTest\.h](../test/ArrayTest.h)\. Must be defined equal to a \(random\) filler function, satisfying [&lt;PT&gt;Action](#user-content-typedef-33725a81)\. Requires `ARRAY_TO_STRING` and not `NDEBUG`\.
 * Standard:  
   C89
 * See also:  
   [Digraph](https://github.com/neil-edelman/Digraph); [List](https://github.com/neil-edelman/List); [Orcish](https://github.com/neil-edelman/Orcish); [Pool](https://github.com/neil-edelman/Pool); [Set](https://github.com/neil-edelman/Set)


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-c92c3b0f" name = "user-content-typedef-c92c3b0f">&lt;PT&gt;ToString</a> ###

<code>typedef void(*<strong>&lt;PT&gt;ToString</strong>)(const T *, char(*)[12]);</code>

Responsible for turning `<T>` \(the first argument\) into a 12 `char` null\-terminated output string \(the second\.\) Used for `ARRAY_TO_STRING`\.



### <a id = "user-content-typedef-33725a81" name = "user-content-typedef-33725a81">&lt;PT&gt;Action</a> ###

<code>typedef void(*<strong>&lt;PT&gt;Action</strong>)(T *data);</code>

Operates by side\-effects on `data` only\.



### <a id = "user-content-typedef-d7c73930" name = "user-content-typedef-d7c73930">&lt;PT&gt;Predicate</a> ###

<code>typedef int(*<strong>&lt;PT&gt;Predicate</strong>)(const T *data);</code>

Given constant `data`, returns a boolean\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-f128eca2" name = "user-content-tag-f128eca2">&lt;T&gt;Array</a> ###

<code>struct <strong>&lt;T&gt;Array</strong>;</code>

The array\. Zeroed data is a valid state\. To instantiate explicitly, see [&lt;T&gt;Array](#user-content-fn-f128eca2) or initialise it with `ARRAY_INIT` or `{0}` \(C99\.\)

![States.](web/states.png)



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-a06d1247">&lt;T&gt;Array_</a></td><td>a</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-f128eca2">&lt;T&gt;Array</a></td><td>a</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-824e26cb">&lt;T&gt;ArraySize</a></td><td>a</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-8267fb66">&lt;T&gt;ArrayRemove</a></td><td>a, data</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-3d3eaaa0">&lt;T&gt;ArrayLazyRemove</a></td><td>a, data</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-7094ab4b">&lt;T&gt;ArrayClear</a></td><td>a</td></tr>

<tr><td align = right>static T *</td><td><a href = "#user-content-fn-3d64b66e">&lt;T&gt;ArrayGet</a></td><td>a</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-2f86cf96">&lt;T&gt;ArrayIndex</a></td><td>a, data</td></tr>

<tr><td align = right>static T *</td><td><a href = "#user-content-fn-168bb5af">&lt;T&gt;ArrayEnd</a></td><td>a</td></tr>

<tr><td align = right>static T *</td><td><a href = "#user-content-fn-f880f61d">&lt;T&gt;ArrayPeek</a></td><td>a</td></tr>

<tr><td align = right>static T *</td><td><a href = "#user-content-fn-c32fdd31">&lt;T&gt;ArrayPop</a></td><td>a</td></tr>

<tr><td align = right>static T *</td><td><a href = "#user-content-fn-20e9e3a1">&lt;T&gt;ArrayBack</a></td><td>a, here</td></tr>

<tr><td align = right>static T *</td><td><a href = "#user-content-fn-4ca0efff">&lt;T&gt;ArrayNext</a></td><td>a, here</td></tr>

<tr><td align = right>static T *</td><td><a href = "#user-content-fn-2895000c">&lt;T&gt;ArrayNew</a></td><td>a</td></tr>

<tr><td align = right>static T *</td><td><a href = "#user-content-fn-e810048b">&lt;T&gt;ArrayUpdateNew</a></td><td>a, update_ptr</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-8fb34e3e">&lt;T&gt;ArrayReserve</a></td><td>a, reserve</td></tr>

<tr><td align = right>static T *</td><td><a href = "#user-content-fn-12fc774c">&lt;T&gt;ArrayBuffer</a></td><td>a, add</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-669dc6cf">&lt;T&gt;ArrayEach</a></td><td>a, action</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-47277df8">&lt;T&gt;ArrayIfEach</a></td><td>a, predicate, action</td></tr>

<tr><td align = right>static T *</td><td><a href = "#user-content-fn-5d3e6684">&lt;T&gt;ArrayAny</a></td><td>a, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-66da1814">&lt;T&gt;ArrayKeepIf</a></td><td>a, keep, destruct</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-761b4122">&lt;T&gt;ArrayTrim</a></td><td>a, predicate</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-bad9dae2">&lt;T&gt;ArraySplice</a></td><td>a, anchor, range, b</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-503c6ec6">&lt;T&gt;ArrayIndexSplice</a></td><td>a, i0, i1, b</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-e365d362">&lt;T&gt;ArrayToString</a></td><td>a</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-a06d1247" name = "user-content-fn-a06d1247">&lt;T&gt;Array_</a> ###

<code>static void <strong>&lt;T&gt;Array_</strong>(struct &lt;T&gt;Array *const <em>a</em>)</code>

Returns `a` to the idle state where it takes no dynamic memory\.

 * Parameter: _a_  
   If null, does nothing\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-f128eca2" name = "user-content-fn-f128eca2">&lt;T&gt;Array</a> ###

<code>static void <strong>&lt;T&gt;Array</strong>(struct &lt;T&gt;Array *const <em>a</em>)</code>

Initialises `a` to be idle\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-824e26cb" name = "user-content-fn-824e26cb">&lt;T&gt;ArraySize</a> ###

<code>static size_t <strong>&lt;T&gt;ArraySize</strong>(const struct &lt;T&gt;Array *const <em>a</em>)</code>

 * Parameter: _a_  
   If null, returns zero\.
 * Return:  
   The size of `a`\.
 * Order:  
   &#927;\(1\)




### <a id = "user-content-fn-8267fb66" name = "user-content-fn-8267fb66">&lt;T&gt;ArrayRemove</a> ###

<code>static int <strong>&lt;T&gt;ArrayRemove</strong>(struct &lt;T&gt;Array *const <em>a</em>, T *const <em>data</em>)</code>

Removes `data` from `a`\. Only defined if not `ARRAY_STACK`\.

 * Parameter: _a_  
   If null, returns false\.
 * Parameter: _data_  
   If null, returns false\. Will be removed; data will remain the same but be updated to the next element, or if this was the last element, the pointer will be past the end\.
 * Return:  
   Success, otherwise `errno` will be set for valid input\.
 * Exceptional return: EDOM  
   `data` is not part of `a`\.
 * Order:  
   &#927;\(n\)\.




### <a id = "user-content-fn-3d3eaaa0" name = "user-content-fn-3d3eaaa0">&lt;T&gt;ArrayLazyRemove</a> ###

<code>static int <strong>&lt;T&gt;ArrayLazyRemove</strong>(struct &lt;T&gt;Array *const <em>a</em>, T *const <em>data</em>)</code>

Removes `data` from `a` and replaces the spot it was in with the tail\. Only defined if not `ARRAY_STACK`\.

 * Parameter: _a_  
   If null, returns false\.
 * Parameter: _data_  
   If null, returns false\. Will be removed; data will remain the same but be updated to the last element, or if this was the last element, the pointer will be past the end\.
 * Return:  
   Success, otherwise `errno` will be set for valid input\.
 * Exceptional return: EDOM  
   `data` is not part of `a`\.
 * Order:  
   &#927;\(1\)\.




### <a id = "user-content-fn-7094ab4b" name = "user-content-fn-7094ab4b">&lt;T&gt;ArrayClear</a> ###

<code>static void <strong>&lt;T&gt;ArrayClear</strong>(struct &lt;T&gt;Array *const <em>a</em>)</code>

Sets `a` to be empty\. That is, the size of `a` will be zero, but if it was previously in an active non\-idle state, it continues to be\. Compare [&lt;T&gt;Array_](#user-content-fn-a06d1247)\.

 * Parameter: _a_  
   If null, does nothing\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-3d64b66e" name = "user-content-fn-3d64b66e">&lt;T&gt;ArrayGet</a> ###

<code>static T *<strong>&lt;T&gt;ArrayGet</strong>(const struct &lt;T&gt;Array *const <em>a</em>)</code>

As long as the size doesn't go up, see [&lt;T&gt;ArrayUpdateNew](#user-content-fn-e810048b)\.

 * Parameter: _a_  
   If null, returns null\.
 * Return:  
   A pointer to the `a`'s data, indexable up to the `a`'s size\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-2f86cf96" name = "user-content-fn-2f86cf96">&lt;T&gt;ArrayIndex</a> ###

<code>static size_t <strong>&lt;T&gt;ArrayIndex</strong>(const struct &lt;T&gt;Array *const <em>a</em>, const T *const <em>data</em>)</code>

Gets an index given `data`\.

 * Parameter: _a_  
   Must be a valid object that stores `data`\.
 * Parameter: _data_  
   If the element is not part of the `a`, behaviour is undefined\.
 * Return:  
   An index\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-168bb5af" name = "user-content-fn-168bb5af">&lt;T&gt;ArrayEnd</a> ###

<code>static T *<strong>&lt;T&gt;ArrayEnd</strong>(const struct &lt;T&gt;Array *const <em>a</em>)</code>

 * Parameter: _a_  
   If null or idle, returns null\.
 * Return:  
   One past the end of the array\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-f880f61d" name = "user-content-fn-f880f61d">&lt;T&gt;ArrayPeek</a> ###

<code>static T *<strong>&lt;T&gt;ArrayPeek</strong>(const struct &lt;T&gt;Array *const <em>a</em>)</code>

 * Parameter: _a_  
   If null, returns null\.
 * Return:  
   The last element or null if the a is empty\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-c32fdd31" name = "user-content-fn-c32fdd31">&lt;T&gt;ArrayPop</a> ###

<code>static T *<strong>&lt;T&gt;ArrayPop</strong>(struct &lt;T&gt;Array *const <em>a</em>)</code>

The same value as [&lt;T&gt;ArrayPeek](#user-content-fn-f880f61d)\.

 * Parameter: _a_  
   If null, returns null\.
 * Return:  
   Value from the the top of the `a` that is removed or null if the stack is empty\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-20e9e3a1" name = "user-content-fn-20e9e3a1">&lt;T&gt;ArrayBack</a> ###

<code>static T *<strong>&lt;T&gt;ArrayBack</strong>(const struct &lt;T&gt;Array *const <em>a</em>, const T *const <em>here</em>)</code>

Iterate through `a` backwards\.

 * Parameter: _a_  
   The array; if null, returns null\.
 * Parameter: _here_  
   Set it to the current element; when null, it will be last element, if it exists\.
 * Return:  
   A pointer to the previous element or null if it does not exist\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-4ca0efff" name = "user-content-fn-4ca0efff">&lt;T&gt;ArrayNext</a> ###

<code>static T *<strong>&lt;T&gt;ArrayNext</strong>(const struct &lt;T&gt;Array *const <em>a</em>, const T *const <em>here</em>)</code>

Iterate through `a`\. It is safe to add using [&lt;T&gt;ArrayUpdateNew](#user-content-fn-e810048b) with the return value as `update`\. Removing an element causes the pointer to go to the next element, if it exists\.

 * Parameter: _a_  
   The array; if null, returns null\.
 * Parameter: _here_  
   Set it to null to get the first element, if it exists\.
 * Return:  
   A pointer to the next element or null if there are no more\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-2895000c" name = "user-content-fn-2895000c">&lt;T&gt;ArrayNew</a> ###

<code>static T *<strong>&lt;T&gt;ArrayNew</strong>(struct &lt;T&gt;Array *const <em>a</em>)</code>

 * Parameter: _a_  
   If is null, returns null\.
 * Return:  
   A new, un\-initialised, element at the back of `a`, or null and `errno` will be set\.
 * Exceptional return: ERANGE  
   Tried allocating more then can fit in `size_t` or `realloc` error and doesn't follow [IEEE Std 1003.1-2001](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html)\.
 * Exceptional return: realloc  
 * Order:  
   Amortised &#927;\(1\)\.




### <a id = "user-content-fn-e810048b" name = "user-content-fn-e810048b">&lt;T&gt;ArrayUpdateNew</a> ###

<code>static T *<strong>&lt;T&gt;ArrayUpdateNew</strong>(struct &lt;T&gt;Array *const <em>a</em>, T **const <em>update_ptr</em>)</code>

 * Parameter: _a_  
   If null, returns null\.
 * Parameter: _update\_ptr_  
   Pointer to update on memory move if it is within the memory region that was changed to accommodate new space\.
 * Return:  
   A new, un\-initialised, element at the back of `a`, or null and `errno` will be set\.
 * Exceptional return: ERANGE  
   Tried allocating more then can fit in `size_t` or `realloc` error and doesn't follow [IEEE Std 1003.1-2001](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html)\.
 * Exceptional return: realloc  
 * Order:  
   Amortised &#927;\(1\)\.




### <a id = "user-content-fn-8fb34e3e" name = "user-content-fn-8fb34e3e">&lt;T&gt;ArrayReserve</a> ###

<code>static int <strong>&lt;T&gt;ArrayReserve</strong>(struct &lt;T&gt;Array *const <em>a</em>, const size_t <em>reserve</em>)</code>

Ensures that `a` array is `reserve` capacity beyond the elements in the array, but doesn't add to the size\.

 * Parameter: _a_  
   If null, returns false\.
 * Parameter: _reserve_  
   If zero, returns true\.
 * Return:  
   Success or `errno` will be set\.
 * Exceptional return: ERANGE  
   Tried allocating more then can fit in `size_t` or `realloc` error and doesn't follow [IEEE Std 1003.1-2001](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html)\.
 * Exceptional return: realloc  
 * Order:  
   Amortised &#927;\(`reserve`\)\.




### <a id = "user-content-fn-12fc774c" name = "user-content-fn-12fc774c">&lt;T&gt;ArrayBuffer</a> ###

<code>static T *<strong>&lt;T&gt;ArrayBuffer</strong>(struct &lt;T&gt;Array *const <em>a</em>, const size_t <em>add</em>)</code>

 * Parameter: _a_  
   If null, returns null\.
 * Parameter: _add_  
   If zero, returns null\.
 * Return:  
   The start of a new, un\-initialised, sub\-array of `add` elements at the back of `a`, or null and `errno` will be set\.
 * Exceptional return: ERANGE  
   Tried allocating more then can fit in `size_t` or `realloc` error and doesn't follow [IEEE Std 1003.1-2001](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html)\.
 * Exceptional return: realloc  
 * Order:  
   Amortised &#927;\(`add`\)\.




### <a id = "user-content-fn-669dc6cf" name = "user-content-fn-669dc6cf">&lt;T&gt;ArrayEach</a> ###

<code>static void <strong>&lt;T&gt;ArrayEach</strong>(struct &lt;T&gt;Array *const <em>a</em>, const &lt;PT&gt;Action <em>action</em>)</code>

Iterates through `a` and calls `action` on all the elements\. The topology of the list can not change while in this function\. That is, don't call [&lt;T&gt;ArrayNew](#user-content-fn-2895000c), [&lt;T&gt;ArrayRemove](#user-content-fn-8267fb66), _etc_ in `action`\.

 * Parameter: _a_  
   If null, does nothing\.
 * Parameter: _action_  
   If null, does nothing\.
 * Order:  
   &#927;\(`size` &#215; `action`\)




### <a id = "user-content-fn-47277df8" name = "user-content-fn-47277df8">&lt;T&gt;ArrayIfEach</a> ###

<code>static void <strong>&lt;T&gt;ArrayIfEach</strong>(struct &lt;T&gt;Array *const <em>a</em>, const &lt;PT&gt;Predicate <em>predicate</em>, const &lt;PT&gt;Action <em>action</em>)</code>

Iterates through `a` and calls `action` on all the elements for which `predicate` returns true\. The topology of the list can not change while in this function\.

 * Parameter: _a_  
   If null, does nothing\.
 * Parameter: _predicate_  
   If null, does nothing\.
 * Parameter: _action_  
   If null, does nothing\.
 * Order:  
   &#927;\(`size` &#215; `action`\)




### <a id = "user-content-fn-5d3e6684" name = "user-content-fn-5d3e6684">&lt;T&gt;ArrayAny</a> ###

<code>static T *<strong>&lt;T&gt;ArrayAny</strong>(const struct &lt;T&gt;Array *const <em>a</em>, const &lt;PT&gt;Predicate <em>predicate</em>)</code>

Iterates through `a` and calls `predicate` until it returns true\.

 * Parameter: _a_  
   If null, returns null\.
 * Parameter: _predicate_  
   If null, returns null\.
 * Return:  
   The first `predicate` that returned true, or, if the statement is false on all, null\.
 * Order:  
   &#927;\(`size` &#215; `action`\)




### <a id = "user-content-fn-66da1814" name = "user-content-fn-66da1814">&lt;T&gt;ArrayKeepIf</a> ###

<code>static void <strong>&lt;T&gt;ArrayKeepIf</strong>(struct &lt;T&gt;Array *const <em>a</em>, const &lt;PT&gt;Predicate <em>keep</em>, const &lt;PT&gt;Action <em>destruct</em>)</code>

For all elements of `a`, calls `keep`, and for each element, if the return value is false, lazy deletes that item, calling `destruct` if not\-null\.

 * Parameter: _a_  
   If null, does nothing\.
 * Parameter: _keep_  
   If null, does nothing\.
 * Order:  
   &#927;\(`size`\)




### <a id = "user-content-fn-761b4122" name = "user-content-fn-761b4122">&lt;T&gt;ArrayTrim</a> ###

<code>static void <strong>&lt;T&gt;ArrayTrim</strong>(struct &lt;T&gt;Array *const <em>a</em>, const &lt;PT&gt;Predicate <em>predicate</em>)</code>

Removes at either end of `a` of things that `predicate` returns true\.

 * Parameter: _a_  
   If null, does nothing\.
 * Parameter: _predicate_  
   If null, does nothing\.
 * Order:  
   &#927;\(`size`\)




### <a id = "user-content-fn-bad9dae2" name = "user-content-fn-bad9dae2">&lt;T&gt;ArraySplice</a> ###

<code>static int <strong>&lt;T&gt;ArraySplice</strong>(struct &lt;T&gt;Array *const <em>a</em>, const T *<em>anchor</em>, const long <em>range</em>, const struct &lt;T&gt;Array *const <em>b</em>)</code>

In `a`, replaces the elements from `anchor` up to `range` with a copy of `b`\.

 * Parameter: _a_  
   If null, returns zero\.
 * Parameter: _anchor_  
   Beginning of the replaced value, inclusive\. If null, appends to the end\.
 * Parameter: _range_  
   How many replaced values in the original; negative values are implicitly plus the length of the array; clamped at the minimum and maximum\.
 * Parameter: _b_  
   The replacement array\. If null, deletes without replacing\. It is more efficient than individual [&lt;T&gt;ArrayRemove](#user-content-fn-8267fb66) to delete several consecutive values\.
 * Return:  
   Success\.
 * Exceptional return: EDOM  
   `a` and `b` are not null and the same\.
 * Exceptional return: ERANGE  
   `anchor` is not null and not in `a`\.
 * Exceptional return: ERANGE  
   `range` is greater then 65535 or smaller then \-65534\.
 * Exceptional return: ERANGE  
   `b` would cause the array to overflow\.
 * Exceptional return: realloc  
 * Order:  
   &#920;\(`b.size`\) if the elements have the same size, otherwise, amortised &#927;\(`a.size` \+ `b.size`\)\.




### <a id = "user-content-fn-503c6ec6" name = "user-content-fn-503c6ec6">&lt;T&gt;ArrayIndexSplice</a> ###

<code>static int <strong>&lt;T&gt;ArrayIndexSplice</strong>(struct &lt;T&gt;Array *const <em>a</em>, const size_t <em>i0</em>, const size_t <em>i1</em>, const struct &lt;T&gt;Array *const <em>b</em>)</code>

In `a`, replaces the elements from indices `i0` \(inclusive\) to `i1` \(exclusive\) with a copy of `b`\.

 * Parameter: _a_  
   If null, returns zero\.
 * Parameter: _i0_  
   The replacement indices, `[i0, i1)`, such that `0 <= i0 <= i1 <= a.size`\.
 * Parameter: _i1_  
   The replacement indices, `[i0, i1)`, such that `0 <= i0 <= i1 <= a.size`\.
 * Parameter: _b_  
   The replacement array\. If null, deletes without replacing\.
 * Return:  
   Success\.
 * Exceptional return: EDOM  
   `a` and `b` are not null and the same\.
 * Exceptional return: EDOM  
   `i0` or `i1` are out\-of\-bounds or `i0 > i1`\.
 * Exceptional return: ERANGE  
   `b` would cause the array to overflow\.
 * Exceptional return: realloc  
 * Order:  
   &#920;\(`b.size`\) if the elements have the same size, otherwise, amortised &#927;\(`a.size` \+ `b.size`\)\.




### <a id = "user-content-fn-e365d362" name = "user-content-fn-e365d362">&lt;T&gt;ArrayToString</a> ###

<code>static const char *<strong>&lt;T&gt;ArrayToString</strong>(const struct &lt;T&gt;Array *const <em>a</em>)</code>

Can print 4 things at once before it overwrites\. One must a `ARRAY_TO_STRING` to a function implementing [&lt;PT&gt;ToString](#user-content-typedef-c92c3b0f) to get this functionality\.

 * Return:  
   Prints `a` in a static buffer\.
 * Order:  
   &#920;\(1\); it has a 255 character limit; every element takes some of it\.






## <a id = "user-content-license" name = "user-content-license">License</a> ##

2016 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



