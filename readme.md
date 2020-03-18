# Trie\.h #

## String Trie ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PN&gt;Type](#user-content-typedef-c45e6761), [&lt;PN&gt;Key](#user-content-typedef-8524f620), [&lt;PN&gt;Replace](#user-content-typedef-38741b27), [&lt;PN&gt;Action](#user-content-typedef-aea37eeb)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;N&gt;Trie](#user-content-tag-8fc8a233)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of trie.](../web/trie.png)

An [&lt;N&gt;Trie](#user-content-tag-8fc8a233) is a trie of byte\-strings ended with `NUL`, compatible with any byte\-encoding with a null\-terminator; in particular, `C` strings, including [modified UTF-8](https://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8)\.

It has the same asymptotic run\-time as keeping a sorted array of pointers, but lookup is faster because it keeps an index; likewise insertion is slower, \(asymptotically, it still has to lookup to insert,\) because it has to update that index\. Experimentally, insertion performs linearly worse, and lookup performs logarithmically worse, then a hash `Set` starting at about 100 items\. However, advantages of this data structure are,

 * because it is deterministic, a stable pointer suffices instead of a node \-\- one can insert the same data into multiple tries;
 * for the same reason, it has no need of a hash function;
 * the keys are packed and in order;
 * moreover, it is easy to search for like keys\.

`Array.h` must be present\. `<N>Trie` is not synchronised\. Errors are returned with `errno`\. The parameters are `#define` preprocessor macros, and are all undefined at the end of the file for convenience\. `assert.h` is used; to stop assertions, use `#define NDEBUG` before inclusion\.



 * Parameter: TRIE\_NAME, TRIE\_TYPE  
   `<N>` that satisfies `C` naming conventions when mangled and an optional returnable type that is declared, \(it is used by reference only\.\) `<PN>` is private, whose names are prefixed in a manner to avoid collisions; any should be re\-defined prior to use elsewhere\.
 * Parameter: TRIE\_KEY  
   A function that satisfies [&lt;PN&gt;Key](#user-content-typedef-8524f620)\. Must be defined if and only if `TRIE_TYPE` is defined\.
 * Parameter: TRIE\_TEST  
   Unit testing framework [&lt;N&gt;TrieTest](#user-content-fn-ae32c087), included in a separate header, [\.\./test/TreeTest\.h](../test/TreeTest.h)\. Must be defined equal to a \(random\) filler function, satisfying [&lt;PN&gt;Action](#user-content-typedef-aea37eeb)\. Requires `TRIE_TO_STRING` and not `NDEBUG`\.
 * Standard:  
   C89
 * Dependancies:  
   [Array.h](../../Array/)
 * Caveat:  
   Have a replace; much faster then remove and add\.
 * See also:  
   [Array](https://github.com/neil-edelman/Array); [List](https://github.com/neil-edelman/List); [Orcish](https://github.com/neil-edelman/Orcish); [Pool](https://github.com/neil-edelman/Pool); [Set](https://github.com/neil-edelman/Set)


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-c45e6761" name = "user-content-typedef-c45e6761">&lt;PN&gt;Type</a> ###

<code>typedef TRIE_TYPE <strong>&lt;PN&gt;Type</strong>;</code>

A valid tag type set by `TRIE_TYPE`; defaults to `const char`\.



### <a id = "user-content-typedef-8524f620" name = "user-content-typedef-8524f620">&lt;PN&gt;Key</a> ###

<code>typedef const char *(*<strong>&lt;PN&gt;Key</strong>)(&lt;PN&gt;Type *);</code>

Responsible for picking out the null\-terminated string\. One must not modify this string while in any trie\.



### <a id = "user-content-typedef-38741b27" name = "user-content-typedef-38741b27">&lt;PN&gt;Replace</a> ###

<code>typedef int(*<strong>&lt;PN&gt;Replace</strong>)(&lt;PN&gt;Type *original, &lt;PN&gt;Type *replace);</code>

A bi\-predicate; returns true if the `replace` replaces the `original`; used in [&lt;N&gt;TriePolicyPut](#user-content-fn-592f827e)\.



### <a id = "user-content-typedef-aea37eeb" name = "user-content-typedef-aea37eeb">&lt;PN&gt;Action</a> ###

<code>typedef void(*<strong>&lt;PN&gt;Action</strong>)(&lt;PN&gt;Type *);</code>

Only used if `TRIE_TEST`\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-8fc8a233" name = "user-content-tag-8fc8a233">&lt;N&gt;Trie</a> ###

<code>struct <strong>&lt;N&gt;Trie</strong>;</code>

To initialise it to an idle state, see [&lt;N&gt;Trie](#user-content-fn-8fc8a233), `TRIE_IDLE`, `{0}` \(`C99`\), or being `static`\.

A full binary tree stored semi\-implicitly in two arrays: one as the branches backed by one as pointers\-to\-[&lt;PN&gt;Type](#user-content-typedef-c45e6761) as leaves\. We take two arrays because it speeds up iteration as the leaves are also an array sorted by key, it is &#927;\(1\) instead of &#927;\(log `items`\) to get an example for comparison in insert, and experimetally it is slightly faster\.

It can be seen as a [binary radix trie](https://en.wikipedia.org/wiki/Radix_tree) or [Morrison, 1968 PATRICiA](https://scholar.google.ca/scholar?q=Morrison%2C+1968+PATRICiA)\. The branches do not store data on the strings, only the positions where the strings are different\.

![States.](../web/states.png)



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-c4d7b004">&lt;N&gt;Trie_</a></td><td>trie</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-8fc8a233">&lt;N&gt;Trie</a></td><td>trie</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-6ddbe8d2">&lt;N&gt;TrieSize</a></td><td>trie</td></tr>

<tr><td align = right>static &lt;PN&gt;Type *const *</td><td><a href = "#user-content-fn-c8de2f88">&lt;N&gt;TrieArray</a></td><td>trie</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-b2f36d9c">&lt;N&gt;TrieClear</a></td><td>trie</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-fad143b6">&lt;N&gt;TrieAdd</a></td><td>trie, data</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-85d52810">&lt;N&gt;TriePut</a></td><td>trie, data, eject</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-592f827e">&lt;N&gt;TriePolicyPut</a></td><td>trie, data, eject, replace</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-f6f3fdef">&lt;N&gt;TrieToString</a></td><td>trie</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-ae32c087">&lt;N&gt;TrieTest</a></td><td></td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-c4d7b004" name = "user-content-fn-c4d7b004">&lt;N&gt;Trie_</a> ###

<code>static void <strong>&lt;N&gt;Trie_</strong>(struct &lt;N&gt;Trie *const <em>trie</em>)</code>

Returns `trie` to the idle state where it takes no dynamic memory\.

 * Parameter: _trie_  
   If null, does nothing\.




### <a id = "user-content-fn-8fc8a233" name = "user-content-fn-8fc8a233">&lt;N&gt;Trie</a> ###

<code>static void <strong>&lt;N&gt;Trie</strong>(struct &lt;N&gt;Trie *const <em>trie</em>)</code>

Initialises `trie` to be idle\.

 * Parameter: _trie_  
   If null, does nothing\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-6ddbe8d2" name = "user-content-fn-6ddbe8d2">&lt;N&gt;TrieSize</a> ###

<code>static size_t <strong>&lt;N&gt;TrieSize</strong>(const struct &lt;N&gt;Trie *const <em>trie</em>)</code>

 * Parameter: _trie_  
   If null, returns zero;
 * Return:  
   The number of elements in the `trie`\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-c8de2f88" name = "user-content-fn-c8de2f88">&lt;N&gt;TrieArray</a> ###

<code>static &lt;PN&gt;Type *const *<strong>&lt;N&gt;TrieArray</strong>(const struct &lt;N&gt;Trie *const <em>trie</em>)</code>

It remains valid up to a structural modification of `trie`\.

 * Parameter: _trie_  
   If null, returns null\.
 * Return:  
   The leaves of `trie`, ordered by key\.




### <a id = "user-content-fn-b2f36d9c" name = "user-content-fn-b2f36d9c">&lt;N&gt;TrieClear</a> ###

<code>static void <strong>&lt;N&gt;TrieClear</strong>(struct &lt;N&gt;Trie *const <em>trie</em>)</code>

Sets `trie` to be empty\. That is, the size of `trie` will be zero, but if it was previously in an active non\-idle state, it continues to be\.

 * Parameter: _trie_  
   If null, does nothing\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-fad143b6" name = "user-content-fn-fad143b6">&lt;N&gt;TrieAdd</a> ###

<code>static int <strong>&lt;N&gt;TrieAdd</strong>(struct &lt;N&gt;Trie *const <em>trie</em>, &lt;PN&gt;Type *const <em>data</em>)</code>

Adds `data` to `trie` if absent\.

 * Parameter: _trie_  
   If null, returns null\.
 * Parameter: _data_  
   If null, returns null\.
 * Return:  
   Success\. If data with the same key is present, returns false, but does not set `errno`\.
 * Exceptional return: realloc, ERANGE  
   There was an error with a re\-sizing\.
 * Exceptional return: ERANGE  
   The key is greater then 510 characters or the trie has reached it's maximum size\.
 * Order:  
   &#927;\(`size`\)




### <a id = "user-content-fn-85d52810" name = "user-content-fn-85d52810">&lt;N&gt;TriePut</a> ###

<code>static int <strong>&lt;N&gt;TriePut</strong>(struct &lt;N&gt;Trie *const <em>trie</em>, &lt;PN&gt;Type *const <em>data</em>, &lt;PN&gt;Type **const <em>eject</em>)</code>

Updates or adds `data` to `trie`\.

 * Parameter: _trie_  
   If null, returns null\.
 * Parameter: _data_  
   If null, returns null\.
 * Parameter: _eject_  
   If not null, on success it will hold the overwritten value or a pointer\-to\-null if it did not overwrite\.
 * Return:  
   Success\.
 * Exceptional return: realloc, ERANGE  
   There was an error with a re\-sizing\.
 * Exceptional return: ERANGE  
   The key is greater then 510 characters or the trie has reached it's maximum size\.
 * Order:  
   &#920;\(`size`\)




### <a id = "user-content-fn-592f827e" name = "user-content-fn-592f827e">&lt;N&gt;TriePolicyPut</a> ###

<code>static int <strong>&lt;N&gt;TriePolicyPut</strong>(struct &lt;N&gt;Trie *const <em>trie</em>, &lt;PN&gt;Type *const <em>data</em>, &lt;PN&gt;Type **const <em>eject</em>, const &lt;PN&gt;Replace <em>replace</em>)</code>

Adds `data` to `trie` only if the entry is absent or if calling `replace` returns true\.

 * Parameter: _trie_  
   If null, returns null\.
 * Parameter: _data_  
   If null, returns null\.
 * Parameter: _eject_  
   If not null, on success it will hold the overwritten value or a pointer\-to\-null if it did not overwrite a previous value\.
 * Parameter: _replace_  
   Called on collision and only replaces it if the function returns true\. If null, it is semantically equivalent to [&lt;N&gt;TriePut](#user-content-fn-85d52810)\.
 * Return:  
   Success\.
 * Exceptional return: realloc, ERANGE  
   There was an error with a re\-sizing\.
 * Exceptional return: ERANGE  
   The key is greater then 510 characters or the trie has reached it's maximum size\.
 * Order:  
   &#927;\(`size`\)




### <a id = "user-content-fn-f6f3fdef" name = "user-content-fn-f6f3fdef">&lt;N&gt;TrieToString</a> ###

<code>static const char *<strong>&lt;N&gt;TrieToString</strong>(const struct &lt;N&gt;Trie *const <em>trie</em>)</code>

Can print four strings at once before it overwrites\.

 * Return:  
   Prints the keys of `trie` in a static buffer\.
 * Order:  
   &#920;\(1\); it has a 255 character limit; every element takes some of it\.




### <a id = "user-content-fn-ae32c087" name = "user-content-fn-ae32c087">&lt;N&gt;TrieTest</a> ###

<code>static void <strong>&lt;N&gt;TrieTest</strong>(void)</code>

Will be tested on stdout\. Requires `TRIE_TEST`, and not `NDEBUG` while defining `assert`\.





## <a id = "user-content-license" name = "user-content-license">License</a> ##

2020 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



