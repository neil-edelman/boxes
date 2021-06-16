# pool\.h #

## Stable Pool ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PP&gt;type](#user-content-typedef-7560d92f), [&lt;PP&gt;action_fn](#user-content-typedef-cefaf27a), [&lt;PZ&gt;to_string_fn](#user-content-typedef-22f3d7f1)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;P&gt;pool](#user-content-tag-8aba39cb), [&lt;PP&gt;iterator](#user-content-tag-d20ef19d)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of Pool](web/pool.png)

[&lt;P&gt;pool](#user-content-tag-8aba39cb) stores `<P>` in a memory pool\. Pointers to valid items in the pool are stable, but not generally in any order or contiguous\. It uses geometrically increasing size\-blocks and when the removal is ongoing and uniformly sampled, \(specifically, old elements are all eventually removed,\) and data reaches a steady\-state size, the data will settle in one allocated region\. In this way, manages a fairly contiguous space for items which have references\.



 * Parameter: POOL\_NAME, POOL\_TYPE  
   `<P>` that satisfies `C` naming conventions when mangled and a valid tag type associated therewith; required\. `<PP>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: POOL\_EXPECT\_TRAIT  
   Do not un\-define certain variables for subsequent inclusion in a trait\.
 * Parameter: POOL\_TO\_STRING\_NAME, POOL\_TO\_STRING  
   To string trait contained in [to\_string\.h](to_string.h); `<Z>` that satisfies `C` naming conventions when mangled and function implementing [&lt;PZ&gt;to_string_fn](#user-content-typedef-22f3d7f1)\. There can be multiple to string traits, but only one can omit `POOL_TO_STRING_NAME`\.
 * Parameter: POOL\_TEST  
   To string trait contained in [\.\./test/pool\_test\.h](../test/pool_test.h); optional unit testing framework using `assert`\. Can only be defined once _per_ pool\. Must be defined equal to a \(random\) filler function, satisfying [&lt;PP&gt;action_fn](#user-content-typedef-cefaf27a)\. Output will be shown with the to string trait in which it's defined; provides tests for the base code and all later traits\.
 * Standard:  
   C89


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-7560d92f" name = "user-content-typedef-7560d92f">&lt;PP&gt;type</a> ###

<code>typedef POOL_TYPE <strong>&lt;PP&gt;type</strong>;</code>

A valid tag type set by `POOL_TYPE`\.



### <a id = "user-content-typedef-cefaf27a" name = "user-content-typedef-cefaf27a">&lt;PP&gt;action_fn</a> ###

<code>typedef void(*<strong>&lt;PP&gt;action_fn</strong>)(&lt;PP&gt;type *const data);</code>

Operates by side\-effects\.



### <a id = "user-content-typedef-22f3d7f1" name = "user-content-typedef-22f3d7f1">&lt;PZ&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PZ&gt;to_string_fn</strong>)(const &lt;PZ&gt;type *, char(*)[12]);</code>

Responsible for turning the first argument into a 12\-`char` null\-terminated output string\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-8aba39cb" name = "user-content-tag-8aba39cb">&lt;P&gt;pool</a> ###

<code>struct <strong>&lt;P&gt;pool</strong>;</code>

Zeroed data is a valid state\. To instantiate to an idle state, see [&lt;P&gt;pool](#user-content-fn-8aba39cb), `POOL_IDLE`, `{0}` \(`C99`,\) or being `static`\.

![States.](web/states.png)



### <a id = "user-content-tag-d20ef19d" name = "user-content-tag-d20ef19d">&lt;PP&gt;iterator</a> ###

<code>struct <strong>&lt;PP&gt;iterator</strong>;</code>

Contains all iteration parameters\.



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-8aba39cb">&lt;P&gt;pool</a></td><td>pool</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-f728a3fc">&lt;P&gt;pool_</a></td><td>pool</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-10c105fc">&lt;P&gt;pool_reserve</a></td><td>pool, min</td></tr>

<tr><td align = right>static &lt;PP&gt;type *</td><td><a href = "#user-content-fn-e71c341a">&lt;P&gt;pool_new</a></td><td>pool</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-95972ccc">&lt;P&gt;pool_remove</a></td><td>pool, datum</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-96f5dc51">&lt;P&gt;pool_clear</a></td><td>pool</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-26536045">&lt;P&gt;pool_for_each</a></td><td>pool, action</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-4ecb4112">&lt;Z&gt;to_string</a></td><td>box</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d026a8f8">&lt;P&gt;pool_test</a></td><td></td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-8aba39cb" name = "user-content-fn-8aba39cb">&lt;P&gt;pool</a> ###

<code>static void <strong>&lt;P&gt;pool</strong>(struct &lt;P&gt;pool *const <em>pool</em>)</code>

Initialises `pool` to idle\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-f728a3fc" name = "user-content-fn-f728a3fc">&lt;P&gt;pool_</a> ###

<code>static void <strong>&lt;P&gt;pool_</strong>(struct &lt;P&gt;pool *const <em>pool</em>)</code>

Destroys `pool` and returns it to idle\.

 * Order:  
   &#927;\(`blocks`\)




### <a id = "user-content-fn-10c105fc" name = "user-content-fn-10c105fc">&lt;P&gt;pool_reserve</a> ###

<code>static int <strong>&lt;P&gt;pool_reserve</strong>(struct &lt;P&gt;pool *const <em>pool</em>, const size_t <em>min</em>)</code>

Pre\-sizes an _idle_ `pool` to ensure that it can hold at least `min` elements\.

 * Parameter: _min_  
   If zero, doesn't do anything and returns true\.
 * Return:  
   Success; the pool becomes active with at least `min` elements\.
 * Exceptional return: EDOM  
   The pool is active and doesn't allow reserving\.
 * Exceptional return: ERANGE, malloc  




### <a id = "user-content-fn-e71c341a" name = "user-content-fn-e71c341a">&lt;P&gt;pool_new</a> ###

<code>static &lt;PP&gt;type *<strong>&lt;P&gt;pool_new</strong>(struct &lt;P&gt;pool *const <em>pool</em>)</code>

 * Return:  
   A new element from `pool`\.
 * Exceptional return: ERANGE, malloc  
 * Order:  
   amortised O\(1\)




### <a id = "user-content-fn-95972ccc" name = "user-content-fn-95972ccc">&lt;P&gt;pool_remove</a> ###

<code>static int <strong>&lt;P&gt;pool_remove</strong>(struct &lt;P&gt;pool *const <em>pool</em>, &lt;PP&gt;type *const <em>datum</em>)</code>

Deletes `datum` from `pool`\.

 * Return:  
   Success\.
 * Exceptional return: EDOM  
   `data` is not part of `pool`\.
 * Order:  
   Amortised &#927;\(1\), if the pool is in steady\-state, but &#927;\(log `pool.items`\) for a small number of deleted items\.




### <a id = "user-content-fn-96f5dc51" name = "user-content-fn-96f5dc51">&lt;P&gt;pool_clear</a> ###

<code>static void <strong>&lt;P&gt;pool_clear</strong>(struct &lt;P&gt;pool *const <em>pool</em>)</code>

Removes all from `pool`, but keeps it's active state\. \(Only freeing the smaller blocks\.\)

 * Order:  
   &#927;\(`pool.blocks`\)




### <a id = "user-content-fn-26536045" name = "user-content-fn-26536045">&lt;P&gt;pool_for_each</a> ###

<code>static void <strong>&lt;P&gt;pool_for_each</strong>(struct &lt;P&gt;pool *const <em>pool</em>, const &lt;PP&gt;action_fn <em>action</em>)</code>

Iterates though `pool` and calls `action` on all the elements\.

 * Order:  
   O\(`capacity` &#215; `action`\)




### <a id = "user-content-fn-4ecb4112" name = "user-content-fn-4ecb4112">&lt;Z&gt;to_string</a> ###

<code>static const char *<strong>&lt;Z&gt;to_string</strong>(const &lt;PZ&gt;box *const <em>box</em>)</code>

 * Return:  
   Print the contents of `box` in a static string buffer of 256 bytes with limitations of only printing 4 things at a time\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-d026a8f8" name = "user-content-fn-d026a8f8">&lt;P&gt;pool_test</a> ###

<code>static void <strong>&lt;P&gt;pool_test</strong>(void)</code>

The list will be tested on stdout; requires `POOL_TEST` and not `NDEBUG`\.





## <a id = "user-content-license" name = "user-content-license">License</a> ##

2016 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



