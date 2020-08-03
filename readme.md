# Pool\.h #

## Stable Pool ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PT&gt;type](#user-content-typedef-245060ab), [&lt;PT&gt;action_fn](#user-content-typedef-ba462b2e), [&lt;PA&gt;to_string_fn](#user-content-typedef-a933c596)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;T&gt;pool](#user-content-tag-d418caef), [&lt;PT&gt;iterator](#user-content-tag-d9d00f09)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of Pool](web/pool.png)

[&lt;T&gt;pool](#user-content-tag-d418caef) stores `<T>` in a memory pool\. Pointers to valid items in the pool are stable, but not generally in any order or contiguous\. It uses geometrically increasing size\-blocks and when the removal is ongoing and uniformly sampled, \(specifically, old elements are all eventually removed,\) and data reaches a steady\-state size, the data will settle in one allocated region\. In this way, manages a fairly contiguous space for items which have references\.

`<T>pool` is not synchronised\. Errors are returned with `errno`\. The parameters are preprocessor macros, and are all undefined at the end of the file for convenience\. `assert.h` is included in this file; to stop the debug assertions, use `#define NDEBUG` before `assert.h`\.



 * Parameter: POOL\_NAME, POOL\_TYPE  
   `<T>` that satisfies `C` naming conventions when mangled and a valid tag type associated therewith; required\. `<PT>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: POOL\_EXPECT\_TRAIT  
   Do not un\-define certain variables for subsequent inclusion in a trait\.
 * Parameter: POOL\_TO\_STRING\_NAME, POOL\_TO\_STRING  
   To string trait contained in [ToString\.h](ToString.h); `<A>` that satisfies `C` naming conventions when mangled and function implementing `<PT>to_string_fn`\. There can be multiple to string traits, but only one can omit `POOL_TO_STRING_NAME`\.
 * Parameter: POOL\_TEST  
   To string trait contained in [\.\./test/PoolTest\.h](../test/PoolTest.h); optional unit testing framework using `assert`\. Can only be defined once _per_ pool\. Must be defined equal to a \(random\) filler function, satisfying [&lt;PT&gt;action_fn](#user-content-typedef-ba462b2e)\. Output will be shown with the to string trait in which it's defined; provides tests for the base code and all later traits\.
 * Standard:  
   C89
 * See also:  
   [Array](https://github.com/neil-edelman/Array); [Heap](https://github.com/neil-edelman/Heap); [List](https://github.com/neil-edelman/List); [Orcish](https://github.com/neil-edelman/Orcish); [Set](https://github.com/neil-edelman/Set); [Trie](https://github.com/neil-edelman/Trie)


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-245060ab" name = "user-content-typedef-245060ab">&lt;PT&gt;type</a> ###

<code>typedef POOL_TYPE <strong>&lt;PT&gt;type</strong>;</code>

A valid tag type set by `POOL_TYPE`\.



### <a id = "user-content-typedef-ba462b2e" name = "user-content-typedef-ba462b2e">&lt;PT&gt;action_fn</a> ###

<code>typedef void(*<strong>&lt;PT&gt;action_fn</strong>)(&lt;PT&gt;type *const data);</code>

Operates by side\-effects\.



### <a id = "user-content-typedef-a933c596" name = "user-content-typedef-a933c596">&lt;PA&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PA&gt;to_string_fn</strong>)(const &lt;PA&gt;type *, char(*)[12]);</code>

Responsible for turning the first argument into a 12\-`char` null\-terminated output string\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-d418caef" name = "user-content-tag-d418caef">&lt;T&gt;pool</a> ###

<code>struct <strong>&lt;T&gt;pool</strong>;</code>

Zeroed data is a valid state\. To instantiate to an idle state, see [&lt;T&gt;pool](#user-content-fn-d418caef), `POOL_IDLE`, `{0}` \(`C99`,\) or being `static`\.

![States.](web/states.png)



### <a id = "user-content-tag-d9d00f09" name = "user-content-tag-d9d00f09">&lt;PT&gt;iterator</a> ###

<code>struct <strong>&lt;PT&gt;iterator</strong>;</code>

Contains all iteration parameters\.



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d418caef">&lt;T&gt;pool</a></td><td>pool</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-93071310">&lt;T&gt;pool_</a></td><td>pool</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-a38f29f8">&lt;T&gt;pool_reserve</a></td><td>pool, min</td></tr>

<tr><td align = right>static &lt;PT&gt;type *</td><td><a href = "#user-content-fn-37a52dae">&lt;T&gt;pool_new</a></td><td>pool</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-1485bf98">&lt;T&gt;pool_remove</a></td><td>pool, datum</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-4cc4b73d">&lt;T&gt;pool_clear</a></td><td>pool</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-794d81a9">&lt;T&gt;pool_for_each</a></td><td>pool, action</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-6fb489ab">&lt;A&gt;to_string</a></td><td>box</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-e5a6cab4">&lt;T&gt;pool_test</a></td><td></td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-d418caef" name = "user-content-fn-d418caef">&lt;T&gt;pool</a> ###

<code>static void <strong>&lt;T&gt;pool</strong>(struct &lt;T&gt;pool *const <em>pool</em>)</code>

Initialises `pool` to idle\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-93071310" name = "user-content-fn-93071310">&lt;T&gt;pool_</a> ###

<code>static void <strong>&lt;T&gt;pool_</strong>(struct &lt;T&gt;pool *const <em>pool</em>)</code>

Destroys `pool` and returns it to idle\.

 * Order:  
   &#927;\(`blocks`\)




### <a id = "user-content-fn-a38f29f8" name = "user-content-fn-a38f29f8">&lt;T&gt;pool_reserve</a> ###

<code>static int <strong>&lt;T&gt;pool_reserve</strong>(struct &lt;T&gt;pool *const <em>pool</em>, const size_t <em>min</em>)</code>

Pre\-sizes an _idle_ `pool` to ensure that it can hold at least `min` elements\.

 * Parameter: _min_  
   If zero, doesn't do anything and returns true\.
 * Return:  
   Success; the pool becomes active with at least `min` elements\.
 * Exceptional return: EDOM  
   The pool is active and doesn't allow reserving\.
 * Exceptional return: ERANGE, malloc  




### <a id = "user-content-fn-37a52dae" name = "user-content-fn-37a52dae">&lt;T&gt;pool_new</a> ###

<code>static &lt;PT&gt;type *<strong>&lt;T&gt;pool_new</strong>(struct &lt;T&gt;pool *const <em>pool</em>)</code>

 * Return:  
   A new element from `pool`\.
 * Exceptional return: ERANGE, malloc  
 * Order:  
   amortised O\(1\)




### <a id = "user-content-fn-1485bf98" name = "user-content-fn-1485bf98">&lt;T&gt;pool_remove</a> ###

<code>static int <strong>&lt;T&gt;pool_remove</strong>(struct &lt;T&gt;pool *const <em>pool</em>, &lt;PT&gt;type *const <em>datum</em>)</code>

Deletes `datum` from `pool`\.

 * Return:  
   Success\.
 * Exceptional return: EDOM  
   `data` is not part of `pool`\.
 * Order:  
   Amortised &#927;\(1\), if the pool is in steady\-state, but &#927;\(log `pool.items`\) for a small number of deleted items\.




### <a id = "user-content-fn-4cc4b73d" name = "user-content-fn-4cc4b73d">&lt;T&gt;pool_clear</a> ###

<code>static void <strong>&lt;T&gt;pool_clear</strong>(struct &lt;T&gt;pool *const <em>pool</em>)</code>

Removes all from `pool`, but keeps it's active state\. \(Only freeing the smaller blocks\.\)

 * Order:  
   &#927;\(`pool.blocks`\)




### <a id = "user-content-fn-794d81a9" name = "user-content-fn-794d81a9">&lt;T&gt;pool_for_each</a> ###

<code>static void <strong>&lt;T&gt;pool_for_each</strong>(struct &lt;T&gt;pool *const <em>pool</em>, const &lt;PT&gt;action_fn <em>action</em>)</code>

Iterates though `pool` and calls `action` on all the elements\.

 * Order:  
   O\(`capacity` &#215; `action`\)




### <a id = "user-content-fn-6fb489ab" name = "user-content-fn-6fb489ab">&lt;A&gt;to_string</a> ###

<code>static const char *<strong>&lt;A&gt;to_string</strong>(const &lt;PA&gt;box *const <em>box</em>)</code>

 * Return:  
   Print the contents of `box` in a static string buffer of 256 bytes with limitations of only printing 4 things at a time\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-e5a6cab4" name = "user-content-fn-e5a6cab4">&lt;T&gt;pool_test</a> ###

<code>static void <strong>&lt;T&gt;pool_test</strong>(void)</code>

The list will be tested on stdout; requires `POOL_TEST` and not `NDEBUG`\.





## <a id = "user-content-license" name = "user-content-license">License</a> ##

2016 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



