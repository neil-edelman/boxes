# heap\.h #

## Priority Queue ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PH&gt;priority](#user-content-typedef-775cba47), [&lt;PH&gt;compare_fn](#user-content-typedef-dee13533), [&lt;PH&gt;adjunct](#user-content-typedef-5aee1bc), [&lt;PH&gt;value](#user-content-typedef-a55b7cd4), [&lt;PH&gt;node](#user-content-typedef-23ae637f), [&lt;PZ&gt;action_fn](#user-content-typedef-9321d9ec), [&lt;PZ&gt;predicate_fn](#user-content-typedef-ad62af5b), [&lt;PZ&gt;to_string_fn](#user-content-typedef-22f3d7f1)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;H&gt;heap_node](#user-content-tag-7243593c), [&lt;H&gt;heap](#user-content-tag-8ef1078f), [&lt;PH&gt;iterator](#user-content-tag-52985d65)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of heap.](web/heap.png)

A [&lt;H&gt;heap](#user-content-tag-8ef1078f) is a priority queue built from [&lt;H&gt;heap_node](#user-content-tag-7243593c)\. It is a binary heap, proposed by [Williams, 1964, Heapsort, p\. 347](https://scholar.google.ca/scholar?q=Williams%2C+1964%2C+Heapsort%2C+p.+347) and using terminology of [Knuth, 1973, Sorting](https://scholar.google.ca/scholar?q=Knuth%2C+1973%2C+Sorting)\. Internally, it is an `<<H>heap_node>array` with implicit heap properties, with an optionally cached [&lt;PH&gt;priority](#user-content-typedef-775cba47) and an optional [&lt;PH&gt;value](#user-content-typedef-a55b7cd4) pointer payload\. As such, one needs to have [array\.h](array.h) file in the same directory\.



 * Parameter: HEAP\_NAME, HEAP\_TYPE  
   `<H>` that satisfies `C` naming conventions when mangled and an assignable type [&lt;PH&gt;priority](#user-content-typedef-775cba47) associated therewith\. `HEAP_NAME` is required but `HEAP_TYPE` defaults to `unsigned int` if not specified\. `<PH>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: HEAP\_COMPARE  
   A function satisfying [&lt;PH&gt;compare_fn](#user-content-typedef-dee13533)\. Defaults to minimum\-hash on `HEAP_TYPE`; as such, required if `HEAP_TYPE` is changed to an incomparable type\.
 * Parameter: HEAP\_VALUE  
   Optional payload [&lt;PH&gt;adjunct](#user-content-typedef-5aee1bc), that is stored as a reference in [&lt;H&gt;heap_node](#user-content-tag-7243593c) as [&lt;PH&gt;value](#user-content-typedef-a55b7cd4); declaring it is sufficient\.
 * Parameter: HEAP\_TEST  
   To string trait contained in [\.\./test/heap\_test\.h](../test/heap_test.h); optional unit testing framework using `assert`\. Must be defined equal to a random filler function, satisfying `void (*<PH>biaction_fn)(<PH>node *, void *)` with the `param` of [&lt;H&gt;heap_test](#user-content-fn-2a4c2c14)\. Must have any To String trait\.
 * Parameter: HEAP\_EXPECT\_TRAIT  
   Do not un\-define certain variables for subsequent inclusion in a parameterized trait\.
 * Parameter: HEAP\_TO\_STRING\_NAME, HEAP\_TO\_STRING  
   To string trait contained in [to\_string\.h](to_string.h); an optional unique `<Z>` that satisfies `C` naming conventions when mangled and function implementing [&lt;PZ&gt;to_string_fn](#user-content-typedef-22f3d7f1)\.
 * Standard:  
   C89
 * Dependancies:  
   [array](https://github.com/neil-edelman/array)
 * Caveat:  
   Add decrease priority\. ([&lt;Z&gt;trim](#user-content-fn-d627ae9f))


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-775cba47" name = "user-content-typedef-775cba47">&lt;PH&gt;priority</a> ###

<code>typedef HEAP_TYPE <strong>&lt;PH&gt;priority</strong>;</code>

Valid assignable type used for priority in [&lt;PH&gt;node](#user-content-typedef-23ae637f)\. Defaults to `unsigned int` if not set by `HEAP_TYPE`\.



### <a id = "user-content-typedef-dee13533" name = "user-content-typedef-dee13533">&lt;PH&gt;compare_fn</a> ###

<code>typedef int(*<strong>&lt;PH&gt;compare_fn</strong>)(const &lt;PH&gt;priority a, const &lt;PH&gt;priority b);</code>

Returns a positive result if `a` comes after `b`, inducing a strict pre\-order of `a` with respect to `b`; this is compatible, but less strict then the comparators from `bsearch` and `qsort`; it only needs to divide entries into two instead of three categories\. The default `HEAP_COMPARE` is `a > b`, which makes a minimum\-hash\.



### <a id = "user-content-typedef-5aee1bc" name = "user-content-typedef-5aee1bc">&lt;PH&gt;adjunct</a> ###

<code>typedef HEAP_VALUE <strong>&lt;PH&gt;adjunct</strong>;</code>

If `HEAP_VALUE` is set, a declared tag type\.



### <a id = "user-content-typedef-a55b7cd4" name = "user-content-typedef-a55b7cd4">&lt;PH&gt;value</a> ###

<code>typedef &lt;PH&gt;adjunct *<strong>&lt;PH&gt;value</strong>;</code>

If `HEAP_VALUE` is set, this is a pointer to it, otherwise a boolean value that is true when there is an item\.



### <a id = "user-content-typedef-23ae637f" name = "user-content-typedef-23ae637f">&lt;PH&gt;node</a> ###

<code>typedef struct &lt;H&gt;heap_node <strong>&lt;PH&gt;node</strong>;</code>

Internal nodes in the heap\. If `HEAP_VALUE` is set, this is a [&lt;H&gt;heap_node](#user-content-tag-7243593c), otherwise it's the same as [&lt;PH&gt;priority](#user-content-typedef-775cba47)\.



### <a id = "user-content-typedef-9321d9ec" name = "user-content-typedef-9321d9ec">&lt;PZ&gt;action_fn</a> ###

<code>typedef void(*<strong>&lt;PZ&gt;action_fn</strong>)(&lt;PZ&gt;type *);</code>

Operates by side\-effects on [&lt;PZ&gt;type](#user-content-typedef-bfd92b5)\.



### <a id = "user-content-typedef-ad62af5b" name = "user-content-typedef-ad62af5b">&lt;PZ&gt;predicate_fn</a> ###

<code>typedef int(*<strong>&lt;PZ&gt;predicate_fn</strong>)(const &lt;PZ&gt;type *);</code>

Returns a boolean given read\-only [&lt;PZ&gt;type](#user-content-typedef-bfd92b5)\.



### <a id = "user-content-typedef-22f3d7f1" name = "user-content-typedef-22f3d7f1">&lt;PZ&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PZ&gt;to_string_fn</strong>)(const &lt;PZ&gt;type *, char(*)[12]);</code>

Responsible for turning the first argument into a 12\-`char` null\-terminated output string\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-7243593c" name = "user-content-tag-7243593c">&lt;H&gt;heap_node</a> ###

<code>struct <strong>&lt;H&gt;heap_node</strong> { &lt;PH&gt;priority priority; &lt;PH&gt;value value; };</code>

If `HEAP_VALUE` is set, creates a value as the payload of [&lt;PH&gt;node](#user-content-typedef-23ae637f)\.



### <a id = "user-content-tag-8ef1078f" name = "user-content-tag-8ef1078f">&lt;H&gt;heap</a> ###

<code>struct <strong>&lt;H&gt;heap</strong>;</code>

Stores the heap as an implicit binary tree in an array called `a`\. To initialize it to an idle state, see [&lt;H&gt;heap](#user-content-fn-8ef1078f), `HEAP_IDLE`, `{0}` \(`C99`\), or being `static`\.

![States.](web/states.png)



### <a id = "user-content-tag-52985d65" name = "user-content-tag-52985d65">&lt;PH&gt;iterator</a> ###

<code>struct <strong>&lt;PH&gt;iterator</strong>;</code>

Contains all the iteration parameters\.



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-8ef1078f">&lt;H&gt;heap</a></td><td>heap</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d56f4c70">&lt;H&gt;heap_</a></td><td>heap</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d3572b1d">&lt;H&gt;heap_clear</a></td><td>heap</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-42cb2b13">&lt;H&gt;heap_add</a></td><td>heap, node</td></tr>

<tr><td align = right>static &lt;PH&gt;node *</td><td><a href = "#user-content-fn-921d7df">&lt;H&gt;heap_peek</a></td><td>heap</td></tr>

<tr><td align = right>static &lt;PH&gt;value</td><td><a href = "#user-content-fn-c69b891">&lt;H&gt;heap_peek_value</a></td><td>heap</td></tr>

<tr><td align = right>static &lt;PH&gt;value</td><td><a href = "#user-content-fn-2cd270b7">&lt;H&gt;heap_pop</a></td><td>heap</td></tr>

<tr><td align = right>static &lt;PH&gt;node *</td><td><a href = "#user-content-fn-4355676a">&lt;H&gt;heap_buffer</a></td><td>heap, n</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-9c9f1648">&lt;H&gt;heap_append</a></td><td>heap, n</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-98a4cc31">&lt;Z&gt;clip</a></td><td>box, i</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-b5eb2ff0">&lt;Z&gt;copy_if</a></td><td>a, copy, b</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-1dbab6d0">&lt;Z&gt;keep_if</a></td><td>box, keep, destruct</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d627ae9f">&lt;Z&gt;trim</a></td><td>box, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-cc3bf1de">&lt;Z&gt;each</a></td><td>box, action</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-e766a80c">&lt;Z&gt;if_each</a></td><td>box, predicate, action</td></tr>

<tr><td align = right>static const &lt;PZ&gt;type *</td><td><a href = "#user-content-fn-95a77627">&lt;Z&gt;any</a></td><td>box, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-2a4c2c14">&lt;H&gt;heap_test</a></td><td>param</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-4ecb4112">&lt;Z&gt;to_string</a></td><td>box</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-8ef1078f" name = "user-content-fn-8ef1078f">&lt;H&gt;heap</a> ###

<code>static void <strong>&lt;H&gt;heap</strong>(struct &lt;H&gt;heap *const <em>heap</em>)</code>

Initializes `heap` to be idle\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-d56f4c70" name = "user-content-fn-d56f4c70">&lt;H&gt;heap_</a> ###

<code>static void <strong>&lt;H&gt;heap_</strong>(struct &lt;H&gt;heap *const <em>heap</em>)</code>

Returns `heap` to the idle state where it takes no dynamic memory\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-d3572b1d" name = "user-content-fn-d3572b1d">&lt;H&gt;heap_clear</a> ###

<code>static void <strong>&lt;H&gt;heap_clear</strong>(struct &lt;H&gt;heap *const <em>heap</em>)</code>

Sets `heap` to be empty\. That is, the size of `heap` will be zero, but if it was previously in an active non\-idle state, it continues to be\.

 * Parameter: _heap_  
   If null, does nothing\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-42cb2b13" name = "user-content-fn-42cb2b13">&lt;H&gt;heap_add</a> ###

<code>static int <strong>&lt;H&gt;heap_add</strong>(struct &lt;H&gt;heap *const <em>heap</em>, &lt;PH&gt;node <em>node</em>)</code>

Copies `node` into `heap`\.

 * Return:  
   Success\.
 * Exceptional return: ERANGE, realloc  
 * Order:  
   &#927;\(log `heap.size`\)




### <a id = "user-content-fn-921d7df" name = "user-content-fn-921d7df">&lt;H&gt;heap_peek</a> ###

<code>static &lt;PH&gt;node *<strong>&lt;H&gt;heap_peek</strong>(const struct &lt;H&gt;heap *const <em>heap</em>)</code>

 * Return:  
   Lowest in `heap` according to `HEAP_COMPARE` or null if the heap is empty\. This pointer is valid only until one makes structural changes to the heap\.
 * Order:  
   &#927;\(1\)




### <a id = "user-content-fn-c69b891" name = "user-content-fn-c69b891">&lt;H&gt;heap_peek_value</a> ###

<code>static &lt;PH&gt;value <strong>&lt;H&gt;heap_peek_value</strong>(struct &lt;H&gt;heap *const <em>heap</em>)</code>

This returns the [&lt;PH&gt;value](#user-content-typedef-a55b7cd4) of the [&lt;PH&gt;node](#user-content-typedef-23ae637f) returned by [&lt;H&gt;heap_peek](#user-content-fn-921d7df), for convenience with some applications\. If `HEAP_VALUE`, this is a child of [&lt;H&gt;heap_peek](#user-content-fn-921d7df), otherwise it is a boolean `int`\.

 * Return:  
   Lowest [&lt;PH&gt;value](#user-content-typedef-a55b7cd4) in `heap` element according to `HEAP_COMPARE`; if the heap is empty, null or zero\.
 * Order:  
   &#927;\(1\)




### <a id = "user-content-fn-2cd270b7" name = "user-content-fn-2cd270b7">&lt;H&gt;heap_pop</a> ###

<code>static &lt;PH&gt;value <strong>&lt;H&gt;heap_pop</strong>(struct &lt;H&gt;heap *const <em>heap</em>)</code>

Remove the lowest element according to `HEAP_COMPARE`\.

 * Parameter: _heap_  
   If null, returns false\.
 * Return:  
   The [&lt;PH&gt;value](#user-content-typedef-a55b7cd4) of the element that was removed; if the heap is empty, null or zero\.
 * Order:  
   &#927;\(log `size`\)




### <a id = "user-content-fn-4355676a" name = "user-content-fn-4355676a">&lt;H&gt;heap_buffer</a> ###

<code>static &lt;PH&gt;node *<strong>&lt;H&gt;heap_buffer</strong>(struct &lt;H&gt;heap *const <em>heap</em>, const size_t <em>n</em>)</code>

The capacity of `heap` will be increased to at least `n` elements beyond the size\. Invalidates pointers in `a`\.

 * Return:  
   The start of the buffered space\. If `a` is idle and `buffer` is zero, a null pointer is returned, otherwise null indicates an error\.
 * Exceptional return: realloc, ERANGE  




### <a id = "user-content-fn-9c9f1648" name = "user-content-fn-9c9f1648">&lt;H&gt;heap_append</a> ###

<code>static int <strong>&lt;H&gt;heap_append</strong>(struct &lt;H&gt;heap *const <em>heap</em>, const size_t <em>n</em>)</code>

Adds and heapifies `n` elements to `heap`\. Uses [Doberkat, 1984, Floyd](https://scholar.google.ca/scholar?q=Doberkat%2C+1984%2C+Floyd) to sift\-down all the internal nodes of heap, including any previous elements\. As such, this function is most efficient on a heap of zero size, and becomes increasingly inefficient as the heap grows\. For heaps that are already in use, it may be better to add each element individually, resulting in a run\-time of &#927;\(`new elements` &#183; log `heap.size`\)\.

 * Parameter: _n_  
   If zero, returns true without heapifying\.
 * Return:  
   Success\.
 * Exceptional return: ERANGE, realloc  
   In practice, pushing uninitialized elements onto the heap does make sense, so [&lt;H&gt;heap_buffer](#user-content-fn-4355676a) `n` will be called first, in which case, one is guaranteed success\.
 * Order:  
   &#927;\(`heap.size` \+ `n`\)




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




### <a id = "user-content-fn-2a4c2c14" name = "user-content-fn-2a4c2c14">&lt;H&gt;heap_test</a> ###

<code>static void <strong>&lt;H&gt;heap_test</strong>(void *const <em>param</em>)</code>

Will be tested on stdout\. Requires `HEAP_TEST`, `HEAP_TO_STRING`, and not `NDEBUG` while defining `assert`\.

 * Parameter: _param_  
   The `void *` parameter in `HEAP_TEST`\. Can be null\.




### <a id = "user-content-fn-4ecb4112" name = "user-content-fn-4ecb4112">&lt;Z&gt;to_string</a> ###

<code>static const char *<strong>&lt;Z&gt;to_string</strong>(const &lt;PZ&gt;box *const <em>box</em>)</code>

 * Return:  
   Print the contents of `box` in a static string buffer of 256 bytes with limitations of only printing 4 things at a time\.
 * Order:  
   &#920;\(1\)






## <a id = "user-content-license" name = "user-content-license">License</a> ##

2020 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



