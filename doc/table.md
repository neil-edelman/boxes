# table\.h #

Stand\-alone header [\.\./src/table\.h](../src/table.h); examples [\.\./test/test\_table\.c](../test/test_table.c); article [\.\./doc/table/table\.pdf](../doc/table/table.pdf)\.

## Hash table ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PN&gt;uint](#user-content-typedef-c13937ad), [&lt;PN&gt;key](#user-content-typedef-e7af8dc0), [&lt;PN&gt;hash_fn](#user-content-typedef-5e79a292), [&lt;PN&gt;unhash_fn](#user-content-typedef-ca7574d7), [&lt;PN&gt;is_equal_fn](#user-content-typedef-52314bb), [&lt;PN&gt;value](#user-content-typedef-218ce716), [&lt;PN&gt;entry](#user-content-typedef-a9017e7), [&lt;PN&gt;policy_fn](#user-content-typedef-1244a528), [&lt;PITR&gt;action_fn](#user-content-typedef-49d9168b), [&lt;PITR&gt;predicate_fn](#user-content-typedef-c5016dba), [&lt;PSTR&gt;to_string_fn](#user-content-typedef-8a8349ca)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [table_result](#user-content-tag-4f1ea759), [&lt;N&gt;table_entry](#user-content-tag-b491b196), [&lt;N&gt;table](#user-content-tag-8f317be5), [&lt;N&gt;table_iterator](#user-content-tag-f67540e4)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of &lt;string&gt;table.](../doc/table/table.png)

[&lt;N&gt;table](#user-content-tag-8f317be5) implements a set or map of [&lt;PN&gt;entry](#user-content-typedef-a9017e7) as a hash table\. It must be supplied [&lt;PN&gt;hash_fn](#user-content-typedef-5e79a292) `<N>hash` and, [&lt;PN&gt;is_equal_fn](#user-content-typedef-52314bb) `<N>is_equal` or [&lt;PN&gt;unhash_fn](#user-content-typedef-ca7574d7) `<N>unhash`\.

These go together to allow exporting non\-static data between compilation units by separating the `TABLE_BODY` refers to `TABLE_HEAD`, and identical

\* [src/iterate\.h](src/iterate.h): defining `HAVE_ITERATE_H` supplies `<ITR>` functions\.

 * Parameter: TABLE\_NAME, TABLE\_KEY  
   `<N>` that satisfies `C` naming conventions when mangled and a valid [&lt;PN&gt;key](#user-content-typedef-e7af8dc0) associated therewith; required\. `<PN>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: TABLE\_UNHASH  
   By default it assumes that `<N>is_equal` is supplied; with this, instead requires `<N>unhash` satisfying [&lt;PN&gt;unhash_fn](#user-content-typedef-ca7574d7)\.
 * Parameter: TABLE\_VALUE  
   An optional type that is the payload of the key, thus making this a map or associative array\.
 * Parameter: TABLE\_UINT  
   This is [&lt;PN&gt;uint](#user-content-typedef-c13937ad), the unsigned type of hash of the key given by [&lt;PN&gt;hash_fn](#user-content-typedef-5e79a292); defaults to `size_t`\.
 * Parameter: TABLE\_DEFAULT  
   Default trait; a [&lt;PN&gt;value](#user-content-typedef-218ce716) used in [&lt;N&gt;table&lt;D&gt;get](#user-content-fn-92774ccb)\.
 * Parameter: TABLE\_TO\_STRING  
   To string trait `<STR>` contained in [src/to\_string\.h](src/to_string.h)\. Require `<name>[<trait>]to_string` be declared as [&lt;PSTR&gt;to_string_fn](#user-content-typedef-8a8349ca)\.
 * Parameter: TABLE\_EXPECT\_TRAIT, TABLE\_TRAIT  
   Named traits are obtained by including `table.h` multiple times with `TABLE_EXPECT_TRAIT` and then subsequently including the name in `TABLE_TRAIT`\.
 * Parameter: TABLE\_HEAD, TABLE\_BODY  
   These go together to allow exporting non\-static data between compilation units by separating the header head from the code body\. `TABLE_HEAD` needs identical `TABLE_NAME`, `TABLE_KEY`, `TABLE_UNHASH`, `TABLE_VALUE`, and `TABLE_UINT`\.
 * Standard:  
   C89


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-c13937ad" name = "user-content-typedef-c13937ad">&lt;PN&gt;uint</a> ###

<code>typedef TABLE_UINT <strong>&lt;PN&gt;uint</strong>;</code>

[&lt;PN&gt;hash_fn](#user-content-typedef-5e79a292) returns this hash type by `TABLE_UINT`, which must be be an unsigned integer\. Places a simplifying limit on the maximum number of elements of half the cardinality\.



### <a id = "user-content-typedef-e7af8dc0" name = "user-content-typedef-e7af8dc0">&lt;PN&gt;key</a> ###

<code>typedef TABLE_KEY <strong>&lt;PN&gt;key</strong>;</code>

Valid tag type defined by `TABLE_KEY` used for keys\. If `TABLE_UNHASH` is not defined, a copy of this value will be stored in the internal buckets\.



### <a id = "user-content-typedef-5e79a292" name = "user-content-typedef-5e79a292">&lt;PN&gt;hash_fn</a> ###

<code>typedef &lt;PN&gt;uint(*<strong>&lt;PN&gt;hash_fn</strong>)(const &lt;PN&gt;key);</code>

A map from [&lt;PN&gt;key_c](#user-content-typedef-46bcab6a) onto [&lt;PN&gt;uint](#user-content-typedef-c13937ad), called `<N>hash`, that, ideally, should be easy to compute while minimizing duplicate addresses\. Must be consistent for each value while in the table\. If [&lt;PN&gt;key](#user-content-typedef-e7af8dc0) is a pointer, one is permitted to have null in the domain\.



### <a id = "user-content-typedef-ca7574d7" name = "user-content-typedef-ca7574d7">&lt;PN&gt;unhash_fn</a> ###

<code>typedef &lt;PN&gt;key(*<strong>&lt;PN&gt;unhash_fn</strong>)(&lt;PN&gt;uint);</code>

Defining `TABLE_UNHASH` says [&lt;PN&gt;hash_fn](#user-content-typedef-5e79a292) forms a bijection between the range in [&lt;PN&gt;key](#user-content-typedef-e7af8dc0) and the image in [&lt;PN&gt;uint](#user-content-typedef-c13937ad), and the inverse is called `<N>unhash`\. In this case, keys are not stored in the hash table, rather they are generated using this inverse\-mapping\.



### <a id = "user-content-typedef-52314bb" name = "user-content-typedef-52314bb">&lt;PN&gt;is_equal_fn</a> ###

<code>typedef int(*<strong>&lt;PN&gt;is_equal_fn</strong>)(&lt;PN&gt;key_c a, &lt;PN&gt;key_c b);</code>

Equivalence relation between [&lt;PN&gt;key](#user-content-typedef-e7af8dc0) that satisfies `<PN>is_equal_fn(a, b) -> <PN>hash(a) == <PN>hash(b)`, called `<N>is_equal`\. If `TABLE_UNHASH` is set, there is no need for this function because the comparison is done directly in hash space\.



### <a id = "user-content-typedef-218ce716" name = "user-content-typedef-218ce716">&lt;PN&gt;value</a> ###

<code>typedef TABLE_VALUE <strong>&lt;PN&gt;value</strong>;</code>

Defining `TABLE_VALUE` produces an associative map, otherwise it is the same as [&lt;PN&gt;key](#user-content-typedef-e7af8dc0)\.



### <a id = "user-content-typedef-a9017e7" name = "user-content-typedef-a9017e7">&lt;PN&gt;entry</a> ###

<code>typedef struct &lt;N&gt;table_entry <strong>&lt;PN&gt;entry</strong>;</code>

If `TABLE_VALUE`, this is [&lt;N&gt;table_entry](#user-content-tag-b491b196); otherwise, it's the same as [&lt;PN&gt;key](#user-content-typedef-e7af8dc0)\.



### <a id = "user-content-typedef-1244a528" name = "user-content-typedef-1244a528">&lt;PN&gt;policy_fn</a> ###

<code>typedef int(*<strong>&lt;PN&gt;policy_fn</strong>)(&lt;PN&gt;key original, &lt;PN&gt;key replace);</code>

Returns true if the `replace` replaces the `original`\. \(Shouldn't it be entry?\)



### <a id = "user-content-typedef-49d9168b" name = "user-content-typedef-49d9168b">&lt;PITR&gt;action_fn</a> ###

<code>typedef void(*<strong>&lt;PITR&gt;action_fn</strong>)(&lt;PITR&gt;element *);</code>

[src/iterate\.h](src/iterate.h): Operates by side\-effects\.



### <a id = "user-content-typedef-c5016dba" name = "user-content-typedef-c5016dba">&lt;PITR&gt;predicate_fn</a> ###

<code>typedef int(*<strong>&lt;PITR&gt;predicate_fn</strong>)(const &lt;PITR&gt;element *);</code>

[src/iterate\.h](src/iterate.h): Returns a boolean given read\-only\.



### <a id = "user-content-typedef-8a8349ca" name = "user-content-typedef-8a8349ca">&lt;PSTR&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PSTR&gt;to_string_fn</strong>)(const &lt;PSTR&gt;element *, char(*)[12]);</code>

[src/to\_string\.h](src/to_string.h): responsible for turning the read\-only argument into a 12\-`char` null\-terminated output string, passed as a pointer in the last argument\. This function can have 2 or 3 arguments, where `<PSTR>element` might be a map with a key\-value pair\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-4f1ea759" name = "user-content-tag-4f1ea759">table_result</a> ###

<code>enum <strong>table_result</strong> { TABLE_RESULT };</code>

A result of modifying the table, of which `TABLE_ERROR` is false\.

![A diagram of the result states.](../doc/table/result.png)



### <a id = "user-content-tag-b491b196" name = "user-content-tag-b491b196">&lt;N&gt;table_entry</a> ###

<code>struct <strong>&lt;N&gt;table_entry</strong> { &lt;PN&gt;key key; &lt;PN&gt;value value; };</code>

Defining `TABLE_VALUE` creates this map from [&lt;PN&gt;key](#user-content-typedef-e7af8dc0) to [&lt;PN&gt;value](#user-content-typedef-218ce716), as an interface with table\.



### <a id = "user-content-tag-8f317be5" name = "user-content-tag-8f317be5">&lt;N&gt;table</a> ###

<code>struct <strong>&lt;N&gt;table</strong> { struct &lt;PN&gt;bucket *buckets; &lt;PN&gt;uint log_capacity, size, top; };</code>

To initialize, see [&lt;N&gt;table](#user-content-fn-8f317be5), `TABLE_IDLE`, `{0}` \(`C99`,\) or being `static`\. The fields should be treated as read\-only; any modification is liable to cause the table to go into an invalid state\.

![States.](../doc/table/states.png)



### <a id = "user-content-tag-f67540e4" name = "user-content-tag-f67540e4">&lt;N&gt;table_iterator</a> ###

<code>struct <strong>&lt;N&gt;table_iterator</strong>;</code>

![States](../doc/table/it.png)

Adding, deleting, successfully looking up entries, or any modification of the table's topology invalidates the iterator\. Iteration usually not in any particular order\. The asymptotic runtime of iterating though the whole table is proportional to the capacity\.



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static struct &lt;N&gt;table</td><td><a href = "#user-content-fn-8f317be5">&lt;N&gt;table</a></td><td></td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-24e5c5ce">&lt;N&gt;table_</a></td><td>table</td></tr>

<tr><td align = right>static struct &lt;N&gt;table_iterator</td><td><a href = "#user-content-fn-f67540e4">&lt;N&gt;table_iterator</a></td><td>table</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-f5d778c3">&lt;N&gt;table_next</a></td><td>it</td></tr>

<tr><td align = right>static &lt;PN&gt;key</td><td><a href = "#user-content-fn-2c046ff5">&lt;N&gt;table_key</a></td><td>it</td></tr>

<tr><td align = right>static &lt;PN&gt;value *</td><td><a href = "#user-content-fn-ab82ba6f">&lt;N&gt;table_value</a></td><td>it</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-c384e71">&lt;N&gt;table_iterator_remove</a></td><td>it</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-4afceb58">&lt;N&gt;table_buffer</a></td><td>table, n</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-abc0643f">&lt;N&gt;table_clear</a></td><td>table</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-8596385b">&lt;N&gt;table_contains</a></td><td>table, key</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-72aa7b72">&lt;N&gt;table_query</a></td><td>table, key, result, value</td></tr>

<tr><td align = right>static &lt;PN&gt;value</td><td><a href = "#user-content-fn-638dcc26">&lt;N&gt;table_get_or</a></td><td>table, key, default_value</td></tr>

<tr><td align = right>static enum table_result</td><td><a href = "#user-content-fn-1680bdf7">&lt;N&gt;table_try</a></td><td>table, key</td></tr>

<tr><td align = right>static enum table_result</td><td><a href = "#user-content-fn-7c006237">&lt;N&gt;table_assign</a></td><td>table, key, content</td></tr>

<tr><td align = right>static enum table_result</td><td><a href = "#user-content-fn-cea327b7">&lt;N&gt;table_update</a></td><td>table, key, eject</td></tr>

<tr><td align = right>static enum table_result</td><td><a href = "#user-content-fn-33008770">&lt;N&gt;table_policy</a></td><td>table, key, eject, policy</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-f3d5d82a">&lt;N&gt;table_remove</a></td><td>table, key</td></tr>

<tr><td align = right>static &lt;PITR&gt;element *</td><td><a href = "#user-content-fn-73c52918">&lt;ITR&gt;any</a></td><td>box, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-96abfbdb">&lt;ITR&gt;each</a></td><td>box, action</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-6e4cf157">&lt;ITR&gt;if_each</a></td><td>box, predicate, action</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-4b2c205b">&lt;ITR&gt;copy_if</a></td><td>dst, src, copy</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d816173b">&lt;ITR&gt;keep_if</a></td><td>box, keep, destruct</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-108e9df6">&lt;ITR&gt;trim</a></td><td>box, predicate</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-751c6337">&lt;STR&gt;to_string</a></td><td>box</td></tr>

<tr><td align = right>static &lt;PN&gt;value</td><td><a href = "#user-content-fn-92774ccb">&lt;N&gt;table&lt;D&gt;get</a></td><td>table, key</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-8f317be5" name = "user-content-fn-8f317be5">&lt;N&gt;table</a> ###

<code>static struct &lt;N&gt;table <strong>&lt;N&gt;table</strong>(void)</code>

Zeroed data \(not all\-bits\-zero\) is initialized\.

 * Return:  
   An idle array\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-24e5c5ce" name = "user-content-fn-24e5c5ce">&lt;N&gt;table_</a> ###

<code>static void <strong>&lt;N&gt;table_</strong>(struct &lt;N&gt;table *const <em>table</em>)</code>

If `table` is not null, destroys and returns it to idle\.



### <a id = "user-content-fn-f67540e4" name = "user-content-fn-f67540e4">&lt;N&gt;table_iterator</a> ###

<code>static struct &lt;N&gt;table_iterator <strong>&lt;N&gt;table_iterator</strong>(struct &lt;N&gt;table *const <em>table</em>)</code>

Loads a non\-null `table` into `it`\.



### <a id = "user-content-fn-f5d778c3" name = "user-content-fn-f5d778c3">&lt;N&gt;table_next</a> ###

<code>static int <strong>&lt;N&gt;table_next</strong>(struct &lt;N&gt;table_iterator *const <em>it</em>)</code>

Advances `it`\.

 * Return:  
   Whether `it` has an element now\.




### <a id = "user-content-fn-2c046ff5" name = "user-content-fn-2c046ff5">&lt;N&gt;table_key</a> ###

<code>static &lt;PN&gt;key <strong>&lt;N&gt;table_key</strong>(const struct &lt;N&gt;table_iterator *const <em>it</em>)</code>

 * Return:  
   If `it` has an element, returns it's key\.




### <a id = "user-content-fn-ab82ba6f" name = "user-content-fn-ab82ba6f">&lt;N&gt;table_value</a> ###

<code>static &lt;PN&gt;value *<strong>&lt;N&gt;table_value</strong>(const struct &lt;N&gt;table_iterator *const <em>it</em>)</code>

 * Return:  
   If `it` has an element, returns it's value, if `TABLE_VALUE`\.




### <a id = "user-content-fn-c384e71" name = "user-content-fn-c384e71">&lt;N&gt;table_iterator_remove</a> ###

<code>static int <strong>&lt;N&gt;table_iterator_remove</strong>(struct &lt;N&gt;table_iterator *const <em>it</em>)</code>

Removes the entry at `it`\. Whereas [&lt;N&gt;table_remove](#user-content-fn-f3d5d82a) invalidates the iterator, this corrects `it` so [&lt;N&gt;table_next](#user-content-fn-f5d778c3) is the next entry\. To use the iterator after this, one must move to the next\.

 * Return:  
   Success, or there was no entry at the iterator's position, \(anymore\.\)




### <a id = "user-content-fn-4afceb58" name = "user-content-fn-4afceb58">&lt;N&gt;table_buffer</a> ###

<code>static int <strong>&lt;N&gt;table_buffer</strong>(struct &lt;N&gt;table *const <em>table</em>, const &lt;PN&gt;uint <em>n</em>)</code>

Reserve at least `n` more empty buckets in `table`\. This may cause the capacity to increase and invalidates any pointers to data in the table\.

 * Return:  
   Success\.
 * Exceptional return: ERANGE  
   The request was unsatisfiable\.
 * Exceptional return: realloc  




### <a id = "user-content-fn-abc0643f" name = "user-content-fn-abc0643f">&lt;N&gt;table_clear</a> ###

<code>static void <strong>&lt;N&gt;table_clear</strong>(struct &lt;N&gt;table *const <em>table</em>)</code>

Clears and removes all buckets from `table`\. The capacity and memory of the `table` is preserved, but all previous values are un\-associated\. \(The load factor will be less until it reaches it's previous size\.\)

 * Order:  
   &#920;\(`table.capacity`\)




### <a id = "user-content-fn-8596385b" name = "user-content-fn-8596385b">&lt;N&gt;table_contains</a> ###

<code>static int <strong>&lt;N&gt;table_contains</strong>(struct &lt;N&gt;table *const <em>table</em>, const &lt;PN&gt;key <em>key</em>)</code>

 * Return:  
   Whether `key` is in `table` \(which can be null\.\)




### <a id = "user-content-fn-72aa7b72" name = "user-content-fn-72aa7b72">&lt;N&gt;table_query</a> ###

<code>static int <strong>&lt;N&gt;table_query</strong>(struct &lt;N&gt;table *const <em>table</em>, const &lt;PN&gt;key <em>key</em>, &lt;PN&gt;key *<em>result</em>, &lt;PN&gt;value *<em>value</em>)</code>

If there can be no default key, use this so separate a null, returns false, from a result\. Otherwise, a more convenient function is [&lt;N&gt;table_get_or](#user-content-fn-638dcc26)\.

 * Parameter: _result_  
   If null, behaves like [&lt;N&gt;table_contains](#user-content-fn-8596385b), otherwise, a [&lt;PN&gt;key](#user-content-typedef-e7af8dc0) which gets filled on true\.
 * Parameter: _value_  
   Only on a map with `TABLE_VALUE`\. If not\-null, stores the value\.
 * Return:  
   Whether `key` is in `table` \(which can be null\.\)




### <a id = "user-content-fn-638dcc26" name = "user-content-fn-638dcc26">&lt;N&gt;table_get_or</a> ###

<code>static &lt;PN&gt;value <strong>&lt;N&gt;table_get_or</strong>(struct &lt;N&gt;table *const <em>table</em>, const &lt;PN&gt;key <em>key</em>, &lt;PN&gt;value <em>default_value</em>)</code>

 * Return:  
   The value associated with `key` in `table`, \(which can be null\.\) If no such value exists, `default_value` is returned\.
 * Order:  
   Average &#927;\(1\); worst &#927;\(n\)\.




### <a id = "user-content-fn-1680bdf7" name = "user-content-fn-1680bdf7">&lt;N&gt;table_try</a> ###

<code>static enum table_result <strong>&lt;N&gt;table_try</strong>(struct &lt;N&gt;table *const <em>table</em>, &lt;PN&gt;key <em>key</em>)</code>

Only if `TABLE_VALUE` is not set; see [&lt;N&gt;table_assign](#user-content-fn-7c006237) for a map\. Puts `key` in set `table` only if absent\.

 * Return:  
   One of: `TABLE_ERROR`, tried putting the entry in the table but failed, the table is not modified; `TABLE_PRESENT`, does nothing if there is another entry with the same key; `TABLE_ABSENT`, put an entry in the table\.
 * Exceptional return: realloc, ERANGE  
   On `TABLE_ERROR`\.
 * Order:  
   Average amortised &#927;\(1\); worst &#927;\(n\)\.




### <a id = "user-content-fn-7c006237" name = "user-content-fn-7c006237">&lt;N&gt;table_assign</a> ###

<code>static enum table_result <strong>&lt;N&gt;table_assign</strong>(struct &lt;N&gt;table *const <em>table</em>, &lt;PN&gt;key <em>key</em>, &lt;PN&gt;value **const <em>content</em>)</code>

Only if `TABLE_VALUE` is set; see [&lt;N&gt;table_try](#user-content-fn-1680bdf7) for a set\. Puts `key` in the map `table` and store the associated value in `content`\.

 * Return:  
   `TABLE_ERROR` does not set `content`; `TABLE_ABSENT`, the `content` will be a pointer to uninitialized memory; `TABLE_PRESENT`, gets the current `content`, \(does not alter the keys, if they are distinguishable\.\)
 * Exceptional return: malloc, ERANGE  
   On `TABLE_ERROR`\.




### <a id = "user-content-fn-cea327b7" name = "user-content-fn-cea327b7">&lt;N&gt;table_update</a> ###

<code>static enum table_result <strong>&lt;N&gt;table_update</strong>(struct &lt;N&gt;table *const <em>table</em>, &lt;PN&gt;key <em>key</em>, &lt;PN&gt;key *<em>eject</em>)</code>

Puts `key` in `table`, replacing an equal\-valued key\. \(If keys are indistinguishable, this function is not very useful, see [&lt;N&gt;table_try](#user-content-fn-1680bdf7) or [&lt;N&gt;table_assign](#user-content-fn-7c006237)\.\)

 * Return:  
   One of: `TABLE_ERROR`, the table is not modified; `TABLE_ABSENT`, the `key` is new; `TABLE_PRESENT`, the `key` displaces another, and if non\-null, `eject` will be filled with the previous entry\.
 * Exceptional return: realloc, ERANGE  
   On `TABLE_ERROR`\.
 * Order:  
   Average amortised &#927;\(1\); worst &#927;\(n\)\.




### <a id = "user-content-fn-33008770" name = "user-content-fn-33008770">&lt;N&gt;table_policy</a> ###

<code>static enum table_result <strong>&lt;N&gt;table_policy</strong>(struct &lt;N&gt;table *const <em>table</em>, &lt;PN&gt;key <em>key</em>, &lt;PN&gt;key *<em>eject</em>, const &lt;PN&gt;policy_fn <em>policy</em>)</code>

Puts `key` in `table` only if absent or if calling `policy` returns true\.

 * Return:  
   One of: `TABLE_ERROR`, the table is not modified; `TABLE_ABSENT`, the `key` is new; `TABLE_PRESENT`, `key` collides, if `policy` returns true, `eject`, if non\-null, will be filled;
 * Exceptional return: realloc, ERANGE  
   On `TABLE_ERROR`\.
 * Order:  
   Average amortised &#927;\(1\); worst &#927;\(n\)\.




### <a id = "user-content-fn-f3d5d82a" name = "user-content-fn-f3d5d82a">&lt;N&gt;table_remove</a> ###

<code>static int <strong>&lt;N&gt;table_remove</strong>(struct &lt;N&gt;table *const <em>table</em>, const &lt;PN&gt;key <em>key</em>)</code>

Removes `key` from `table` \(which could be null\.\)

 * Return:  
   Whether that `key` was in `table`\.
 * Order:  
   Average &#927;\(1\), \(hash distributes elements uniformly\); worst &#927;\(n\)\.




### <a id = "user-content-fn-73c52918" name = "user-content-fn-73c52918">&lt;ITR&gt;any</a> ###

<code>static &lt;PITR&gt;element *<strong>&lt;ITR&gt;any</strong>(const &lt;PITR&gt;box *const <em>box</em>, const &lt;PITR&gt;predicate_fn <em>predicate</em>)</code>

[src/iterate\.h](src/iterate.h): Iterates through `box` and calls `predicate` until it returns true\.

 * Return:  
   The first `predicate` that returned true, or, if the statement is false on all, null\.
 * Order:  
   &#927;\(`box.size`\) &#215; &#927;\(`predicate`\)




### <a id = "user-content-fn-96abfbdb" name = "user-content-fn-96abfbdb">&lt;ITR&gt;each</a> ###

<code>static void <strong>&lt;ITR&gt;each</strong>(&lt;PITR&gt;box *const <em>box</em>, const &lt;PITR&gt;action_fn <em>action</em>)</code>

[src/iterate\.h](src/iterate.h): Iterates through `box` and calls `action` on all the elements\. Differs calling `action` until the iterator is one\-ahead, so can delete elements as long as it doesn't affect the next, \(specifically, a linked\-list\.\)

 * Order:  
   &#927;\(|`box`|\) &#215; &#927;\(`action`\)




### <a id = "user-content-fn-6e4cf157" name = "user-content-fn-6e4cf157">&lt;ITR&gt;if_each</a> ###

<code>static void <strong>&lt;ITR&gt;if_each</strong>(&lt;PITR&gt;box *const <em>box</em>, const &lt;PITR&gt;predicate_fn <em>predicate</em>, const &lt;PITR&gt;action_fn <em>action</em>)</code>

[src/iterate\.h](src/iterate.h): Iterates through `box` and calls `action` on all the elements for which `predicate` returns true\.

 * Order:  
   &#927;\(`box.size`\) &#215; \(&#927;\(`predicate`\) \+ &#927;\(`action`\)\)




### <a id = "user-content-fn-4b2c205b" name = "user-content-fn-4b2c205b">&lt;ITR&gt;copy_if</a> ###

<code>static int <strong>&lt;ITR&gt;copy_if</strong>(&lt;PITR&gt;box *restrict const <em>dst</em>, const &lt;PITR&gt;box *restrict const <em>src</em>, const &lt;PITR&gt;predicate_fn <em>copy</em>)</code>

[src/iterate\.h](src/iterate.h), `BOX_CONTIGUOUS`: For all elements of `src`, calls `copy`, and if true, lazily copies the elements to `dst`\. `dst` and `src` can not be the same but `src` can be null, \(in which case, it does nothing\.\)

 * Exceptional return: realloc  
 * Order:  
   &#927;\(|`src`|\) &#215; &#927;\(`copy`\)




### <a id = "user-content-fn-d816173b" name = "user-content-fn-d816173b">&lt;ITR&gt;keep_if</a> ###

<code>static void <strong>&lt;ITR&gt;keep_if</strong>(&lt;PITR&gt;box *const <em>box</em>, const &lt;PITR&gt;predicate_fn <em>keep</em>, const &lt;PITR&gt;action_fn <em>destruct</em>)</code>

[src/iterate\.h](src/iterate.h): For all elements of `box`, calls `keep`, and if false, if contiguous, lazy deletes that item, if not, eagerly\. Calls `destruct` if not\-null before deleting\.

 * Order:  
   &#927;\(|`box`|\) \(&#215; O\(`keep`\) \+ O\(`destruct`\)\)




### <a id = "user-content-fn-108e9df6" name = "user-content-fn-108e9df6">&lt;ITR&gt;trim</a> ###

<code>static void <strong>&lt;ITR&gt;trim</strong>(&lt;PITR&gt;box *const <em>box</em>, const &lt;PITR&gt;predicate_fn <em>predicate</em>)</code>

[src/iterate\.h](src/iterate.h), `BOX_CONTIGUOUS`: Removes at either end of `box` the things that `predicate`, if it exists, returns true\.

 * Order:  
   &#927;\(`box.size`\) &#215; &#927;\(`predicate`\)




### <a id = "user-content-fn-751c6337" name = "user-content-fn-751c6337">&lt;STR&gt;to_string</a> ###

<code>static const char *<strong>&lt;STR&gt;to_string</strong>(const &lt;PSTR&gt;box *const <em>box</em>)</code>

[src/to\_string\.h](src/to_string.h): print the contents of `box` in a static string buffer of 256 bytes, with limitations of only printing 4 things in a single sequence point\.

 * Return:  
   Address of the static buffer\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-92774ccb" name = "user-content-fn-92774ccb">&lt;N&gt;table&lt;D&gt;get</a> ###

<code>static &lt;PN&gt;value <strong>&lt;N&gt;table&lt;D&gt;get</strong>(struct &lt;N&gt;table *const <em>table</em>, const &lt;PN&gt;key <em>key</em>)</code>

This is functionally identical to [&lt;N&gt;table_get_or](#user-content-fn-638dcc26), but a with a trait specifying a constant default value\.

 * Return:  
   The value associated with `key` in `table`, \(which can be null\.\) If no such value exists, the `TABLE_DEFAULT` is returned\.
 * Order:  
   Average &#927;\(1\); worst &#927;\(n\)\.






## <a id = "user-content-license" name = "user-content-license">License</a> ##

2019 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



