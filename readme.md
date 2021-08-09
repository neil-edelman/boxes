# trie\.h #

## Prefix Tree ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PT&gt;type](#user-content-typedef-245060ab), [&lt;PT&gt;key_fn](#user-content-typedef-1e6e6b3f), [&lt;PZ&gt;to_string_fn](#user-content-typedef-22f3d7f1), [&lt;PT&gt;action_fn](#user-content-typedef-ba462b2e)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;PT&gt;any_tree](#user-content-tag-f3b4d764), [&lt;PT&gt;leaf](#user-content-tag-44821167), [&lt;PT&gt;tree](#user-content-tag-134b950f), [&lt;T&gt;trie](#user-content-tag-754a10a5), [&lt;PT&gt;iterator](#user-content-tag-d9d00f09)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of trie.](web/trie.png)

A [&lt;T&gt;trie](#user-content-tag-754a10a5) is a prefix tree, digital tree, or trie, implemented as an array of pointers\-to\-`T` and an index on the key string\. It can be seen as a [Morrison, 1968 PATRICiA](https://scholar.google.ca/scholar?q=Morrison%2C+1968+PATRICiA): a compact [binary radix trie](https://en.wikipedia.org/wiki/Radix_tree), only storing the where the keys are different\. Strings can be any encoding with a byte null\-terminator, including [modified UTF-8](https://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8)\.

In memory, it is similar to [Bayer, McCreight, 1972 Large](https://scholar.google.ca/scholar?q=Bayer%2C+McCreight%2C+1972+Large)\. Using [Knuth, 1998 Art 3](https://scholar.google.ca/scholar?q=Knuth%2C+1998+Art+3) terminology, but instead of a B\-tree of order\-n nodes, it is a forest of non\-empty complete binary trees\. Thus the leaves in a tree are also the branching factor; the maximum is the order, fixed by compilation macros\.



 * Parameter: TRIE\_NAME, TRIE\_TYPE  
   [&lt;PT&gt;type](#user-content-typedef-245060ab) that satisfies `C` naming conventions when mangled and an optional returnable type that is declared, \(it is used by reference only except if `TRIE_TEST`\.\) `<PT>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: TRIE\_KEY  
   A function that satisfies [&lt;PT&gt;key_fn](#user-content-typedef-1e6e6b3f)\. Must be defined if and only if `TRIE_TYPE` is defined\.
 * Parameter: TRIE\_TO\_STRING  
   Defining this includes [to\_string\.h](to_string.h), with the keys as the string\.
 * Parameter: TRIE\_TEST  
   Unit testing framework [&lt;T&gt;trie_test](#user-content-fn-ae9d3396), included in a separate header, [\.\./test/test\_trie\.h](../test/test_trie.h)\. Must be defined equal to a \(random\) filler function, satisfying [&lt;PT&gt;action_fn](#user-content-typedef-ba462b2e)\. Requires `TRIE_TO_STRING` and that `NDEBUG` not be defined\.
 * Standard:  
   C89


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-245060ab" name = "user-content-typedef-245060ab">&lt;PT&gt;type</a> ###

<code>typedef TRIE_TYPE <strong>&lt;PT&gt;type</strong>;</code>

Declared type of the trie; `char` default\.



### <a id = "user-content-typedef-1e6e6b3f" name = "user-content-typedef-1e6e6b3f">&lt;PT&gt;key_fn</a> ###

<code>typedef const char *(*<strong>&lt;PT&gt;key_fn</strong>)(const &lt;PT&gt;type *);</code>

Responsible for picking out the null\-terminated string\. Modifying the string while in any trie causes the trie to go into an undefined state\.



### <a id = "user-content-typedef-22f3d7f1" name = "user-content-typedef-22f3d7f1">&lt;PZ&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PZ&gt;to_string_fn</strong>)(const &lt;PZ&gt;type *, char(*)[12]);</code>

Responsible for turning the first argument into a 12\-`char` null\-terminated output string\.



### <a id = "user-content-typedef-ba462b2e" name = "user-content-typedef-ba462b2e">&lt;PT&gt;action_fn</a> ###

<code>typedef void(*<strong>&lt;PT&gt;action_fn</strong>)(&lt;PT&gt;type *);</code>

Works by side\-effects, _ie_ fills the type with data\. Only defined if `TRIE_TEST`\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-f3b4d764" name = "user-content-tag-f3b4d764">&lt;PT&gt;any_tree</a> ###

<code>union <strong>&lt;PT&gt;any_tree</strong> { struct trie_info *info; TRIE_TREE_X };</code>

Pointers to generic trees stored in memory, and part of the B\-forest\. Points to a non\-empty semi\-implicit complete binary tree of a fixed\-maximum\-size; reading `key.tree` will tell which tree it is\.



### <a id = "user-content-tag-44821167" name = "user-content-tag-44821167">&lt;PT&gt;leaf</a> ###

<code>union <strong>&lt;PT&gt;leaf</strong> { &lt;PT&gt;type *data; union &lt;PT&gt;any_tree child; };</code>

A leaf is either data or another child tree; the `children` of [&lt;PT&gt;tree](#user-content-tag-134b950f) is a bitmap that tells which\.



### <a id = "user-content-tag-134b950f" name = "user-content-tag-134b950f">&lt;PT&gt;tree</a> ###

<code>struct <strong>&lt;PT&gt;tree</strong> { unsigned bsize, no; struct trie_branch *branches; unsigned char *children; union &lt;PT&gt;leaf *leaves; };</code>

A working tree of any size extracted from different\-width storage by [&lt;PT&gt;extract](#user-content-fn-6c67df90)\.



### <a id = "user-content-tag-754a10a5" name = "user-content-tag-754a10a5">&lt;T&gt;trie</a> ###

<code>struct <strong>&lt;T&gt;trie</strong> { union &lt;PT&gt;any_tree root; };</code>

To initialize it to an idle state, see [&lt;T&gt;trie](#user-content-fn-754a10a5), `TRIE_IDLE`, `{0}` \(`C99`\), or being `static`\.

![States.](web/states.png)



### <a id = "user-content-tag-d9d00f09" name = "user-content-tag-d9d00f09">&lt;PT&gt;iterator</a> ###

<code>struct <strong>&lt;PT&gt;iterator</strong>;</code>

Contains all iteration parameters\.



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-754a10a5">&lt;T&gt;trie</a></td><td>trie</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-9d98b98e">&lt;T&gt;trie_</a></td><td>trie</td></tr>

<tr><td align = right>static &lt;PT&gt;type *</td><td><a href = "#user-content-fn-d0ca0cba">&lt;T&gt;trie_get</a></td><td>trie, key</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-70c096ed">&lt;T&gt;trie_add</a></td><td>trie, x</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-4ecb4112">&lt;Z&gt;to_string</a></td><td>box</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-ae9d3396">&lt;T&gt;trie_test</a></td><td></td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-754a10a5" name = "user-content-fn-754a10a5">&lt;T&gt;trie</a> ###

<code>static void <strong>&lt;T&gt;trie</strong>(struct &lt;T&gt;trie *const <em>trie</em>)</code>

Initialises `trie` to idle\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-9d98b98e" name = "user-content-fn-9d98b98e">&lt;T&gt;trie_</a> ###

<code>static void <strong>&lt;T&gt;trie_</strong>(struct &lt;T&gt;trie *const <em>trie</em>)</code>

Returns an initialised `trie` to idle\.



### <a id = "user-content-fn-d0ca0cba" name = "user-content-fn-d0ca0cba">&lt;T&gt;trie_get</a> ###

<code>static &lt;PT&gt;type *<strong>&lt;T&gt;trie_get</strong>(const struct &lt;T&gt;trie *const <em>trie</em>, const char *const <em>key</em>)</code>

 * Return:  
   The [&lt;PT&gt;type](#user-content-typedef-245060ab) with `key` in `trie` or null no such item exists\.
 * Order:  
   &#927;\(|`key`|\), [Thareja 2011, Data](https://scholar.google.ca/scholar?q=Thareja+2011%2C+Data)\.




### <a id = "user-content-fn-70c096ed" name = "user-content-fn-70c096ed">&lt;T&gt;trie_add</a> ###

<code>static int <strong>&lt;T&gt;trie_add</strong>(struct &lt;T&gt;trie *const <em>trie</em>, &lt;PT&gt;type *const <em>x</em>)</code>

 * Return:  
   If `x` is already in `trie`, returns false, otherwise success\.
 * Exceptional return: realloc, ERANGE  




### <a id = "user-content-fn-4ecb4112" name = "user-content-fn-4ecb4112">&lt;Z&gt;to_string</a> ###

<code>static const char *<strong>&lt;Z&gt;to_string</strong>(const &lt;PZ&gt;box *const <em>box</em>)</code>

 * Return:  
   Print the contents of `box` in a static string buffer of 256 bytes with limitations of only printing 4 things at a time\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-ae9d3396" name = "user-content-fn-ae9d3396">&lt;T&gt;trie_test</a> ###

<code>static void <strong>&lt;T&gt;trie_test</strong>(void)</code>

Will be tested on stdout\. Requires `TRIE_TEST`, and not `NDEBUG` while defining `assert`\.





## <a id = "user-content-license" name = "user-content-license">License</a> ##

2020 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



