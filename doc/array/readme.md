# array\.h #

Header [\.\./\.\./src/array\.h](../../src/array.h); examples [\.\./\.\./test/test\_array\.c](../../test/test_array.c)\.

## Contiguous dynamic array ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;pT&gt;type](#user-content-typedef-9b5be28b), [&lt;pT&gt;action_fn](#user-content-typedef-348726ce), [&lt;pT&gt;predicate_fn](#user-content-typedef-ad32e23d), [&lt;pT&gt;to_string_fn](#user-content-typedef-4442127b), [&lt;pT&gt;compare_fn](#user-content-typedef-223b1937), [&lt;pT&gt;bipredicate_fn](#user-content-typedef-4ab43b88), [&lt;pT&gt;biaction_fn](#user-content-typedef-5082f141)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;t&gt;array](#user-content-tag-9c4cf562)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of array.](../../doc/array/array.png)

[&lt;t&gt;array](#user-content-tag-9c4cf562) is a dynamic array that stores contiguous [&lt;pT&gt;type](#user-content-typedef-9b5be28b)\. Resizing may be necessary when increasing the size of the array; this incurs amortised cost\. As such, the contents are not stable\.

\* [\.\./\.\./src/iterate\.h](../../src/iterate.h): defining `HAVE_ITERATE_H` supplies functions\.

 * Parameter: ARRAY\_NAME, ARRAY\_TYPE  
   `<t>` that satisfies `C` naming conventions when mangled and a valid tag\-type, [&lt;pT&gt;type](#user-content-typedef-9b5be28b), associated therewith; required\.
 * Parameter: ARRAY\_COMPARE, ARRAY\_IS\_EQUAL  
   Compare trait contained in [\.\./\.\./src/compare\.h](../../src/compare.h)\. See [&lt;pT&gt;compare_fn](#user-content-typedef-223b1937) or [&lt;pT&gt;bipredicate_fn](#user-content-typedef-4ab43b88), \(but not both\.\)
 * Parameter: ARRAY\_TO\_STRING  
   To string trait contained in [\.\./\.\./src/to\_string\.h](../../src/to_string.h)\. See [&lt;pT&gt;to_string_fn](#user-content-typedef-4442127b)\.
 * Parameter: ARRAY\_EXPECT\_TRAIT, ARRAY\_TRAIT  
   Named traits are obtained by including `array.h` multiple times with `ARRAY_EXPECT_TRAIT` and then subsequently including the name in `ARRAY_TRAIT`\.
 * Parameter: ARRAY\_DECLARE\_ONLY  
   For headers in different compilation units\.
 * Standard:  
   C89
 * Dependancies:  
   [box](../../src/box.h)


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-9b5be28b" name = "user-content-typedef-9b5be28b">&lt;pT&gt;type</a> ###

<code>typedef ARRAY_TYPE <strong>&lt;pT&gt;type</strong>;</code>

A valid tag type set by `ARRAY_TYPE`\.



### <a id = "user-content-typedef-348726ce" name = "user-content-typedef-348726ce">&lt;pT&gt;action_fn</a> ###

<code>typedef void(*<strong>&lt;pT&gt;action_fn</strong>)(&lt;pT&gt;type *);</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h): Operates by side\-effects\.



### <a id = "user-content-typedef-ad32e23d" name = "user-content-typedef-ad32e23d">&lt;pT&gt;predicate_fn</a> ###

<code>typedef int(*<strong>&lt;pT&gt;predicate_fn</strong>)(const &lt;pT&gt;type *);</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h): Returns a boolean given read\-only\.



### <a id = "user-content-typedef-4442127b" name = "user-content-typedef-4442127b">&lt;pT&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;pT&gt;to_string_fn</strong>)(const &lt;pT&gt;type *, char(*)[12]);</code>

The type of the required `<tr>to_string`\. Responsible for turning the read\-only argument into a 12\-max\-`char` output string\.



### <a id = "user-content-typedef-223b1937" name = "user-content-typedef-223b1937">&lt;pT&gt;compare_fn</a> ###

<code>typedef int(*<strong>&lt;pT&gt;compare_fn</strong>)(const &lt;pT&gt;type *restrict a, const &lt;pT&gt;type *restrict b);</code>

[\.\./\.\./src/compare\.h](../../src/compare.h): The type of the required `<tr>compare`\. Three\-way comparison on a totally order set; returns an integer value less than, equal to, greater than zero, if `a < b`, `a == b`, `a > b`, respectively\.



### <a id = "user-content-typedef-4ab43b88" name = "user-content-typedef-4ab43b88">&lt;pT&gt;bipredicate_fn</a> ###

<code>typedef int(*<strong>&lt;pT&gt;bipredicate_fn</strong>)(&lt;pT&gt;type *restrict, &lt;pT&gt;type *restrict);</code>

[\.\./\.\./src/compare\.h](../../src/compare.h): The type of the required `<tr>is_equal`\. Returns a symmetric boolean given two read\-only elements\.



### <a id = "user-content-typedef-5082f141" name = "user-content-typedef-5082f141">&lt;pT&gt;biaction_fn</a> ###

<code>typedef int(*<strong>&lt;pT&gt;biaction_fn</strong>)(&lt;pT&gt;type *restrict, &lt;pT&gt;type *restrict);</code>

[\.\./\.\./src/compare\.h](../../src/compare.h): Returns a boolean given two modifiable arguments\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-9c4cf562" name = "user-content-tag-9c4cf562">&lt;t&gt;array</a> ###

<code>struct <strong>&lt;t&gt;array</strong> { size_t size, capacity; &lt;pT&gt;type *data; };</code>

Manages the array field `data` which has `size` elements\. The space is indexed up to `capacity`, which is at least `size`\.

![States.](../../doc/array/states.png)



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static struct &lt;t&gt;array</td><td><a href = "#user-content-fn-9c4cf562">&lt;t&gt;array</a></td><td></td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-4a260f07">&lt;t&gt;array_</a></td><td>a</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-8319dfb7">&lt;T&gt;reserve</a></td><td>a, min</td></tr>

<tr><td align = right>static &lt;pT&gt;type *</td><td><a href = "#user-content-fn-c6b6f48f">&lt;T&gt;buffer</a></td><td>a, n</td></tr>

<tr><td align = right>static &lt;pT&gt;type *</td><td><a href = "#user-content-fn-faa8ce4d">&lt;T&gt;append</a></td><td>a, n</td></tr>

<tr><td align = right>static &lt;pT&gt;type *</td><td><a href = "#user-content-fn-e80ff7d4">&lt;T&gt;insert</a></td><td>a, n, at</td></tr>

<tr><td align = right>static &lt;pT&gt;type *</td><td><a href = "#user-content-fn-222fef85">&lt;T&gt;new</a></td><td>a</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-56806709">&lt;T&gt;remove</a></td><td>a, element</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-5b1cd3ec">&lt;T&gt;lazy_remove</a></td><td>a, datum</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-7f4a964e">&lt;T&gt;clear</a></td><td>a</td></tr>

<tr><td align = right>static &lt;pT&gt;type *</td><td><a href = "#user-content-fn-1900dfa2">&lt;T&gt;peek</a></td><td>a</td></tr>

<tr><td align = right>static &lt;pT&gt;type *</td><td><a href = "#user-content-fn-3e8e8234">&lt;T&gt;pop</a></td><td>a</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-dbd2279">&lt;T&gt;splice</a></td><td>a, b, i0, i1</td></tr>

<tr><td align = right>static &lt;pT&gt;type *</td><td><a href = "#user-content-fn-443f2b31">&lt;TR&gt;any</a></td><td>box, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-51d87ca4">&lt;TR&gt;each</a></td><td>box, action</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-21ef106e">&lt;TR&gt;if_each</a></td><td>box, predicate, action</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-f61ec8de">&lt;TR&gt;copy_if</a></td><td>dst, src, copy</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-8bb1c0a2">&lt;TR&gt;keep_if</a></td><td>box, keep, destruct</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-a76df7bd">&lt;TR&gt;trim</a></td><td>box, predicate</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-260f8348">&lt;TR&gt;to_string</a></td><td>box</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-aa7d8478">&lt;TR&gt;compare</a></td><td>a, b</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-b6d70ac1">&lt;TR&gt;lower_bound</a></td><td>box, element</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-bbcea84">&lt;TR&gt;upper_bound</a></td><td>box, element</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-3ff5a0f5">&lt;TR&gt;insert_after</a></td><td>box, element</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-17397135">&lt;TR&gt;sort</a></td><td>box</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d9028091">&lt;TR&gt;reverse</a></td><td>box</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-72cedc06">&lt;TR&gt;is_equal</a></td><td>a, b</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-24266775">&lt;TR&gt;unique_merge</a></td><td>box, merge</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-2527597a">&lt;TR&gt;unique</a></td><td>box</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-9c4cf562" name = "user-content-fn-9c4cf562">&lt;t&gt;array</a> ###

<code>static struct &lt;t&gt;array <strong>&lt;t&gt;array</strong>(void)</code>

Zeroed data \(not all\-bits\-zero\) is initialized, as well\.

 * Return:  
   An idle array\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-4a260f07" name = "user-content-fn-4a260f07">&lt;t&gt;array_</a> ###

<code>static void <strong>&lt;t&gt;array_</strong>(struct &lt;t&gt;array *const <em>a</em>)</code>

If `a` is not null, destroys and returns it to idle\.



### <a id = "user-content-fn-8319dfb7" name = "user-content-fn-8319dfb7">&lt;T&gt;reserve</a> ###

<code>static int <strong>&lt;T&gt;reserve</strong>(struct &lt;t&gt;array *const <em>a</em>, const size_t <em>min</em>)</code>

Ensures `min` capacity of `a`\. Invalidates pointers in `a`\.

 * Parameter: _min_  
   If zero, does nothing\.
 * Return:  
   Success; otherwise, `errno` will be set\.
 * Exceptional return: ERANGE  
   Tried allocating more then can fit in `size_t` or `realloc` doesn't follow POSIX\.
 * Exceptional return: realloc  




### <a id = "user-content-fn-c6b6f48f" name = "user-content-fn-c6b6f48f">&lt;T&gt;buffer</a> ###

<code>static &lt;pT&gt;type *<strong>&lt;T&gt;buffer</strong>(struct &lt;t&gt;array *const <em>a</em>, const size_t <em>n</em>)</code>

The capacity of `a` will be increased to at least `n` elements beyond the size\. Invalidates any pointers in `a`\.

 * Return:  
   The start of the buffered space at the back of the array\. If `a` is idle and `buffer` is zero, a null pointer is returned, otherwise null indicates an error\.
 * Exceptional return: realloc  




### <a id = "user-content-fn-faa8ce4d" name = "user-content-fn-faa8ce4d">&lt;T&gt;append</a> ###

<code>static &lt;pT&gt;type *<strong>&lt;T&gt;append</strong>(struct &lt;t&gt;array *const <em>a</em>, const size_t <em>n</em>)</code>

Adds `n` elements to the back of `a`\. It will invalidate pointers in `a` if `n` is greater than the buffer space\.

 * Return:  
   A pointer to the elements\. If `a` is idle and `n` is zero, a null pointer will be returned, otherwise null indicates an error\.
 * Exceptional return: realloc, ERANGE  
 * Implements:  
   `append` from `BOX_CONTIGUOUS`




### <a id = "user-content-fn-e80ff7d4" name = "user-content-fn-e80ff7d4">&lt;T&gt;insert</a> ###

<code>static &lt;pT&gt;type *<strong>&lt;T&gt;insert</strong>(struct &lt;t&gt;array *const <em>a</em>, const size_t <em>n</em>, const size_t <em>at</em>)</code>

Adds `n` un\-initialised elements at position `at` in `a`\. It will invalidate any pointers in `a` if the buffer holds too few elements\.

 * Parameter: _at_  
   A number smaller than or equal to `a.size`; if `a.size`, this function behaves as [&lt;T&gt;append](#user-content-fn-faa8ce4d)\.
 * Return:  
   A pointer to the start of the new region, where there are `n` elements\.
 * Exceptional return: realloc, ERANGE  




### <a id = "user-content-fn-222fef85" name = "user-content-fn-222fef85">&lt;T&gt;new</a> ###

<code>static &lt;pT&gt;type *<strong>&lt;T&gt;new</strong>(struct &lt;t&gt;array *const <em>a</em>)</code>

 * Return:  
   Adds \(push back\) one new element of `a`\. The buffer space holds at least one element, or it may invalidate pointers in `a`\.
 * Exceptional return: realloc, ERANGE  
 * Order:  
   amortised &#927;\(1\)




### <a id = "user-content-fn-56806709" name = "user-content-fn-56806709">&lt;T&gt;remove</a> ###

<code>static void <strong>&lt;T&gt;remove</strong>(struct &lt;t&gt;array *const <em>a</em>, &lt;pT&gt;type *const <em>element</em>)</code>

Removes `element` from `a`\. Do not attempt to remove an element that is not in `a`\.

 * Order:  
   &#927;\(`a.size`\)\.




### <a id = "user-content-fn-5b1cd3ec" name = "user-content-fn-5b1cd3ec">&lt;T&gt;lazy_remove</a> ###

<code>static void <strong>&lt;T&gt;lazy_remove</strong>(struct &lt;t&gt;array *const <em>a</em>, &lt;pT&gt;type *const <em>datum</em>)</code>

Removes `datum` from `a` and replaces it with the tail\. Do not attempt to remove an element that is not in `a`\.

 * Order:  
   &#927;\(1\)\.




### <a id = "user-content-fn-7f4a964e" name = "user-content-fn-7f4a964e">&lt;T&gt;clear</a> ###

<code>static void <strong>&lt;T&gt;clear</strong>(struct &lt;t&gt;array *const <em>a</em>)</code>

Sets `a` to be empty\. That is, the size of `a` will be zero, but if it was previously in an active non\-idle state, it continues to be\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-1900dfa2" name = "user-content-fn-1900dfa2">&lt;T&gt;peek</a> ###

<code>static &lt;pT&gt;type *<strong>&lt;T&gt;peek</strong>(const struct &lt;t&gt;array *const <em>a</em>)</code>

 * Return:  
   The last element or null if `a` is empty\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-3e8e8234" name = "user-content-fn-3e8e8234">&lt;T&gt;pop</a> ###

<code>static &lt;pT&gt;type *<strong>&lt;T&gt;pop</strong>(struct &lt;t&gt;array *const <em>a</em>)</code>

 * Return:  
   Value from the the top of `a` that is removed or null if the array is empty\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-dbd2279" name = "user-content-fn-dbd2279">&lt;T&gt;splice</a> ###

<code>static int <strong>&lt;T&gt;splice</strong>(struct &lt;t&gt;array *restrict const <em>a</em>, const struct &lt;t&gt;array *restrict const <em>b</em>, const size_t <em>i0</em>, const size_t <em>i1</em>)</code>

Indices \[`i0`, `i1`\) of `a` will be replaced with a copy of `b`\.

 * Parameter: _b_  
   Can be null, which acts as empty, but cannot overlap with `a`\.
 * Return:  
   Success\.
 * Exceptional return: realloc, ERANGE  




### <a id = "user-content-fn-443f2b31" name = "user-content-fn-443f2b31">&lt;TR&gt;any</a> ###

<code>static &lt;pT&gt;type *<strong>&lt;TR&gt;any</strong>(const &lt;pT&gt;box *const <em>box</em>, const &lt;pTR&gt;predicate_fn <em>predicate</em>)</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h): Iterates through `box` and calls `predicate` until it returns true\.

 * Return:  
   The first `predicate` that returned true, or, if the statement is false on all, null\.
 * Order:  
   &#927;\(`box.size`\) &#215; &#927;\(`predicate`\)




### <a id = "user-content-fn-51d87ca4" name = "user-content-fn-51d87ca4">&lt;TR&gt;each</a> ###

<code>static void <strong>&lt;TR&gt;each</strong>(&lt;pT&gt;box *const <em>box</em>, const &lt;pTR&gt;action_fn <em>action</em>)</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h): Iterates through `box` and calls `action` on all the elements\. Differs calling `action` until the iterator is one\-ahead, so can delete elements as long as it doesn't affect the next, \(specifically, a linked\-list\.\)

 * Order:  
   &#927;\(|`box`|\) &#215; &#927;\(`action`\)




### <a id = "user-content-fn-21ef106e" name = "user-content-fn-21ef106e">&lt;TR&gt;if_each</a> ###

<code>static void <strong>&lt;TR&gt;if_each</strong>(&lt;pT&gt;box *const <em>box</em>, const &lt;pTR&gt;predicate_fn <em>predicate</em>, const &lt;pTR&gt;action_fn <em>action</em>)</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h): Iterates through `box` and calls `action` on all the elements for which `predicate` returns true\.

 * Order:  
   &#927;\(`box.size`\) &#215; \(&#927;\(`predicate`\) \+ &#927;\(`action`\)\)




### <a id = "user-content-fn-f61ec8de" name = "user-content-fn-f61ec8de">&lt;TR&gt;copy_if</a> ###

<code>static int <strong>&lt;TR&gt;copy_if</strong>(&lt;pT&gt;box *restrict const <em>dst</em>, const &lt;pTR&gt;box *restrict const <em>src</em>, const &lt;pTR&gt;predicate_fn <em>copy</em>)</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h), `pT_CONTIGUOUS`: For all elements of `src`, calls `copy`, and if true, lazily copies the elements to `dst`\. `dst` and `src` can not be the same but `src` can be null, \(in which case, it does nothing\.\)

 * Exceptional return: realloc  
 * Order:  
   &#927;\(|`src`|\) &#215; &#927;\(`copy`\)




### <a id = "user-content-fn-8bb1c0a2" name = "user-content-fn-8bb1c0a2">&lt;TR&gt;keep_if</a> ###

<code>static void <strong>&lt;TR&gt;keep_if</strong>(&lt;pT&gt;box *const <em>box</em>, const &lt;pTR&gt;predicate_fn <em>keep</em>, const &lt;pTR&gt;action_fn <em>destruct</em>)</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h): For all elements of `box`, calls `keep`, and if false, if contiguous, lazy deletes that item, if not, eagerly\. Calls `destruct` if not\-null before deleting\.

 * Order:  
   &#927;\(|`box`|\) \(&#215; O\(`keep`\) \+ O\(`destruct`\)\)




### <a id = "user-content-fn-a76df7bd" name = "user-content-fn-a76df7bd">&lt;TR&gt;trim</a> ###

<code>static void <strong>&lt;TR&gt;trim</strong>(&lt;pT&gt;box *const <em>box</em>, const &lt;pTR&gt;predicate_fn <em>predicate</em>)</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h), `pT_CONTIGUOUS`: Removes at either end of `box` the things that `predicate`, if it exists, returns true\.

 * Order:  
   &#927;\(`box.size`\) &#215; &#927;\(`predicate`\)




### <a id = "user-content-fn-260f8348" name = "user-content-fn-260f8348">&lt;TR&gt;to_string</a> ###

<code>static const char *<strong>&lt;TR&gt;to_string</strong>(const &lt;pT&gt;box *const <em>box</em>)</code>

[\.\./\.\./src/to\_string\.h](../../src/to_string.h): print the contents of `box` in a static string buffer of 256 bytes, with limitations of only printing 4 things in a single sequence point\.

 * Return:  
   Address of the static buffer\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-aa7d8478" name = "user-content-fn-aa7d8478">&lt;TR&gt;compare</a> ###

<code>static int <strong>&lt;TR&gt;compare</strong>(const &lt;pT&gt;box *restrict const <em>a</em>, const &lt;pT&gt;box *restrict const <em>b</em>)</code>

[\.\./\.\./src/compare\.h](../../src/compare.h), `COMPARE`: Lexicographically compares `a` to `b`\. Both can be null, with null values before everything\.

 * Return:  
   `a < b`: negative; `a == b`: zero; `a > b`: positive\.
 * Order:  
   &#927;\(`|a|` & `|b|`\)




### <a id = "user-content-fn-b6d70ac1" name = "user-content-fn-b6d70ac1">&lt;TR&gt;lower_bound</a> ###

<code>static size_t <strong>&lt;TR&gt;lower_bound</strong>(const &lt;pT&gt;box *const <em>box</em>, const &lt;pT&gt;type *const <em>element</em>)</code>

[\.\./\.\./src/compare\.h](../../src/compare.h), `COMPARE`, `BOX_ACCESS`: `box` should be partitioned true/false with less\-then `element`\.

 * Return:  
   The first index of `a` that is not less than `cursor`\.
 * Order:  
   &#927;\(log `a.size`\)




### <a id = "user-content-fn-bbcea84" name = "user-content-fn-bbcea84">&lt;TR&gt;upper_bound</a> ###

<code>static size_t <strong>&lt;TR&gt;upper_bound</strong>(const &lt;pT&gt;box *const <em>box</em>, const &lt;pT&gt;type *const <em>element</em>)</code>

[\.\./\.\./src/compare\.h](../../src/compare.h), `COMPARE`, `BOX_ACCESS`: `box` should be partitioned false/true with greater\-than or equal\-to `element`\.

 * Return:  
   The first index of `box` that is greater than `element`\.
 * Order:  
   &#927;\(log |`box`|\)




### <a id = "user-content-fn-3ff5a0f5" name = "user-content-fn-3ff5a0f5">&lt;TR&gt;insert_after</a> ###

<code>static int <strong>&lt;TR&gt;insert_after</strong>(&lt;pT&gt;box *const <em>box</em>, const &lt;pT&gt;type *const <em>element</em>)</code>

[\.\./\.\./src/compare\.h](../../src/compare.h), `COMPARE`, `BOX_CONTIGUOUS`: Copies `element` at the upper bound of a sorted `box`\.

 * Return:  
   Success\.
 * Exceptional return: realloc, ERANGE  
 * Order:  
   &#927;\(`a.size`\)




### <a id = "user-content-fn-17397135" name = "user-content-fn-17397135">&lt;TR&gt;sort</a> ###

<code>static void <strong>&lt;TR&gt;sort</strong>(&lt;pT&gt;box *const <em>box</em>)</code>

[\.\./\.\./src/compare\.h](../../src/compare.h), `COMPARE`, `BOX_CONTIGUOUS`: Sorts `box` by `qsort`, \(which has a high\-context\-switching cost, but is easy\.\)

 * Order:  
   &#927;\(|`box`| log |`box`|\)




### <a id = "user-content-fn-d9028091" name = "user-content-fn-d9028091">&lt;TR&gt;reverse</a> ###

<code>static void <strong>&lt;TR&gt;reverse</strong>(&lt;pT&gt;box *const <em>box</em>)</code>

[\.\./\.\./src/compare\.h](../../src/compare.h), `COMPARE`, `BOX_CONTIGUOUS`: Sorts `box` in reverse by `qsort`\.

 * Order:  
   &#927;\(|`box`| log |`box`|\)




### <a id = "user-content-fn-72cedc06" name = "user-content-fn-72cedc06">&lt;TR&gt;is_equal</a> ###

<code>static int <strong>&lt;TR&gt;is_equal</strong>(const &lt;pT&gt;box *restrict const <em>a</em>, const &lt;pT&gt;box *restrict const <em>b</em>)</code>

[\.\./\.\./src/compare\.h](../../src/compare.h)

 * Return:  
   If `a` piecewise equals `b`, which both can be null\.
 * Order:  
   &#927;\(|`a`| & |`b`|\)




### <a id = "user-content-fn-24266775" name = "user-content-fn-24266775">&lt;TR&gt;unique_merge</a> ###

<code>static void <strong>&lt;TR&gt;unique_merge</strong>(&lt;pT&gt;box *const <em>box</em>, const &lt;pT&gt;biaction_fn <em>merge</em>)</code>

[\.\./\.\./src/compare\.h](../../src/compare.h), `BOX_CONTIGUOUS`: Removes consecutive duplicate elements in `box` lazily\.

 * Parameter: _merge_  
   Controls surjection\. Called with duplicate elements, if false `(x, y)->(x)`, if true `(x,y)->(y)`\. More complex functions, `(x, y)->(x+y)` can be simulated by mixing the two in the value returned\. Can be null: behaves like false, always deleting the second element\.
 * Order:  
   &#927;\(|`box`|\) &#215; &#927;\(`merge`\)




### <a id = "user-content-fn-2527597a" name = "user-content-fn-2527597a">&lt;TR&gt;unique</a> ###

<code>static void <strong>&lt;TR&gt;unique</strong>(&lt;pT&gt;box *const <em>box</em>)</code>

[\.\./\.\./src/compare\.h](../../src/compare.h), `BOX_CONTIGUOUS`: Removes consecutive duplicate elements in `box`\.

 * Order:  
   &#927;\(|`box`|\)






## <a id = "user-content-license" name = "user-content-license">License</a> ##

2016 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



