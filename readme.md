# trie\.h #

Header [src/trie\.h](src/trie.h) requires [src/bmp\.h](src/bmp.h); examples [test/test\_trie\.c](test/test_trie.c); article [doc/trie\.pdf](doc/trie.pdf)\. On a compatible workstation, `make` creates the test suite of the examples\.

## Prefix tree ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PT&gt;key](#user-content-typedef-eeee1b4a), [&lt;PT&gt;key_to_string_fn](#user-content-typedef-b2d72b0f), [&lt;PT&gt;key_fn](#user-content-typedef-1e6e6b3f), [&lt;PSTR&gt;to_string_fn](#user-content-typedef-8a8349ca)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [trie_result](#user-content-tag-eb9850a3), [&lt;T&gt;trie_entry](#user-content-tag-1422bb56), [&lt;T&gt;trie](#user-content-tag-754a10a5), [&lt;T&gt;trie_iterator](#user-content-tag-854250a4)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of trie.](doc/trie.png)

A [&lt;T&gt;trie](#user-content-tag-754a10a5) is a prefix\-tree, digital\-tree, or trie: an ordered set or map of immutable key strings allowing efficient prefix queries\. Any encoding with a byte null\-terminator is supported, including ASCII and [modified UTF-8](https://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8)\. The implementation is as [Morrison, 1968 PATRICiA](https://scholar.google.ca/scholar?q=Morrison%2C+1968+PATRICiA): a compact [binary radix trie](https://en.wikipedia.org/wiki/Radix_tree) that acts as a compact index, only storing the where the key bits are different\. The keys are grouped in fixed\-size nodes in a relaxed version of [Bayer, McCreight, 1972 Large](https://scholar.google.ca/scholar?q=Bayer%2C+McCreight%2C+1972+Large), where the height is dynamic\.

![Bit view of the trie.](doc/trie-bits.png)



 * Parameter: TRIE\_NAME  
   Required `<T>` that satisfies `C` naming conventions when mangled\. `<PT>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: TRIE\_KEY, TRIE\_KEY\_TO\_STRING  
   Normally, the key is compatible with `const char *`\. Optionally, one can set `TRIE_KEY` to a custom type [&lt;PT&gt;key](#user-content-typedef-eeee1b4a) needing `TRIE_KEY_TO_STRING` as an indirection function satisfying [&lt;PT&gt;key_to_string_fn](#user-content-typedef-b2d72b0f)\.
 * Parameter: TRIE\_VALUE, TRIE\_KEY\_IN\_VALUE  
   `TRIE_VALUE` is an optional payload type to go with the key\. Further, `TRIE_KEY_IN_VALUE` is an optional [&lt;PT&gt;key_fn](#user-content-typedef-1e6e6b3f) that picks out the key from the [&lt;PT&gt;value](#user-content-typedef-cc753b30), otherwise it is an associative array from a key to value, [&lt;T&gt;trie_entry](#user-content-tag-1422bb56)\.
 * Parameter: TRIE\_TO\_STRING  
   Defining this includes [src/to\_string\.h](src/to_string.h), with the key strings\.
 * Parameter: TRIE\_DEFAULT\_NAME, TRIE\_DEFAULT  
   Get or default set default\. FIXME: upcoming\.
 * Standard:  
   C89 \(Specifically, ISO/IEC 9899/AMD1:1995 because it uses EILSEQ\.\)
 * Caveat:  
   ([&lt;T&gt;trie_from_array](#user-content-fn-3554106c), [&lt;T&gt;trie_size](#user-content-fn-b7ff4bcf))


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-eeee1b4a" name = "user-content-typedef-eeee1b4a">&lt;PT&gt;key</a> ###

<code>typedef TRIE_KEY <strong>&lt;PT&gt;key</strong>;</code>

The default is assignable `const char *`\. If one sets `TRIE_KEY` to something other then that, then one must also set [&lt;PT&gt;key_to_string_fn](#user-content-typedef-b2d72b0f) by `TRIE_KEY_TO_STRING`\.



### <a id = "user-content-typedef-b2d72b0f" name = "user-content-typedef-b2d72b0f">&lt;PT&gt;key_to_string_fn</a> ###

<code>typedef const char *(*<strong>&lt;PT&gt;key_to_string_fn</strong>)(&lt;PT&gt;key);</code>

Transforms a [&lt;PT&gt;key](#user-content-typedef-eeee1b4a) into a `const char *` for `TRIE_KEY_TO_STRING`\.



### <a id = "user-content-typedef-1e6e6b3f" name = "user-content-typedef-1e6e6b3f">&lt;PT&gt;key_fn</a> ###

<code>typedef &lt;PT&gt;key(*<strong>&lt;PT&gt;key_fn</strong>)(const &lt;PT&gt;value *);</code>

If `TRIE_KEY_IN_VALUE`, extracts the key from `TRIE_VALUE`; in this case, the user makes a contract to set the key on new entries before using the trie again, \(mostly, can still match, but not reliably modify the topology\.\)



### <a id = "user-content-typedef-8a8349ca" name = "user-content-typedef-8a8349ca">&lt;PSTR&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PSTR&gt;to_string_fn</strong>)(&lt;PSTR&gt;element_c, char(*)[12]);</code>

[src/to\_string\.h](src/to_string.h): responsible for turning the argument into a 12\-`char` null\-terminated output string\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-eb9850a3" name = "user-content-tag-eb9850a3">trie_result</a> ###

<code>enum <strong>trie_result</strong> { TRIE_RESULT };</code>

A result of modifying the table, of which `TRIE_ERROR` is false\.

![A diagram of the result states.](doc/result.png)



### <a id = "user-content-tag-1422bb56" name = "user-content-tag-1422bb56">&lt;T&gt;trie_entry</a> ###

<code>struct <strong>&lt;T&gt;trie_entry</strong> { &lt;PT&gt;key key; &lt;PT&gt;value value; };</code>

On `KEY_VALUE` but not `KEY_KEY_IN_VALUE`, defines an entry\.



### <a id = "user-content-tag-754a10a5" name = "user-content-tag-754a10a5">&lt;T&gt;trie</a> ###

<code>struct <strong>&lt;T&gt;trie</strong>;</code>

To initialize it to an idle state, see [&lt;T&gt;trie](#user-content-fn-754a10a5), `{0}`, or being `static`\.

![States.](doc/states.png)



### <a id = "user-content-tag-854250a4" name = "user-content-tag-854250a4">&lt;T&gt;trie_iterator</a> ###

<code>struct <strong>&lt;T&gt;trie_iterator</strong>;</code>

Represents a range of in\-order keys\.



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static struct &lt;T&gt;trie</td><td><a href = "#user-content-fn-754a10a5">&lt;T&gt;trie</a></td><td></td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-3554106c">&lt;T&gt;trie_from_array</a></td><td>trie, array, array_size</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-9d98b98e">&lt;T&gt;trie_</a></td><td>trie</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-1e455cff">&lt;T&gt;trie_clear</a></td><td>trie</td></tr>

<tr><td align = right>static &lt;PT&gt;entry *</td><td><a href = "#user-content-fn-46d99cc7">&lt;T&gt;trie_match</a></td><td>trie, string</td></tr>

<tr><td align = right>static &lt;PT&gt;entry *</td><td><a href = "#user-content-fn-d0ca0cba">&lt;T&gt;trie_get</a></td><td>trie, string</td></tr>

<tr><td align = right>static struct &lt;T&gt;trie_iterator</td><td><a href = "#user-content-fn-b720a682">&lt;T&gt;trie_prefix</a></td><td>trie, prefix</td></tr>

<tr><td align = right>static &lt;PT&gt;entry *</td><td><a href = "#user-content-fn-f36d1483">&lt;T&gt;trie_next</a></td><td>it</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-b7ff4bcf">&lt;T&gt;trie_size</a></td><td>it</td></tr>

<tr><td align = right>static enum trie_result</td><td><a href = "#user-content-fn-6750ab7">&lt;T&gt;trie_try</a></td><td>trie, key, value</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-7b28a4ea">&lt;T&gt;trie_remove</a></td><td>trie, string</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-751c6337">&lt;STR&gt;to_string</a></td><td>box</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-754a10a5" name = "user-content-fn-754a10a5">&lt;T&gt;trie</a> ###

<code>static struct &lt;T&gt;trie <strong>&lt;T&gt;trie</strong>(void)</code>

Zeroed data \(not all\-bits\-zero\) is initialized\.

 * Return:  
   An idle tree\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-3554106c" name = "user-content-fn-3554106c">&lt;T&gt;trie_from_array</a> ###

<code>static int <strong>&lt;T&gt;trie_from_array</strong>(struct &lt;T&gt;trie *const <em>trie</em>, &lt;PT&gt;type *const *const <em>array</em>, const size_t <em>array_size</em>)</code>

Initializes `trie` from an `array` of pointers\-to\-`<T>` of `array_size`\.

 * Return:  
   Success\.
 * Exceptional return: realloc  
 * Order:  
   &#927;\(`array_size`\)
 * Caveat:  
   Write this function, somehow\.




### <a id = "user-content-fn-9d98b98e" name = "user-content-fn-9d98b98e">&lt;T&gt;trie_</a> ###

<code>static void <strong>&lt;T&gt;trie_</strong>(struct &lt;T&gt;trie *const <em>trie</em>)</code>

Returns any initialized `trie` \(can be null\) to idle\.

 * Order:  
   &#927;\(|`trie`|\)




### <a id = "user-content-fn-1e455cff" name = "user-content-fn-1e455cff">&lt;T&gt;trie_clear</a> ###

<code>static void <strong>&lt;T&gt;trie_clear</strong>(struct &lt;T&gt;trie *const <em>trie</em>)</code>

Clears every entry in a valid `trie` \(can be null\), but it continues to be active if it is not idle\.

 * Order:  
   &#927;\(|`trie`|\)




### <a id = "user-content-fn-46d99cc7" name = "user-content-fn-46d99cc7">&lt;T&gt;trie_match</a> ###

<code>static &lt;PT&gt;entry *<strong>&lt;T&gt;trie_match</strong>(const struct &lt;T&gt;trie *const <em>trie</em>, const char *const <em>string</em>)</code>

Looks at only the index of `trie` for potential `string` \(can both be null\) matches\. Does not access the string itself, thus will ignore the bits that are not in the index\.

 * Return:  
   A candidate match for `string` or null\.
 * Order:  
   &#927;\(|`key`|\)




### <a id = "user-content-fn-d0ca0cba" name = "user-content-fn-d0ca0cba">&lt;T&gt;trie_get</a> ###

<code>static &lt;PT&gt;entry *<strong>&lt;T&gt;trie_get</strong>(const struct &lt;T&gt;trie *const <em>trie</em>, const char *const <em>string</em>)</code>

 * Return:  
   Exact `string` match for `trie` or null, \(both can be null\.\)
 * Order:  
   &#927;\(|`key`|\)




### <a id = "user-content-fn-b720a682" name = "user-content-fn-b720a682">&lt;T&gt;trie_prefix</a> ###

<code>static struct &lt;T&gt;trie_iterator <strong>&lt;T&gt;trie_prefix</strong>(struct &lt;T&gt;trie *const <em>trie</em>, const char *const <em>prefix</em>)</code>

 * Parameter: _prefix_  
   To fill with the entire `trie`, use the empty string\.
 * Return:  
   An iterator set to strings that start with `prefix` in `trie`\. It is valid until a topological change to `trie`\. Calling [&lt;T&gt;trie_next](#user-content-fn-f36d1483) will iterate them in order\.
 * Order:  
   &#927;\(|`prefix`|\)




### <a id = "user-content-fn-f36d1483" name = "user-content-fn-f36d1483">&lt;T&gt;trie_next</a> ###

<code>static &lt;PT&gt;entry *<strong>&lt;T&gt;trie_next</strong>(struct &lt;T&gt;trie_iterator *const <em>it</em>)</code>

 * Return:  
   Advances `it` and returns the entry, or, at the end, returns null\.




### <a id = "user-content-fn-b7ff4bcf" name = "user-content-fn-b7ff4bcf">&lt;T&gt;trie_size</a> ###

<code>static size_t <strong>&lt;T&gt;trie_size</strong>(const struct &lt;T&gt;trie_iterator *const <em>it</em>)</code>

Counts the of the items in `it`\.

 * Order:  
   &#927;\(|`it`|\)
 * Caveat:  
   Doesn't work at all\.




### <a id = "user-content-fn-6750ab7" name = "user-content-fn-6750ab7">&lt;T&gt;trie_try</a> ###

<code>static enum trie_result <strong>&lt;T&gt;trie_try</strong>(struct &lt;T&gt;trie *const <em>trie</em>, const &lt;PT&gt;key <em>key</em>, &lt;PT&gt;value **const <em>value</em>)</code>

Adds `key` to `trie` if it doesn't exist already\.

 * Parameter: _value_  
   Only if `TRIE_VALUE` is set will this parameter exist\. Output pointer\. Can be null only if `TRIE_KEY_IN_VALUE` was not defined\.
 * Return:  
   One of, `TRIE_ERROR`, `errno` is set and `value` is not; `TRIE_UNIQUE`, added to `trie`, and uninitialized `value` is associated with `key`; `TRIE_PRESENT`, the value associated with `key`\. If `TRIE_IN_VALUE`, was specified and the return is `TRIE_UNIQUE`, the trie is in an invalid state until filling in the key in value by `key`\.
 * Exceptional return: EILSEQ  
   The string has a distinguishing run of bytes that is too long\. On most platforms, this is about 31 bytes the same\.
 * Exceptional return: malloc  
 * Order:  
   &#927;\(max\(|`trie.keys`|\)\)




### <a id = "user-content-fn-7b28a4ea" name = "user-content-fn-7b28a4ea">&lt;T&gt;trie_remove</a> ###

<code>static int <strong>&lt;T&gt;trie_remove</strong>(struct &lt;T&gt;trie *const <em>trie</em>, const char *const <em>string</em>)</code>

Tries to remove `string` from `trie`\.

 * Return:  
   Success\. If either parameter is null or the `string` is not in `trie`, returns false without setting `errno`\.
 * Exceptional return: EILSEQ  
   The deletion of `string` would cause an overflow with the rest of the strings\.




### <a id = "user-content-fn-751c6337" name = "user-content-fn-751c6337">&lt;STR&gt;to_string</a> ###

<code>static const char *<strong>&lt;STR&gt;to_string</strong>(const &lt;PSTR&gt;box *const <em>box</em>)</code>

[src/to\_string\.h](src/to_string.h): print the contents of `box` in a static string buffer of 256 bytes, with limitations of only printing 4 things at a time\. `<STR>` is loosely contracted to be a name `<X>box[<X_TO_STRING_NAME>]`\.

 * Return:  
   Address of the static buffer\.
 * Order:  
   &#920;\(1\)






## <a id = "user-content-license" name = "user-content-license">License</a> ##

2020 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



