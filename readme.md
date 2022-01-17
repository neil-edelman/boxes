# set\.h #

## Hash table ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PS&gt;uint](#user-content-typedef-f1ed2088), [&lt;PS&gt;key](#user-content-typedef-759eb157), [&lt;PS&gt;ckey](#user-content-typedef-6ff89358), [&lt;PS&gt;hash_fn](#user-content-typedef-87d76975), [&lt;PS&gt;inverse_hash_fn](#user-content-typedef-1c193eba), [&lt;PS&gt;is_equal_fn](#user-content-typedef-bbf0b37c), [&lt;PS&gt;value](#user-content-typedef-2830cf59), [&lt;PS&gt;entry](#user-content-typedef-3ef38eec), [&lt;PS&gt;policy_fn](#user-content-typedef-ff188dd7), [&lt;PSZ&gt;box](#user-content-typedef-ace240bb), [&lt;PSZ&gt;key](#user-content-typedef-bd74ee05), [&lt;PSZ&gt;to_string_fn](#user-content-typedef-8b890812)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [set_result](#user-content-tag-f250624d), [&lt;S&gt;set_entry](#user-content-tag-ef912361), [&lt;S&gt;set](#user-content-tag-54aaac2), [&lt;S&gt;set_iterator](#user-content-tag-f91e42cd)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of &lt;string&gt;set.](web/set.png)

[&lt;S&gt;set](#user-content-tag-54aaac2) is a set or map of [&lt;PS&gt;entry](#user-content-typedef-3ef38eec) implemented as a hash table\. It must be supplied a [&lt;PS&gt;hash_fn](#user-content-typedef-87d76975) and [&lt;PS&gt;is_equal_fn](#user-content-typedef-bbf0b37c)\.

fixme

 * Parameter: SET\_NAME, SET\_KEY  
   `<S>` that satisfies `C` naming conventions when mangled and a valid [&lt;PS&gt;key](#user-content-typedef-759eb157) associated therewith; required\. `<PS>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: SET\_HASH, SET\_IS\_EQUAL  
   A function satisfying [&lt;PS&gt;hash_fn](#user-content-typedef-87d76975) and [&lt;PS&gt;is_equal_fn](#user-content-typedef-bbf0b37c)\. `SET_HASH` and either `SET_IS_EQUAL` or `SET_INVERSE`, but not both, are required\.
 * Parameter: SET\_VALUE  
   An optional type that is the payload of the key, thus making this an associative array\. If the key is part of an aggregate value, it will be more efficient and robust to use a type conversion instead of storing related pointers\.
 * Parameter: SET\_UINT  
   This is [&lt;PS&gt;uint](#user-content-typedef-f1ed2088), the unsigned type of hash hash of the key given by [&lt;PS&gt;hash_fn](#user-content-typedef-87d76975); defaults to `size_t`\.
 * Parameter: SET\_INVERSE  
   Function satisfying [&lt;PS&gt;inverse_hash_fn](#user-content-typedef-1c193eba); this avoids storing the key, but calculates it from the hashed value\. The hashes are now unique, so there is no need for a `SET_IS_EQUAL`\.
 * Parameter: SET\_EXPECT\_TRAIT  
   Do not un\-define certain variables for subsequent inclusion in a trait\.
 * Parameter: SET\_TO\_STRING\_NAME, SET\_TO\_STRING  
   To string trait contained in [to\_string\.h](to_string.h); `<SZ>` that satisfies `C` naming conventions when mangled and function implementing [&lt;PSZ&gt;to_string_fn](#user-content-typedef-8b890812)\. There can be multiple to string traits, but only one can omit `SET_TO_STRING_NAME`\.
 * Standard:  
   C89


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-f1ed2088" name = "user-content-typedef-f1ed2088">&lt;PS&gt;uint</a> ###

<code>typedef SET_UINT <strong>&lt;PS&gt;uint</strong>;</code>

[&lt;PS&gt;hash_fn](#user-content-typedef-87d76975) returns this hash type by `SET_UINT`, which must be be an unsigned integer\. Places a simplifying limit on the maximum number of items in this container of half the cardinality\.



### <a id = "user-content-typedef-759eb157" name = "user-content-typedef-759eb157">&lt;PS&gt;key</a> ###

<code>typedef SET_KEY <strong>&lt;PS&gt;key</strong>;</code>

Valid tag type defined by `SET_KEY` used for keys\.



### <a id = "user-content-typedef-6ff89358" name = "user-content-typedef-6ff89358">&lt;PS&gt;ckey</a> ###

<code>typedef const SET_KEY <strong>&lt;PS&gt;ckey</strong>;</code>

Read\-only [&lt;PS&gt;key](#user-content-typedef-759eb157)\. Makes the simplifying assumption that this is not `const`\-qualified\.



### <a id = "user-content-typedef-87d76975" name = "user-content-typedef-87d76975">&lt;PS&gt;hash_fn</a> ###

<code>typedef &lt;PS&gt;uint(*<strong>&lt;PS&gt;hash_fn</strong>)(&lt;PS&gt;ckey);</code>

A map from [&lt;PS&gt;ckey](#user-content-typedef-6ff89358) onto [&lt;PS&gt;uint](#user-content-typedef-f1ed2088), \(any will do, but the performance may suffer if too many entries are hashed to the same buckets\.\) If [&lt;PS&gt;key](#user-content-typedef-759eb157) is a pointer, one is permitted to have null in the domain\.



### <a id = "user-content-typedef-1c193eba" name = "user-content-typedef-1c193eba">&lt;PS&gt;inverse_hash_fn</a> ###

<code>typedef &lt;PS&gt;key(*<strong>&lt;PS&gt;inverse_hash_fn</strong>)(&lt;PS&gt;uint);</code>

Defining `SET_INVERSE` says [&lt;PS&gt;hash_fn](#user-content-typedef-87d76975) forms a bijection between the range in [&lt;PS&gt;key](#user-content-typedef-759eb157) and the image in [&lt;PS&gt;uint](#user-content-typedef-f1ed2088)\. This is the inverse\-mapping\.



### <a id = "user-content-typedef-bbf0b37c" name = "user-content-typedef-bbf0b37c">&lt;PS&gt;is_equal_fn</a> ###

<code>typedef int(*<strong>&lt;PS&gt;is_equal_fn</strong>)(&lt;PS&gt;ckey a, &lt;PS&gt;ckey b);</code>

Equivalence relation between [&lt;PS&gt;key](#user-content-typedef-759eb157) that satisfies `<PS>is_equal_fn(a, b) -> <PS>hash(a) == <PS>hash(b)`\.



### <a id = "user-content-typedef-2830cf59" name = "user-content-typedef-2830cf59">&lt;PS&gt;value</a> ###

<code>typedef SET_VALUE <strong>&lt;PS&gt;value</strong>;</code>

Defining `SET_VALUE` produces an associative map, otherwise it is the same as [&lt;PS&gt;key](#user-content-typedef-759eb157)\.



### <a id = "user-content-typedef-3ef38eec" name = "user-content-typedef-3ef38eec">&lt;PS&gt;entry</a> ###

<code>typedef struct &lt;S&gt;set_entry <strong>&lt;PS&gt;entry</strong>;</code>

If `SET_VALUE`, this is [&lt;S&gt;set_entry](#user-content-tag-ef912361); otherwise, it's the same as [&lt;PS&gt;key](#user-content-typedef-759eb157)\.



### <a id = "user-content-typedef-ff188dd7" name = "user-content-typedef-ff188dd7">&lt;PS&gt;policy_fn</a> ###

<code>typedef int(*<strong>&lt;PS&gt;policy_fn</strong>)(&lt;PS&gt;key original, &lt;PS&gt;key replace);</code>

Returns true if the `replace` replaces the `original`\.



### <a id = "user-content-typedef-ace240bb" name = "user-content-typedef-ace240bb">&lt;PSZ&gt;box</a> ###

<code>typedef BOX_CONTAINER <strong>&lt;PSZ&gt;box</strong>;</code>

[to\_string\.h](to_string.h): an alias to the box\.



### <a id = "user-content-typedef-bd74ee05" name = "user-content-typedef-bd74ee05">&lt;PSZ&gt;key</a> ###

<code>typedef BOX_CONTENTS <strong>&lt;PSZ&gt;key</strong>;</code>

[to\_string\.h](to_string.h): an alias to the individual key contained in the box\.



### <a id = "user-content-typedef-8b890812" name = "user-content-typedef-8b890812">&lt;PSZ&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PSZ&gt;to_string_fn</strong>)(const &lt;PSZ&gt;key *, char(*)[12]);</code>

Responsible for turning the argument [&lt;PSZ&gt;key](#user-content-typedef-bd74ee05) into a 12\-`char` null\-terminated output string\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-f250624d" name = "user-content-tag-f250624d">set_result</a> ###

<code>enum <strong>set_result</strong> { SET_RESULT };</code>

This is the result of modifying the table\. An `enum` of `SET_*`, of which `SET_ERROR` is false\. ![A diagram of the result states.](web/put.png)



### <a id = "user-content-tag-ef912361" name = "user-content-tag-ef912361">&lt;S&gt;set_entry</a> ###

<code>struct <strong>&lt;S&gt;set_entry</strong> { &lt;PS&gt;key key; &lt;PS&gt;value value; };</code>

Defining `SET_VALUE` creates this map from [&lt;PS&gt;key](#user-content-typedef-759eb157) to [&lt;PS&gt;value](#user-content-typedef-2830cf59) as an interface with set\.



### <a id = "user-content-tag-54aaac2" name = "user-content-tag-54aaac2">&lt;S&gt;set</a> ###

<code>struct <strong>&lt;S&gt;set</strong> { struct &lt;PS&gt;bucket *buckets; &lt;PS&gt;uint log_capacity, size, top; };</code>

To initialize, see [&lt;S&gt;set](#user-content-fn-54aaac2), `SET_IDLE`, `{0}` \(`C99`,\) or being `static`\. The fields should be treated as read\-only; any modification is liable to cause the set to go into an invalid state\.

![States.](web/states.png)



### <a id = "user-content-tag-f91e42cd" name = "user-content-tag-f91e42cd">&lt;S&gt;set_iterator</a> ###

<code>struct <strong>&lt;S&gt;set_iterator</strong> { struct &lt;PS&gt;iterator it; };</code>

Iteration usually not in any particular order\. The asymptotic runtime is proportional to the hash capacity\.



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-54aaac2">&lt;S&gt;set</a></td><td>set</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-f18a9527">&lt;S&gt;set_</a></td><td>set</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-d425739d">&lt;S&gt;set_buffer</a></td><td>set, n</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-b2194878">&lt;S&gt;set_clear</a></td><td>set</td></tr>

<tr><td align = right>static &lt;PS&gt;key</td><td><a href = "#user-content-fn-4b32a391">&lt;S&gt;set_get</a></td><td>hash, key</td></tr>

<tr><td align = right>static enum set_result</td><td><a href = "#user-content-fn-8d876df6">&lt;S&gt;set_update</a></td><td>set, entry, eject, update</td></tr>

<tr><td align = right>static struct &lt;S&gt;setlink *</td><td><a href = "#user-content-fn-f336902b">&lt;S&gt;set_remove</a></td><td>hash, data</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-b11709d3">&lt;SZ&gt;to_string</a></td><td>box</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-54aaac2" name = "user-content-fn-54aaac2">&lt;S&gt;set</a> ###

<code>static void <strong>&lt;S&gt;set</strong>(struct &lt;S&gt;set *const <em>set</em>)</code>

Initialises `set` to idle\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-f18a9527" name = "user-content-fn-f18a9527">&lt;S&gt;set_</a> ###

<code>static void <strong>&lt;S&gt;set_</strong>(struct &lt;S&gt;set *const <em>set</em>)</code>

Destroys `set` and returns it to idle\.



### <a id = "user-content-fn-d425739d" name = "user-content-fn-d425739d">&lt;S&gt;set_buffer</a> ###

<code>static int <strong>&lt;S&gt;set_buffer</strong>(struct &lt;S&gt;set *const <em>set</em>, const &lt;PS&gt;uint <em>n</em>)</code>

Reserve at least `n` space for buckets of `set`\. This will ensure that there is space for those buckets and may increase iteration time\.

 * Return:  
   Success\.
 * Exceptional return: ERANGE  
   The request was unsatisfiable\.
 * Exceptional return: realloc  




### <a id = "user-content-fn-b2194878" name = "user-content-fn-b2194878">&lt;S&gt;set_clear</a> ###

<code>static void <strong>&lt;S&gt;set_clear</strong>(struct &lt;S&gt;set *const <em>set</em>)</code>

Clears and removes all buckets from `set`\. The capacity and memory of the `set` is preserved, but all previous values are un\-associated\. \(The load factor will be less until it reaches it's previous size\.\)

 * Order:  
   &#920;\(`set.capacity`\)




### <a id = "user-content-fn-4b32a391" name = "user-content-fn-4b32a391">&lt;S&gt;set_get</a> ###

<code>static &lt;PS&gt;key <strong>&lt;S&gt;set_get</strong>(struct &lt;S&gt;set *const <em>hash</em>, const &lt;PS&gt;key <em>key</em>)</code>

 * Return:  
   The value in `hash` which is equal `key`, or, if no such value exists, null\.
 * Order:  
   Average &#927;\(1\), \(hash distributes elements uniformly\); worst &#927;\(n\)\.




### <a id = "user-content-fn-8d876df6" name = "user-content-fn-8d876df6">&lt;S&gt;set_update</a> ###

<code>static enum set_result <strong>&lt;S&gt;set_update</strong>(struct &lt;S&gt;set *const <em>set</em>, &lt;PS&gt;entry <em>entry</em>, &lt;PS&gt;entry *<em>eject</em>, const &lt;PS&gt;policy_fn <em>update</em>)</code>

Puts `entry` in `set` only if absent or if calling `update` returns true\.

 * Return:  
   One of: `SET_ERROR` the set is not modified; `SET_REPLACE` if `update` is non\-null and returns true, `eject`, if non\-null, will be filled; `SET_YIELD` if `replace` is null or false; `SET_GROW`, on unique entry\.
 * Exceptional return: realloc, ERANGE  
   There was an error with resizing\.
 * Order:  
   Average amortised &#927;\(1\), \(hash distributes keys uniformly\); worst &#927;\(n\)\.




### <a id = "user-content-fn-f336902b" name = "user-content-fn-f336902b">&lt;S&gt;set_remove</a> ###

<code>static struct &lt;S&gt;setlink *<strong>&lt;S&gt;set_remove</strong>(struct &lt;S&gt;set *const <em>hash</em>, const &lt;PS&gt;mtype <em>data</em>)</code>

Removes an element `data` from `hash`\.

 * Return:  
   Successfully ejected element or null\.
 * Order:  
   Average &#927;\(1\), \(hash distributes elements uniformly\); worst &#927;\(n\)\.




### <a id = "user-content-fn-b11709d3" name = "user-content-fn-b11709d3">&lt;SZ&gt;to_string</a> ###

<code>static const char *<strong>&lt;SZ&gt;to_string</strong>(const &lt;PSZ&gt;box *const <em>box</em>)</code>

 * Return:  
   Print the contents of [&lt;PSZ&gt;box](#user-content-typedef-ace240bb) `box` in a static string buffer of 256 bytes with limitations of only printing 4 things at a time\.
 * Order:  
   &#920;\(1\)






## <a id = "user-content-license" name = "user-content-license">License</a> ##

2019 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



