# array\.h #

## Contiguous dynamic array ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PA&gt;type](#user-content-typedef-a8a4b08a), [&lt;PCG&gt;box](#user-content-typedef-9c8158f8), [&lt;PCG&gt;type](#user-content-typedef-1c7f487f), [&lt;PCG&gt;action_fn](#user-content-typedef-d8b6d30a), [&lt;PCG&gt;predicate_fn](#user-content-typedef-dfee9029), [&lt;PA&gt;action_fn](#user-content-typedef-b531bc05), [&lt;PSZ&gt;to_string_fn](#user-content-typedef-8b890812), [&lt;PCM&gt;box](#user-content-typedef-ec6edbaa), [&lt;PCM&gt;type](#user-content-typedef-cee32005), [&lt;PCM&gt;bipredicate_fn](#user-content-typedef-ea6988c2), [&lt;PCM&gt;biaction_fn](#user-content-typedef-6f7f0563), [&lt;PCM&gt;compare_fn](#user-content-typedef-64a034e9)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;A&gt;array](#user-content-tag-8049be0d), [&lt;PA&gt;iterator](#user-content-tag-e6ddd8f0)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of array.](web/array.png)

[&lt;A&gt;array](#user-content-tag-8049be0d) is a dynamic array that stores contiguous [&lt;PA&gt;type](#user-content-typedef-a8a4b08a)\. Resizing may be necessary when increasing the size of the array; this incurs amortised cost, and any pointers to this memory may become stale\.



 * Parameter: ARRAY\_NAME, ARRAY\_TYPE  
   `<A>` that satisfies `C` naming conventions when mangled and a valid tag\-type, [&lt;PA&gt;type](#user-content-typedef-a8a4b08a), associated therewith; required\. `<PA>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: ARRAY\_CONTIGUOUS  
   Include the singleton trait contained in [contiguous\.h](contiguous.h) that takes no options\. Rational for the design decision to make it a singleton class, instead of including it always, is `array` is used as a backing for other boxes; it would just increase compilation in most cases\.
 * Parameter: ARRAY\_MIN\_CAPACITY  
   Default is 3; optional number in `[2, SIZE_MAX]` that the capacity can not go below\.
 * Parameter: ARRAY\_TEST  
   Optional function implementing [&lt;PA&gt;action_fn](#user-content-typedef-b531bc05) that fills the [&lt;PA&gt;type](#user-content-typedef-a8a4b08a) from uninitialized to random for unit testing framework using `assert`\. Testing array contained in [\.\./test/test\_array\.h](../test/test_array.h)\. Must have at least one `to_string` trait included, and any tests of traits must come before `to_string`\.
 * Parameter: ARRAY\_EXPECT\_TRAIT  
   Do not un\-define certain variables for subsequent inclusion in a parameterized trait\.
 * Parameter: ARRAY\_COMPARE\_NAME, ARRAY\_COMPARE, ARRAY\_IS\_EQUAL  
   Compare trait contained in [compare\.h](compare.h)\. An optional mangled name for uniqueness and a function implementing [&lt;PCM&gt;compare_fn](#user-content-typedef-64a034e9) xor [&lt;PCM&gt;bipredicate_fn](#user-content-typedef-ea6988c2)\.
 * Parameter: ARRAY\_TO\_STRING\_NAME, ARRAY\_TO\_STRING  
   To string trait contained in [to\_string\.h](to_string.h)\. An optional mangled name for uniqueness and function implementing [&lt;PSZ&gt;to_string_fn](#user-content-typedef-8b890812)\.
 * Standard:  
   C89


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-a8a4b08a" name = "user-content-typedef-a8a4b08a">&lt;PA&gt;type</a> ###

<code>typedef ARRAY_TYPE <strong>&lt;PA&gt;type</strong>;</code>

A valid tag type set by `ARRAY_TYPE`\.



### <a id = "user-content-typedef-9c8158f8" name = "user-content-typedef-9c8158f8">&lt;PCG&gt;box</a> ###

<code>typedef BOX_CONTAINER <strong>&lt;PCG&gt;box</strong>;</code>

In [contiguous\.h](contiguous.h), an alias to the box\.



### <a id = "user-content-typedef-1c7f487f" name = "user-content-typedef-1c7f487f">&lt;PCG&gt;type</a> ###

<code>typedef BOX_CONTENTS <strong>&lt;PCG&gt;type</strong>;</code>

In [contiguous\.h](contiguous.h), an alias to the individual type contained in the box\.



### <a id = "user-content-typedef-d8b6d30a" name = "user-content-typedef-d8b6d30a">&lt;PCG&gt;action_fn</a> ###

<code>typedef void(*<strong>&lt;PCG&gt;action_fn</strong>)(&lt;PCG&gt;type *);</code>

Operates by side\-effects on [&lt;PCG&gt;type](#user-content-typedef-1c7f487f)\.



### <a id = "user-content-typedef-dfee9029" name = "user-content-typedef-dfee9029">&lt;PCG&gt;predicate_fn</a> ###

<code>typedef int(*<strong>&lt;PCG&gt;predicate_fn</strong>)(const &lt;PCG&gt;type *);</code>

Returns a boolean given read\-only [&lt;PCG&gt;type](#user-content-typedef-1c7f487f)\.



### <a id = "user-content-typedef-b531bc05" name = "user-content-typedef-b531bc05">&lt;PA&gt;action_fn</a> ###

<code>typedef void(*<strong>&lt;PA&gt;action_fn</strong>)(&lt;PA&gt;type *);</code>

[test\_array\.h](test_array.h): operates by side\-effects on [&lt;PA&gt;type](#user-content-typedef-a8a4b08a)\.



### <a id = "user-content-typedef-8b890812" name = "user-content-typedef-8b890812">&lt;PSZ&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PSZ&gt;to_string_fn</strong>)(const &lt;PSZ&gt;type *, char(*)[12]);</code>

Responsible for turning the first argument into a 12\-`char` null\-terminated output string\.



### <a id = "user-content-typedef-ec6edbaa" name = "user-content-typedef-ec6edbaa">&lt;PCM&gt;box</a> ###

<code>typedef BOX_CONTAINER <strong>&lt;PCM&gt;box</strong>;</code>

[compare\.h](compare.h): an alias to the box\.



### <a id = "user-content-typedef-cee32005" name = "user-content-typedef-cee32005">&lt;PCM&gt;type</a> ###

<code>typedef BOX_CONTENTS <strong>&lt;PCM&gt;type</strong>;</code>

[compare\.h](compare.h): an alias to the individual type contained in the box\.



### <a id = "user-content-typedef-ea6988c2" name = "user-content-typedef-ea6988c2">&lt;PCM&gt;bipredicate_fn</a> ###

<code>typedef int(*<strong>&lt;PCM&gt;bipredicate_fn</strong>)(const &lt;PCM&gt;type *, const &lt;PCM&gt;type *);</code>

[compare\.h](compare.h): returns a boolean given two read\-only [&lt;PCM&gt;type](#user-content-typedef-cee32005)\.



### <a id = "user-content-typedef-6f7f0563" name = "user-content-typedef-6f7f0563">&lt;PCM&gt;biaction_fn</a> ###

<code>typedef int(*<strong>&lt;PCM&gt;biaction_fn</strong>)(&lt;PCM&gt;type *, &lt;PCM&gt;type *);</code>

[compare\.h](compare.h): returns a boolean given two [&lt;PCM&gt;type](#user-content-typedef-cee32005)\.



### <a id = "user-content-typedef-64a034e9" name = "user-content-typedef-64a034e9">&lt;PCM&gt;compare_fn</a> ###

<code>typedef int(*<strong>&lt;PCM&gt;compare_fn</strong>)(const &lt;PCM&gt;type *a, const &lt;PCM&gt;type *b);</code>

[compare\.h](compare.h): three\-way comparison on a totally order set; returns an integer value less then, equal to, greater then zero, if `a < b`, `a == b`, `a > b`, respectively\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-8049be0d" name = "user-content-tag-8049be0d">&lt;A&gt;array</a> ###

<code>struct <strong>&lt;A&gt;array</strong> { &lt;PA&gt;type *data; size_t size, capacity; };</code>

Manages the array field `data` which has `size` elements\. The space is indexed up to `capacity`, which is at least `size`\. To initialize it to an idle state, see [&lt;A&gt;array](#user-content-fn-8049be0d), `ARRAY_IDLE`, `{0}` \(`C99`,\) or being `static`\.

![States.](web/states.png)



### <a id = "user-content-tag-e6ddd8f0" name = "user-content-tag-e6ddd8f0">&lt;PA&gt;iterator</a> ###

<code>struct <strong>&lt;PA&gt;iterator</strong>;</code>

Contains all iteration parameters\.



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-8049be0d">&lt;A&gt;array</a></td><td>a</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-46169b16">&lt;A&gt;array_</a></td><td>a</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-fffa504a">&lt;A&gt;array_reserve</a></td><td>a, min</td></tr>

<tr><td align = right>static &lt;PA&gt;type *</td><td><a href = "#user-content-fn-8eb786c0">&lt;A&gt;array_buffer</a></td><td>a, n</td></tr>

<tr><td align = right>static &lt;PA&gt;type *</td><td><a href = "#user-content-fn-2d92b62">&lt;A&gt;array_append</a></td><td>a, n</td></tr>

<tr><td align = right>static &lt;PA&gt;type *</td><td><a href = "#user-content-fn-bc954312">&lt;A&gt;array_append_at</a></td><td>a, n, at</td></tr>

<tr><td align = right>static &lt;PA&gt;type *</td><td><a href = "#user-content-fn-1b423580">&lt;A&gt;array_new</a></td><td>a</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d5035752">&lt;A&gt;array_remove</a></td><td>a, datum</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d2a95b41">&lt;A&gt;array_lazy_remove</a></td><td>a, datum</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-e3f5267">&lt;A&gt;array_clear</a></td><td>a</td></tr>

<tr><td align = right>static &lt;PA&gt;type *</td><td><a href = "#user-content-fn-8af01fa1">&lt;A&gt;array_peek</a></td><td>a</td></tr>

<tr><td align = right>static &lt;PA&gt;type *</td><td><a href = "#user-content-fn-7bf4e995">&lt;A&gt;array_pop</a></td><td>a</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-8025f997">&lt;CG&gt;clip</a></td><td>box, i</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-7c7e919e">&lt;CG&gt;copy_if</a></td><td>a, copy, b</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-b95adf62">&lt;CG&gt;keep_if</a></td><td>box, keep, destruct</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-672d297d">&lt;CG&gt;trim</a></td><td>box, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-2869fa64">&lt;CG&gt;each</a></td><td>box, action</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-cfdb8e2e">&lt;CG&gt;if_each</a></td><td>box, predicate, action</td></tr>

<tr><td align = right>static const &lt;PCG&gt;type *</td><td><a href = "#user-content-fn-37394af1">&lt;CG&gt;any</a></td><td>box, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-c5d3559e">&lt;A&gt;array_test</a></td><td></td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-73575483">&lt;CM&gt;compare_test</a></td><td></td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-b11709d3">&lt;SZ&gt;to_string</a></td><td>box</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-7e7ebd12">&lt;CM&gt;compare</a></td><td>a, b</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-62df0883">&lt;CM&gt;lower_bound</a></td><td>a, value</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-179bac56">&lt;CM&gt;upper_bound</a></td><td>a, value</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-bfb5a80f">&lt;CM&gt;insert_after</a></td><td>a, datum</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-3e4620eb">&lt;CM&gt;sort</a></td><td>a</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-2022b037">&lt;CM&gt;reverse</a></td><td>a</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-30c13ca0">&lt;CM&gt;is_equal</a></td><td>a, b</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-a92a161f">&lt;CM&gt;unique_merge</a></td><td>a, merge</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-f17606f0">&lt;CM&gt;unique</a></td><td>a</td></tr>

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




### <a id = "user-content-fn-2d92b62" name = "user-content-fn-2d92b62">&lt;A&gt;array_append</a> ###

<code>static &lt;PA&gt;type *<strong>&lt;A&gt;array_append</strong>(struct &lt;A&gt;array *const <em>a</em>, const size_t <em>n</em>)</code>

Adds `n` elements to the back of `a`\. It will invalidate pointers in `a` if `n` is greater than the buffer space\.

 * Return:  
   A pointer to the elements\. If `a` is idle and `n` is zero, a null pointer will be returned, otherwise null indicates an error\.
 * Exceptional return: realloc, ERANGE  




### <a id = "user-content-fn-bc954312" name = "user-content-fn-bc954312">&lt;A&gt;array_append_at</a> ###

<code>static &lt;PA&gt;type *<strong>&lt;A&gt;array_append_at</strong>(struct &lt;A&gt;array *const <em>a</em>, const size_t <em>n</em>, const size_t <em>at</em>)</code>

Adds `n` un\-initialised elements at position `at` in `a`\. The buffer holds enough elements or it will invalidate pointers in `a`\.

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




### <a id = "user-content-fn-8025f997" name = "user-content-fn-8025f997">&lt;CG&gt;clip</a> ###

<code>static size_t <strong>&lt;CG&gt;clip</strong>(const &lt;PCG&gt;box *const <em>box</em>, const long <em>i</em>)</code>

 * Return:  
   Converts `i` to an index in `box` from \[0, `a.size`\]\. Negative values are implicitly plus `box.size`\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-7c7e919e" name = "user-content-fn-7c7e919e">&lt;CG&gt;copy_if</a> ###

<code>static int <strong>&lt;CG&gt;copy_if</strong>(&lt;PCG&gt;box *const <em>a</em>, const &lt;PCG&gt;predicate_fn <em>copy</em>, const &lt;PCG&gt;box *const <em>b</em>)</code>

For all elements of `b`, calls `copy`, and if true, lazily copies the elements to `a`\. `a` and `b` can not be the same but `b` can be null, \(in which case, it does nothing\.\)

 * Exceptional return: ERANGE, realloc  
 * Order:  
   &#927;\(`b.size` &#215; `copy`\)




### <a id = "user-content-fn-b95adf62" name = "user-content-fn-b95adf62">&lt;CG&gt;keep_if</a> ###

<code>static void <strong>&lt;CG&gt;keep_if</strong>(&lt;PCG&gt;box *const <em>box</em>, const &lt;PCG&gt;predicate_fn <em>keep</em>, const &lt;PCG&gt;action_fn <em>destruct</em>)</code>

For all elements of `box`, calls `keep`, and if false, lazy deletes that item, calling `destruct` if not\-null\.

 * Order:  
   &#927;\(`a.size` &#215; `keep` &#215; `destruct`\)




### <a id = "user-content-fn-672d297d" name = "user-content-fn-672d297d">&lt;CG&gt;trim</a> ###

<code>static void <strong>&lt;CG&gt;trim</strong>(&lt;PCG&gt;box *const <em>box</em>, const &lt;PCG&gt;predicate_fn <em>predicate</em>)</code>

Removes at either end of `box` of things that `predicate` returns true\.

 * Order:  
   &#927;\(`box.size` &#215; `predicate`\)




### <a id = "user-content-fn-2869fa64" name = "user-content-fn-2869fa64">&lt;CG&gt;each</a> ###

<code>static void <strong>&lt;CG&gt;each</strong>(&lt;PCG&gt;box *const <em>box</em>, const &lt;PCG&gt;action_fn <em>action</em>)</code>

Iterates through `box` and calls `action` on all the elements\. The topology of the list should not change while in this function\.

 * Order:  
   &#927;\(`box.size` &#215; `action`\)




### <a id = "user-content-fn-cfdb8e2e" name = "user-content-fn-cfdb8e2e">&lt;CG&gt;if_each</a> ###

<code>static void <strong>&lt;CG&gt;if_each</strong>(&lt;PCG&gt;box *const <em>box</em>, const &lt;PCG&gt;predicate_fn <em>predicate</em>, const &lt;PCG&gt;action_fn <em>action</em>)</code>

Iterates through `box` and calls `action` on all the elements for which `predicate` returns true\. The topology of the list should not change while in this function\.

 * Order:  
   &#927;\(`box.size` &#215; `predicate` &#215; `action`\)




### <a id = "user-content-fn-37394af1" name = "user-content-fn-37394af1">&lt;CG&gt;any</a> ###

<code>static const &lt;PCG&gt;type *<strong>&lt;CG&gt;any</strong>(const &lt;PCG&gt;box *const <em>box</em>, const &lt;PCG&gt;predicate_fn <em>predicate</em>)</code>

Requires iterate interface\. Iterates through `box` and calls `predicate` until it returns true\.

 * Return:  
   The first `predicate` that returned true, or, if the statement is false on all, null\.
 * Order:  
   &#927;\(`box.size` &#215; `predicate`\)




### <a id = "user-content-fn-c5d3559e" name = "user-content-fn-c5d3559e">&lt;A&gt;array_test</a> ###

<code>static void <strong>&lt;A&gt;array_test</strong>(void)</code>

`ARRAY_TEST`, `ARRAY_TO_STRING`, \!`NDEBUG`: will be tested on stdout\.



### <a id = "user-content-fn-73575483" name = "user-content-fn-73575483">&lt;CM&gt;compare_test</a> ###

<code>static void <strong>&lt;CM&gt;compare_test</strong>(void)</code>

`ARRAY_TEST`, `ARRAY_COMPARE` \-> `ARRAY_TO_STRING`, \!`NDEBUG`: will be tested on stdout\.



### <a id = "user-content-fn-b11709d3" name = "user-content-fn-b11709d3">&lt;SZ&gt;to_string</a> ###

<code>static const char *<strong>&lt;SZ&gt;to_string</strong>(const &lt;PSZ&gt;box *const <em>box</em>)</code>

 * Return:  
   Print the contents of `box` in a static string buffer of 256 bytes with limitations of only printing 4 things at a time\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-7e7ebd12" name = "user-content-fn-7e7ebd12">&lt;CM&gt;compare</a> ###

<code>static int <strong>&lt;CM&gt;compare</strong>(const &lt;PCM&gt;box *const <em>a</em>, const &lt;PCM&gt;box *const <em>b</em>)</code>

[compare\.h](compare.h): lexicographically compares `a` to `b`\. Null values are before everything\.

 * Return:  
   `a < b`: negative; `a == b`: zero; `a > b`: positive\.
 * Order:  
   &#927;\(`a.size`\)




### <a id = "user-content-fn-62df0883" name = "user-content-fn-62df0883">&lt;CM&gt;lower_bound</a> ###

<code>static size_t <strong>&lt;CM&gt;lower_bound</strong>(const &lt;PCM&gt;box *const <em>a</em>, const &lt;PCM&gt;type *const <em>value</em>)</code>

[compare\.h](compare.h): `a` should be partitioned true/false with less\-then `value`\.

 * Return:  
   The first index of `a` that is not less than `value`\.
 * Order:  
   &#927;\(log `a.size`\)




### <a id = "user-content-fn-179bac56" name = "user-content-fn-179bac56">&lt;CM&gt;upper_bound</a> ###

<code>static size_t <strong>&lt;CM&gt;upper_bound</strong>(const &lt;PCM&gt;box *const <em>a</em>, const &lt;PCM&gt;type *const <em>value</em>)</code>

[compare\.h](compare.h): `a` should be partitioned false/true with greater\-than or equal `value`\.

 * Return:  
   The first index of `a` that is greater than `value`\.
 * Order:  
   &#927;\(log `a.size`\)




### <a id = "user-content-fn-bfb5a80f" name = "user-content-fn-bfb5a80f">&lt;CM&gt;insert_after</a> ###

<code>static int <strong>&lt;CM&gt;insert_after</strong>(&lt;PCM&gt;box *const <em>a</em>, const &lt;PCM&gt;type *const <em>datum</em>)</code>

[compare\.h](compare.h): copies `datum` at the upper bound of a sorted `a`\.

 * Return:  
   Success\.
 * Exceptional return: realloc, ERANGE  
 * Order:  
   &#927;\(`a.size`\)




### <a id = "user-content-fn-3e4620eb" name = "user-content-fn-3e4620eb">&lt;CM&gt;sort</a> ###

<code>static void <strong>&lt;CM&gt;sort</strong>(&lt;PCM&gt;box *const <em>a</em>)</code>

[compare\.h](compare.h): sorts `a` by `qsort`\.

 * Order:  
   &#927;\(`a.size` \\log `a.size`\)




### <a id = "user-content-fn-2022b037" name = "user-content-fn-2022b037">&lt;CM&gt;reverse</a> ###

<code>static void <strong>&lt;CM&gt;reverse</strong>(&lt;PCM&gt;box *const <em>a</em>)</code>

[compare\.h](compare.h): sorts `a` in reverse by `qsort`\.

 * Order:  
   &#927;\(`a.size` \\log `a.size`\)




### <a id = "user-content-fn-30c13ca0" name = "user-content-fn-30c13ca0">&lt;CM&gt;is_equal</a> ###

<code>static int <strong>&lt;CM&gt;is_equal</strong>(const &lt;PCM&gt;box *const <em>a</em>, const &lt;PCM&gt;box *const <em>b</em>)</code>

[compare\.h](compare.h)

 * Return:  
   If `a` piecewise equals `b`, which both can be null\.
 * Order:  
   &#927;\(`size`\)




### <a id = "user-content-fn-a92a161f" name = "user-content-fn-a92a161f">&lt;CM&gt;unique_merge</a> ###

<code>static void <strong>&lt;CM&gt;unique_merge</strong>(&lt;PCM&gt;box *const <em>a</em>, const &lt;PCM&gt;biaction_fn <em>merge</em>)</code>

[compare\.h](compare.h): removes consecutive duplicate elements in `a`\.

 * Parameter: _merge_  
   Controls surjection\. Called with duplicate elements, if false `(x, y)->(x)`, if true `(x,y)->(y)`\. More complex functions, `(x, y)->(x+y)` can be simulated by mixing the two in the value returned\. Can be null: behaves like false\.
 * Order:  
   &#927;\(`a.size` &#215; `merge`\)




### <a id = "user-content-fn-f17606f0" name = "user-content-fn-f17606f0">&lt;CM&gt;unique</a> ###

<code>static void <strong>&lt;CM&gt;unique</strong>(&lt;PCM&gt;box *const <em>a</em>)</code>

[compare\.h](compare.h): removes consecutive duplicate elements in `a`\.

 * Order:  
   &#927;\(`a.size`\)






## <a id = "user-content-license" name = "user-content-license">License</a> ##

2016 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



