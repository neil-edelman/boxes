# set\.h #

## Hash set ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PS&gt;uint](#user-content-typedef-f1ed2088), [&lt;PS&gt;type](#user-content-typedef-5ef437c0), [&lt;PS&gt;ctype](#user-content-typedef-ed763cb9), [&lt;PS&gt;hash_fn](#user-content-typedef-87d76975), [&lt;PS&gt;inverse_hash_fn](#user-content-typedef-1c193eba), [&lt;PS&gt;is_equal_fn](#user-content-typedef-bbf0b37c), [&lt;PS&gt;replace_fn](#user-content-typedef-ccec694d), [&lt;PSZ&gt;box](#user-content-typedef-ace240bb), [&lt;PSZ&gt;type](#user-content-typedef-d1a7c35e), [&lt;PSZ&gt;to_string_fn](#user-content-typedef-8b890812)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;PS&gt;entry](#user-content-tag-3ef38eec), [&lt;S&gt;set](#user-content-tag-54aaac2)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of &lt;string&gt;set.](web/set.png)

[&lt;S&gt;set](#user-content-tag-54aaac2) is a hash set of unordered [&lt;PS&gt;type](#user-content-typedef-5ef437c0) that doesn't allow duplication\. It must be supplied a hash function and equality function\.

This code is simple by design\. Enclosing a pointer [&lt;PS&gt;type](#user-content-typedef-5ef437c0) in a larger `struct` can give an associative array\. Compile\-time constant sets are better handled with [gperf](https://www.gnu.org/software/gperf/)\. Also, [CMPH](http://cmph.sourceforge.net/) is a minimal perfect hashing library that provides performance for large sets\.



 * Parameter: SET\_NAME, SET\_TYPE  
   `<S>` that satisfies `C` naming conventions when mangled and a valid [&lt;PS&gt;type](#user-content-typedef-5ef437c0) associated therewith; required\. Type is copied extensively, so if it's a large, making it a pointer may improve performance\. `<PS>` is private, whose names are prefixed in a manner to avoid collisions; any should be re\-defined prior to use elsewhere\.
 * Parameter: SET\_HASH, SET\_IS\_EQUAL  
   A function satisfying [&lt;PS&gt;hash_fn](#user-content-typedef-87d76975) and [&lt;PS&gt;is_equal_fn](#user-content-typedef-bbf0b37c); required\.
 * Parameter: SET\_UINT  
   This is [&lt;PS&gt;uint](#user-content-typedef-f1ed2088), the unsigned hash type, and defaults to `size_t`\.
 * Parameter: SET\_RECALCULATE  
   Don't cache the hash, but calculate every time; this avoids storing [&lt;PS&gt;uint](#user-content-typedef-f1ed2088) _per_ entry, but can be slower when the hash is non\-trivial to compute\.
 * Parameter: SET\_INVERSE\_HASH  
   Function satisfying [&lt;PS&gt;inverse_hash_fn](#user-content-typedef-1c193eba) that avoids storing the key, but calculates it from the hashed value\. As such, incompatible with `SET_RECALCULATE`\.
 * Parameter: SET\_EXPECT\_TRAIT  
   Do not un\-define certain variables for subsequent inclusion in a trait\.
 * Parameter: SET\_TO\_STRING  
   To string trait contained in [to\_string\.h](to_string.h); `<SZ>` that satisfies `C` naming conventions when mangled and function implementing [&lt;PSZ&gt;to_string_fn](#user-content-typedef-8b890812)\. There can be multiple to string traits, but only one can omit `SET_TO_STRING_NAME`\.
 * Standard:  
   C89
 * Caveat:  
   ([&lt;PS&gt;ctype](#user-content-typedef-ed763cb9))


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-f1ed2088" name = "user-content-typedef-f1ed2088">&lt;PS&gt;uint</a> ###

<code>typedef SET_UINT <strong>&lt;PS&gt;uint</strong>;</code>

Unsigned integer type used for hash values as well as placing a limit on how many items can be in this set\. [&lt;PS&gt;hash_fn](#user-content-typedef-87d76975) returns this type\.



### <a id = "user-content-typedef-5ef437c0" name = "user-content-typedef-5ef437c0">&lt;PS&gt;type</a> ###

<code>typedef SET_TYPE <strong>&lt;PS&gt;type</strong>;</code>

Valid tag type defined by `SET_TYPE`\.



### <a id = "user-content-typedef-ed763cb9" name = "user-content-typedef-ed763cb9">&lt;PS&gt;ctype</a> ###

<code>typedef const SET_TYPE <strong>&lt;PS&gt;ctype</strong>;</code>

Used on read\-only\.

 * Caveat:  
   Including `const` qualifier in `SET_TYPE` is not supported and will lead to errors\.




### <a id = "user-content-typedef-87d76975" name = "user-content-typedef-87d76975">&lt;PS&gt;hash_fn</a> ###

<code>typedef &lt;PS&gt;uint(*<strong>&lt;PS&gt;hash_fn</strong>)(&lt;PS&gt;ctype);</code>

A map from [&lt;PS&gt;ctype](#user-content-typedef-ed763cb9) onto [&lt;PS&gt;uint](#user-content-typedef-f1ed2088)\. Usually should use all the the argument and output should be as close as possible to a discrete uniform distribution\. It is up to the user to provide an appropriate hash function\. In general, see: [https://github\.com/skeeto/hash\-prospector](https://github.com/skeeto/hash-prospector), [https://github\.com/aappleby/smhasher/](https://github.com/aappleby/smhasher/), [https://github\.com/sindresorhus/fnv1a](https://github.com/sindresorhus/fnv1a)\.



### <a id = "user-content-typedef-1c193eba" name = "user-content-typedef-1c193eba">&lt;PS&gt;inverse_hash_fn</a> ###

<code>typedef &lt;PS&gt;type(*<strong>&lt;PS&gt;inverse_hash_fn</strong>)(&lt;PS&gt;uint);</code>

Defining `SET_INVERSE_HASH` says that the [&lt;PS&gt;type](#user-content-typedef-5ef437c0) forms a bijection with [&lt;PS&gt;uint](#user-content-typedef-f1ed2088); this is inverse\-mapping to [&lt;PS&gt;hash_fn](#user-content-typedef-87d76975)\. Used to avoid having to store the [&lt;PS&gt;type](#user-content-typedef-5ef437c0)\.



### <a id = "user-content-typedef-bbf0b37c" name = "user-content-typedef-bbf0b37c">&lt;PS&gt;is_equal_fn</a> ###

<code>typedef int(*<strong>&lt;PS&gt;is_equal_fn</strong>)(const &lt;PS&gt;ctype a, const &lt;PS&gt;ctype b);</code>

Equivalence relation between [&lt;PS&gt;ctype](#user-content-typedef-ed763cb9) that satisfies `<PS>is_equal_fn(a, b) -> <PS>SET_HASH(a) == <PS>SET_HASH(b)`\.



### <a id = "user-content-typedef-ccec694d" name = "user-content-typedef-ccec694d">&lt;PS&gt;replace_fn</a> ###

<code>typedef int(*<strong>&lt;PS&gt;replace_fn</strong>)(&lt;PS&gt;type original, &lt;PS&gt;type replace);</code>

A bi\-predicate; returns true if the `replace` replaces the `original`\.



### <a id = "user-content-typedef-ace240bb" name = "user-content-typedef-ace240bb">&lt;PSZ&gt;box</a> ###

<code>typedef BOX_CONTAINER <strong>&lt;PSZ&gt;box</strong>;</code>

[to\_string\.h](to_string.h): an alias to the box\.



### <a id = "user-content-typedef-d1a7c35e" name = "user-content-typedef-d1a7c35e">&lt;PSZ&gt;type</a> ###

<code>typedef BOX_CONTENTS <strong>&lt;PSZ&gt;type</strong>;</code>

[to\_string\.h](to_string.h): an alias to the individual type contained in the box\.



### <a id = "user-content-typedef-8b890812" name = "user-content-typedef-8b890812">&lt;PSZ&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PSZ&gt;to_string_fn</strong>)(const &lt;PSZ&gt;type *, char(*)[12]);</code>

Responsible for turning the argument [&lt;PSZ&gt;type](#user-content-typedef-d1a7c35e) into a 12\-`char` null\-terminated output string\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-3ef38eec" name = "user-content-tag-3ef38eec">&lt;PS&gt;entry</a> ###

<code>struct <strong>&lt;PS&gt;entry</strong> { &lt;PS&gt;uint next; &lt;PS&gt;uint hash; &lt;PS&gt;type key; };</code>

Buckets are linked\-lists of entries, and entries are stored in a hash table\. When a collision occurs, we push the entry out to an unoccupied stack, growing from the back\.



### <a id = "user-content-tag-54aaac2" name = "user-content-tag-54aaac2">&lt;S&gt;set</a> ###

<code>struct <strong>&lt;S&gt;set</strong> { struct &lt;PS&gt;entry *entries; &lt;PS&gt;uint size; &lt;PS&gt;uint top; unsigned log_capacity, unused; };</code>

To initialize, see [&lt;S&gt;set](#user-content-fn-54aaac2), `SET_IDLE`, `{0}` \(`C99`,\) or being `static`\.

![States.](web/states.png)



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-54aaac2">&lt;S&gt;set</a></td><td>set</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-f18a9527">&lt;S&gt;set_</a></td><td>set</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-b2194878">&lt;S&gt;set_clear</a></td><td>set</td></tr>

<tr><td align = right>static &lt;PS&gt;type</td><td><a href = "#user-content-fn-4b32a391">&lt;S&gt;set_get</a></td><td>set, key</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-d425739d">&lt;S&gt;set_buffer</a></td><td>hash, reserve</td></tr>

<tr><td align = right>static &lt;PS&gt;type</td><td><a href = "#user-content-fn-fd0e5a1c">&lt;S&gt;set_put</a></td><td>hash, key</td></tr>

<tr><td align = right>static &lt;PS&gt;type</td><td><a href = "#user-content-fn-e5100be7">&lt;S&gt;set_policy_put</a></td><td>hash, key, replace</td></tr>

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



### <a id = "user-content-fn-b2194878" name = "user-content-fn-b2194878">&lt;S&gt;set_clear</a> ###

<code>static void <strong>&lt;S&gt;set_clear</strong>(struct &lt;S&gt;set *const <em>set</em>)</code>

Clears and removes all entries from `set`\. The capacity and memory of the hash table is preserved, but all previous values are un\-associated\. The load factor will be less until it reaches it's previous size\.

 * Order:  
   &#920;\(`set.entries`\)




### <a id = "user-content-fn-4b32a391" name = "user-content-fn-4b32a391">&lt;S&gt;set_get</a> ###

<code>static &lt;PS&gt;type <strong>&lt;S&gt;set_get</strong>(struct &lt;S&gt;set *const <em>set</em>, const &lt;PS&gt;type <em>key</em>)</code>

 * Return:  
   The value in `set` which [&lt;PS&gt;is_equal_fn](#user-content-typedef-bbf0b37c) `SET_IS_EQUAL` `key`, or, if no such value exists, null\.
 * Order:  
   Average &#927;\(1\), \(hash distributes elements uniformly\); worst &#927;\(n\)\.




### <a id = "user-content-fn-d425739d" name = "user-content-fn-d425739d">&lt;S&gt;set_buffer</a> ###

<code>static int <strong>&lt;S&gt;set_buffer</strong>(struct &lt;S&gt;set *const <em>hash</em>, const size_t <em>reserve</em>)</code>

Reserve at least `reserve`, divided by the maximum load factor, space in the entries of `hash`\.

 * Return:  
   Success\.
 * Exceptional return: ERANGE  
   `reserve` plus the size would take a bigger number then could fit in a `size_t`\.
 * Exceptional return: realloc  




### <a id = "user-content-fn-fd0e5a1c" name = "user-content-fn-fd0e5a1c">&lt;S&gt;set_put</a> ###

<code>static &lt;PS&gt;type <strong>&lt;S&gt;set_put</strong>(struct &lt;S&gt;set *const <em>hash</em>, const &lt;PS&gt;type <em>key</em>)</code>

Puts `key` in `hash`\.

 * Return:  
   Any ejected key or null\.
 * Exceptional return: realloc, ERANGE  
   There was an error with a re\-sizing\. It is not always possible to tell the difference between an error and a unique key\. If needed, before calling this, successfully calling [&lt;S&gt;set_buffer](#user-content-fn-d425739d), or setting `errno` to zero\.
 * Order:  
   Average amortised &#927;\(1\), \(hash distributes keys uniformly\); worst &#927;\(n\) \(are you sure that's up to date?\)\.




### <a id = "user-content-fn-e5100be7" name = "user-content-fn-e5100be7">&lt;S&gt;set_policy_put</a> ###

<code>static &lt;PS&gt;type <strong>&lt;S&gt;set_policy_put</strong>(struct &lt;S&gt;set *const <em>hash</em>, const &lt;PS&gt;type <em>key</em>, const &lt;PS&gt;replace_fn <em>replace</em>)</code>

Puts `key` in `hash` only if the entry is absent or if calling `replace` returns true\.

 * Parameter: _replace_  
   If null, doesn't do any replacement on collision\.
 * Return:  
   Any ejected element or null\. On collision, if `replace` returns false or `replace` is null, returns `key` and leaves the other element in the hash\.
 * Exceptional return: realloc, ERANGE  
   There was an error with a re\-sizing\. Successfully calling [&lt;S&gt;set_buffer](#user-content-fn-d425739d) ensures that this does not happen\.
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



