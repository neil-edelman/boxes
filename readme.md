# array\.h #

## Contiguous Dynamic Array \(Vector\) ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PA&gt;type](#user-content-typedef-a8a4b08a), [&lt;PZ&gt;action_fn](#user-content-typedef-9321d9ec), [&lt;PZ&gt;predicate_fn](#user-content-typedef-ad62af5b), [&lt;PZ&gt;to_string_fn](#user-content-typedef-22f3d7f1), [&lt;PZ&gt;bipredicate_fn](#user-content-typedef-a15f7652), [&lt;PZ&gt;biaction_fn](#user-content-typedef-aeba3b53), [&lt;PZ&gt;compare_fn](#user-content-typedef-22dcf1d9)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;A&gt;array](#user-content-tag-8049be0d), [&lt;PA&gt;iterator](#user-content-tag-e6ddd8f0)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of array.](web/array.png)

[&lt;A&gt;array](#user-content-tag-8049be0d) is a dynamic array that stores contiguous [&lt;PA&gt;type](#user-content-typedef-a8a4b08a)\. Resizing may be necessary when increasing the size of the array; this incurs amortised cost, and any pointers to this memory may become stale\.



 * Parameter: ARRAY\_NAME, ARRAY\_TYPE  
   `<A>` that satisfies `C` naming conventions when mangled and a valid tag\-type, [&lt;PA&gt;type](#user-content-typedef-a8a4b08a), associated therewith; required\. `<PA>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: ARRAY\_FUNCTION  
   Include Function trait contained in [function\.h](function.h)\.
 * Parameter: ARRAY\_TEST  
   Optional function implementing [&lt;PZ&gt;action_fn](#user-content-typedef-9321d9ec) that fills the [&lt;PA&gt;type](#user-content-typedef-a8a4b08a) from uninitialized to random for unit testing framework using `assert`\. Testing array contained in [\.\./test/test\_array\.h](../test/test_array.h)\. Must have any To String trait\.
 * Parameter: ARRAY\_EXPECT\_TRAIT  
   Do not un\-define certain variables for subsequent inclusion in a parameterized trait\.
 * Parameter: ARRAY\_COMPARE\_NAME, ARRAY\_COMPARE, ARRAY\_IS\_EQUAL  
   Compare trait contained in [compare\.h](compare.h)\. An optional mangled name for uniqueness and a function implementing [&lt;PZ&gt;compare_fn](#user-content-typedef-22dcf1d9) xor [&lt;PZ&gt;bipredicate_fn](#user-content-typedef-a15f7652)\.
 * Parameter: ARRAY\_TO\_STRING\_NAME, ARRAY\_TO\_STRING  
   To string trait contained in [to\_string\.h](to_string.h)\. An optional mangled name for uniqueness and function implementing [&lt;PZ&gt;to_string_fn](#user-content-typedef-22f3d7f1)\.
 * Standard:  
   C89


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-a8a4b08a" name = "user-content-typedef-a8a4b08a">&lt;PA&gt;type</a> ###

<code>typedef ARRAY_TYPE <strong>&lt;PA&gt;type</strong>;</code>

A valid tag type set by `ARRAY_TYPE`\.



### <a id = "user-content-typedef-9321d9ec" name = "user-content-typedef-9321d9ec">&lt;PZ&gt;action_fn</a> ###

<code>typedef void(*<strong>&lt;PZ&gt;action_fn</strong>)(&lt;PZ&gt;type *);</code>

Operates by side\-effects on [&lt;PZ&gt;type](#user-content-typedef-bfd92b5)\.



### <a id = "user-content-typedef-ad62af5b" name = "user-content-typedef-ad62af5b">&lt;PZ&gt;predicate_fn</a> ###

<code>typedef int(*<strong>&lt;PZ&gt;predicate_fn</strong>)(const &lt;PZ&gt;type *);</code>

Returns a boolean given read\-only [&lt;PZ&gt;type](#user-content-typedef-bfd92b5)\.



### <a id = "user-content-typedef-22f3d7f1" name = "user-content-typedef-22f3d7f1">&lt;PZ&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PZ&gt;to_string_fn</strong>)(const &lt;PZ&gt;type *, char(*)[12]);</code>

Responsible for turning the first argument into a 12\-`char` null\-terminated output string\.



### <a id = "user-content-typedef-a15f7652" name = "user-content-typedef-a15f7652">&lt;PZ&gt;bipredicate_fn</a> ###

<code>typedef int(*<strong>&lt;PZ&gt;bipredicate_fn</strong>)(const &lt;PZ&gt;type *, const &lt;PZ&gt;type *);</code>

Returns a boolean given two read\-only [&lt;PZ&gt;type](#user-content-typedef-bfd92b5)\.



### <a id = "user-content-typedef-aeba3b53" name = "user-content-typedef-aeba3b53">&lt;PZ&gt;biaction_fn</a> ###

<code>typedef int(*<strong>&lt;PZ&gt;biaction_fn</strong>)(&lt;PZ&gt;type *, &lt;PZ&gt;type *);</code>

Returns a boolean given two [&lt;PZ&gt;type](#user-content-typedef-bfd92b5)\.



### <a id = "user-content-typedef-22dcf1d9" name = "user-content-typedef-22dcf1d9">&lt;PZ&gt;compare_fn</a> ###

<code>typedef int(*<strong>&lt;PZ&gt;compare_fn</strong>)(const &lt;PZ&gt;type *a, const &lt;PZ&gt;type *b);</code>

Three\-way comparison on a totally order set; returns an integer value less then, equal to, greater then zero, if `a < b`, `a == b`, `a > b`, respectively\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-8049be0d" name = "user-content-tag-8049be0d">&lt;A&gt;array</a> ###

<code>struct <strong>&lt;A&gt;array</strong>;</code>

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

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d5035752">&lt;A&gt;array_remove</a></td><td>a, datum</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d2a95b41">&lt;A&gt;array_lazy_remove</a></td><td>a, datum</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-e3f5267">&lt;A&gt;array_clear</a></td><td>a</td></tr>

<tr><td align = right>static &lt;PA&gt;type *</td><td><a href = "#user-content-fn-8af01fa1">&lt;A&gt;array_peek</a></td><td>a</td></tr>

<tr><td align = right>static &lt;PA&gt;type *</td><td><a href = "#user-content-fn-7bf4e995">&lt;A&gt;array_pop</a></td><td>a</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-98a4cc31">&lt;Z&gt;clip</a></td><td>box, i</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-b5eb2ff0">&lt;Z&gt;copy_if</a></td><td>a, copy, b</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-1dbab6d0">&lt;Z&gt;keep_if</a></td><td>box, keep, destruct</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d627ae9f">&lt;Z&gt;trim</a></td><td>box, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-cc3bf1de">&lt;Z&gt;each</a></td><td>box, action</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-e766a80c">&lt;Z&gt;if_each</a></td><td>box, predicate, action</td></tr>

<tr><td align = right>static const &lt;PZ&gt;type *</td><td><a href = "#user-content-fn-95a77627">&lt;Z&gt;any</a></td><td>box, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-c5d3559e">&lt;A&gt;array_test</a></td><td></td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-ff2ca488">&lt;T&gt;array&lt;C&gt;comparable_test</a></td><td></td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-4ecb4112">&lt;Z&gt;to_string</a></td><td>box</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-96cc6d46">&lt;Z&gt;compare</a></td><td>a, b</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-f8a39da7">&lt;Z&gt;lower_bound</a></td><td>a, value</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-2e48da1a">&lt;Z&gt;upper_bound</a></td><td>a, value</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-66a7cd9f">&lt;Z&gt;sort</a></td><td>a</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-445f7973">&lt;Z&gt;reverse</a></td><td>a</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d94768eb">&lt;Z&gt;unique_merge</a></td><td>a, merge</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-a1f02db4">&lt;Z&gt;unique</a></td><td>a</td></tr>

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

The capacity of `a` will be increased to at least `n` elements beyond the size\. Invalidates pointers in `a`\.

 * Return:  
   The start of the buffered space, \(the back of the array\.\) If `a` is idle and `buffer` is zero, a null pointer is returned, otherwise null indicates an error\.
 * Exceptional return: realloc, ERANGE  




### <a id = "user-content-fn-2d92b62" name = "user-content-fn-2d92b62">&lt;A&gt;array_append</a> ###

<code>static &lt;PA&gt;type *<strong>&lt;A&gt;array_append</strong>(struct &lt;A&gt;array *const <em>a</em>, const size_t <em>n</em>)</code>

Adds `n` elements to the back of `a`\. The buffer holds enough elements or it will invalidate pointers in `a`\.

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




### <a id = "user-content-fn-98a4cc31" name = "user-content-fn-98a4cc31">&lt;Z&gt;clip</a> ###

<code>static size_t <strong>&lt;Z&gt;clip</strong>(const &lt;PZ&gt;box *const <em>box</em>, const long <em>i</em>)</code>

 * Return:  
   Converts `i` to an index in `box` from \[0, `a.size`\]\. Negative values are implicitly plus `box.size`\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-b5eb2ff0" name = "user-content-fn-b5eb2ff0">&lt;Z&gt;copy_if</a> ###

<code>static int <strong>&lt;Z&gt;copy_if</strong>(&lt;PZ&gt;box *const <em>a</em>, const &lt;PZ&gt;predicate_fn <em>copy</em>, const &lt;PZ&gt;box *const <em>b</em>)</code>

Needs iterate and copy interfaces\. For all elements of `b`, calls `copy`, and if true, lazily copies the elements to `a`\. `a` and `b` can not be the same but `b` can be null, \(in which case, it does nothing\.\)

 * Exceptional return: ERANGE, realloc  
 * Order:  
   &#927;\(`b.size` &#215; `copy`\)




### <a id = "user-content-fn-1dbab6d0" name = "user-content-fn-1dbab6d0">&lt;Z&gt;keep_if</a> ###

<code>static void <strong>&lt;Z&gt;keep_if</strong>(&lt;PZ&gt;box *const <em>box</em>, const &lt;PZ&gt;predicate_fn <em>keep</em>, const &lt;PZ&gt;action_fn <em>destruct</em>)</code>

For all elements of `box`, calls `keep`, and if false, lazy deletes that item, calling `destruct` if not\-null\.

 * Order:  
   &#927;\(`a.size` &#215; `keep` &#215; `destruct`\)




### <a id = "user-content-fn-d627ae9f" name = "user-content-fn-d627ae9f">&lt;Z&gt;trim</a> ###

<code>static void <strong>&lt;Z&gt;trim</strong>(&lt;PZ&gt;box *const <em>box</em>, const &lt;PZ&gt;predicate_fn <em>predicate</em>)</code>

Requires iterate, reverse, and copy interfaces\. Removes at either end of `box` of things that `predicate` returns true\.

 * Order:  
   &#927;\(`box.size` &#215; `predicate`\)




### <a id = "user-content-fn-cc3bf1de" name = "user-content-fn-cc3bf1de">&lt;Z&gt;each</a> ###

<code>static void <strong>&lt;Z&gt;each</strong>(&lt;PZ&gt;box *const <em>box</em>, const &lt;PZ&gt;action_fn <em>action</em>)</code>

Iterates through `box` and calls `action` on all the elements\. The topology of the list should not change while in this function\.

 * Order:  
   &#927;\(`box.size` &#215; `action`\)




### <a id = "user-content-fn-e766a80c" name = "user-content-fn-e766a80c">&lt;Z&gt;if_each</a> ###

<code>static void <strong>&lt;Z&gt;if_each</strong>(&lt;PZ&gt;box *const <em>box</em>, const &lt;PZ&gt;predicate_fn <em>predicate</em>, const &lt;PZ&gt;action_fn <em>action</em>)</code>

Iterates through `box` and calls `action` on all the elements for which `predicate` returns true\. The topology of the list should not change while in this function\.

 * Order:  
   &#927;\(`box.size` &#215; `predicate` &#215; `action`\)




### <a id = "user-content-fn-95a77627" name = "user-content-fn-95a77627">&lt;Z&gt;any</a> ###

<code>static const &lt;PZ&gt;type *<strong>&lt;Z&gt;any</strong>(const &lt;PZ&gt;box *const <em>box</em>, const &lt;PZ&gt;predicate_fn <em>predicate</em>)</code>

Requires iterate interface\. Iterates through `box` and calls `predicate` until it returns true\.

 * Return:  
   The first `predicate` that returned true, or, if the statement is false on all, null\.
 * Order:  
   &#927;\(`box.size` &#215; `predicate`\)




### <a id = "user-content-fn-c5d3559e" name = "user-content-fn-c5d3559e">&lt;A&gt;array_test</a> ###

<code>static void <strong>&lt;A&gt;array_test</strong>(void)</code>

Will be tested on stdout\. Requires `ARRAY_TEST`, `ARRAY_TO_STRING`, and not `NDEBUG` while defining `assert`\.



### <a id = "user-content-fn-ff2ca488" name = "user-content-fn-ff2ca488">&lt;T&gt;array&lt;C&gt;comparable_test</a> ###

<code>static void <strong>&lt;T&gt;array&lt;C&gt;comparable_test</strong>(void)</code>

Will be tested on stdout\. Requires `ARRAY_TEST`, `ARRAY_TO_STRING`, and not `NDEBUG` while defining `assert`\.



### <a id = "user-content-fn-4ecb4112" name = "user-content-fn-4ecb4112">&lt;Z&gt;to_string</a> ###

<code>static const char *<strong>&lt;Z&gt;to_string</strong>(const &lt;PZ&gt;box *const <em>box</em>)</code>

 * Return:  
   Print the contents of `box` in a static string buffer of 256 bytes with limitations of only printing 4 things at a time\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-96cc6d46" name = "user-content-fn-96cc6d46">&lt;Z&gt;compare</a> ###

<code>static int <strong>&lt;Z&gt;compare</strong>(const &lt;PZ&gt;box *const <em>a</em>, const &lt;PZ&gt;box *const <em>b</em>)</code>

Lexicographically compares `a` to `b`\. Null values are before everything\.

 * Return:  
   `a < b`: negative; `a == b`: zero; `a > b`: positive\.
 * Order:  
   &#927;\(`a.size`\)




### <a id = "user-content-fn-f8a39da7" name = "user-content-fn-f8a39da7">&lt;Z&gt;lower_bound</a> ###

<code>static size_t <strong>&lt;Z&gt;lower_bound</strong>(const &lt;PZ&gt;box *const <em>a</em>, const &lt;PA&gt;type *const <em>value</em>)</code>

`a` should be partitioned true/false with less\-then `value`\.

 * Return:  
   The first index of `a` that is not less than `value`\.
 * Order:  
   &#927;\(log `a.size`\)




### <a id = "user-content-fn-2e48da1a" name = "user-content-fn-2e48da1a">&lt;Z&gt;upper_bound</a> ###

<code>static size_t <strong>&lt;Z&gt;upper_bound</strong>(const &lt;PZ&gt;box *const <em>a</em>, const &lt;PA&gt;type *const <em>value</em>)</code>

`a` should be partitioned false/true with greater\-than or equals `value`\.

 * Return:  
   The first index of `a` that is greater than `value`\.
 * Order:  
   &#927;\(log `a.size`\)




### <a id = "user-content-fn-66a7cd9f" name = "user-content-fn-66a7cd9f">&lt;Z&gt;sort</a> ###

<code>static void <strong>&lt;Z&gt;sort</strong>(&lt;PZ&gt;box *const <em>a</em>)</code>

Sorts `a` by `qsort` on `ARRAY_COMPARE`\.

 * Order:  
   &#927;\(`a.size` \\log `a.size`\)




### <a id = "user-content-fn-445f7973" name = "user-content-fn-445f7973">&lt;Z&gt;reverse</a> ###

<code>static void <strong>&lt;Z&gt;reverse</strong>(&lt;PZ&gt;box *const <em>a</em>)</code>

Sorts `a` in reverse by `qsort` on `ARRAY_COMPARE`\.

 * Order:  
   &#927;\(`a.size` \\log `a.size`\)




### <a id = "user-content-fn-d94768eb" name = "user-content-fn-d94768eb">&lt;Z&gt;unique_merge</a> ###

<code>static void <strong>&lt;Z&gt;unique_merge</strong>(&lt;PZ&gt;box *const <em>a</em>, const &lt;PZ&gt;biaction_fn <em>merge</em>)</code>

Removes consecutive duplicate elements in `a`\.

 * Parameter: _merge_  
   Controls surjection\. Called with duplicate elements, if false `(x, y)->(x)`, if true `(x,y)->(y)`\. More complex functions, `(x, y)->(x+y)` can be simulated by mixing the two in the value returned\. Can be null: behaves like false\.
 * Order:  
   &#927;\(`a.size` &#215; `merge`\)




### <a id = "user-content-fn-a1f02db4" name = "user-content-fn-a1f02db4">&lt;Z&gt;unique</a> ###

<code>static void <strong>&lt;Z&gt;unique</strong>(&lt;PZ&gt;box *const <em>a</em>)</code>

Removes consecutive duplicate elements in `a`\.

 * Order:  
   &#927;\(`a.size`\)






## <a id = "user-content-license" name = "user-content-license">License</a> ##

2016 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



