# deque\.h #

Header [\.\./\.\./src/deque\.h](../../src/deque.h); examples [\.\./\.\./test/test\_deque\.c](../../test/test_deque.c)\.

## Deque ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;pT&gt;type](#user-content-typedef-9b5be28b), [&lt;pT&gt;action_fn](#user-content-typedef-348726ce), [&lt;pT&gt;predicate_fn](#user-content-typedef-ad32e23d), [&lt;pT&gt;to_string_fn](#user-content-typedef-4442127b)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;pT&gt;block](#user-content-tag-9384b18c), [&lt;t&gt;deque](#user-content-tag-f6ed81df), [table_stats](#user-content-tag-89e31bf3)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

[&lt;t&gt;deque](#user-content-tag-f6ed81df) is a stable container that stores [&lt;pT&gt;type](#user-content-typedef-9b5be28b); it grows as the capacity increases but is not necessarily contagious\. The default behaviour is a stack that grows downwards\.

 * Parameter: DEQUE\_NAME, DEQUE\_TYPE  
   `<t>` that satisfies `C` naming conventions when mangled and a valid tag\-type, [&lt;pT&gt;type](#user-content-typedef-9b5be28b), associated therewith; required\.
 * Parameter: DEQUE\_FRONT  
   This adds double\-linking in the blocks so the deque can iterated forwards\. Changes the order of the default iteration\.
 * Parameter: DEQUE\_PUSH\_FRONT  
   The default deque is only a stack that grows down\. This replaces the index size with a range in all the blocks\. Implies `DEQUE_FRONT`\. \(Not implemented yet; I just use this as a stable memory store\.\)
 * Parameter: DEQUE\_TO\_STRING  
   To string trait contained in [\.\./\.\./src/to\_string\.h](../../src/to_string.h)\. See [&lt;pT&gt;to_string_fn](#user-content-typedef-4442127b)\.
 * Parameter: DEQUE\_DECLARE\_ONLY, DEQUE\_NON\_STATIC  
   For headers in different compilation units\.
 * Standard:  
   C89, but recommend C99 flexible array members instead of "struct hack"\.
 * Dependancies:  
   [box](../../src/box.h)




## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-9b5be28b" name = "user-content-typedef-9b5be28b">&lt;pT&gt;type</a> ###

<code>typedef DEQUE_TYPE <strong>&lt;pT&gt;type</strong>;</code>

A valid tag type set by `DEQUE_TYPE`\.



### <a id = "user-content-typedef-348726ce" name = "user-content-typedef-348726ce">&lt;pT&gt;action_fn</a> ###

<code>typedef void(*<strong>&lt;pT&gt;action_fn</strong>)(&lt;pT&gt;type *);</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h): Operates by side\-effects\.



### <a id = "user-content-typedef-ad32e23d" name = "user-content-typedef-ad32e23d">&lt;pT&gt;predicate_fn</a> ###

<code>typedef int(*<strong>&lt;pT&gt;predicate_fn</strong>)(const &lt;pT&gt;type *);</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h): Returns a boolean given read\-only\.



### <a id = "user-content-typedef-4442127b" name = "user-content-typedef-4442127b">&lt;pT&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;pT&gt;to_string_fn</strong>)(const &lt;pT&gt;type *, char(*)[12]);</code>

The type of the required `<tr>to_string`\. Responsible for turning the read\-only argument into a 12\-max\-`char` output string\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-9384b18c" name = "user-content-tag-9384b18c">&lt;pT&gt;block</a> ###

<code>struct <strong>&lt;pT&gt;block</strong> { struct &lt;pT&gt;block *previous; struct &lt;pT&gt;block *next; size_t capacity, size; &lt;pT&gt;type data[]; &lt;pT&gt;type data[1]; };</code>

A linked\-list of blocks\.



### <a id = "user-content-tag-f6ed81df" name = "user-content-tag-f6ed81df">&lt;t&gt;deque</a> ###

<code>struct <strong>&lt;t&gt;deque</strong> { struct &lt;pT&gt;block *back; struct &lt;pT&gt;block *front; };</code>

Manages a linked\-list of blocks\. Only the front can have a block\-size of zero\.



### <a id = "user-content-tag-89e31bf3" name = "user-content-tag-89e31bf3">table_stats</a> ###

<code>struct <strong>table_stats</strong> { size_t n, max; double mean, ssdm; };</code>

[Welford1962Note](https://scholar.google.ca/scholar?q=Welford1962Note): population variance: `ssdm/n`, sample variance: `ssdm/(n-1)`\.



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>struct &lt;T&gt;cursor</td><td><a href = "#user-content-fn-446c82de">&lt;T&gt;end</a></td><td>&lt;t&gt;deque</td></tr>

<tr><td align = right>int</td><td><a href = "#user-content-fn-dd6c86e1">&lt;T&gt;exists</a></td><td>&lt;T&gt;cursor</td></tr>

<tr><td align = right>&lt;pT&gt;type *</td><td><a href = "#user-content-fn-1d176e37">&lt;T&gt;entry</a></td><td>&lt;T&gt;cursor</td></tr>

<tr><td align = right>void</td><td><a href = "#user-content-fn-d6c331fc">&lt;T&gt;previous</a></td><td>&lt;T&gt;cursor</td></tr>

<tr><td align = right>struct &lt;T&gt;cursor</td><td><a href = "#user-content-fn-80df50b2">&lt;T&gt;begin</a></td><td>&lt;t&gt;deque</td></tr>

<tr><td align = right>void</td><td><a href = "#user-content-fn-d0790d04">&lt;T&gt;next</a></td><td>&lt;T&gt;cursor</td></tr>

<tr><td align = right>struct &lt;t&gt;deque</td><td><a href = "#user-content-fn-f6ed81df">&lt;t&gt;deque</a></td><td></td></tr>

<tr><td align = right>void</td><td><a href = "#user-content-fn-37e2dc80">&lt;t&gt;deque_</a></td><td>const</td></tr>

<tr><td align = right>&lt;pT&gt;type *</td><td><a href = "#user-content-fn-e412658d">&lt;T&gt;new_back</a></td><td>&lt;t&gt;deque</td></tr>

<tr><td align = right>&lt;pT&gt;type *</td><td><a href = "#user-content-fn-3f8fb885">&lt;T&gt;append_back</a></td><td>&lt;t&gt;deque, size_t</td></tr>

<tr><td align = right>void</td><td><a href = "#user-content-fn-7f4a964e">&lt;T&gt;clear</a></td><td>&lt;t&gt;deque</td></tr>

<tr><td align = right>static struct &lt;t&gt;deque</td><td><a href = "#user-content-fn-f6ed81df">&lt;t&gt;deque</a></td><td></td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-37e2dc80">&lt;t&gt;deque_</a></td><td>deque</td></tr>

<tr><td align = right>static &lt;pT&gt;type *</td><td><a href = "#user-content-fn-e412658d">&lt;T&gt;new_back</a></td><td>deque</td></tr>

<tr><td align = right>static &lt;pT&gt;type *</td><td><a href = "#user-content-fn-3f8fb885">&lt;T&gt;append_back</a></td><td>deque, n</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-7f4a964e">&lt;T&gt;clear</a></td><td>deque</td></tr>

<tr><td align = right>&lt;pT&gt;type *</td><td><a href = "#user-content-fn-443f2b31">&lt;TR&gt;any</a></td><td>&lt;pT&gt;box, &lt;pTR&gt;predicate_fn</td></tr>

<tr><td align = right>void</td><td><a href = "#user-content-fn-51d87ca4">&lt;TR&gt;each</a></td><td>&lt;pT&gt;box, &lt;pTR&gt;action_fn</td></tr>

<tr><td align = right>void</td><td><a href = "#user-content-fn-21ef106e">&lt;TR&gt;if_each</a></td><td>&lt;pT&gt;box, &lt;pTR&gt;predicate_fn, &lt;pTR&gt;action_fn</td></tr>

<tr><td align = right>int</td><td><a href = "#user-content-fn-f61ec8de">&lt;TR&gt;copy_if</a></td><td>restrict, restrict, &lt;pTR&gt;predicate_fn</td></tr>

<tr><td align = right>static &lt;pT&gt;type *</td><td><a href = "#user-content-fn-443f2b31">&lt;TR&gt;any</a></td><td>box, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-51d87ca4">&lt;TR&gt;each</a></td><td>box, action</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-21ef106e">&lt;TR&gt;if_each</a></td><td>box, predicate, action</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-f61ec8de">&lt;TR&gt;copy_if</a></td><td>dst, src, copy</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-8bb1c0a2">&lt;TR&gt;keep_if</a></td><td>box, keep, destruct</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-a76df7bd">&lt;TR&gt;trim</a></td><td>box, predicate</td></tr>

<tr><td align = right>const char *</td><td><a href = "#user-content-fn-260f8348">&lt;TR&gt;to_string</a></td><td>box</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-260f8348">&lt;TR&gt;to_string</a></td><td>box</td></tr>

<tr><td align = right>void</td><td><a href = "#user-content-fn-4e047ffb">&lt;T&gt;graph</a></td><td>&lt;pT&gt;box</td></tr>

<tr><td align = right>int</td><td><a href = "#user-content-fn-6c32bc30">&lt;T&gt;graph_fn</a></td><td>&lt;pT&gt;box, char</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-446c82de" name = "user-content-fn-446c82de">&lt;T&gt;end</a> ###

<code>struct &lt;T&gt;cursor <strong>&lt;T&gt;end</strong>(const struct <em>&lt;t&gt;deque</em> *);</code>



### <a id = "user-content-fn-dd6c86e1" name = "user-content-fn-dd6c86e1">&lt;T&gt;exists</a> ###

<code>int <strong>&lt;T&gt;exists</strong>(const struct <em>&lt;T&gt;cursor</em> *);</code>



### <a id = "user-content-fn-1d176e37" name = "user-content-fn-1d176e37">&lt;T&gt;entry</a> ###

<code>&lt;pT&gt;type *<strong>&lt;T&gt;entry</strong>(struct <em>&lt;T&gt;cursor</em> *);</code>



### <a id = "user-content-fn-d6c331fc" name = "user-content-fn-d6c331fc">&lt;T&gt;previous</a> ###

<code>void <strong>&lt;T&gt;previous</strong>(struct <em>&lt;T&gt;cursor</em> *);</code>



### <a id = "user-content-fn-80df50b2" name = "user-content-fn-80df50b2">&lt;T&gt;begin</a> ###

<code>struct &lt;T&gt;cursor <strong>&lt;T&gt;begin</strong>(const struct <em>&lt;t&gt;deque</em> *);</code>



### <a id = "user-content-fn-d0790d04" name = "user-content-fn-d0790d04">&lt;T&gt;next</a> ###

<code>void <strong>&lt;T&gt;next</strong>(struct <em>&lt;T&gt;cursor</em> *);</code>



### <a id = "user-content-fn-f6ed81df" name = "user-content-fn-f6ed81df">&lt;t&gt;deque</a> ###

<code>struct &lt;t&gt;deque <strong>&lt;t&gt;deque</strong>(void);</code>



### <a id = "user-content-fn-37e2dc80" name = "user-content-fn-37e2dc80">&lt;t&gt;deque_</a> ###

<code>void <strong>&lt;t&gt;deque_</strong>(struct &lt;t&gt;deque *<em>const</em>);</code>



### <a id = "user-content-fn-e412658d" name = "user-content-fn-e412658d">&lt;T&gt;new_back</a> ###

<code>&lt;pT&gt;type *<strong>&lt;T&gt;new_back</strong>(struct <em>&lt;t&gt;deque</em> *);</code>



### <a id = "user-content-fn-3f8fb885" name = "user-content-fn-3f8fb885">&lt;T&gt;append_back</a> ###

<code>&lt;pT&gt;type *<strong>&lt;T&gt;append_back</strong>(struct <em>&lt;t&gt;deque</em> *, <em>size_t</em>);</code>



### <a id = "user-content-fn-7f4a964e" name = "user-content-fn-7f4a964e">&lt;T&gt;clear</a> ###

<code>void <strong>&lt;T&gt;clear</strong>(struct <em>&lt;t&gt;deque</em> *);</code>



### <a id = "user-content-fn-f6ed81df" name = "user-content-fn-f6ed81df">&lt;t&gt;deque</a> ###

<code>static struct &lt;t&gt;deque <strong>&lt;t&gt;deque</strong>(void)</code>

Zeroed data \(not all\-bits\-zero\) is initialized, as well\.

 * Return:  
   An idle deque\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-37e2dc80" name = "user-content-fn-37e2dc80">&lt;t&gt;deque_</a> ###

<code>static void <strong>&lt;t&gt;deque_</strong>(struct &lt;t&gt;deque *const <em>deque</em>)</code>

If `deque` is not null, returns the idle zeroed state where it takes no dynamic memory\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-e412658d" name = "user-content-fn-e412658d">&lt;T&gt;new_back</a> ###

<code>static &lt;pT&gt;type *<strong>&lt;T&gt;new_back</strong>(struct &lt;t&gt;deque *const <em>deque</em>)</code>

 * Return:  
   Adds one new element to the back of `deque`\.
 * Exceptional return: realloc, ERANGE  
 * Order:  
   amortised &#927;\(1\)




### <a id = "user-content-fn-3f8fb885" name = "user-content-fn-3f8fb885">&lt;T&gt;append_back</a> ###

<code>static &lt;pT&gt;type *<strong>&lt;T&gt;append_back</strong>(struct &lt;t&gt;deque *const <em>deque</em>, const size_t <em>n</em>)</code>

Adds `n` contiguous elements to the back of `deque`\.

 * Return:  
   A pointer to the elements\. If `n` is zero, a null pointer will be returned, otherwise null indicates an error\.
 * Exceptional return: realloc, ERANGE  




### <a id = "user-content-fn-7f4a964e" name = "user-content-fn-7f4a964e">&lt;T&gt;clear</a> ###

<code>static void <strong>&lt;T&gt;clear</strong>(struct &lt;t&gt;deque *const <em>deque</em>)</code>

Sets `deque` to be empty\. That is, the size will be zero, but if it was previously in an active non\-idle state, it continues to be\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-443f2b31" name = "user-content-fn-443f2b31">&lt;TR&gt;any</a> ###

<code>&lt;pT&gt;type *<strong>&lt;TR&gt;any</strong>(const <em>&lt;pT&gt;box</em> *, <em>&lt;pTR&gt;predicate_fn</em>);</code>



### <a id = "user-content-fn-51d87ca4" name = "user-content-fn-51d87ca4">&lt;TR&gt;each</a> ###

<code>void <strong>&lt;TR&gt;each</strong>(<em>&lt;pT&gt;box</em> *, <em>&lt;pTR&gt;action_fn</em>);</code>



### <a id = "user-content-fn-21ef106e" name = "user-content-fn-21ef106e">&lt;TR&gt;if_each</a> ###

<code>void <strong>&lt;TR&gt;if_each</strong>(<em>&lt;pT&gt;box</em> *, <em>&lt;pTR&gt;predicate_fn</em>, <em>&lt;pTR&gt;action_fn</em>);</code>



### <a id = "user-content-fn-f61ec8de" name = "user-content-fn-f61ec8de">&lt;TR&gt;copy_if</a> ###

<code>int <strong>&lt;TR&gt;copy_if</strong>(&lt;pT&gt;box *<em>restrict</em>, const &lt;pTR&gt;box *<em>restrict</em>, <em>&lt;pTR&gt;predicate_fn</em>);</code>



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

[\.\./\.\./src/iterate\.h](../../src/iterate.h), `BOX_CONTIGUOUS`: For all elements of `src`, calls `copy`, and if true, lazily copies the elements to `dst`\. `dst` and `src` can not be the same but `src` can be null, \(in which case, it does nothing\.\)

 * Exceptional return: realloc  
 * Order:  
   &#927;\(|`src`|\) &#215; &#927;\(`copy`\)




### <a id = "user-content-fn-8bb1c0a2" name = "user-content-fn-8bb1c0a2">&lt;TR&gt;keep_if</a> ###

<code>static void <strong>&lt;TR&gt;keep_if</strong>(&lt;pT&gt;box *const <em>box</em>, const &lt;pTR&gt;predicate_fn <em>keep</em>, const &lt;pTR&gt;action_fn <em>destruct</em>)</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h) `BOX_CONTIGUOUS`: For all elements of `box`, calls `keep`, and if false, if contiguous, lazy deletes that item, if not, eagerly\. Calls `destruct` if not\-null before deleting\.

 * Order:  
   &#927;\(|`box`|\) \(&#215; O\(`keep`\) \+ O\(`destruct`\)\)




### <a id = "user-content-fn-a76df7bd" name = "user-content-fn-a76df7bd">&lt;TR&gt;trim</a> ###

<code>static void <strong>&lt;TR&gt;trim</strong>(&lt;pT&gt;box *const <em>box</em>, const &lt;pTR&gt;predicate_fn <em>predicate</em>)</code>

[\.\./\.\./src/iterate\.h](../../src/iterate.h), `BOX_CONTIGUOUS`: Removes at either end of `box` the things that `predicate`, if it exists, returns true\.

 * Order:  
   &#927;\(`box.size`\) &#215; &#927;\(`predicate`\)




### <a id = "user-content-fn-260f8348" name = "user-content-fn-260f8348">&lt;TR&gt;to_string</a> ###

<code>const char *<strong>&lt;TR&gt;to_string</strong>(const &lt;pT&gt;box *const <em>box</em>);</code>



### <a id = "user-content-fn-260f8348" name = "user-content-fn-260f8348">&lt;TR&gt;to_string</a> ###

<code>static const char *<strong>&lt;TR&gt;to_string</strong>(const &lt;pT&gt;box *const <em>box</em>)</code>

[\.\./\.\./src/to\_string\.h](../../src/to_string.h): print the contents of `box` in a static string buffer of 256 bytes, with limitations of only printing 4 things in a single sequence point\.

 * Return:  
   Address of the static buffer\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-4e047ffb" name = "user-content-fn-4e047ffb">&lt;T&gt;graph</a> ###

<code>void <strong>&lt;T&gt;graph</strong>(const <em>&lt;pT&gt;box</em> *, FILE *);</code>



### <a id = "user-content-fn-6c32bc30" name = "user-content-fn-6c32bc30">&lt;T&gt;graph_fn</a> ###

<code>int <strong>&lt;T&gt;graph_fn</strong>(const <em>&lt;pT&gt;box</em> *, const <em>char</em> *);</code>





## <a id = "user-content-license" name = "user-content-license">License</a> ##

2025 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



