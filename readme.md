# Heap\.h #

## Parameterised Priority Queue ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PH&gt;Priority](#user-content-typedef-57e15d67), [&lt;PH&gt;Compare](#user-content-typedef-27ee3a1e), [&lt;PH&gt;Type](#user-content-typedef-b7099207), [&lt;PH&gt;Value](#user-content-typedef-4d915774), [&lt;PH&gt;ToString](#user-content-typedef-81d59eb3), [&lt;PH&gt;BiAction](#user-content-typedef-65e63188)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;H&gt;HeapNode](#user-content-tag-ba24d32f), [&lt;H&gt;Heap](#user-content-tag-f1ee6af)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of heap.](web/heap.png)

A [&lt;H&gt;Heap](#user-content-tag-f1ee6af) is a priority queue built from [&lt;H&gt;HeapNode](#user-content-tag-ba24d32f)\. It is a binary heap, proposed by [Williams, 1964, Heapsort, p\. 347](https://scholar.google.ca/scholar?q=Williams%2C+1964%2C+Heapsort%2C+p.+347) and using terminology of [Knuth, 1973, Sorting](https://scholar.google.ca/scholar?q=Knuth%2C+1973%2C+Sorting)\. Internally, it is an array `<<H>HeapNode>Array` with heap properties; as such, one needs to have the `Array.h` file in the same directory\.

`<H>Heap` is not synchronised\. Errors are returned with `errno`\. The parameters are `#define` preprocessor macros, and are all undefined at the end of the file for convenience\. `assert.h` is used; to stop assertions, use `#define NDEBUG` before inclusion\.



 * Parameter: HEAP\_NAME, HEAP\_TYPE  
   `<H>` that satisfies `C` naming conventions when mangled and an optional [&lt;PH&gt;Type](#user-content-typedef-b7099207) associated therewith; `HEAP_NAME` is required\. `<PH>` is private, whose names are prefixed in a manner to avoid collisions; any should be re\-defined prior to use elsewhere\. Note that `HEAP_TYPE` is only used by reference; it can be forward\-declared or left out entirely, and the implementation is [responsble for it's storage](https://github.com/neil-edelman/Pool)\.
 * Parameter: HEAP\_COMPARE  
   A function satisfying [&lt;PH&gt;Compare](#user-content-typedef-27ee3a1e)\. Defaults to minimum\-hash using less\-then on `HEAP_PRIORITY`\.
 * Parameter: HEAP\_PRIORITY  
   This is [&lt;PH&gt;Priority](#user-content-typedef-57e15d67) and defaults to `unsigned int`\. Gets combined with `HEAP_TYPE` \(if it exists\) to form [&lt;H&gt;HeapNode](#user-content-tag-ba24d32f)\.
 * Parameter: HEAP\_TO\_STRING  
   Optional print function implementing [&lt;PH&gt;ToString](#user-content-typedef-81d59eb3); makes available [&lt;H&gt;HeapToString](#user-content-fn-2dd2ccc3)\.
 * Parameter: HEAP\_TEST  
   Unit testing framework [&lt;H&gt;HeapTest](#user-content-fn-17b017db), included in a separate header, [\.\./test/HeapTest\.h](../test/HeapTest.h)\. Must be defined equal to a \(random\) filler function, satisfying [&lt;PH&gt;BiAction](#user-content-typedef-65e63188)\. Requires `HEAP_TO_STRING` and not `NDEBUG`\.
 * Standard:  
   C89
 * Dependancies:  
   [Array.h](../Array/)
 * See also:  
   [Array](https://github.com/neil-edelman/Array); [List](https://github.com/neil-edelman/List); [Orcish](https://github.com/neil-edelman/Orcish); [Pool](https://github.com/neil-edelman/Pool); [Set](https://github.com/neil-edelman/Set)


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-57e15d67" name = "user-content-typedef-57e15d67">&lt;PH&gt;Priority</a> ###

<code>typedef HEAP_PRIORITY <strong>&lt;PH&gt;Priority</strong>;</code>

Valid type used for caching priority, used in [&lt;H&gt;HeapNode](#user-content-tag-ba24d32f)\. Defaults to `unsigned int`\.



### <a id = "user-content-typedef-27ee3a1e" name = "user-content-typedef-27ee3a1e">&lt;PH&gt;Compare</a> ###

<code>typedef int(*<strong>&lt;PH&gt;Compare</strong>)(const &lt;PH&gt;Priority, const &lt;PH&gt;Priority);</code>

Returns a positive result if `a` comes after `b`, inducing a pre\-order of `a` with respect to `b`; this is compatible, but less strict then the comparators from `bsearch` and `qsort`; it only needs to divide entries into two instead of three categories\. The default `HEAP_COMPARE` is `a > b`, which makes a minimum\-hash\.



### <a id = "user-content-typedef-b7099207" name = "user-content-typedef-b7099207">&lt;PH&gt;Type</a> ###

<code>typedef HEAP_TYPE <strong>&lt;PH&gt;Type</strong>;</code>

If `HEAP_TYPE`, a valid tag type set by `HEAP_TYPE`, used in [&lt;H&gt;HeapNode](#user-content-tag-ba24d32f)\.



### <a id = "user-content-typedef-4d915774" name = "user-content-typedef-4d915774">&lt;PH&gt;Value</a> ###

<code>typedef &lt;PH&gt;Type *<strong>&lt;PH&gt;Value</strong>;</code>

This represents the value of the node\. If `HEAP_TYPE`, a pointer to the [&lt;PH&gt;Type](#user-content-typedef-b7099207); may be null if one has put null values in or if the node is null\. If not `HEAP_TYPE`, a boolean `int` value that is true \(one\) if the value was there and false \(zero\) if not\.



### <a id = "user-content-typedef-81d59eb3" name = "user-content-typedef-81d59eb3">&lt;PH&gt;ToString</a> ###

<code>typedef void(*<strong>&lt;PH&gt;ToString</strong>)(const struct &lt;H&gt;HeapNode *, char(*)[12]);</code>

Responsible for turning [&lt;H&gt;HeapNode](#user-content-tag-ba24d32f) into a maximum 11\-`char` string\. Used for `HEAP_TO_STRING`\.



### <a id = "user-content-typedef-65e63188" name = "user-content-typedef-65e63188">&lt;PH&gt;BiAction</a> ###

<code>typedef void(*<strong>&lt;PH&gt;BiAction</strong>)(struct &lt;H&gt;HeapNode *, void *);</code>

Operates by side\-effects\. Used for `HEAP_TEST`\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-ba24d32f" name = "user-content-tag-ba24d32f">&lt;H&gt;HeapNode</a> ###

<code>struct <strong>&lt;H&gt;HeapNode</strong>;</code>

Stores a [&lt;PH&gt;Priority](#user-content-typedef-57e15d67) as `priority`, and, if `HASH_TYPE`, a [&lt;PH&gt;Type](#user-content-typedef-b7099207) pointer called `value`\. `value` is just the payload, if the `value` has [&lt;PH&gt;Priority](#user-content-typedef-57e15d67) in it, \(as most other heap implementations,\) one has to cache the the sub\-structure \(default `unsigned int`\) of value to the `priority` such that the `priority` does not need a second de\-reference\.



### <a id = "user-content-tag-f1ee6af" name = "user-content-tag-f1ee6af">&lt;H&gt;Heap</a> ###

<code>struct <strong>&lt;H&gt;Heap</strong>;</code>

Stores the heap as an implicit binary tree in an array\. To initialise it to an idle state, see [&lt;H&gt;Heap](#user-content-fn-f1ee6af), `HEAP_IDLE`, `{0}` \(`C99`\), or being `static`\.

![States.](web/states.png)



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-bda58bd0">&lt;H&gt;Heap_</a></td><td>heap</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-f1ee6af">&lt;H&gt;Heap</a></td><td>heap</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-4070d9e6">&lt;H&gt;HeapSize</a></td><td>heap</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d238d100">&lt;H&gt;HeapClear</a></td><td>heap</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-63b31c6a">&lt;H&gt;HeapAdd</a></td><td>heap, node</td></tr>

<tr><td align = right>static struct &lt;H&gt;HeapNode *</td><td><a href = "#user-content-fn-12af7c44">&lt;H&gt;HeapPeek</a></td><td>heap</td></tr>

<tr><td align = right>static &lt;PH&gt;Value</td><td><a href = "#user-content-fn-d587663d">&lt;H&gt;HeapPeekValue</a></td><td>heap</td></tr>

<tr><td align = right>static &lt;PH&gt;Value</td><td><a href = "#user-content-fn-a1a31b62">&lt;H&gt;HeapPop</a></td><td>heap</td></tr>

<tr><td align = right>static struct &lt;H&gt;HeapNode *</td><td><a href = "#user-content-fn-508f0f49">&lt;H&gt;HeapReserve</a></td><td>heap, reserve</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-bd0281c5">&lt;H&gt;HeapBuffer</a></td><td>heap, add</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-2dd2ccc3">&lt;H&gt;HeapToString</a></td><td>heap</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-17b017db">&lt;H&gt;HeapTest</a></td><td>param</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-bda58bd0" name = "user-content-fn-bda58bd0">&lt;H&gt;Heap_</a> ###

<code>static void <strong>&lt;H&gt;Heap_</strong>(struct &lt;H&gt;Heap *const <em>heap</em>)</code>

Returns `heap` to the idle state where it takes no dynamic memory\.

 * Parameter: _heap_  
   If null, does nothing\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-f1ee6af" name = "user-content-fn-f1ee6af">&lt;H&gt;Heap</a> ###

<code>static void <strong>&lt;H&gt;Heap</strong>(struct &lt;H&gt;Heap *const <em>heap</em>)</code>

Initialises `heap` to be idle\.

 * Parameter: _heap_  
   If null, does nothing\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-4070d9e6" name = "user-content-fn-4070d9e6">&lt;H&gt;HeapSize</a> ###

<code>static size_t <strong>&lt;H&gt;HeapSize</strong>(const struct &lt;H&gt;Heap *const <em>heap</em>)</code>

 * Parameter: _heap_  
   If null, returns zero;
 * Return:  
   The size of `heap`\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-d238d100" name = "user-content-fn-d238d100">&lt;H&gt;HeapClear</a> ###

<code>static void <strong>&lt;H&gt;HeapClear</strong>(struct &lt;H&gt;Heap *const <em>heap</em>)</code>

Sets `heap` to be empty\. That is, the size of `heap` will be zero, but if it was previously in an active non\-idle state, it continues to be\.

 * Parameter: _heap_  
   If null, does nothing\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-63b31c6a" name = "user-content-fn-63b31c6a">&lt;H&gt;HeapAdd</a> ###

<code>static int <strong>&lt;H&gt;HeapAdd</strong>(struct &lt;H&gt;Heap *const <em>heap</em>, struct &lt;H&gt;HeapNode <em>node</em>)</code>

Copies `node` into `heap`\.

 * Parameter: _heap_  
   If null, returns false\.
 * Return:  
   Success\.
 * Exceptional return: realloc  
 * Order:  
   &#927;\(log `size`\)




### <a id = "user-content-fn-12af7c44" name = "user-content-fn-12af7c44">&lt;H&gt;HeapPeek</a> ###

<code>static struct &lt;H&gt;HeapNode *<strong>&lt;H&gt;HeapPeek</strong>(struct &lt;H&gt;Heap *const <em>heap</em>)</code>

 * Parameter: _heap_  
   If null, returns null\.
 * Return:  
   Lowest in `heap` according to `HEAP_COMPARE` or null if the heap is empty\. This pointer is valid only until one makes structural changes to the heap\.
 * Order:  
   &#927;\(1\)




### <a id = "user-content-fn-d587663d" name = "user-content-fn-d587663d">&lt;H&gt;HeapPeekValue</a> ###

<code>static &lt;PH&gt;Value <strong>&lt;H&gt;HeapPeekValue</strong>(struct &lt;H&gt;Heap *const <em>heap</em>)</code>

This returns a child of that accessible from [&lt;H&gt;HeapPeek](#user-content-fn-12af7c44), for convenience with some applications\.

 * Parameter: _heap_  
   If null, returns null\.
 * Return:  
   Lowest [&lt;PH&gt;Value](#user-content-typedef-4d915774) in `heap` element according to `HEAP_COMPARE`, \(which may be null,\) or null or zero if the heap is empty\.
 * Order:  
   &#927;\(1\)




### <a id = "user-content-fn-a1a31b62" name = "user-content-fn-a1a31b62">&lt;H&gt;HeapPop</a> ###

<code>static &lt;PH&gt;Value <strong>&lt;H&gt;HeapPop</strong>(struct &lt;H&gt;Heap *const <em>heap</em>)</code>

Remove the lowest element according to `HEAP_COMPARE`\.

 * Parameter: _heap_  
   If null, returns false\.
 * Return:  
   The [&lt;PH&gt;Value](#user-content-typedef-4d915774) of the element that was removed; if the heap is empty, null or zero\.
 * Order:  
   &#927;\(log `size`\)




### <a id = "user-content-fn-508f0f49" name = "user-content-fn-508f0f49">&lt;H&gt;HeapReserve</a> ###

<code>static struct &lt;H&gt;HeapNode *<strong>&lt;H&gt;HeapReserve</strong>(struct &lt;H&gt;Heap *const <em>heap</em>, const size_t <em>reserve</em>)</code>

Ensures that `heap` is `reserve` capacity beyond the elements already in the heap, but doesn't add to the size\.

 * Parameter: _heap_  
   If null, returns false\.
 * Parameter: _reserve_  
   If zero, returns true\.
 * Return:  
   The end of the `heap`, where are `reserve` elements, or null and `errno` will be set\. Writing on this memory space is safe, but one will have to increase the size manually, \(see [&lt;H&gt;HeapBuffer](#user-content-fn-bd0281c5)\.\)
 * Exceptional return: ERANGE  
   Tried allocating more then can fit in `size_t` or `realloc` error and doesn't follow [IEEE Std 1003.1-2001](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html)\.
 * Exceptional return: realloc  
 * Order:  
   Amortised &#927;\(`reserve`\)\.




### <a id = "user-content-fn-bd0281c5" name = "user-content-fn-bd0281c5">&lt;H&gt;HeapBuffer</a> ###

<code>static int <strong>&lt;H&gt;HeapBuffer</strong>(struct &lt;H&gt;Heap *const <em>heap</em>, const size_t <em>add</em>)</code>

Adds `add` elements to `heap`\. Uses [Doberkat, 1984, Floyd](https://scholar.google.ca/scholar?q=Doberkat%2C+1984%2C+Floyd) to sift\-down all the internal nodes of heap\. As such, this function is most efficient on a heap of zero size, and becomes more inefficient as the existing heap grows\. For heaps that are already in use, it may be better to add each element individually, resulting in a run\-time of &#927;\(`new elements` &#183; log `size`\)\.

 * Parameter: _heap_  
   If null, returns null\.
 * Parameter: _add_  
   If zero, returns null\.
 * Return:  
   Success\.
 * Exceptional return: ERANGE  
   Tried allocating more then can fit in `size_t` or `realloc` error and doesn't follow [IEEE Std 1003.1-2001](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html)\. If [&lt;H&gt;HeapReserve](#user-content-fn-508f0f49) has been successful in reserving at least `add` elements, one is guaranteed success\. Practically, it really doesn't make any sense to call this without calling [&lt;H&gt;HeapReserve](#user-content-fn-508f0f49) because then one would be inserting un\-initialised values on the heap\.
 * Exceptional return: realloc  
 * Order:  
   &#927;\(`new size`\)




### <a id = "user-content-fn-2dd2ccc3" name = "user-content-fn-2dd2ccc3">&lt;H&gt;HeapToString</a> ###

<code>static const char *<strong>&lt;H&gt;HeapToString</strong>(const struct &lt;H&gt;Heap *const <em>heap</em>)</code>

Can print 4 things at once before it overwrites\. One must a `HEAP_TO_STRING` to a function implementing [&lt;PH&gt;ToString](#user-content-typedef-81d59eb3) to get this functionality\.

 * Return:  
   Prints `heap` in a static buffer\.
 * Order:  
   &#920;\(1\); it has a 255 character limit; every element takes some of it\.




### <a id = "user-content-fn-17b017db" name = "user-content-fn-17b017db">&lt;H&gt;HeapTest</a> ###

<code>static void <strong>&lt;H&gt;HeapTest</strong>(void *const <em>param</em>)</code>

Will be tested on stdout\. Requires `HEAP_TEST`, `HEAP_TO_STRING`, and not `NDEBUG` while defining `assert`\.

 * Parameter: _param_  
   The parameter to call [&lt;PH&gt;BiAction](#user-content-typedef-65e63188) `HEAP_TEST`\.






## <a id = "user-content-license" name = "user-content-license">License</a> ##

2020 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



