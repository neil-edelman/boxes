# Array\.h #

## Contiguous Dynamic Array \(Vector\) ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PT&gt;Type](#user-content-typedef-8b318acb), [&lt;PT&gt;Action](#user-content-typedef-33725a81), [&lt;PT&gt;Predicate](#user-content-typedef-d7c73930), [&lt;PT&gt;Bipredicate](#user-content-typedef-49a99943), [&lt;PT&gt;Biproject](#user-content-typedef-269e157f), [&lt;PT&gt;Compare](#user-content-typedef-abfbe632), [&lt;PT&gt;ToString](#user-content-typedef-c92c3b0f)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;T&gt;Array](#user-content-tag-f128eca2), [&lt;PT&gt;Iterator](#user-content-tag-25ae129)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of Array](web/array.png)

[&lt;T&gt;Array](#user-content-tag-f128eca2) is a dynamic array that stores contiguous `<T>`\. When modifying the array, to ensure that the capacity is greater then or equal to the size, resizing may be necessary\. This incurs amortised cost and any pointers to this memory may become stale\.

`<T>Array` is not synchronised\. Errors are returned with `errno`\. The parameters are preprocessor macros, and are all undefined at the end of the file for convenience\. Assertions are used in this file; to stop them, define `NDEBUG` before `assert.h`\.



 * Parameter: ARRAY\_NAME, ARRAY\_TYPE  
   `<T>` that satisfies `C` naming conventions when mangled and a valid tag\-type associated therewith; required\. `<PT>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: ARRAY\_EXPECT\_TRAIT  
   Do not un\-define certain variables for subsequent inclusion in a trait\.
 * Parameter: ARRAY\_TO\_STRING\_NAME, ARRAY\_TO\_STRING  
   To string trait contained in [ToString\.h](ToString.h); `<A>` that satisfies `C` naming conventions when mangled and function implementing [&lt;PT&gt;ToString](#user-content-typedef-c92c3b0f)\. There can be multiple to string traits, but only one can omit `ARRAY_TO_STRING_NAME`\.
 * Parameter: ARRAY\_TEST  
   To string trait contained in [\.\./test/ArrayTest\.h](../test/ArrayTest.h); optional unit testing framework using `assert`\. Can only be defined once _per_ `Array`\. Must be defined equal to a \(random\) filler function, satisfying [&lt;PT&gt;Action](#user-content-typedef-33725a81)\. Output will be shown with the to string interface in which it's defined; provides tests for the base code and all later interfaces\.
 * Parameter: ARRAY\_COMPARABLE\_NAME, ARRAY\_IS\_EQUAL, ARRAY\_COMPARE  
   Comparable trait; `<C>` that satisfies `C` naming conventions when mangled and a function implementing, for `ARRAY_IS_EQUAL` [&lt;PT&gt;Bipredicate](#user-content-typedef-49a99943) that establishes an equivalence relation, or for `ARRAY_COMPARE` [&lt;PT&gt;Compare](#user-content-typedef-abfbe632) that establishes a total order\. There can be multiple contrast traits, but only one can omit `ARRAY_COMPARABLE_NAME`\.
 * Standard:  
   C89
 * See also:  
   [Heap](https://github.com/neil-edelman/Heap); [List](https://github.com/neil-edelman/List); [Orcish](https://github.com/neil-edelman/Orcish); [Pool](https://github.com/neil-edelman/Pool); [Set](https://github.com/neil-edelman/Set); [Trie](https://github.com/neil-edelman/Trie)


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-8b318acb" name = "user-content-typedef-8b318acb">&lt;PT&gt;Type</a> ###

<code>typedef ARRAY_TYPE <strong>&lt;PT&gt;Type</strong>;</code>

A valid tag type set by `ARRAY_TYPE`\. This becomes `T`\.



### <a id = "user-content-typedef-33725a81" name = "user-content-typedef-33725a81">&lt;PT&gt;Action</a> ###

<code>typedef void(*<strong>&lt;PT&gt;Action</strong>)(T *);</code>

Operates by side\-effects\.



### <a id = "user-content-typedef-d7c73930" name = "user-content-typedef-d7c73930">&lt;PT&gt;Predicate</a> ###

<code>typedef int(*<strong>&lt;PT&gt;Predicate</strong>)(const T *);</code>

Returns a boolean given read\-only `<T>`\.



### <a id = "user-content-typedef-49a99943" name = "user-content-typedef-49a99943">&lt;PT&gt;Bipredicate</a> ###

<code>typedef int(*<strong>&lt;PT&gt;Bipredicate</strong>)(const T *, const T *);</code>

Returns a boolean given two read\-only `<T>`\.



### <a id = "user-content-typedef-269e157f" name = "user-content-typedef-269e157f">&lt;PT&gt;Biproject</a> ###

<code>typedef int(*<strong>&lt;PT&gt;Biproject</strong>)(T *, T *);</code>

Returns a boolean given two `<T>`\.



### <a id = "user-content-typedef-abfbe632" name = "user-content-typedef-abfbe632">&lt;PT&gt;Compare</a> ###

<code>typedef int(*<strong>&lt;PT&gt;Compare</strong>)(const T *a, const T *b);</code>

Three\-way comparison on a totally order set; returns an integer value less then, equal to, greater then zero, if `a < b`, `a == b`, `a > b`, respectively\.



### <a id = "user-content-typedef-c92c3b0f" name = "user-content-typedef-c92c3b0f">&lt;PT&gt;ToString</a> ###

<code>typedef void(*<strong>&lt;PT&gt;ToString</strong>)(const T *, char(*)[12]);</code>

Responsible for turning the first argument into a 12\-`char` null\-terminated output string\. Used for `ARRAY_TO_STRING`\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-f128eca2" name = "user-content-tag-f128eca2">&lt;T&gt;Array</a> ###

<code>struct <strong>&lt;T&gt;Array</strong>;</code>

Manages the array field `data`, which is indexed up to `size`\. To initialise it to an idle state, see [&lt;T&gt;Array](#user-content-fn-f128eca2), `ARRAY_IDLE`, `{0}` \(`C99`,\) or being `static`\.

![States.](web/states.png)



### <a id = "user-content-tag-25ae129" name = "user-content-tag-25ae129">&lt;PT&gt;Iterator</a> ###

<code>struct <strong>&lt;PT&gt;Iterator</strong>;</code>

Contains all iteration parameters in one\.



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-f128eca2">&lt;T&gt;Array</a></td><td>a</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-a06d1247">&lt;T&gt;Array_</a></td><td>a</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-8267fb66">&lt;T&gt;ArrayRemove</a></td><td>a, datum</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-3d3eaaa0">&lt;T&gt;ArrayLazyRemove</a></td><td>a, datum</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-7094ab4b">&lt;T&gt;ArrayClear</a></td><td>a</td></tr>

<tr><td align = right>static T *</td><td><a href = "#user-content-fn-f880f61d">&lt;T&gt;ArrayPeek</a></td><td>a</td></tr>

<tr><td align = right>static T *</td><td><a href = "#user-content-fn-c32fdd31">&lt;T&gt;ArrayPop</a></td><td>a</td></tr>

<tr><td align = right>static T *</td><td><a href = "#user-content-fn-2895000c">&lt;T&gt;ArrayNew</a></td><td>a</td></tr>

<tr><td align = right>static T *</td><td><a href = "#user-content-fn-e810048b">&lt;T&gt;ArrayUpdateNew</a></td><td>a, update_ptr</td></tr>

<tr><td align = right>static T *</td><td><a href = "#user-content-fn-8fb34e3e">&lt;T&gt;ArrayReserve</a></td><td>a, reserve</td></tr>

<tr><td align = right>static T *</td><td><a href = "#user-content-fn-12fc774c">&lt;T&gt;ArrayBuffer</a></td><td>a, add</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-669dc6cf">&lt;T&gt;ArrayEach</a></td><td>a, action</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-47277df8">&lt;T&gt;ArrayIfEach</a></td><td>a, predicate, action</td></tr>

<tr><td align = right>static T *</td><td><a href = "#user-content-fn-5d3e6684">&lt;T&gt;ArrayAny</a></td><td>a, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-66da1814">&lt;T&gt;ArrayKeepIf</a></td><td>a, keep, destruct</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-761b4122">&lt;T&gt;ArrayTrim</a></td><td>a, predicate</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-bad9dae2">&lt;T&gt;ArraySplice</a></td><td>a, anchor, range, b</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-503c6ec6">&lt;T&gt;ArrayIndexSplice</a></td><td>a, i0, i1, b</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-d031e21d">&lt;T&gt;Array&lt;A&gt;ToString</a></td><td>a</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-b03500d7">&lt;T&gt;Array&lt;C&gt;Sort</a></td><td>a</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-4163de2b">&lt;T&gt;Array&lt;C&gt;Reverse</a></td><td>a</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-eb4c64d4">&lt;T&gt;Array&lt;C&gt;LowerBound</a></td><td>a, value</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-29222727">&lt;T&gt;Array&lt;C&gt;UpperBound</a></td><td>a, value</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-4cefae16">&lt;T&gt;Array&lt;C&gt;Insert</a></td><td>a, datum</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-f75d7962">&lt;T&gt;Array&lt;C&gt;Compactify</a></td><td>a, merge</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-8737e8e2">&lt;T&gt;ArrayTest</a></td><td></td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-e2e9ab81">&lt;T&gt;Array&lt;C&gt;ContrastTest</a></td><td></td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-f128eca2" name = "user-content-fn-f128eca2">&lt;T&gt;Array</a> ###

<code>static void <strong>&lt;T&gt;Array</strong>(struct &lt;T&gt;Array *const <em>a</em>)</code>

Initialises `a` to be idle\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-a06d1247" name = "user-content-fn-a06d1247">&lt;T&gt;Array_</a> ###

<code>static void <strong>&lt;T&gt;Array_</strong>(struct &lt;T&gt;Array *const <em>a</em>)</code>

Returns `a` to the idle state where it takes no dynamic memory\.

 * Parameter: _a_  
   If null, does nothing\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-8267fb66" name = "user-content-fn-8267fb66">&lt;T&gt;ArrayRemove</a> ###

<code>static int <strong>&lt;T&gt;ArrayRemove</strong>(struct &lt;T&gt;Array *const <em>a</em>, T *const <em>datum</em>)</code>

Removes `datum` from `a`\.

 * Parameter: _a_  
   If null, returns false\.
 * Parameter: _datum_  
   If null, returns false\.
 * Return:  
   Success, otherwise `errno` will be set for valid input\.
 * Exceptional return: EDOM  
   `datum` is not part of `a`\.
 * Order:  
   &#927;\(n\)\.




### <a id = "user-content-fn-3d3eaaa0" name = "user-content-fn-3d3eaaa0">&lt;T&gt;ArrayLazyRemove</a> ###

<code>static int <strong>&lt;T&gt;ArrayLazyRemove</strong>(struct &lt;T&gt;Array *const <em>a</em>, T *const <em>datum</em>)</code>

Removes `datum` from `a` and replaces it with the tail\.

 * Parameter: _a_  
   If null, returns false\.
 * Parameter: _datum_  
   If null, returns false\.
 * Return:  
   Success, otherwise `errno` will be set for valid input\.
 * Exceptional return: EDOM  
   `datum` is not part of `a`\.
 * Order:  
   &#927;\(1\)\.




### <a id = "user-content-fn-7094ab4b" name = "user-content-fn-7094ab4b">&lt;T&gt;ArrayClear</a> ###

<code>static void <strong>&lt;T&gt;ArrayClear</strong>(struct &lt;T&gt;Array *const <em>a</em>)</code>

Sets `a` to be empty\. That is, the size of `a` will be zero, but if it was previously in an active non\-idle state, it continues to be\.

 * Parameter: _a_  
   If null, does nothing\.
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




### <a id = "user-content-fn-2895000c" name = "user-content-fn-2895000c">&lt;T&gt;ArrayNew</a> ###

<code>static T *<strong>&lt;T&gt;ArrayNew</strong>(struct &lt;T&gt;Array *const <em>a</em>)</code>

 * Parameter: _a_  
   If is null, returns null\.
 * Return:  
   A new, un\-initialised, element at the back of `a`, or null and `errno` will be set\.
 * Exceptional return: ERANGE  
   Tried allocating more then can fit in `size_t` or `realloc` error and doesn't follow [POSIX](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html)\.
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
   Tried allocating more then can fit in `size_t` or `realloc` error and doesn't follow [POSIX](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html)\.
 * Exceptional return: realloc  
 * Order:  
   Amortised &#927;\(1\)\.




### <a id = "user-content-fn-8fb34e3e" name = "user-content-fn-8fb34e3e">&lt;T&gt;ArrayReserve</a> ###

<code>static T *<strong>&lt;T&gt;ArrayReserve</strong>(struct &lt;T&gt;Array *const <em>a</em>, const size_t <em>reserve</em>)</code>

Ensures that `a` is `reserve` capacity beyond the elements in the array\.

 * Parameter: _a_  
   If null, returns null\.
 * Return:  
   The previous end of `a`, where are `reserve` elements, or null and `errno` will be set\. Writing on this memory space is safe on success, up to `reserve` elements, but one will have to increase the size, \(see [&lt;T&gt;ArrayBuffer](#user-content-fn-12fc774c)\.\)
 * Exceptional return: ERANGE  
   Tried allocating more then can fit in `size_t` or `realloc` error and doesn't follow [POSIX](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html)\.
 * Exceptional return: realloc  




### <a id = "user-content-fn-12fc774c" name = "user-content-fn-12fc774c">&lt;T&gt;ArrayBuffer</a> ###

<code>static T *<strong>&lt;T&gt;ArrayBuffer</strong>(struct &lt;T&gt;Array *const <em>a</em>, const size_t <em>add</em>)</code>

Adds `add` elements to `a`\.

 * Parameter: _a_  
   If null, returns null\.
 * Parameter: _add_  
   If zero, returns null\.
 * Return:  
   The start of a new sub\-array of `add` elements at the previous end of `a`, or null and `errno` will be set\.
 * Exceptional return: ERANGE  
   Tried allocating more then can fit in `size_t` or `realloc` error and doesn't follow [POSIX](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html)\. If [&lt;T&gt;ArrayReserve](#user-content-fn-8fb34e3e) has been successful in reserving at least `add` elements, one is guaranteed success\.
 * Exceptional return: realloc  
 * Order:  
   Amortised &#927;\(`add`\)\.




### <a id = "user-content-fn-669dc6cf" name = "user-content-fn-669dc6cf">&lt;T&gt;ArrayEach</a> ###

<code>static void <strong>&lt;T&gt;ArrayEach</strong>(struct &lt;T&gt;Array *const <em>a</em>, const &lt;PT&gt;Action <em>action</em>)</code>

Iterates through `a` and calls `action` on all the elements\. The topology of the list should not change while in this function\.

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
   &#927;\(`size` &#215; `predicate`\)




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




### <a id = "user-content-fn-d031e21d" name = "user-content-fn-d031e21d">&lt;T&gt;Array&lt;A&gt;ToString</a> ###

<code>static const char *<strong>&lt;T&gt;Array&lt;A&gt;ToString</strong>(const struct &lt;T&gt;Array *const <em>a</em>)</code>

 * Return:  
   Print the contents of `a` in a static string buffer with the limitations of `ToString.h`\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-b03500d7" name = "user-content-fn-b03500d7">&lt;T&gt;Array&lt;C&gt;Sort</a> ###

<code>static void <strong>&lt;T&gt;Array&lt;C&gt;Sort</strong>(struct &lt;T&gt;Array *const <em>a</em>)</code>

Sorts `a` by `qsort`\.



### <a id = "user-content-fn-4163de2b" name = "user-content-fn-4163de2b">&lt;T&gt;Array&lt;C&gt;Reverse</a> ###

<code>static void <strong>&lt;T&gt;Array&lt;C&gt;Reverse</strong>(struct &lt;T&gt;Array *const <em>a</em>)</code>

Sorts `a` in reverse by `qsort`\.



### <a id = "user-content-fn-eb4c64d4" name = "user-content-fn-eb4c64d4">&lt;T&gt;Array&lt;C&gt;LowerBound</a> ###

<code>static size_t <strong>&lt;T&gt;Array&lt;C&gt;LowerBound</strong>(struct &lt;T&gt;Array *const <em>a</em>, const T *const <em>value</em>)</code>

`a` is a random\-access array which should be partitioned true/false with less\-then `value`\.

 * Parameter: _a_  
   If null, returns zero\.
 * Return:  
   The first index of `a` that is not less then `value`\.
 * Order:  
   &#927;\(log `size`\)




### <a id = "user-content-fn-29222727" name = "user-content-fn-29222727">&lt;T&gt;Array&lt;C&gt;UpperBound</a> ###

<code>static size_t <strong>&lt;T&gt;Array&lt;C&gt;UpperBound</strong>(struct &lt;T&gt;Array *const <em>a</em>, const T *const <em>value</em>)</code>

`a` is a random\-access array which should be partitioned false/true with greater\-than or equals `value`\.

 * Parameter: _a_  
   If null, returns zero\.
 * Return:  
   The first index of `a` that is greater then `value`\.
 * Order:  
   &#927;\(log `size`\)




### <a id = "user-content-fn-4cefae16" name = "user-content-fn-4cefae16">&lt;T&gt;Array&lt;C&gt;Insert</a> ###

<code>static int <strong>&lt;T&gt;Array&lt;C&gt;Insert</strong>(struct &lt;T&gt;Array *const <em>a</em>, const T *const <em>datum</em>)</code>

Inserts a copy of `datum` in `a` at the lower bound\.

 * Parameter: _a_  
   If null, does nothing\.
 * Parameter: _datum_  
   If null, does nothing\.
 * Return:  
   Success\.
 * Exceptional return: ERANGE  
   Tried allocating more then can fit in `size_t` or `realloc` error and doesn't follow [POSIX](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html)\.
 * Exceptional return: realloc  
 * Order:  
   Amortised &#927;\(1\)\.




### <a id = "user-content-fn-f75d7962" name = "user-content-fn-f75d7962">&lt;T&gt;Array&lt;C&gt;Compactify</a> ###

<code>static void <strong>&lt;T&gt;Array&lt;C&gt;Compactify</strong>(struct &lt;T&gt;Array *const <em>a</em>, const &lt;PT&gt;Biproject <em>merge</em>)</code>

Interfaces `ARRAY_COMPARE` or `ARRAY_IS_EQUAL`\. For each consecutive and equal pair of elements in `a`, surjects to one according to `merge`\.

 * Parameter: _a_  
   If null, does nothing\.
 * Parameter: _merge_  
   Can be null, in which case, all duplicate entries are erased\.
 * Order:  
   &#927;\(`a.size`\)




### <a id = "user-content-fn-8737e8e2" name = "user-content-fn-8737e8e2">&lt;T&gt;ArrayTest</a> ###

<code>static void <strong>&lt;T&gt;ArrayTest</strong>(void)</code>

Will be tested on stdout\. Requires `ARRAY_TEST`, `ARRAY_TO_STRING`, and not `NDEBUG` while defining `assert`\.



### <a id = "user-content-fn-e2e9ab81" name = "user-content-fn-e2e9ab81">&lt;T&gt;Array&lt;C&gt;ContrastTest</a> ###

<code>static void <strong>&lt;T&gt;Array&lt;C&gt;ContrastTest</strong>(void)</code>

Will be tested on stdout\. Requires `ARRAY_TEST`, `ARRAY_TO_STRING`, and not `NDEBUG` while defining `assert`\.





## <a id = "user-content-license" name = "user-content-license">License</a> ##

2016 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



