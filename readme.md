# heap\.h #

## Priority Queue ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PH&gt;priority](#user-content-typedef-775cba47), [&lt;PH&gt;compare_fn](#user-content-typedef-dee13533), [&lt;PH&gt;node](#user-content-typedef-23ae637f), [&lt;PH&gt;biaction_fn](#user-content-typedef-7e815a45), [&lt;PSZ&gt;to_string_fn](#user-content-typedef-8b890812)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;H&gt;heap_node](#user-content-tag-7243593c), [&lt;H&gt;heap](#user-content-tag-8ef1078f), [&lt;PH&gt;iterator](#user-content-tag-52985d65)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of heap.](web/heap.png)

A [&lt;H&gt;heap](#user-content-tag-8ef1078f) is a binary heap, proposed by [Williams, 1964, Heapsort, p\. 347](https://scholar.google.ca/scholar?q=Williams%2C+1964%2C+Heapsort%2C+p.+347) and using terminology of [Knuth, 1973, Sorting](https://scholar.google.ca/scholar?q=Knuth%2C+1973%2C+Sorting)\. It can be used as an implementation of a priority queue; internally, it is a `<<H>heap_node>array` with implicit heap properties on [&lt;PH&gt;priority](#user-content-typedef-775cba47) and an optional [&lt;PH&gt;value](#user-content-typedef-a55b7cd4) pointer value\.



 * Parameter: HEAP\_NAME, HEAP\_TYPE  
   `<H>` that satisfies `C` naming conventions when mangled and an assignable type [&lt;PH&gt;priority](#user-content-typedef-775cba47) associated therewith\. `HEAP_NAME` is required; `HEAP_TYPE` defaults to `unsigned int`\. `<PH>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: HEAP\_COMPARE  
   A function satisfying [&lt;PH&gt;compare_fn](#user-content-typedef-dee13533)\. Defaults to minimum\-hash\. Required if `HEAP_TYPE` is changed to an incomparable type\.
 * Parameter: HEAP\_VALUE  
   Optional value [&lt;PH&gt;value](#user-content-typedef-a55b7cd4), that is stored as a reference in [&lt;H&gt;heap_node](#user-content-tag-7243593c); declaring it is sufficient\. If set, has no effect on the ranking, but affects [&lt;PH&gt;value](#user-content-typedef-a55b7cd4)\.
 * Parameter: HEAP\_TEST  
   To string trait contained in [\.\./test/heap\_test\.h](../test/heap_test.h); optional unit testing framework using `assert`\. Must be defined equal to a random filler function, satisfying [&lt;PH&gt;biaction_fn](#user-content-typedef-7e815a45) with the `param` of [&lt;H&gt;heap_test](#user-content-fn-2a4c2c14)\. Must have any `to string` trait\.
 * Parameter: HEAP\_EXPECT\_TRAIT  
   Do not un\-define certain variables for subsequent inclusion in a parameterized trait\.
 * Parameter: HEAP\_TO\_STRING\_NAME, HEAP\_TO\_STRING  
   To string trait contained in [to\_string\.h](to_string.h); an optional unique `<SZ>` that satisfies `C` naming conventions when mangled and function implementing [&lt;PSZ&gt;to_string_fn](#user-content-typedef-8b890812)\.
 * Standard:  
   C89
 * Dependancies:  
   [array](https://github.com/neil-edelman/array)
 * Caveat:  
   Add decrease priority\. Add replace\.


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-775cba47" name = "user-content-typedef-775cba47">&lt;PH&gt;priority</a> ###

<code>typedef HEAP_TYPE <strong>&lt;PH&gt;priority</strong>;</code>

Valid assignable type used for priority in [&lt;PH&gt;node](#user-content-typedef-23ae637f)\. Defaults to `unsigned int` if not set by `HEAP_TYPE`\.



### <a id = "user-content-typedef-dee13533" name = "user-content-typedef-dee13533">&lt;PH&gt;compare_fn</a> ###

<code>typedef int(*<strong>&lt;PH&gt;compare_fn</strong>)(const &lt;PH&gt;priority a, const &lt;PH&gt;priority b);</code>

Returns a positive result if `a` is out\-of\-order with respect to `b`, inducing a strict pre\-order\. This is compatible, but less strict then the comparators from `bsearch` and `qsort`; it only needs to divide entries into two instead of three categories\.



### <a id = "user-content-typedef-23ae637f" name = "user-content-typedef-23ae637f">&lt;PH&gt;node</a> ###

<code>typedef struct &lt;H&gt;heap_node <strong>&lt;PH&gt;node</strong>;</code>

If `HEAP_VALUE` is set, \(priority, value\) set by [&lt;H&gt;heap_node](#user-content-tag-7243593c), otherwise it's a \(priority\) set directly by [&lt;PH&gt;priority](#user-content-typedef-775cba47)\.



### <a id = "user-content-typedef-7e815a45" name = "user-content-typedef-7e815a45">&lt;PH&gt;biaction_fn</a> ###

<code>typedef void(*<strong>&lt;PH&gt;biaction_fn</strong>)(&lt;PH&gt;node *, void *);</code>

Used for `HEAP_TEST`, operates by side\-effects\.



### <a id = "user-content-typedef-8b890812" name = "user-content-typedef-8b890812">&lt;PSZ&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PSZ&gt;to_string_fn</strong>)(const &lt;PSZ&gt;type *, char(*)[12]);</code>

Responsible for turning the first argument into a 12\-`char` null\-terminated output string\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-7243593c" name = "user-content-tag-7243593c">&lt;H&gt;heap_node</a> ###

<code>struct <strong>&lt;H&gt;heap_node</strong> { &lt;PH&gt;priority priority; &lt;PH&gt;value value; };</code>

If `HEAP_VALUE` is set, this becomes [&lt;PH&gt;node](#user-content-typedef-23ae637f)\. Memory management for this structure is the responsibility of the caller\.



### <a id = "user-content-tag-8ef1078f" name = "user-content-tag-8ef1078f">&lt;H&gt;heap</a> ###

<code>struct <strong>&lt;H&gt;heap</strong> { struct &lt;PH&gt;node_array a; };</code>

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

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-2f5a4cc1">&lt;H&gt;heap_size</a></td><td>heap</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-42cb2b13">&lt;H&gt;heap_add</a></td><td>heap, node</td></tr>

<tr><td align = right>static &lt;PH&gt;value</td><td><a href = "#user-content-fn-921d7df">&lt;H&gt;heap_peek</a></td><td>heap</td></tr>

<tr><td align = right>static &lt;PH&gt;value</td><td><a href = "#user-content-fn-2cd270b7">&lt;H&gt;heap_pop</a></td><td>heap</td></tr>

<tr><td align = right>static &lt;PH&gt;node *</td><td><a href = "#user-content-fn-4355676a">&lt;H&gt;heap_buffer</a></td><td>heap, n</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-9c9f1648">&lt;H&gt;heap_append</a></td><td>heap, n</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-2a4c2c14">&lt;H&gt;heap_test</a></td><td>param</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-b11709d3">&lt;SZ&gt;to_string</a></td><td>box</td></tr>

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




### <a id = "user-content-fn-2f5a4cc1" name = "user-content-fn-2f5a4cc1">&lt;H&gt;heap_size</a> ###

<code>static size_t <strong>&lt;H&gt;heap_size</strong>(const struct &lt;H&gt;heap *const <em>heap</em>)</code>

Empty is `!size`\.

 * Return:  
   Size of the `heap`\.




### <a id = "user-content-fn-42cb2b13" name = "user-content-fn-42cb2b13">&lt;H&gt;heap_add</a> ###

<code>static int <strong>&lt;H&gt;heap_add</strong>(struct &lt;H&gt;heap *const <em>heap</em>, &lt;PH&gt;node <em>node</em>)</code>

Copies `node` into `heap`\.

 * Return:  
   Success\.
 * Exceptional return: ERANGE, realloc  
 * Order:  
   &#927;\(log `heap.size`\)




### <a id = "user-content-fn-921d7df" name = "user-content-fn-921d7df">&lt;H&gt;heap_peek</a> ###

<code>static &lt;PH&gt;value <strong>&lt;H&gt;heap_peek</strong>(const struct &lt;H&gt;heap *const <em>heap</em>)</code>

 * Return:  
   The lowest element in `heap` according to `HEAP_COMPARE` or null/zero if the heap is empty\. On some heaps, one may have to call [&lt;H&gt;heap_size](#user-content-fn-2f5a4cc1) in order to differentiate\.
 * Order:  
   &#927;\(1\)




### <a id = "user-content-fn-2cd270b7" name = "user-content-fn-2cd270b7">&lt;H&gt;heap_pop</a> ###

<code>static &lt;PH&gt;value <strong>&lt;H&gt;heap_pop</strong>(struct &lt;H&gt;heap *const <em>heap</em>)</code>

Remove the lowest element in `heap` according to `HEAP_COMPARE`\.

 * Return:  
   The same as [&lt;H&gt;heap_peek](#user-content-fn-921d7df)\.
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
   In practice, pushing uninitialized elements onto the heap does not make sense, so [&lt;H&gt;heap_buffer](#user-content-fn-4355676a) `n` will be called first, in which case, one is guaranteed success\.
 * Order:  
   &#927;\(`heap.size` \+ `n`\)




### <a id = "user-content-fn-2a4c2c14" name = "user-content-fn-2a4c2c14">&lt;H&gt;heap_test</a> ###

<code>static void <strong>&lt;H&gt;heap_test</strong>(void *const <em>param</em>)</code>

Will be tested on stdout\. Requires `HEAP_TEST`, `HEAP_TO_STRING`, and not `NDEBUG` while defining `assert`\.

 * Parameter: _param_  
   The `void *` parameter in `HEAP_TEST`\. Can be null\.




### <a id = "user-content-fn-b11709d3" name = "user-content-fn-b11709d3">&lt;SZ&gt;to_string</a> ###

<code>static const char *<strong>&lt;SZ&gt;to_string</strong>(const &lt;PSZ&gt;box *const <em>box</em>)</code>

 * Return:  
   Print the contents of `box` in a static string buffer of 256 bytes with limitations of only printing 4 things at a time\.
 * Order:  
   &#920;\(1\)






## <a id = "user-content-license" name = "user-content-license">License</a> ##

2020 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



