# trie\.h #

Header [\.\./\.\./src/trie\.h](../../src/trie.h) requires [\.\./\.\./src/bmp\.h](../../src/bmp.h); examples [\.\./\.\./test/test\_trie\.c](../../test/test_trie.c); article [\.\./trie/trie\.pdf](../trie/trie.pdf)\.

## Prefix tree ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;pT&gt;key](#user-content-typedef-95e6d0aa), [&lt;pT&gt;entry](#user-content-typedef-9be2614d), [&lt;pT&gt;remit](#user-content-typedef-26fd9b58), [&lt;pT&gt;string_fn](#user-content-typedef-9cf8629b), [&lt;pT&gt;key_fn](#user-content-typedef-d71854df), [&lt;pT&gt;to_string_fn](#user-content-typedef-4442127b)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [trie_result](#user-content-tag-eb9850a3), [&lt;t&gt;trie](#user-content-tag-21f3c845), [table_stats](#user-content-tag-89e31bf3)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of trie.](../../doc/trie/trie.png)

A [&lt;t&gt;trie](#user-content-tag-21f3c845) is a prefix\-tree, digital\-tree, or trie: an ordered set or map of byte null\-terminated immutable key strings allowing efficient prefix queries\. The implementation is as [Morrison, 1968 PATRICiA](https://scholar.google.ca/scholar?q=Morrison%2C+1968+PATRICiA): a compact [binary radix trie](https://en.wikipedia.org/wiki/Radix_tree) that acts as an index, only storing the where the key bits are different\. The keys are grouped in fixed\-size nodes in a relaxed version of a B\-tree, as [Bayer, McCreight, 1972 Large](https://scholar.google.ca/scholar?q=Bayer%2C+McCreight%2C+1972+Large), where the height is no longer fixed\.

In general, multiple trees are equivalent by rotations\. A trie is single one of the trees that aligns with the data\. Thus, instead of using binary search, one uses the bits of the key directly; there is just one entry that has to be compared\. Where the fine\-grained structure of the B\-tree is implicit, a trie needs a specific shape\. This is given constant size \(16 bytes\) _per_ entry cache\.

![Bit view of the trie.](../../doc/trie/trie-bits.png)

While the worse\-case run\-time of querying or modifying is bounded by &#927;\(|`string`|\), [Tong, Goebel, Lin, 2015, Smoothed](https://scholar.google.ca/scholar?q=Tong%2C+Goebel%2C+Lin%2C+2015%2C+Smoothed) show that, in an iid model, a better fit is &#927;\(log |`trie`|\), which is seen and reported here\. It is not stable\. It does not need `<t>less`\. In practice, most applications will not see a difference between the tree and trie, but may find one more convenient\.



 * Parameter: TRIE\_NAME  
   Required `<t>` that satisfies `C` naming conventions when mangled\.
 * Parameter: TRIE\_KEY  
   Optional [&lt;pT&gt;key](#user-content-typedef-95e6d0aa), the default of which is `const char *`\. Requires implementation of [&lt;pT&gt;string_fn](#user-content-typedef-9cf8629b) `<t>string` to convert [&lt;pT&gt;key](#user-content-typedef-95e6d0aa) to a `const char *`\.
 * Parameter: TRIE\_ENTRY  
   Optional [&lt;pT&gt;entry](#user-content-typedef-9be2614d) that contains the key, the default of which is the entry is the key\. Requires [&lt;pT&gt;key_fn](#user-content-typedef-d71854df) `<t>key`, that picks out [&lt;pT&gt;key](#user-content-typedef-95e6d0aa) from [&lt;pT&gt;entry](#user-content-typedef-9be2614d)\.
 * Parameter: TRIE\_TO\_STRING, TRIE\_KEY\_TO\_STRING  
   To string trait contained in [src/to\_string\.h](src/to_string.h)\. For `TRIE_TO_STRING`, see [&lt;pT&gt;to_string_fn](#user-content-typedef-4442127b); alternately, for `TRIE_KEY_TO_STRING`, the key is suppled to the string directly, under the assumption that it can be truncated at the last code\-point\. \(If one's data is not a subset of utf\-8 and has the highest\-bit set, it may be prematurely and unpredictably truncated if one uses `TRIE_KEY_TO_STRING`\.\)
 * Parameter: TRIE\_EXPECT\_TRAIT, TRIE\_TRAIT  
   Named traits are obtained by including `trie.h` multiple times with `TRIE_EXPECT_TRAIT` and then subsequently including the name in `TRIE_TRAIT`\.
 * Parameter: TRIE\_DECLARE\_ONLY, TRIE\_NON\_STATIC  
   For headers in different compilation units\.
 * Standard:  
   C89 \(Specifically, ISO/IEC 9899/AMD1:1995 because it uses EILSEQ\.\)
 * Dependancies:  
   [box](../../src/box.h), [bmp](../../src/bmp.h)
 * Caveat:  
   ([&lt;T&gt;from_array](#user-content-fn-bd6b720b))


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-95e6d0aa" name = "user-content-typedef-95e6d0aa">&lt;pT&gt;key</a> ###

<code>typedef TRIE_KEY <strong>&lt;pT&gt;key</strong>;</code>

The default is `const char *`\. If one sets `TRIE_KEY` to a different type, then one must also declare `<t>string` as a [&lt;pT&gt;string_fn](#user-content-typedef-9cf8629b)\.



### <a id = "user-content-typedef-9be2614d" name = "user-content-typedef-9be2614d">&lt;pT&gt;entry</a> ###

<code>typedef TRIE_ENTRY <strong>&lt;pT&gt;entry</strong>;</code>

If `TRIE_ENTRY` is set, one must provide `<t>key` as a [&lt;pT&gt;key_fn](#user-content-typedef-d71854df); otherwise a set and [&lt;pT&gt;entry](#user-content-typedef-9be2614d) and [&lt;pT&gt;key](#user-content-typedef-95e6d0aa) are the same\.



### <a id = "user-content-typedef-26fd9b58" name = "user-content-typedef-26fd9b58">&lt;pT&gt;remit</a> ###

<code>typedef &lt;pT&gt;entry *<strong>&lt;pT&gt;remit</strong>;</code>

Remit is either an extra indirection on [&lt;pT&gt;entry](#user-content-typedef-9be2614d) on `TRIE_ENTRY` or not in the case of a string—it already has a star, and we can't modify it anyway, so it would be awkward to return a pointer to a string\.



### <a id = "user-content-typedef-9cf8629b" name = "user-content-typedef-9cf8629b">&lt;pT&gt;string_fn</a> ###

<code>typedef const char *(*<strong>&lt;pT&gt;string_fn</strong>)(&lt;pT&gt;key);</code>

Transforms a [&lt;pT&gt;key](#user-content-typedef-95e6d0aa) into a `const char *`\.



### <a id = "user-content-typedef-d71854df" name = "user-content-typedef-d71854df">&lt;pT&gt;key_fn</a> ###

<code>typedef &lt;pT&gt;key(*<strong>&lt;pT&gt;key_fn</strong>)(const &lt;pT&gt;entry *);</code>

Extracts [&lt;pT&gt;key](#user-content-typedef-95e6d0aa) from [&lt;pT&gt;entry](#user-content-typedef-9be2614d)\.



### <a id = "user-content-typedef-4442127b" name = "user-content-typedef-4442127b">&lt;pT&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;pT&gt;to_string_fn</strong>)(const &lt;pT&gt;entry *, char(*)[12]);</code>

The type of the required `<tr>to_string`\. Responsible for turning the read\-only argument into a 12\-max\-`char` output string\. `<pT>value` is omitted when it's a set\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-eb9850a3" name = "user-content-tag-eb9850a3">trie_result</a> ###

<code>enum <strong>trie_result</strong> { TRIE_RESULT };</code>

A result of modifying the table, of which `TRIE_ERROR` is false\.

![A diagram of the result states.](../../doc/trie/result.png)



### <a id = "user-content-tag-21f3c845" name = "user-content-tag-21f3c845">&lt;t&gt;trie</a> ###

<code>struct <strong>&lt;t&gt;trie</strong>;</code>

To initialize it to an idle state, see [&lt;t&gt;trie](#user-content-fn-21f3c845), `{0}`, or being `static`\.

![States.](../../doc/trie/states.png)



### <a id = "user-content-tag-89e31bf3" name = "user-content-tag-89e31bf3">table_stats</a> ###

<code>struct <strong>table_stats</strong> { size_t n, max; double mean, ssdm; };</code>

[Welford1962Note](https://scholar.google.ca/scholar?q=Welford1962Note): population variance: `ssdm/n`, sample variance: `ssdm/(n-1)`\.



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>struct &lt;T&gt;cursor</td><td><a href = "#user-content-fn-80df50b2">&lt;T&gt;begin</a></td><td>&lt;t&gt;trie</td></tr>

<tr><td align = right>int</td><td><a href = "#user-content-fn-dd6c86e1">&lt;T&gt;exists</a></td><td>&lt;T&gt;cursor</td></tr>

<tr><td align = right>&lt;pT&gt;remit</td><td><a href = "#user-content-fn-1d176e37">&lt;T&gt;entry</a></td><td>&lt;T&gt;cursor</td></tr>

<tr><td align = right>void</td><td><a href = "#user-content-fn-d0790d04">&lt;T&gt;next</a></td><td>&lt;T&gt;cursor</td></tr>

<tr><td align = right>struct &lt;T&gt;cursor</td><td><a href = "#user-content-fn-331bec0d">&lt;T&gt;prefix</a></td><td>&lt;t&gt;trie, char</td></tr>

<tr><td align = right>struct &lt;t&gt;trie</td><td><a href = "#user-content-fn-21f3c845">&lt;t&gt;trie</a></td><td></td></tr>

<tr><td align = right>void</td><td><a href = "#user-content-fn-8cc400ee">&lt;t&gt;trie_</a></td><td>&lt;t&gt;trie</td></tr>

<tr><td align = right>void</td><td><a href = "#user-content-fn-7f4a964e">&lt;T&gt;clear</a></td><td>&lt;t&gt;trie</td></tr>

<tr><td align = right>&lt;pT&gt;remit</td><td><a href = "#user-content-fn-8c6438a2">&lt;T&gt;match</a></td><td>&lt;t&gt;trie, char</td></tr>

<tr><td align = right>&lt;pT&gt;remit</td><td><a href = "#user-content-fn-2b98edfb">&lt;T&gt;get</a></td><td>trie, char</td></tr>

<tr><td align = right>enum trie_result</td><td><a href = "#user-content-fn-8c6438a2">&lt;T&gt;match</a></td><td>&lt;t&gt;trie, char, &lt;pT&gt;remit</td></tr>

<tr><td align = right>enum trie_result</td><td><a href = "#user-content-fn-2b98edfb">&lt;T&gt;get</a></td><td>&lt;t&gt;trie, char, &lt;pT&gt;remit</td></tr>

<tr><td align = right>enum trie_result</td><td><a href = "#user-content-fn-7b8ec2e0">&lt;T&gt;add</a></td><td>&lt;t&gt;trie, &lt;pT&gt;key</td></tr>

<tr><td align = right>enum trie_result</td><td><a href = "#user-content-fn-7b8ec2e0">&lt;T&gt;add</a></td><td>&lt;t&gt;trie, &lt;pT&gt;key, &lt;pT&gt;entry</td></tr>

<tr><td align = right>int</td><td><a href = "#user-content-fn-56806709">&lt;T&gt;remove</a></td><td>&lt;t&gt;trie, char</td></tr>

<tr><td align = right>static &lt;pT&gt;remit</td><td><a href = "#user-content-fn-1d176e37">&lt;T&gt;entry</a></td><td>cur</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d0790d04">&lt;T&gt;next</a></td><td>cur</td></tr>

<tr><td align = right>static struct &lt;T&gt;cursor</td><td><a href = "#user-content-fn-331bec0d">&lt;T&gt;prefix</a></td><td>trie, prefix</td></tr>

<tr><td align = right>static struct &lt;t&gt;trie</td><td><a href = "#user-content-fn-21f3c845">&lt;t&gt;trie</a></td><td></td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-bd6b720b">&lt;T&gt;from_array</a></td><td>trie, array, array_size</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-8cc400ee">&lt;t&gt;trie_</a></td><td>trie</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-7f4a964e">&lt;T&gt;clear</a></td><td>trie</td></tr>

<tr><td align = right>static &lt;pT&gt;remit</td><td><a href = "#user-content-fn-8c6438a2">&lt;T&gt;match</a></td><td>trie, string</td></tr>

<tr><td align = right>static &lt;pT&gt;remit</td><td><a href = "#user-content-fn-2b98edfb">&lt;T&gt;get</a></td><td>trie, string</td></tr>

<tr><td align = right>static enum trie_result</td><td><a href = "#user-content-fn-7b8ec2e0">&lt;T&gt;add</a></td><td>trie, key, put_entry_here</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-56806709">&lt;T&gt;remove</a></td><td>trie, string</td></tr>

<tr><td align = right>const char *</td><td><a href = "#user-content-fn-260f8348">&lt;TR&gt;to_string</a></td><td>box</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-260f8348">&lt;TR&gt;to_string</a></td><td>box</td></tr>

<tr><td align = right>void</td><td><a href = "#user-content-fn-4e047ffb">&lt;T&gt;graph</a></td><td>&lt;pT&gt;box</td></tr>

<tr><td align = right>int</td><td><a href = "#user-content-fn-6c32bc30">&lt;T&gt;graph_fn</a></td><td>&lt;pT&gt;box, char</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-80df50b2" name = "user-content-fn-80df50b2">&lt;T&gt;begin</a> ###

<code>struct &lt;T&gt;cursor <strong>&lt;T&gt;begin</strong>(const struct <em>&lt;t&gt;trie</em> *);</code>



### <a id = "user-content-fn-dd6c86e1" name = "user-content-fn-dd6c86e1">&lt;T&gt;exists</a> ###

<code>int <strong>&lt;T&gt;exists</strong>(const struct <em>&lt;T&gt;cursor</em> *);</code>



### <a id = "user-content-fn-1d176e37" name = "user-content-fn-1d176e37">&lt;T&gt;entry</a> ###

<code>&lt;pT&gt;remit <strong>&lt;T&gt;entry</strong>(const struct <em>&lt;T&gt;cursor</em> *);</code>



### <a id = "user-content-fn-d0790d04" name = "user-content-fn-d0790d04">&lt;T&gt;next</a> ###

<code>void <strong>&lt;T&gt;next</strong>(struct <em>&lt;T&gt;cursor</em> *);</code>



### <a id = "user-content-fn-331bec0d" name = "user-content-fn-331bec0d">&lt;T&gt;prefix</a> ###

<code>struct &lt;T&gt;cursor <strong>&lt;T&gt;prefix</strong>(struct <em>&lt;t&gt;trie</em> *, const <em>char</em> *);</code>



### <a id = "user-content-fn-21f3c845" name = "user-content-fn-21f3c845">&lt;t&gt;trie</a> ###

<code>struct &lt;t&gt;trie <strong>&lt;t&gt;trie</strong>(void);</code>



### <a id = "user-content-fn-8cc400ee" name = "user-content-fn-8cc400ee">&lt;t&gt;trie_</a> ###

<code>void <strong>&lt;t&gt;trie_</strong>(struct <em>&lt;t&gt;trie</em> *);</code>



### <a id = "user-content-fn-7f4a964e" name = "user-content-fn-7f4a964e">&lt;T&gt;clear</a> ###

<code>void <strong>&lt;T&gt;clear</strong>(struct <em>&lt;t&gt;trie</em> *);</code>



### <a id = "user-content-fn-8c6438a2" name = "user-content-fn-8c6438a2">&lt;T&gt;match</a> ###

<code>&lt;pT&gt;remit <strong>&lt;T&gt;match</strong>(const struct <em>&lt;t&gt;trie</em> *, const <em>char</em> *);</code>



### <a id = "user-content-fn-2b98edfb" name = "user-content-fn-2b98edfb">&lt;T&gt;get</a> ###

<code>&lt;pT&gt;remit <strong>&lt;T&gt;get</strong>(const struct &lt;t&gt;trie *const <em>trie</em>, const <em>char</em> *);</code>



### <a id = "user-content-fn-8c6438a2" name = "user-content-fn-8c6438a2">&lt;T&gt;match</a> ###

<code>enum trie_result <strong>&lt;T&gt;match</strong>(const struct <em>&lt;t&gt;trie</em> *, const <em>char</em> *, <em>&lt;pT&gt;remit</em> *);</code>



### <a id = "user-content-fn-2b98edfb" name = "user-content-fn-2b98edfb">&lt;T&gt;get</a> ###

<code>enum trie_result <strong>&lt;T&gt;get</strong>(const struct <em>&lt;t&gt;trie</em> *, const <em>char</em> *, <em>&lt;pT&gt;remit</em> *);</code>



### <a id = "user-content-fn-7b8ec2e0" name = "user-content-fn-7b8ec2e0">&lt;T&gt;add</a> ###

<code>enum trie_result <strong>&lt;T&gt;add</strong>(struct <em>&lt;t&gt;trie</em> *, <em>&lt;pT&gt;key</em>);</code>



### <a id = "user-content-fn-7b8ec2e0" name = "user-content-fn-7b8ec2e0">&lt;T&gt;add</a> ###

<code>enum trie_result <strong>&lt;T&gt;add</strong>(struct <em>&lt;t&gt;trie</em> *, <em>&lt;pT&gt;key</em>, <em>&lt;pT&gt;entry</em> **);</code>



### <a id = "user-content-fn-56806709" name = "user-content-fn-56806709">&lt;T&gt;remove</a> ###

<code>int <strong>&lt;T&gt;remove</strong>(struct <em>&lt;t&gt;trie</em> *, const <em>char</em> *);</code>



### <a id = "user-content-fn-1d176e37" name = "user-content-fn-1d176e37">&lt;T&gt;entry</a> ###

<code>static &lt;pT&gt;remit <strong>&lt;T&gt;entry</strong>(const struct &lt;T&gt;cursor *const <em>cur</em>)</code>

 * Return:  
   The entry at a valid, non\-null `cur`\.




### <a id = "user-content-fn-d0790d04" name = "user-content-fn-d0790d04">&lt;T&gt;next</a> ###

<code>static void <strong>&lt;T&gt;next</strong>(struct &lt;T&gt;cursor *const <em>cur</em>)</code>

Advancing `cur` to the next element\.

 * Order:  
   &#927;\(log |`trie`|\)




### <a id = "user-content-fn-331bec0d" name = "user-content-fn-331bec0d">&lt;T&gt;prefix</a> ###

<code>static struct &lt;T&gt;cursor <strong>&lt;T&gt;prefix</strong>(struct &lt;t&gt;trie *const <em>trie</em>, const char *const <em>prefix</em>)</code>

 * Parameter: _prefix_  
   To fill with the entire `trie`, use the empty string\.
 * Return:  
   A set to strings that start with `prefix` in `trie`\. It is valid until a topological change to `trie`\. Calling [&lt;T&gt;next](#user-content-fn-d0790d04) will iterate them in order\.
 * Order:  
   &#927;\(log |`trie`|\)




### <a id = "user-content-fn-21f3c845" name = "user-content-fn-21f3c845">&lt;t&gt;trie</a> ###

<code>static struct &lt;t&gt;trie <strong>&lt;t&gt;trie</strong>(void)</code>

Zeroed data \(not all\-bits\-zero\) is initialized\.

 * Return:  
   An idle tree\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-bd6b720b" name = "user-content-fn-bd6b720b">&lt;T&gt;from_array</a> ###

<code>static int <strong>&lt;T&gt;from_array</strong>(struct &lt;T&gt;trie *const <em>trie</em>, &lt;pT&gt;type *const *const <em>array</em>, const size_t <em>array_size</em>)</code>

Initializes `trie` from an `array` of pointers\-to\-`<T>` of `array_size`\.

 * Return:  
   Success\.
 * Exceptional return: realloc  
 * Order:  
   &#927;\(`array_size`\)
 * Caveat:  
   Write this function, somehow\.




### <a id = "user-content-fn-8cc400ee" name = "user-content-fn-8cc400ee">&lt;t&gt;trie_</a> ###

<code>static void <strong>&lt;t&gt;trie_</strong>(struct &lt;t&gt;trie *const <em>trie</em>)</code>

Returns any initialized `trie` \(can be null\) to idle\.

 * Order:  
   &#927;\(|`trie`|\)




### <a id = "user-content-fn-7f4a964e" name = "user-content-fn-7f4a964e">&lt;T&gt;clear</a> ###

<code>static void <strong>&lt;T&gt;clear</strong>(struct &lt;t&gt;trie *const <em>trie</em>)</code>

Clears every entry in a valid `trie` \(can be null\), but it continues to be active if it is not idle\.

 * Order:  
   &#927;\(|`trie`|\)




### <a id = "user-content-fn-8c6438a2" name = "user-content-fn-8c6438a2">&lt;T&gt;match</a> ###

<code>static &lt;pT&gt;remit <strong>&lt;T&gt;match</strong>(const struct &lt;t&gt;trie *const <em>trie</em>, const char *const <em>string</em>)</code>

Looks at only the index of `trie` for potential `string` \(can both be null\) matches\. Does not access the string itself, thus will ignore the bits that are not in the index\. If may not have a null, the `remit` is stuck as a pointer on the end and a `trie_result` is returned\.

 * Return:  
   A candidate match for `string` or null\.
 * Order:  
   &#927;\(|`string`|\)




### <a id = "user-content-fn-2b98edfb" name = "user-content-fn-2b98edfb">&lt;T&gt;get</a> ###

<code>static &lt;pT&gt;remit <strong>&lt;T&gt;get</strong>(const struct &lt;t&gt;trie *const <em>trie</em>, const char *const <em>string</em>)</code>

If may not have a null, the `remit` is stuck as a pointer on the end and a `trie_result` is returned\.

 * Return:  
   Exact `string` match for `trie` or null, \(both can be null\.\)
 * Order:  
   &#927;\(log |`trie`|\) iid




### <a id = "user-content-fn-7b8ec2e0" name = "user-content-fn-7b8ec2e0">&lt;T&gt;add</a> ###

<code>static enum trie_result <strong>&lt;T&gt;add</strong>(struct &lt;t&gt;trie *const <em>trie</em>, const &lt;pT&gt;key <em>key</em>, &lt;pT&gt;entry **const <em>put_entry_here</em>)</code>

Adds `key` to `trie` if it doesn't exist already\.

If `TRIE_ENTRY` was specified and the return is `TRIE_ABSENT`, the trie is in an invalid state until filling in the key with an equivalent `key`\. \(Because [&lt;pT&gt;key](#user-content-typedef-95e6d0aa) is not invertible in this case, it is agnostic of the method of setting the key\.\)



 * Return:  
   One of, `TRIE_ERROR`, `errno` is set and `entry` is not; `TRIE_ABSENT`, `key` is added to `trie`; `TRIE_PRESENT`, the value associated with `key`\.
 * Exceptional return: EILSEQ  
   The string has a distinguishing run of bytes with a neighbouring string that is too long\. On most platforms, this is about 32 bytes the same\.
 * Exceptional return: malloc  
 * Order:  
   &#927;\(log |`trie`|\)




### <a id = "user-content-fn-56806709" name = "user-content-fn-56806709">&lt;T&gt;remove</a> ###

<code>static int <strong>&lt;T&gt;remove</strong>(struct &lt;t&gt;trie *const <em>trie</em>, const char *const <em>string</em>)</code>

Tries to remove `string` from `trie`\.

 * Return:  
   Success\. If either parameter is null or the `string` is not in `trie`, returns false without setting `errno`\.
 * Exceptional return: EILSEQ  
   The deletion of `string` would cause an overflow with the rest of the strings\.
 * Order:  
   &#927;\(log |`trie`|\)




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

2020 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



