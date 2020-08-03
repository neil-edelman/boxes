# Heap\.h #

## Priority Queue ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PH&gt;priority](#user-content-typedef-775cba47), [&lt;PH&gt;compare_fn](#user-content-typedef-dee13533), [&lt;PH&gt;value](#user-content-typedef-a55b7cd4), [&lt;PH&gt;pvalue](#user-content-typedef-eccf9f42), [&lt;PA&gt;to_string_fn](#user-content-typedef-a933c596), [&lt;PH&gt;biaction_fn](#user-content-typedef-7e815a45)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;H&gt;heap_node](#user-content-tag-7243593c), [&lt;H&gt;heap](#user-content-tag-8ef1078f), [&lt;PH&gt;iterator](#user-content-tag-52985d65)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of heap.](web/heap.png)

A [&lt;H&gt;heap](#user-content-tag-8ef1078f) is a priority queue built from [&lt;H&gt;heap_node](#user-content-tag-7243593c)\. It is a binary heap, proposed by [Williams, 1964, Heapsort, p\. 347](https://scholar.google.ca/scholar?q=Williams%2C+1964%2C+Heapsort%2C+p.+347) and using terminology of [Knuth, 1973, Sorting](https://scholar.google.ca/scholar?q=Knuth%2C+1973%2C+Sorting)\. Internally, it is an `<<H>heap_node>array` with implicit heap properties, with an optionally cached [&lt;PH&gt;priority](#user-content-typedef-775cba47) and an optional [&lt;PH&gt;value](#user-content-typedef-a55b7cd4) pointer payload\. As such, one needs to have `Array.h` file in the same directory\.

`<H>heap` is not synchronised\. Errors are returned with `errno`\. The parameters are `#define` preprocessor macros, and are all undefined at the end of the file for convenience\. Assertions are used in this file; to stop them, define `NDEBUG` before `assert.h`\.



 * Parameter: HEAP\_NAME, HEAP\_TYPE  
   `<H>` that satisfies `C` naming conventions when mangled and an assignable type [&lt;PH&gt;priority](#user-content-typedef-775cba47) associated therewith\. `HEAP_NAME` is required but `HEAP_TYPE` defaults to `unsigned int` if not specified\. `<PH>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: HEAP\_COMPARE  
   A function satisfying [&lt;PH&gt;compare_fn](#user-content-typedef-dee13533)\. Defaults to minimum\-hash on `HEAP_TYPE`; as such, required if `HEAP_TYPE` is changed to an incomparable type\.
 * Parameter: HEAP\_VALUE  
   Optional payload [&lt;PH&gt;value](#user-content-typedef-a55b7cd4), that is stored as a reference in [&lt;H&gt;heap_node](#user-content-tag-7243593c); declaring it is sufficient\.
 * Parameter: HEAP\_EXPECT\_TRAIT  
   Do not un\-define certain variables for subsequent inclusion in a trait\.
 * Parameter: HEAP\_TO\_STRING\_NAME, HEAP\_TO\_STRING  
   To string trait contained in [ToString\.h](ToString.h); `<A>` that satisfies `C` naming conventions when mangled and function implementing [&lt;PA&gt;to_string_fn](#user-content-typedef-a933c596)\. There can be multiple to string traits, but only one can omit `HEAP_TO_STRING_NAME`\.
 * Parameter: HEAP\_TEST  
   To string trait contained in [\.\./test/HeapTest\.h](../test/HeapTest.h); optional unit testing framework using `assert`\. Can only be defined once _per_ `Heap`\. Must be defined equal to a \(random\) filler function, satisfying [&lt;PH&gt;biaction_fn](#user-content-typedef-7e815a45)\. Output will be shown with the to string trait in which it's defined; provides tests for the base code and all later traits\.
 * Standard:  
   C89
 * Dependancies:  
   [Array.h](../Array/)
 * Caveat:  
   Add decrease priority\.
 * See also:  
   [Array](https://github.com/neil-edelman/Array); [List](https://github.com/neil-edelman/List); [Orcish](https://github.com/neil-edelman/Orcish); [Pool](https://github.com/neil-edelman/Pool); [Set](https://github.com/neil-edelman/Set); [Trie](https://github.com/neil-edelman/Trie)


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-775cba47" name = "user-content-typedef-775cba47">&lt;PH&gt;priority</a> ###

<code>typedef HEAP_TYPE <strong>&lt;PH&gt;priority</strong>;</code>

Valid assignable type used for priority in [&lt;H&gt;heap_node](#user-content-tag-7243593c)\. Defaults to `unsigned int` if not set by `HEAP_TYPE`\.



### <a id = "user-content-typedef-dee13533" name = "user-content-typedef-dee13533">&lt;PH&gt;compare_fn</a> ###

<code>typedef int(*<strong>&lt;PH&gt;compare_fn</strong>)(const &lt;PH&gt;priority a, const &lt;PH&gt;priority b);</code>

Returns a positive result if `a` comes after `b`, inducing a strict pre\-order of `a` with respect to `b`; this is compatible, but less strict then the comparators from `bsearch` and `qsort`; it only needs to divide entries into two instead of three categories\. The default `HEAP_COMPARE` is `a > b`, which makes a minimum\-hash\.



### <a id = "user-content-typedef-a55b7cd4" name = "user-content-typedef-a55b7cd4">&lt;PH&gt;value</a> ###

<code>typedef HEAP_VALUE <strong>&lt;PH&gt;value</strong>;</code>

If `HEAP_VALUE` is set, a valid tag type, used as a pointer in [&lt;H&gt;heap_node](#user-content-tag-7243593c)\.



### <a id = "user-content-typedef-eccf9f42" name = "user-content-typedef-eccf9f42">&lt;PH&gt;pvalue</a> ###

<code>typedef &lt;PH&gt;value *<strong>&lt;PH&gt;pvalue</strong>;</code>

If `HEAP_VALUE` is set, a pointer to the [&lt;PH&gt;value](#user-content-typedef-a55b7cd4), otherwise a boolean `int` that is true \(one\) if the value exists and false \(zero\) if not\.



### <a id = "user-content-typedef-a933c596" name = "user-content-typedef-a933c596">&lt;PA&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PA&gt;to_string_fn</strong>)(const &lt;PA&gt;type *, char(*)[12]);</code>

Responsible for turning the first argument into a 12\-`char` null\-terminated output string\.



### <a id = "user-content-typedef-7e815a45" name = "user-content-typedef-7e815a45">&lt;PH&gt;biaction_fn</a> ###

<code>typedef void(*<strong>&lt;PH&gt;biaction_fn</strong>)(struct &lt;H&gt;heap_node *, void *);</code>

Operates by side\-effects\. Used for `HEAP_TEST`\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-7243593c" name = "user-content-tag-7243593c">&lt;H&gt;heap_node</a> ###

<code>struct <strong>&lt;H&gt;heap_node</strong>;</code>

Stores a [&lt;PH&gt;priority](#user-content-typedef-775cba47) as `priority`, which can be set by `HEAP_TYPE`\. If `HEAP_VALUE` is set, also stores a pointer [&lt;PH&gt;pvalue](#user-content-typedef-eccf9f42) called `value`\.



### <a id = "user-content-tag-8ef1078f" name = "user-content-tag-8ef1078f">&lt;H&gt;heap</a> ###

<code>struct <strong>&lt;H&gt;heap</strong>;</code>

Stores the heap as an implicit binary tree in an array\. To initialise it to an idle state, see [&lt;H&gt;heap](#user-content-fn-8ef1078f), `HEAP_IDLE`, `{0}` \(`C99`\), or being `static`\.

![States.](web/states.png)



### <a id = "user-content-tag-52985d65" name = "user-content-tag-52985d65">&lt;PH&gt;iterator</a> ###

<code>struct <strong>&lt;PH&gt;iterator</strong>;</code>

Contains all iteration parameters\.



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-8ef1078f">&lt;H&gt;heap</a></td><td>heap</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d56f4c70">&lt;H&gt;heap_</a></td><td>heap</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-2f5a4cc1">&lt;H&gt;heap_size</a></td><td>heap</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d3572b1d">&lt;H&gt;heap_clear</a></td><td>heap</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-42cb2b13">&lt;H&gt;heap_add</a></td><td>heap, node</td></tr>

<tr><td align = right>static struct &lt;H&gt;heap_node *</td><td><a href = "#user-content-fn-921d7df">&lt;H&gt;heap_peek</a></td><td>heap</td></tr>

<tr><td align = right>static &lt;PH&gt;pvalue</td><td><a href = "#user-content-fn-c69b891">&lt;H&gt;heap_peek_value</a></td><td>heap</td></tr>

<tr><td align = right>static &lt;PH&gt;pvalue</td><td><a href = "#user-content-fn-2cd270b7">&lt;H&gt;heap_pop</a></td><td>heap</td></tr>

<tr><td align = right>static struct &lt;H&gt;heap_node *</td><td><a href = "#user-content-fn-1e9bf0d8">&lt;H&gt;heap_reserve</a></td><td>heap, reserve</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-4355676a">&lt;H&gt;heap_buffer</a></td><td>heap, add</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-6fb489ab">&lt;A&gt;to_string</a></td><td>box</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-2a4c2c14">&lt;H&gt;heap_test</a></td><td>param</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-8ef1078f" name = "user-content-fn-8ef1078f">&lt;H&gt;heap</a> ###

<code>static void <strong>&lt;H&gt;heap</strong>(struct &lt;H&gt;heap *const <em>heap</em>)</code>

Initialises `heap` to be idle\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-d56f4c70" name = "user-content-fn-d56f4c70">&lt;H&gt;heap_</a> ###

<code>static void <strong>&lt;H&gt;heap_</strong>(struct &lt;H&gt;heap *const <em>heap</em>)</code>

Returns `heap` to the idle state where it takes no dynamic memory\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-2f5a4cc1" name = "user-content-fn-2f5a4cc1">&lt;H&gt;heap_size</a> ###

<code>static size_t <strong>&lt;H&gt;heap_size</strong>(const struct &lt;H&gt;heap *const <em>heap</em>)</code>

 * Return:  
   The size of `heap`\.
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

<code>static int <strong>&lt;H&gt;heap_add</strong>(struct &lt;H&gt;heap *const <em>heap</em>, struct &lt;H&gt;heap_node <em>node</em>)</code>

Copies `node` into `heap`\.

 * Return:  
   Success\.
 * Exceptional return: ERANGE, realloc  
 * Order:  
   &#927;\(log `heap.size`\)




### <a id = "user-content-fn-921d7df" name = "user-content-fn-921d7df">&lt;H&gt;heap_peek</a> ###

<code>static struct &lt;H&gt;heap_node *<strong>&lt;H&gt;heap_peek</strong>(const struct &lt;H&gt;heap *const <em>heap</em>)</code>

 * Return:  
   Lowest in `heap` according to `HEAP_COMPARE` or null if the heap is empty\. This pointer is valid only until one makes structural changes to the heap\.
 * Order:  
   &#927;\(1\)




### <a id = "user-content-fn-c69b891" name = "user-content-fn-c69b891">&lt;H&gt;heap_peek_value</a> ###

<code>static &lt;PH&gt;pvalue <strong>&lt;H&gt;heap_peek_value</strong>(struct &lt;H&gt;heap *const <em>heap</em>)</code>

This returns the [&lt;PH&gt;pvalue](#user-content-typedef-eccf9f42) of the [&lt;H&gt;heap_node](#user-content-tag-7243593c) returned by [&lt;H&gt;heap_peek](#user-content-fn-921d7df), for convenience with some applications\. If `HEAP_VALUE`, this is a child of [&lt;H&gt;heap_peek](#user-content-fn-921d7df), otherwise it is a boolean `int`\.

 * Return:  
   Lowest [&lt;PH&gt;value](#user-content-typedef-a55b7cd4) in `heap` element according to `HEAP_COMPARE`; if the heap is empty, null or zero\.
 * Order:  
   &#927;\(1\)




### <a id = "user-content-fn-2cd270b7" name = "user-content-fn-2cd270b7">&lt;H&gt;heap_pop</a> ###

<code>static &lt;PH&gt;pvalue <strong>&lt;H&gt;heap_pop</strong>(struct &lt;H&gt;heap *const <em>heap</em>)</code>

Remove the lowest element according to `HEAP_COMPARE`\.

 * Parameter: _heap_  
   If null, returns false\.
 * Return:  
   The [&lt;PH&gt;pvalue](#user-content-typedef-eccf9f42) of the element that was removed; if the heap is empty, null or zero\.
 * Order:  
   &#927;\(log `size`\)




### <a id = "user-content-fn-1e9bf0d8" name = "user-content-fn-1e9bf0d8">&lt;H&gt;heap_reserve</a> ###

<code>static struct &lt;H&gt;heap_node *<strong>&lt;H&gt;heap_reserve</strong>(struct &lt;H&gt;heap *const <em>heap</em>, const size_t <em>reserve</em>)</code>

Ensures that `heap` is `reserve` capacity beyond the elements already in the heap, but doesn't add to the size\.

 * Parameter: _reserve_  
   If zero and idle, returns null
 * Return:  
   The end of the `heap`, where are `reserve` elements\. Writing on this memory space is safe, but one will have to increase the size manually, \(see [&lt;H&gt;heap_buffer](#user-content-fn-4355676a)\.\)
 * Exceptional return: ERANGE, realloc  
 * Order:  
   Amortised &#927;\(`reserve`\)\.




### <a id = "user-content-fn-4355676a" name = "user-content-fn-4355676a">&lt;H&gt;heap_buffer</a> ###

<code>static int <strong>&lt;H&gt;heap_buffer</strong>(struct &lt;H&gt;heap *const <em>heap</em>, const size_t <em>add</em>)</code>

Adds and heapifies `add` elements to `heap`\. Uses [Doberkat, 1984, Floyd](https://scholar.google.ca/scholar?q=Doberkat%2C+1984%2C+Floyd) to sift\-down all the internal nodes of heap, including any previous elements\. As such, this function is most efficient on a heap of zero size, and becomes more inefficient as the existing heap grows\. For heaps that are already in use, it may be better to add each element individually, resulting in a run\-time of &#927;\(`new elements` &#183; log `heap.size`\)\.

 * Parameter: _add_  
   If zero, returns true\.
 * Return:  
   Success\.
 * Exceptional return: ERANGE, realloc  
   If [&lt;H&gt;heap_reserve](#user-content-fn-1e9bf0d8) has been successful in reserving at least `add` elements, one is guaranteed success\. Practically, it really doesn't make any sense to call this without calling [&lt;H&gt;heap_reserve](#user-content-fn-1e9bf0d8) and setting the values, because then one would be inserting un\-initialised values on the heap\.
 * Order:  
   &#927;\(`heap.size` \+ `add`\)




### <a id = "user-content-fn-6fb489ab" name = "user-content-fn-6fb489ab">&lt;A&gt;to_string</a> ###

<code>static const char *<strong>&lt;A&gt;to_string</strong>(const &lt;PA&gt;box *const <em>box</em>)</code>

 * Return:  
   Print the contents of `box` in a static string buffer of 256 bytes with limitations of only printing 4 things at a time\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-2a4c2c14" name = "user-content-fn-2a4c2c14">&lt;H&gt;heap_test</a> ###

<code>static void <strong>&lt;H&gt;heap_test</strong>(void *const <em>param</em>)</code>

Will be tested on stdout\. Requires `HEAP_TEST`, `HEAP_TO_STRING`, and not `NDEBUG` while defining `assert`\.

 * Parameter: _param_  
   The parameter to call [&lt;PH&gt;biaction_fn](#user-content-typedef-7e815a45) `HEAP_TEST`\.






## <a id = "user-content-license" name = "user-content-license">License</a> ##

2020 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



