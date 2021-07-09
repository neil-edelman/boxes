# pool\.h #

## Stable Pool ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [pool_slot](#user-content-typedef-9e92efe8), [&lt;PP&gt;type](#user-content-typedef-7560d92f), [&lt;PP&gt;action_fn](#user-content-typedef-cefaf27a), [&lt;PZ&gt;to_string_fn](#user-content-typedef-22f3d7f1)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [pool_chunk](#user-content-tag-667964d9), [&lt;P&gt;pool](#user-content-tag-8aba39cb)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of Pool](web/pool.png)

[&lt;P&gt;pool](#user-content-tag-8aba39cb) is a memory pool that stores [&lt;PP&gt;type](#user-content-typedef-7560d92f)\. Pointers to valid items in the pool are stable, but not generally in any order or contiguous\. It uses geometrically increasing size\-blocks, so, if data reaches a steady\-state size, when the removal is uniformly sampled, it will settle in one allocated region\.



 * Parameter: POOL\_NAME, POOL\_TYPE  
   `<P>` that satisfies `C` naming conventions when mangled and a valid tag type associated therewith; required\. `<PP>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: POOL\_TEST  
   To string trait contained in [\.\./test/pool\_test\.h](../test/pool_test.h); optional unit testing framework using `assert`\. Must be defined equal to a \(random\) filler function, satisfying [&lt;PP&gt;action_fn](#user-content-typedef-cefaf27a)\. Needs at least one to string trait\.
 * Parameter: POOL\_EXPECT\_TRAIT  
   Do not un\-define certain variables for subsequent inclusion in a trait\.
 * Parameter: POOL\_TO\_STRING\_NAME, POOL\_TO\_STRING  
   To string trait contained in [to\_string\.h](to_string.h); `<A>` that satisfies `C` naming conventions when mangled and function implementing [&lt;PZ&gt;to_string_fn](#user-content-typedef-22f3d7f1)\. There can be multiple to string traits, but only one can omit `POOL_TO_STRING_NAME`\.
 * Standard:  
   C89
 * Dependancies:  
   [array](https://github.com/neil-edelman/array), [heap](https://github.com/neil-edelman/heap)


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-9e92efe8" name = "user-content-typedef-9e92efe8">pool_slot</a> ###

<code>typedef struct pool_chunk *<strong>pool_slot</strong>;</code>

A slot is a pointer to a stable chunk\. It makes the source much more readable to have this instead of a `**chunk`\.



### <a id = "user-content-typedef-7560d92f" name = "user-content-typedef-7560d92f">&lt;PP&gt;type</a> ###

<code>typedef POOL_TYPE <strong>&lt;PP&gt;type</strong>;</code>

A valid tag type set by `POOL_TYPE`\.



### <a id = "user-content-typedef-cefaf27a" name = "user-content-typedef-cefaf27a">&lt;PP&gt;action_fn</a> ###

<code>typedef void(*<strong>&lt;PP&gt;action_fn</strong>)(&lt;PP&gt;type *);</code>

Operates by side\-effects\.



### <a id = "user-content-typedef-22f3d7f1" name = "user-content-typedef-22f3d7f1">&lt;PZ&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PZ&gt;to_string_fn</strong>)(const &lt;PZ&gt;type *, char(*)[12]);</code>

Responsible for turning the first argument into a 12\-`char` null\-terminated output string\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-667964d9" name = "user-content-tag-667964d9">pool_chunk</a> ###

<code>struct <strong>pool_chunk</strong> { size_t size; };</code>

Stable chunk followed by data; explicit naming to avoid confusion\.



### <a id = "user-content-tag-8aba39cb" name = "user-content-tag-8aba39cb">&lt;P&gt;pool</a> ###

<code>struct <strong>&lt;P&gt;pool</strong>;</code>

Consists of a map of several chunks of increasing size and a free\-list\. Zeroed data is a valid state\. To instantiate to an idle state, see [&lt;P&gt;pool](#user-content-fn-8aba39cb), `POOL_IDLE`, `{0}` \(`C99`,\) or being `static`\.

![States.](web/states.png)



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-8aba39cb">&lt;P&gt;pool</a></td><td>pool</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-f728a3fc">&lt;P&gt;pool_</a></td><td>pool</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-3579e316">&lt;P&gt;pool_buffer</a></td><td>pool, n</td></tr>

<tr><td align = right>static &lt;PP&gt;type *</td><td><a href = "#user-content-fn-e71c341a">&lt;P&gt;pool_new</a></td><td>pool</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-95972ccc">&lt;P&gt;pool_remove</a></td><td>pool, datum</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-96f5dc51">&lt;P&gt;pool_clear</a></td><td>pool</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d026a8f8">&lt;P&gt;pool_test</a></td><td></td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-4ecb4112">&lt;Z&gt;to_string</a></td><td>box</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-8aba39cb" name = "user-content-fn-8aba39cb">&lt;P&gt;pool</a> ###

<code>static void <strong>&lt;P&gt;pool</strong>(struct &lt;P&gt;pool *const <em>pool</em>)</code>

Initializes `pool` to idle\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-f728a3fc" name = "user-content-fn-f728a3fc">&lt;P&gt;pool_</a> ###

<code>static void <strong>&lt;P&gt;pool_</strong>(struct &lt;P&gt;pool *const <em>pool</em>)</code>

Destroys `pool` and returns it to idle\.

 * Order:  
   &#927;\(\\log `data`\)




### <a id = "user-content-fn-3579e316" name = "user-content-fn-3579e316">&lt;P&gt;pool_buffer</a> ###

<code>static int <strong>&lt;P&gt;pool_buffer</strong>(struct &lt;P&gt;pool *const <em>pool</em>, const size_t <em>n</em>)</code>

Ensure capacity of at least `n` items in `pool`\. Pre\-sizing is better for contiguous blocks\.

 * Return:  
   Success\.
 * Exceptional return: ERANGE, malloc  




### <a id = "user-content-fn-e71c341a" name = "user-content-fn-e71c341a">&lt;P&gt;pool_new</a> ###

<code>static &lt;PP&gt;type *<strong>&lt;P&gt;pool_new</strong>(struct &lt;P&gt;pool *const <em>pool</em>)</code>

This pointer is constant until it gets removed\.

 * Return:  
   A pointer to a new uninitialized element from `pool`\.
 * Exceptional return: ERANGE, malloc  
 * Order:  
   amortised O\(1\)




### <a id = "user-content-fn-95972ccc" name = "user-content-fn-95972ccc">&lt;P&gt;pool_remove</a> ###

<code>static int <strong>&lt;P&gt;pool_remove</strong>(struct &lt;P&gt;pool *const <em>pool</em>, &lt;PP&gt;type *const <em>datum</em>)</code>

Deletes `datum` from `pool`\. Do not remove data that is not in `pool`\.

 * Return:  
   Success\.
 * Order:  
   &#927;\(\\log \\log `items`\)




### <a id = "user-content-fn-96f5dc51" name = "user-content-fn-96f5dc51">&lt;P&gt;pool_clear</a> ###

<code>static void <strong>&lt;P&gt;pool_clear</strong>(struct &lt;P&gt;pool *const <em>pool</em>)</code>

Removes all from `pool`, but keeps it's active state, only freeing the smaller blocks\.

 * Order:  
   &#927;\(\\log `items`\)




### <a id = "user-content-fn-d026a8f8" name = "user-content-fn-d026a8f8">&lt;P&gt;pool_test</a> ###

<code>static void <strong>&lt;P&gt;pool_test</strong>(void)</code>

The list will be tested on stdout; requires `POOL_TEST` and not `NDEBUG`\.



### <a id = "user-content-fn-4ecb4112" name = "user-content-fn-4ecb4112">&lt;Z&gt;to_string</a> ###

<code>static const char *<strong>&lt;Z&gt;to_string</strong>(const &lt;PZ&gt;box *const <em>box</em>)</code>

 * Return:  
   Print the contents of `box` in a static string buffer of 256 bytes with limitations of only printing 4 things at a time\.
 * Order:  
   &#920;\(1\)






## <a id = "user-content-license" name = "user-content-license">License</a> ##

2021 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



