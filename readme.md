# array\.h #

## Contiguous Dynamic Array \(Vector\) ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PT&gt;type](#user-content-typedef-245060ab), [&lt;PT&gt;action_fn](#user-content-typedef-ba462b2e), [&lt;PT&gt;predicate_fn](#user-content-typedef-4608409d), [&lt;PT&gt;bipredicate_fn](#user-content-typedef-1c565568), [&lt;PT&gt;biproject_fn](#user-content-typedef-d76f4c34), [&lt;PT&gt;compare_fn](#user-content-typedef-70f17597), [&lt;PA&gt;to_string_fn](#user-content-typedef-a933c596)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;T&gt;array](#user-content-tag-96e5f142), [&lt;PT&gt;iterator](#user-content-tag-d9d00f09)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of array.](web/array.png)

[&lt;T&gt;array](#user-content-tag-96e5f142) is a dynamic array that stores contiguous `ARRAY_TYPE`\. Resizing may be necessary when increasing the size of the array\. This incurs amortised cost and any pointers to this memory may become stale\.



 * Parameter: ARRAY\_NAME, ARRAY\_TYPE  
   `<T>` that satisfies `C` naming conventions when mangled and a valid tag\-type associated therewith; required\. `<PT>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: ARRAY\_EXPECT\_TRAIT  
   Do not un\-define certain variables for subsequent inclusion in a trait\.
 * Parameter: ARRAY\_TO\_STRING\_NAME, ARRAY\_TO\_STRING  
   To string trait contained in [to\_string\.h](to_string.h); `<A>` that satisfies `C` naming conventions when mangled and function implementing [&lt;PA&gt;to_string_fn](#user-content-typedef-a933c596)\. There can be multiple to string traits, but only one can omit `ARRAY_TO_STRING_NAME`\.
 * Parameter: ARRAY\_TEST  
   To string trait contained in [\.\./test/array\_test\.h](../test/array_test.h); optional unit testing framework using `assert`\. Can only be defined once _per_ array\. Must be defined equal to a \(random\) filler function, satisfying [&lt;PT&gt;action_fn](#user-content-typedef-ba462b2e)\. Output will be shown with the to string trait in which it's defined; provides tests for the base code and all later traits\.
 * Parameter: ARRAY\_COMPARABLE\_NAME, ARRAY\_IS\_EQUAL, ARRAY\_COMPARE  
   Comparable trait; `<C>` that satisfies `C` naming conventions when mangled and a function implementing, for `ARRAY_IS_EQUAL` [&lt;PT&gt;bipredicate_fn](#user-content-typedef-1c565568) that establishes an equivalence relation, or for `ARRAY_COMPARE` [&lt;PT&gt;compare_fn](#user-content-typedef-70f17597) that establishes a total order\. There can be multiple comparable traits, but only one can omit `ARRAY_COMPARABLE_NAME`\.
 * Standard:  
   C89


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-245060ab" name = "user-content-typedef-245060ab">&lt;PT&gt;type</a> ###

<code>typedef ARRAY_TYPE <strong>&lt;PT&gt;type</strong>;</code>

A valid tag type set by `ARRAY_TYPE`\.



### <a id = "user-content-typedef-ba462b2e" name = "user-content-typedef-ba462b2e">&lt;PT&gt;action_fn</a> ###

<code>typedef void(*<strong>&lt;PT&gt;action_fn</strong>)(&lt;PT&gt;type *);</code>

Operates by side\-effects\.



### <a id = "user-content-typedef-4608409d" name = "user-content-typedef-4608409d">&lt;PT&gt;predicate_fn</a> ###

<code>typedef int(*<strong>&lt;PT&gt;predicate_fn</strong>)(const &lt;PT&gt;type *);</code>

Returns a boolean given read\-only `<T>`\.



### <a id = "user-content-typedef-1c565568" name = "user-content-typedef-1c565568">&lt;PT&gt;bipredicate_fn</a> ###

<code>typedef int(*<strong>&lt;PT&gt;bipredicate_fn</strong>)(const &lt;PT&gt;type *, const &lt;PT&gt;type *);</code>

Returns a boolean given two read\-only `<T>`\.



### <a id = "user-content-typedef-d76f4c34" name = "user-content-typedef-d76f4c34">&lt;PT&gt;biproject_fn</a> ###

<code>typedef int(*<strong>&lt;PT&gt;biproject_fn</strong>)(&lt;PT&gt;type *, &lt;PT&gt;type *);</code>

Returns a boolean given two `<T>`\.



### <a id = "user-content-typedef-70f17597" name = "user-content-typedef-70f17597">&lt;PT&gt;compare_fn</a> ###

<code>typedef int(*<strong>&lt;PT&gt;compare_fn</strong>)(const &lt;PT&gt;type *a, const &lt;PT&gt;type *b);</code>

Three\-way comparison on a totally order set; returns an integer value less then, equal to, greater then zero, if `a < b`, `a == b`, `a > b`, respectively\.



### <a id = "user-content-typedef-a933c596" name = "user-content-typedef-a933c596">&lt;PA&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PA&gt;to_string_fn</strong>)(const &lt;PA&gt;type *, char(*)[12]);</code>

Responsible for turning the first argument into a 12\-`char` null\-terminated output string\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-96e5f142" name = "user-content-tag-96e5f142">&lt;T&gt;array</a> ###

<code>struct <strong>&lt;T&gt;array</strong>;</code>

Manages the array field `data`, which is indexed up to `size`\. To initialise it to an idle state, see [&lt;T&gt;array](#user-content-fn-96e5f142), `ARRAY_IDLE`, `{0}` \(`C99`,\) or being `static`\.

![States.](web/states.png)



### <a id = "user-content-tag-d9d00f09" name = "user-content-tag-d9d00f09">&lt;PT&gt;iterator</a> ###

<code>struct <strong>&lt;PT&gt;iterator</strong>;</code>

Contains all iteration parameters\.



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-96e5f142">&lt;T&gt;array</a></td><td>a</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-a8fa90a7">&lt;T&gt;array_</a></td><td>a</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-6e0bca81">&lt;T&gt;array_clip</a></td><td>a, i</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-17759c31">&lt;T&gt;array_reserve</a></td><td>a, min</td></tr>

<tr><td align = right>static &lt;PT&gt;type *</td><td><a href = "#user-content-fn-cd39931d">&lt;T&gt;array_buffer</a></td><td>a, buffer</td></tr>

<tr><td align = right>static &lt;PT&gt;type *</td><td><a href = "#user-content-fn-d2dc554f">&lt;T&gt;array_append</a></td><td>a, n</td></tr>

<tr><td align = right>static &lt;PT&gt;type *</td><td><a href = "#user-content-fn-7cb999d5">&lt;T&gt;array_append_at</a></td><td>a, n, at</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-ec4aafab">&lt;T&gt;array_remove</a></td><td>a, datum</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-c5b0aea">&lt;T&gt;array_lazy_remove</a></td><td>a, datum</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-8ba65af8">&lt;T&gt;array_clear</a></td><td>a</td></tr>

<tr><td align = right>static &lt;PT&gt;type *</td><td><a href = "#user-content-fn-7993382c">&lt;T&gt;array_peek</a></td><td>a</td></tr>

<tr><td align = right>static &lt;PT&gt;type *</td><td><a href = "#user-content-fn-14f6c2ba">&lt;T&gt;array_pop</a></td><td>a</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-37e10160">&lt;T&gt;array_copy_if</a></td><td>a, copy, b</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-bf0a45e0">&lt;T&gt;array_keep_if</a></td><td>a, keep, destruct</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-e65bff8f">&lt;T&gt;array_trim</a></td><td>a, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-daefd78e">&lt;T&gt;array_each</a></td><td>a, action</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-51c854fc">&lt;T&gt;array_if_each</a></td><td>a, predicate, action</td></tr>

<tr><td align = right>static &lt;PT&gt;type *</td><td><a href = "#user-content-fn-cfeeb3d7">&lt;T&gt;array_any</a></td><td>a, predicate</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-6fb489ab">&lt;A&gt;to_string</a></td><td>box</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-6e93ba93">&lt;T&gt;array_test</a></td><td></td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-ff2ca488">&lt;T&gt;array&lt;C&gt;comparable_test</a></td><td></td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-bffba15e">&lt;T&gt;array&lt;C&gt;compare</a></td><td>a, b</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-492cb74f">&lt;T&gt;array&lt;C&gt;lower_bound</a></td><td>a, value</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-b2e7b882">&lt;T&gt;array&lt;C&gt;upper_bound</a></td><td>a, value</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-ab41c197">&lt;T&gt;array&lt;C&gt;sort</a></td><td>a</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-449174eb">&lt;T&gt;array&lt;C&gt;reverse</a></td><td>a</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-44a2ae09">&lt;T&gt;array&lt;C&gt;merge_unique</a></td><td>a, merge</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-9d746afc">&lt;T&gt;array&lt;C&gt;unique</a></td><td>a</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-96e5f142" name = "user-content-fn-96e5f142">&lt;T&gt;array</a> ###

<code>static void <strong>&lt;T&gt;array</strong>(struct &lt;T&gt;array *const <em>a</em>)</code>

Initialises `a` to idle\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-a8fa90a7" name = "user-content-fn-a8fa90a7">&lt;T&gt;array_</a> ###

<code>static void <strong>&lt;T&gt;array_</strong>(struct &lt;T&gt;array *const <em>a</em>)</code>

Destroys `a` and returns it to idle\.



### <a id = "user-content-fn-6e0bca81" name = "user-content-fn-6e0bca81">&lt;T&gt;array_clip</a> ###

<code>static size_t <strong>&lt;T&gt;array_clip</strong>(const struct &lt;T&gt;array *const <em>a</em>, const long <em>i</em>)</code>

 * Return:  
   Converts `i` to an index in `a` from \[0, `a.size`\]\. Negative values are implicitly plus `a.size`\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-17759c31" name = "user-content-fn-17759c31">&lt;T&gt;array_reserve</a> ###

<code>static int <strong>&lt;T&gt;array_reserve</strong>(struct &lt;T&gt;array *const <em>a</em>, const size_t <em>min</em>)</code>

Ensures `min` capacity of `a`\. Invalidates pointers in `a`\.

 * Parameter: _min_  
   If zero, does nothing\.
 * Return:  
   Success; otherwise, `errno` will be set\.
 * Exceptional return: ERANGE  
   Tried allocating more then can fit in `size_t` or `realloc` doesn't follow [POSIX](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html)\.
 * Exceptional return: realloc  




### <a id = "user-content-fn-cd39931d" name = "user-content-fn-cd39931d">&lt;T&gt;array_buffer</a> ###

<code>static &lt;PT&gt;type *<strong>&lt;T&gt;array_buffer</strong>(struct &lt;T&gt;array *const <em>a</em>, const size_t <em>buffer</em>)</code>

The capacity of `a` will be increased to at least `buffer` elements beyond the size\. Invalidates pointers in `a`\.

 * Return:  
   The start of the buffered space, \(the back of the array\.\) If `a` is idle and `buffer` is zero, a null pointer is returned, otherwise null indicates an error\.
 * Exceptional return: realloc, ERANGE  




### <a id = "user-content-fn-d2dc554f" name = "user-content-fn-d2dc554f">&lt;T&gt;array_append</a> ###

<code>static &lt;PT&gt;type *<strong>&lt;T&gt;array_append</strong>(struct &lt;T&gt;array *const <em>a</em>, const size_t <em>n</em>)</code>

Adds `n` elements to the back of `a`\. The buffer holds enough elements or it will invalidate pointers in `a`\.

 * Return:  
   A pointer to the elements\. If `a` is idle and `n` is zero, a null pointer will be returned, otherwise null indicates an error\.
 * Exceptional return: realloc, ERANGE  




### <a id = "user-content-fn-7cb999d5" name = "user-content-fn-7cb999d5">&lt;T&gt;array_append_at</a> ###

<code>static &lt;PT&gt;type *<strong>&lt;T&gt;array_append_at</strong>(struct &lt;T&gt;array *const <em>a</em>, const size_t <em>n</em>, const size_t <em>at</em>)</code>

Adds `n` un\-initialised elements at position `at` in `a`\. The buffer holds enough elements or it will invalidate pointers in `a`\.

 * Parameter: _at_  
   A number smaller than or equal to `a.size`; if `a.size`, this function behaves as [&lt;T&gt;array_append](#user-content-fn-d2dc554f)\.
 * Return:  
   A pointer to the start of the new region, where there are `n` elements\.
 * Exceptional return: realloc, ERANGE  




### <a id = "user-content-fn-ec4aafab" name = "user-content-fn-ec4aafab">&lt;T&gt;array_remove</a> ###

<code>static void <strong>&lt;T&gt;array_remove</strong>(struct &lt;T&gt;array *const <em>a</em>, &lt;PT&gt;type *const <em>datum</em>)</code>

Removes `datum` from `a`\.

 * Order:  
   &#927;\(`a.size`\)\.




### <a id = "user-content-fn-c5b0aea" name = "user-content-fn-c5b0aea">&lt;T&gt;array_lazy_remove</a> ###

<code>static void <strong>&lt;T&gt;array_lazy_remove</strong>(struct &lt;T&gt;array *const <em>a</em>, &lt;PT&gt;type *const <em>datum</em>)</code>

Removes `datum` from `a` and replaces it with the tail\.

 * Order:  
   &#927;\(1\)\.




### <a id = "user-content-fn-8ba65af8" name = "user-content-fn-8ba65af8">&lt;T&gt;array_clear</a> ###

<code>static void <strong>&lt;T&gt;array_clear</strong>(struct &lt;T&gt;array *const <em>a</em>)</code>

Sets `a` to be empty\. That is, the size of `a` will be zero, but if it was previously in an active non\-idle state, it continues to be\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-7993382c" name = "user-content-fn-7993382c">&lt;T&gt;array_peek</a> ###

<code>static &lt;PT&gt;type *<strong>&lt;T&gt;array_peek</strong>(const struct &lt;T&gt;array *const <em>a</em>)</code>

 * Return:  
   The last element or null if `a` is empty\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-14f6c2ba" name = "user-content-fn-14f6c2ba">&lt;T&gt;array_pop</a> ###

<code>static &lt;PT&gt;type *<strong>&lt;T&gt;array_pop</strong>(struct &lt;T&gt;array *const <em>a</em>)</code>

 * Return:  
   Value from the the top of `a` that is removed or null if the array is empty\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-37e10160" name = "user-content-fn-37e10160">&lt;T&gt;array_copy_if</a> ###

<code>static int <strong>&lt;T&gt;array_copy_if</strong>(struct &lt;T&gt;array *const <em>a</em>, const &lt;PT&gt;predicate_fn <em>copy</em>, const struct &lt;T&gt;array *const <em>b</em>)</code>

For all elements of `b`, calls `copy`, and if true, lazily copies the elements to `a`\. `a` and `b` can not be the same but `b` can be null\.

 * Exceptional return: ERANGE, realloc  
 * Order:  
   &#927;\(`b.size` &#215; `copy`\)




### <a id = "user-content-fn-bf0a45e0" name = "user-content-fn-bf0a45e0">&lt;T&gt;array_keep_if</a> ###

<code>static void <strong>&lt;T&gt;array_keep_if</strong>(struct &lt;T&gt;array *const <em>a</em>, const &lt;PT&gt;predicate_fn <em>keep</em>, const &lt;PT&gt;action_fn <em>destruct</em>)</code>

For all elements of `a`, calls `keep`, and if false, lazy deletes that item, calling `destruct` if not\-null\.

 * Order:  
   &#927;\(`a.size` &#215; `keep` &#215; `destruct`\)




### <a id = "user-content-fn-e65bff8f" name = "user-content-fn-e65bff8f">&lt;T&gt;array_trim</a> ###

<code>static void <strong>&lt;T&gt;array_trim</strong>(struct &lt;T&gt;array *const <em>a</em>, const &lt;PT&gt;predicate_fn <em>predicate</em>)</code>

Removes at either end of `a` of things that `predicate` returns true\.

 * Order:  
   &#927;\(`a.size` &#215; `predicate`\)




### <a id = "user-content-fn-daefd78e" name = "user-content-fn-daefd78e">&lt;T&gt;array_each</a> ###

<code>static void <strong>&lt;T&gt;array_each</strong>(struct &lt;T&gt;array *const <em>a</em>, const &lt;PT&gt;action_fn <em>action</em>)</code>

Iterates through `a` and calls `action` on all the elements\. The topology of the list should not change while in this function\.

 * Order:  
   &#927;\(`a.size` &#215; `action`\)




### <a id = "user-content-fn-51c854fc" name = "user-content-fn-51c854fc">&lt;T&gt;array_if_each</a> ###

<code>static void <strong>&lt;T&gt;array_if_each</strong>(struct &lt;T&gt;array *const <em>a</em>, const &lt;PT&gt;predicate_fn <em>predicate</em>, const &lt;PT&gt;action_fn <em>action</em>)</code>

Iterates through `a` and calls `action` on all the elements for which `predicate` returns true\. The topology of the list should not change while in this function\.

 * Order:  
   &#927;\(`a.size` &#215; `predicate` &#215; `action`\)




### <a id = "user-content-fn-cfeeb3d7" name = "user-content-fn-cfeeb3d7">&lt;T&gt;array_any</a> ###

<code>static &lt;PT&gt;type *<strong>&lt;T&gt;array_any</strong>(const struct &lt;T&gt;array *const <em>a</em>, const &lt;PT&gt;predicate_fn <em>predicate</em>)</code>

Iterates through `a` and calls `predicate` until it returns true\.

 * Return:  
   The first `predicate` that returned true, or, if the statement is false on all, null\.
 * Order:  
   &#927;\(`a.size` &#215; `predicate`\)




### <a id = "user-content-fn-6fb489ab" name = "user-content-fn-6fb489ab">&lt;A&gt;to_string</a> ###

<code>static const char *<strong>&lt;A&gt;to_string</strong>(const &lt;PA&gt;box *const <em>box</em>)</code>

 * Return:  
   Print the contents of `box` in a static string buffer of 256 bytes with limitations of only printing 4 things at a time\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-6e93ba93" name = "user-content-fn-6e93ba93">&lt;T&gt;array_test</a> ###

<code>static void <strong>&lt;T&gt;array_test</strong>(void)</code>

Will be tested on stdout\. Requires `ARRAY_TEST`, `ARRAY_TO_STRING`, and not `NDEBUG` while defining `assert`\.



### <a id = "user-content-fn-ff2ca488" name = "user-content-fn-ff2ca488">&lt;T&gt;array&lt;C&gt;comparable_test</a> ###

<code>static void <strong>&lt;T&gt;array&lt;C&gt;comparable_test</strong>(void)</code>

Will be tested on stdout\. Requires `ARRAY_TEST`, `ARRAY_TO_STRING`, and not `NDEBUG` while defining `assert`\.



### <a id = "user-content-fn-bffba15e" name = "user-content-fn-bffba15e">&lt;T&gt;array&lt;C&gt;compare</a> ###

<code>static int <strong>&lt;T&gt;array&lt;C&gt;compare</strong>(const struct &lt;T&gt;array *const <em>a</em>, const struct &lt;T&gt;array *const <em>b</em>)</code>

Lexagraphically compares `a` to `b`, which both can be null\.

 * Return:  
   \{ `a < b`: negative, `a == b`: zero, `a > b`: positive \}\.
 * Order:  
   &#927;\(`a.size`\)




### <a id = "user-content-fn-492cb74f" name = "user-content-fn-492cb74f">&lt;T&gt;array&lt;C&gt;lower_bound</a> ###

<code>static size_t <strong>&lt;T&gt;array&lt;C&gt;lower_bound</strong>(const struct &lt;T&gt;array *const <em>a</em>, const &lt;PT&gt;type *const <em>value</em>)</code>

`a` should be partitioned true/false with less\-then `value`\.

 * Return:  
   The first index of `a` that is not less than `value`\.
 * Order:  
   &#927;\(log `a.size`\)




### <a id = "user-content-fn-b2e7b882" name = "user-content-fn-b2e7b882">&lt;T&gt;array&lt;C&gt;upper_bound</a> ###

<code>static size_t <strong>&lt;T&gt;array&lt;C&gt;upper_bound</strong>(const struct &lt;T&gt;array *const <em>a</em>, const &lt;PT&gt;type *const <em>value</em>)</code>

`a` should be partitioned false/true with greater\-than or equals `value`\.

 * Return:  
   The first index of `a` that is greater than `value`\.
 * Order:  
   &#927;\(log `a.size`\)




### <a id = "user-content-fn-ab41c197" name = "user-content-fn-ab41c197">&lt;T&gt;array&lt;C&gt;sort</a> ###

<code>static void <strong>&lt;T&gt;array&lt;C&gt;sort</strong>(struct &lt;T&gt;array *const <em>a</em>)</code>

Sorts `a` by `qsort` on `ARRAY_COMPARE`\.

 * Order:  
   &#927;\(`a.size` \\log `a.size`\)




### <a id = "user-content-fn-449174eb" name = "user-content-fn-449174eb">&lt;T&gt;array&lt;C&gt;reverse</a> ###

<code>static void <strong>&lt;T&gt;array&lt;C&gt;reverse</strong>(struct &lt;T&gt;array *const <em>a</em>)</code>

Sorts `a` in reverse by `qsort` on `ARRAY_COMPARE`\.

 * Order:  
   &#927;\(`a.size` \\log `a.size`\)




### <a id = "user-content-fn-44a2ae09" name = "user-content-fn-44a2ae09">&lt;T&gt;array&lt;C&gt;merge_unique</a> ###

<code>static void <strong>&lt;T&gt;array&lt;C&gt;merge_unique</strong>(struct &lt;T&gt;array *const <em>a</em>, const &lt;PT&gt;biproject_fn <em>merge</em>)</code>

Removes consecutive duplicate elements in `a`\.

 * Parameter: _merge_  
   Controls surjection\. Called with duplicate elements, if false `(x, y)->(x)`, if true `(x,y)->(y)`\. More complex functions, `(x, y)->(x+y)` can be simulated by mixing the two in the value returned\. Can be null: behaves like false\.
 * Order:  
   &#927;\(`a.size` &#215; `merge`\)




### <a id = "user-content-fn-9d746afc" name = "user-content-fn-9d746afc">&lt;T&gt;array&lt;C&gt;unique</a> ###

<code>static void <strong>&lt;T&gt;array&lt;C&gt;unique</strong>(struct &lt;T&gt;array *const <em>a</em>)</code>

Removes consecutive duplicate elements in `a`\.

 * Order:  
   &#927;\(`a.size`\)






## <a id = "user-content-license" name = "user-content-license">License</a> ##

2016 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



