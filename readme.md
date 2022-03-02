# array\.h #

Stand\-alone header [src/array\.h](src/array.h); examples [test/test\_array\.c](test/test_array.c); if on a compatible workstation, `make` creates the test suite of the examples\.

## Contiguous dynamic array ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PA&gt;type](#user-content-typedef-a8a4b08a), [&lt;PAC&gt;box](#user-content-typedef-66d14de2), [&lt;PAC&gt;type](#user-content-typedef-b6e4909d), [&lt;PAC&gt;action_fn](#user-content-typedef-6f318a4), [&lt;PAC&gt;predicate_fn](#user-content-typedef-13605483), [&lt;PAC&gt;bipredicate_fn](#user-content-typedef-6cd1ff8a), [&lt;PAC&gt;compare_fn](#user-content-typedef-355f3451), [&lt;PAC&gt;biaction_fn](#user-content-typedef-a314f7fb), [&lt;PSZ&gt;to_string_fn](#user-content-typedef-8b890812)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;A&gt;array](#user-content-tag-8049be0d)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of array.](doc/array.png)

[&lt;A&gt;array](#user-content-tag-8049be0d) is a dynamic array that stores contiguous [&lt;PA&gt;type](#user-content-typedef-a8a4b08a)\. Resizing may be necessary when increasing the size of the array; this incurs amortised cost, and any pointers to this memory may become stale\.



 * Parameter: ARRAY\_NAME, ARRAY\_TYPE  
   `<A>` that satisfies `C` naming conventions when mangled and a valid tag\-type, [&lt;PA&gt;type](#user-content-typedef-a8a4b08a), associated therewith; required\. `<PA>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: ARRAY\_CODA  
   Include more functions contained in [src/array\_coda\.h](src/array_coda.h), where `<AC>` is `<A>array`\.
 * Parameter: ARRAY\_MIN\_CAPACITY  
   Default is 3; optional number in `[2, SIZE_MAX]` that the capacity can not go below\.
 * Parameter: ARRAY\_EXPECT\_TRAIT  
   Do not un\-define certain variables for subsequent inclusion in a parameterized trait\.
 * Parameter: ARRAY\_COMPARE\_NAME, ARRAY\_COMPARE, ARRAY\_IS\_EQUAL  
   Compare trait contained in [src/array\_coda\.h](src/array_coda.h)\. An optional mangled name for uniqueness and a function implementing either [&lt;PAC&gt;compare_fn](#user-content-typedef-355f3451) or [&lt;PAC&gt;bipredicate_fn](#user-content-typedef-6cd1ff8a)\.
 * Parameter: ARRAY\_TO\_STRING\_NAME, ARRAY\_TO\_STRING  
   To string trait contained in [src/to\_string\.h](src/to_string.h)\. An optional mangled name for uniqueness and function implementing [&lt;PSZ&gt;to_string_fn](#user-content-typedef-8b890812)\.
 * Standard:  
   C89


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-a8a4b08a" name = "user-content-typedef-a8a4b08a">&lt;PA&gt;type</a> ###

<code>typedef ARRAY_TYPE <strong>&lt;PA&gt;type</strong>;</code>

A valid tag type set by `ARRAY_TYPE`\.



### <a id = "user-content-typedef-66d14de2" name = "user-content-typedef-66d14de2">&lt;PAC&gt;box</a> ###

<code>typedef BOX_CONTAINER <strong>&lt;PAC&gt;box</strong>;</code>

[src/array\_coda\.h](src/array_coda.h): an alias to the box\.



### <a id = "user-content-typedef-b6e4909d" name = "user-content-typedef-b6e4909d">&lt;PAC&gt;type</a> ###

<code>typedef BOX_CONTENTS <strong>&lt;PAC&gt;type</strong>;</code>

[src/array\_coda\.h](src/array_coda.h): an alias to the individual type contained in the box\.



### <a id = "user-content-typedef-6f318a4" name = "user-content-typedef-6f318a4">&lt;PAC&gt;action_fn</a> ###

<code>typedef void(*<strong>&lt;PAC&gt;action_fn</strong>)(&lt;PAC&gt;type *);</code>

[src/array\_coda\.h](src/array_coda.h): Operates by side\-effects on [&lt;PAC&gt;type](#user-content-typedef-b6e4909d)\.



### <a id = "user-content-typedef-13605483" name = "user-content-typedef-13605483">&lt;PAC&gt;predicate_fn</a> ###

<code>typedef int(*<strong>&lt;PAC&gt;predicate_fn</strong>)(const &lt;PAC&gt;type *);</code>

[src/array\_coda\.h](src/array_coda.h): Returns a boolean given read\-only [&lt;PAC&gt;type](#user-content-typedef-b6e4909d)\.



### <a id = "user-content-typedef-6cd1ff8a" name = "user-content-typedef-6cd1ff8a">&lt;PAC&gt;bipredicate_fn</a> ###

<code>typedef int(*<strong>&lt;PAC&gt;bipredicate_fn</strong>)(const &lt;PAC&gt;type *, const &lt;PAC&gt;type *);</code>

[src/array\_coda\.h](src/array_coda.h): Returns a boolean given two read\-only [&lt;PAC&gt;type](#user-content-typedef-b6e4909d)\.



### <a id = "user-content-typedef-355f3451" name = "user-content-typedef-355f3451">&lt;PAC&gt;compare_fn</a> ###

<code>typedef int(*<strong>&lt;PAC&gt;compare_fn</strong>)(const &lt;PAC&gt;type *a, const &lt;PAC&gt;type *b);</code>

[src/array\_coda\.h](src/array_coda.h): Three\-way comparison on a totally order set of [&lt;PAC&gt;type](#user-content-typedef-b6e4909d); returns an integer value less then, equal to, greater then zero, if `a < b`, `a == b`, `a > b`, respectively\.



### <a id = "user-content-typedef-a314f7fb" name = "user-content-typedef-a314f7fb">&lt;PAC&gt;biaction_fn</a> ###

<code>typedef int(*<strong>&lt;PAC&gt;biaction_fn</strong>)(&lt;PAC&gt;type *, &lt;PAC&gt;type *);</code>

[src/array\_coda\.h](src/array_coda.h): Returns a boolean given two [&lt;PAC&gt;type](#user-content-typedef-b6e4909d)\.



### <a id = "user-content-typedef-8b890812" name = "user-content-typedef-8b890812">&lt;PSZ&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PSZ&gt;to_string_fn</strong>)(const &lt;PSZ&gt;type *, char(*)[12]);</code>

[to\_string\.h](to_string.h): responsible for turning the argument into a 12\-`char` null\-terminated output string\. `<PSZ>type` is contracted to be an internal iteration type of the box\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-8049be0d" name = "user-content-tag-8049be0d">&lt;A&gt;array</a> ###

<code>struct <strong>&lt;A&gt;array</strong> { &lt;PA&gt;type *data; size_t size, capacity; };</code>

Manages the array field `data` which has `size` elements\. The space is indexed up to `capacity`, which is at least `size`\. To initialize it to an idle state, see [&lt;A&gt;array](#user-content-fn-8049be0d), `ARRAY_IDLE`, `{0}` \(`C99`,\) or being `static`\. The fields should be treated as read\-only; any modification is liable to cause the array to go into an invalid state\.

![States.](doc/states.png)



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-8049be0d">&lt;A&gt;array</a></td><td>a</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-46169b16">&lt;A&gt;array_</a></td><td>a</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-fffa504a">&lt;A&gt;array_reserve</a></td><td>a, min</td></tr>

<tr><td align = right>static &lt;PA&gt;type *</td><td><a href = "#user-content-fn-8eb786c0">&lt;A&gt;array_buffer</a></td><td>a, n</td></tr>

<tr><td align = right>static &lt;PA&gt;type *</td><td><a href = "#user-content-fn-c0c02447">&lt;A&gt;array_insert</a></td><td>a, n, at</td></tr>

<tr><td align = right>static &lt;PA&gt;type *</td><td><a href = "#user-content-fn-1b423580">&lt;A&gt;array_new</a></td><td>a</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d5035752">&lt;A&gt;array_remove</a></td><td>a, datum</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d2a95b41">&lt;A&gt;array_lazy_remove</a></td><td>a, datum</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-e3f5267">&lt;A&gt;array_clear</a></td><td>a</td></tr>

<tr><td align = right>static &lt;PA&gt;type *</td><td><a href = "#user-content-fn-8af01fa1">&lt;A&gt;array_peek</a></td><td>a</td></tr>

<tr><td align = right>static &lt;PA&gt;type *</td><td><a href = "#user-content-fn-7bf4e995">&lt;A&gt;array_pop</a></td><td>a</td></tr>

<tr><td align = right>static &lt;PA&gt;type *</td><td><a href = "#user-content-fn-2d92b62">&lt;A&gt;array_append</a></td><td>a, n</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-bce1c326">&lt;A&gt;array_splice</a></td><td>a, b, i0, i1</td></tr>

<tr><td align = right>static &lt;PAC&gt;type *</td><td><a href = "#user-content-fn-b346708e">&lt;AC&gt;previous</a></td><td>box, x</td></tr>

<tr><td align = right>static &lt;PAC&gt;type *</td><td><a href = "#user-content-fn-60e57f42">&lt;AC&gt;next</a></td><td>box, x</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-86c2328d">&lt;AC&gt;clip</a></td><td>box, i</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-cf7c3074">&lt;AC&gt;copy_if</a></td><td>a, copy, b</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-a6bb890c">&lt;AC&gt;keep_if</a></td><td>box, keep, destruct</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-7924b33">&lt;AC&gt;trim</a></td><td>box, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-884c09a">&lt;AC&gt;each</a></td><td>box, action</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d0650740">&lt;AC&gt;if_each</a></td><td>box, predicate, action</td></tr>

<tr><td align = right>static const &lt;PAC&gt;type *</td><td><a href = "#user-content-fn-ed02e52b">&lt;AC&gt;any</a></td><td>box, predicate</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-a4e64b3d">&lt;ACC&gt;compare</a></td><td>a, b</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-5214478c">&lt;ACC&gt;lower_bound</a></td><td>box, value</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-aa46e23d">&lt;ACC&gt;upper_bound</a></td><td>box, value</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-4789a122">&lt;ACC&gt;insert_after</a></td><td>box, value</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-9d49726">&lt;ACC&gt;sort</a></td><td>box</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-1cd40178">&lt;ACC&gt;reverse</a></td><td>box</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-92b388f5">&lt;ACC&gt;is_equal</a></td><td>a, b</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-30137672">&lt;ACC&gt;unique_merge</a></td><td>box, merge</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-367663f1">&lt;ACC&gt;unique</a></td><td>a</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-b11709d3">&lt;SZ&gt;to_string</a></td><td>box</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-8049be0d" name = "user-content-fn-8049be0d">&lt;A&gt;array</a> ###

<code>static void <strong>&lt;A&gt;array</strong>(struct &lt;A&gt;array *const <em>a</em>)</code>

Initialises `a` to idle\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-46169b16" name = "user-content-fn-46169b16">&lt;A&gt;array_</a> ###

<code>static void <strong>&lt;A&gt;array_</strong>(struct &lt;A&gt;array *const <em>a</em>)</code>

Destroys `a` and returns it to idle\.



### <a id = "user-content-fn-fffa504a" name = "user-content-fn-fffa504a">&lt;A&gt;array_reserve</a> ###

<code>static int <strong>&lt;A&gt;array_reserve</strong>(struct &lt;A&gt;array *const <em>a</em>, const size_t <em>min</em>)</code>

Ensures `min` capacity of `a`\. Invalidates pointers in `a`\.

 * Parameter: _min_  
   If zero, does nothing\.
 * Return:  
   Success; otherwise, `errno` will be set\.
 * Exceptional return: ERANGE  
   Tried allocating more then can fit in `size_t` or `realloc` doesn't follow POSIX\.
 * Exceptional return: realloc  




### <a id = "user-content-fn-8eb786c0" name = "user-content-fn-8eb786c0">&lt;A&gt;array_buffer</a> ###

<code>static &lt;PA&gt;type *<strong>&lt;A&gt;array_buffer</strong>(struct &lt;A&gt;array *const <em>a</em>, const size_t <em>n</em>)</code>

The capacity of `a` will be increased to at least `n` elements beyond the size\. Invalidates any pointers in `a`\.

 * Return:  
   The start of the buffered space at the back of the array\. If `a` is idle and `buffer` is zero, a null pointer is returned, otherwise null indicates an error\.
 * Exceptional return: realloc, ERANGE  




### <a id = "user-content-fn-c0c02447" name = "user-content-fn-c0c02447">&lt;A&gt;array_insert</a> ###

<code>static &lt;PA&gt;type *<strong>&lt;A&gt;array_insert</strong>(struct &lt;A&gt;array *const <em>a</em>, const size_t <em>n</em>, const size_t <em>at</em>)</code>

Adds `n` un\-initialised elements at position `at` in `a`\. The buffer holds enough elements or it will invalidate any pointers in `a`\.

 * Parameter: _at_  
   A number smaller than or equal to `a.size`; if `a.size`, this function behaves as [&lt;A&gt;array_append](#user-content-fn-2d92b62)\.
 * Return:  
   A pointer to the start of the new region, where there are `n` elements\.
 * Exceptional return: realloc, ERANGE  




### <a id = "user-content-fn-1b423580" name = "user-content-fn-1b423580">&lt;A&gt;array_new</a> ###

<code>static &lt;PA&gt;type *<strong>&lt;A&gt;array_new</strong>(struct &lt;A&gt;array *const <em>a</em>)</code>

 * Return:  
   Adds \(push back\) one new element of `a`\. The buffer holds an element or it will invalidate pointers in `a`\.
 * Exceptional return: realloc, ERANGE  
 * Order:  
   amortised &#927;\(1\)




### <a id = "user-content-fn-d5035752" name = "user-content-fn-d5035752">&lt;A&gt;array_remove</a> ###

<code>static void <strong>&lt;A&gt;array_remove</strong>(struct &lt;A&gt;array *const <em>a</em>, &lt;PA&gt;type *const <em>datum</em>)</code>

Removes `datum` from `a`\.

 * Order:  
   &#927;\(`a.size`\)\.




### <a id = "user-content-fn-d2a95b41" name = "user-content-fn-d2a95b41">&lt;A&gt;array_lazy_remove</a> ###

<code>static void <strong>&lt;A&gt;array_lazy_remove</strong>(struct &lt;A&gt;array *const <em>a</em>, &lt;PA&gt;type *const <em>datum</em>)</code>

Removes `datum` from `a` and replaces it with the tail\.

 * Order:  
   &#927;\(1\)\.




### <a id = "user-content-fn-e3f5267" name = "user-content-fn-e3f5267">&lt;A&gt;array_clear</a> ###

<code>static void <strong>&lt;A&gt;array_clear</strong>(struct &lt;A&gt;array *const <em>a</em>)</code>

Sets `a` to be empty\. That is, the size of `a` will be zero, but if it was previously in an active non\-idle state, it continues to be\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-8af01fa1" name = "user-content-fn-8af01fa1">&lt;A&gt;array_peek</a> ###

<code>static &lt;PA&gt;type *<strong>&lt;A&gt;array_peek</strong>(const struct &lt;A&gt;array *const <em>a</em>)</code>

 * Return:  
   The last element or null if `a` is empty\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-7bf4e995" name = "user-content-fn-7bf4e995">&lt;A&gt;array_pop</a> ###

<code>static &lt;PA&gt;type *<strong>&lt;A&gt;array_pop</strong>(struct &lt;A&gt;array *const <em>a</em>)</code>

 * Return:  
   Value from the the top of `a` that is removed or null if the array is empty\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-2d92b62" name = "user-content-fn-2d92b62">&lt;A&gt;array_append</a> ###

<code>static &lt;PA&gt;type *<strong>&lt;A&gt;array_append</strong>(struct &lt;A&gt;array *const <em>a</em>, const size_t <em>n</em>)</code>

Adds `n` elements to the back of `a`\. It will invalidate pointers in `a` if `n` is greater than the buffer space\.

 * Return:  
   A pointer to the elements\. If `a` is idle and `n` is zero, a null pointer will be returned, otherwise null indicates an error\.
 * Exceptional return: realloc, ERANGE  




### <a id = "user-content-fn-bce1c326" name = "user-content-fn-bce1c326">&lt;A&gt;array_splice</a> ###

<code>static int <strong>&lt;A&gt;array_splice</strong>(struct &lt;A&gt;array *const <em>a</em>, const struct &lt;A&gt;array *const <em>b</em>, const size_t <em>i0</em>, const size_t <em>i1</em>)</code>

Indices \[`i0`, `i1`\) of `a` will be replaced with a copy of `b`\.

 * Parameter: _b_  
   Can be null, which acts as empty, but cannot be `a`\.
 * Return:  
   Success\.
 * Exceptional return: realloc, ERANGE  




### <a id = "user-content-fn-b346708e" name = "user-content-fn-b346708e">&lt;AC&gt;previous</a> ###

<code>static &lt;PAC&gt;type *<strong>&lt;AC&gt;previous</strong>(const &lt;PAC&gt;box *const <em>box</em>, const &lt;PAC&gt;type *const <em>x</em>)</code>

[src/array\_coda\.h](src/array_coda.h)

 * Parameter: _x_  
   A valid entry or null to start from the last\.
 * Return:  
   The previous valid entry from `box` \(which could be null\) or null if this was the first\.




### <a id = "user-content-fn-60e57f42" name = "user-content-fn-60e57f42">&lt;AC&gt;next</a> ###

<code>static &lt;PAC&gt;type *<strong>&lt;AC&gt;next</strong>(const &lt;PAC&gt;box *const <em>box</em>, const &lt;PAC&gt;type *const <em>x</em>)</code>

[src/array\_coda\.h](src/array_coda.h)

 * Parameter: _x_  
   A valid entry or null to start from the first\.
 * Return:  
   The next valid entry from `box` \(which could be null\) or null if this was the last\.




### <a id = "user-content-fn-86c2328d" name = "user-content-fn-86c2328d">&lt;AC&gt;clip</a> ###

<code>static size_t <strong>&lt;AC&gt;clip</strong>(const &lt;PAC&gt;box *const <em>box</em>, const long <em>i</em>)</code>

[src/array\_coda\.h](src/array_coda.h)

 * Return:  
   Converts `i` to an index in `box` from \[0, `box.size`\]\. Negative values are wrapped\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-cf7c3074" name = "user-content-fn-cf7c3074">&lt;AC&gt;copy_if</a> ###

<code>static int <strong>&lt;AC&gt;copy_if</strong>(&lt;PAC&gt;box *const <em>a</em>, const &lt;PAC&gt;predicate_fn <em>copy</em>, const &lt;PAC&gt;box *const <em>b</em>)</code>

[src/array\_coda\.h](src/array_coda.h): For all elements of `b`, calls `copy`, and if true, lazily copies the elements to `a`\. `a` and `b` can not be the same but `b` can be null, \(in which case, it does nothing\.\)

 * Exceptional return: ERANGE, realloc  
 * Order:  
   &#927;\(`b.size` &#215; `copy`\)




### <a id = "user-content-fn-a6bb890c" name = "user-content-fn-a6bb890c">&lt;AC&gt;keep_if</a> ###

<code>static void <strong>&lt;AC&gt;keep_if</strong>(&lt;PAC&gt;box *const <em>box</em>, const &lt;PAC&gt;predicate_fn <em>keep</em>, const &lt;PAC&gt;action_fn <em>destruct</em>)</code>

[src/array\_coda\.h](src/array_coda.h): For all elements of `box`, calls `keep`, and if false, lazy deletes that item, calling `destruct` \(if not\-null\)\.

 * Order:  
   &#927;\(`a.size` &#215; `keep` &#215; `destruct`\)




### <a id = "user-content-fn-7924b33" name = "user-content-fn-7924b33">&lt;AC&gt;trim</a> ###

<code>static void <strong>&lt;AC&gt;trim</strong>(&lt;PAC&gt;box *const <em>box</em>, const &lt;PAC&gt;predicate_fn <em>predicate</em>)</code>

[src/array\_coda\.h](src/array_coda.h): Removes at either end of `box` of things that `predicate` returns true\.

 * Order:  
   &#927;\(`box.size` &#215; `predicate`\)




### <a id = "user-content-fn-884c09a" name = "user-content-fn-884c09a">&lt;AC&gt;each</a> ###

<code>static void <strong>&lt;AC&gt;each</strong>(&lt;PAC&gt;box *const <em>box</em>, const &lt;PAC&gt;action_fn <em>action</em>)</code>

[src/array\_coda\.h](src/array_coda.h): Iterates through `box` and calls `action` on all the elements\. The topology of the list should not change while in this function\.

 * Order:  
   &#927;\(`box.size` &#215; `action`\)




### <a id = "user-content-fn-d0650740" name = "user-content-fn-d0650740">&lt;AC&gt;if_each</a> ###

<code>static void <strong>&lt;AC&gt;if_each</strong>(&lt;PAC&gt;box *const <em>box</em>, const &lt;PAC&gt;predicate_fn <em>predicate</em>, const &lt;PAC&gt;action_fn <em>action</em>)</code>

[src/array\_coda\.h](src/array_coda.h): Iterates through `box` and calls `action` on all the elements for which `predicate` returns true\. The topology of the list should not change while in this function\.

 * Order:  
   &#927;\(`box.size` &#215; `predicate` &#215; `action`\)




### <a id = "user-content-fn-ed02e52b" name = "user-content-fn-ed02e52b">&lt;AC&gt;any</a> ###

<code>static const &lt;PAC&gt;type *<strong>&lt;AC&gt;any</strong>(const &lt;PAC&gt;box *const <em>box</em>, const &lt;PAC&gt;predicate_fn <em>predicate</em>)</code>

[src/array\_coda\.h](src/array_coda.h): Iterates through `box` and calls `predicate` until it returns true\.

 * Return:  
   The first `predicate` that returned true, or, if the statement is false on all, null\.
 * Order:  
   &#927;\(`box.size` &#215; `predicate`\)




### <a id = "user-content-fn-a4e64b3d" name = "user-content-fn-a4e64b3d">&lt;ACC&gt;compare</a> ###

<code>static int <strong>&lt;ACC&gt;compare</strong>(const &lt;PAC&gt;box *const <em>a</em>, const &lt;PAC&gt;box *const <em>b</em>)</code>

[src/array\_coda\.h](src/array_coda.h): Lexicographically compares `a` to `b`\. Both can be null, with null values before everything\.

 * Return:  
   `a < b`: negative; `a == b`: zero; `a > b`: positive\.
 * Order:  
   &#927;\(`a.size`\)




### <a id = "user-content-fn-5214478c" name = "user-content-fn-5214478c">&lt;ACC&gt;lower_bound</a> ###

<code>static size_t <strong>&lt;ACC&gt;lower_bound</strong>(const &lt;PAC&gt;box *const <em>box</em>, const &lt;PAC&gt;type *const <em>value</em>)</code>

[src/array\_coda\.h](src/array_coda.h): `box` should be partitioned true/false with less\-then `value`\.

 * Return:  
   The first index of `a` that is not less than `value`\.
 * Order:  
   &#927;\(log `a.size`\)




### <a id = "user-content-fn-aa46e23d" name = "user-content-fn-aa46e23d">&lt;ACC&gt;upper_bound</a> ###

<code>static size_t <strong>&lt;ACC&gt;upper_bound</strong>(const &lt;PAC&gt;box *const <em>box</em>, const &lt;PAC&gt;type *const <em>value</em>)</code>

[src/array\_coda\.h](src/array_coda.h): `box` should be partitioned false/true with greater\-than or equal\-to [&lt;PAC&gt;type](#user-content-typedef-b6e4909d) `value`\.

 * Return:  
   The first index of `box` that is greater than `value`\.
 * Order:  
   &#927;\(log `a.size`\)




### <a id = "user-content-fn-4789a122" name = "user-content-fn-4789a122">&lt;ACC&gt;insert_after</a> ###

<code>static int <strong>&lt;ACC&gt;insert_after</strong>(&lt;PAC&gt;box *const <em>box</em>, const &lt;PAC&gt;type *const <em>value</em>)</code>

[src/array\_coda\.h](src/array_coda.h): Copies `value` at the upper bound of a sorted `box`\.

 * Return:  
   Success\.
 * Exceptional return: realloc, ERANGE  
 * Order:  
   &#927;\(`a.size`\)




### <a id = "user-content-fn-9d49726" name = "user-content-fn-9d49726">&lt;ACC&gt;sort</a> ###

<code>static void <strong>&lt;ACC&gt;sort</strong>(&lt;PAC&gt;box *const <em>box</em>)</code>

[src/array\_coda\.h](src/array_coda.h): Sorts `box` by `qsort`\.

 * Order:  
   &#927;\(`a.size` \\log `box.size`\)




### <a id = "user-content-fn-1cd40178" name = "user-content-fn-1cd40178">&lt;ACC&gt;reverse</a> ###

<code>static void <strong>&lt;ACC&gt;reverse</strong>(&lt;PAC&gt;box *const <em>box</em>)</code>

[src/array\_coda\.h](src/array_coda.h): Sorts `box` in reverse by `qsort`\.

 * Order:  
   &#927;\(`a.size` \\log `a.size`\)




### <a id = "user-content-fn-92b388f5" name = "user-content-fn-92b388f5">&lt;ACC&gt;is_equal</a> ###

<code>static int <strong>&lt;ACC&gt;is_equal</strong>(const &lt;PAC&gt;box *const <em>a</em>, const &lt;PAC&gt;box *const <em>b</em>)</code>

[src/array\_coda\.h](src/array_coda.h)

 * Return:  
   If `a` piecewise equals `b`, which both can be null\.
 * Order:  
   &#927;\(`size`\)




### <a id = "user-content-fn-30137672" name = "user-content-fn-30137672">&lt;ACC&gt;unique_merge</a> ###

<code>static void <strong>&lt;ACC&gt;unique_merge</strong>(&lt;PAC&gt;box *const <em>box</em>, const &lt;PAC&gt;biaction_fn <em>merge</em>)</code>

[src/array\_coda\.h](src/array_coda.h): Removes consecutive duplicate elements in `box`\.

 * Parameter: _merge_  
   Controls surjection\. Called with duplicate elements, if false `(x, y)->(x)`, if true `(x,y)->(y)`\. More complex functions, `(x, y)->(x+y)` can be simulated by mixing the two in the value returned\. Can be null: behaves like false\.
 * Order:  
   &#927;\(`a.size` &#215; `merge`\)




### <a id = "user-content-fn-367663f1" name = "user-content-fn-367663f1">&lt;ACC&gt;unique</a> ###

<code>static void <strong>&lt;ACC&gt;unique</strong>(&lt;PAC&gt;box *const <em>a</em>)</code>

[src/array\_coda\.h](src/array_coda.h): Removes consecutive duplicate elements in `a`\.

 * Order:  
   &#927;\(`a.size`\)




### <a id = "user-content-fn-b11709d3" name = "user-content-fn-b11709d3">&lt;SZ&gt;to_string</a> ###

<code>static const char *<strong>&lt;SZ&gt;to_string</strong>(const &lt;PSZ&gt;box *const <em>box</em>)</code>

[src/to\_string\.h](src/to_string.h): print the contents of `box` in a static string buffer of 256 bytes, with limitations of only printing 4 things at a time\. `<PSZ>box` is contracted to be the box itself\. `<SZ>` is loosely contracted to be a name `<X>box[<X_TO_STRING_NAME>]`\.

 * Return:  
   Address of the static buffer\.
 * Order:  
   &#920;\(1\)






## <a id = "user-content-license" name = "user-content-license">License</a> ##

2016 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



