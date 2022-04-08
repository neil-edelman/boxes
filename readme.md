# tree\.h #

Stand\-alone header [src/tree\.h](src/tree.h); examples [test/test\_tree\.c](test/test_tree.c)\. On a compatible workstation, `make` creates the test suite of the examples\.

## Ordered tree ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PB&gt;type](#user-content-typedef-30fc6ebd), [&lt;PB&gt;value](#user-content-typedef-1740653a), [&lt;PB&gt;compare_fn](#user-content-typedef-35616b31), [&lt;PB&gt;entry](#user-content-typedef-8e330c63), [&lt;PSZ&gt;to_string_fn](#user-content-typedef-8b890812)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;B&gt;tree_entry](#user-content-tag-9e3caf18), [&lt;B&gt;tree](#user-content-tag-a36433e3), [&lt;B&gt;tree_iterator](#user-content-tag-18b8c30e)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

A [&lt;B&gt;tree](#user-content-tag-a36433e3) is an ordered collection of read\-only [&lt;PB&gt;type](#user-content-typedef-30fc6ebd), and an optional [&lt;PB&gt;value](#user-content-typedef-1740653a) to go with them\. One can make this a map or set, but in general, it can have identical keys, \(a multi\-map\)\. Internally, this is a B\-tree, described in [Bayer, McCreight, 1972 Large](https://scholar.google.ca/scholar?q=Bayer%2C+McCreight%2C+1972+Large)\.

 * Parameter: TREE\_NAME, TREE\_TYPE  
   `<B>` that satisfies `C` naming conventions when mangled, required, and an integral type, [&lt;PB&gt;type](#user-content-typedef-30fc6ebd), whose default is `unsigned int`\. `<PB>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: TREE\_VALUE  
   `TRIE_VALUE` is an optional payload to go with the type, [&lt;PB&gt;value](#user-content-typedef-1740653a)\.
 * Parameter: TREE\_COMPARE  
   A function satisfying [&lt;PB&gt;compare_fn](#user-content-typedef-35616b31)\. Defaults to ascending order\. Required if `TREE_TYPE` is changed to an incomparable type\.
 * Parameter: TREE\_EXPECT\_TRAIT  
   Do not un\-define certain variables for subsequent inclusion in a parameterized trait\.
 * Parameter: TREE\_TO\_STRING\_NAME, TREE\_TO\_STRING  
   To string trait contained in [to\_string\.h](to_string.h); an optional unique `<SZ>` that satisfies `C` naming conventions when mangled and function implementing [&lt;PSZ&gt;to_string_fn](#user-content-typedef-8b890812)\.
 * Standard:  
   C89




## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-30fc6ebd" name = "user-content-typedef-30fc6ebd">&lt;PB&gt;type</a> ###

<code>typedef TREE_TYPE <strong>&lt;PB&gt;type</strong>;</code>

A comparable type, defaults to `unsigned`\.



### <a id = "user-content-typedef-1740653a" name = "user-content-typedef-1740653a">&lt;PB&gt;value</a> ###

<code>typedef TREE_VALUE <strong>&lt;PB&gt;value</strong>;</code>

On `TREE_VALUE`, otherwise just a set of integers\.



### <a id = "user-content-typedef-35616b31" name = "user-content-typedef-35616b31">&lt;PB&gt;compare_fn</a> ###

<code>typedef int(*<strong>&lt;PB&gt;compare_fn</strong>)(const &lt;PB&gt;type a, const &lt;PB&gt;type b);</code>

Returns a positive result if `a` is out\-of\-order with respect to `b`, inducing a total order\. This is compatible, but less strict then the comparators from `bsearch` and `qsort`; it only needs to divide entries into two instead of three categories\.



### <a id = "user-content-typedef-8e330c63" name = "user-content-typedef-8e330c63">&lt;PB&gt;entry</a> ###

<code>typedef struct &lt;B&gt;tree_entry <strong>&lt;PB&gt;entry</strong>;</code>

On `TREE_VALUE`, otherwise it's just an alias for [&lt;PB&gt;type](#user-content-typedef-30fc6ebd)\.



### <a id = "user-content-typedef-8b890812" name = "user-content-typedef-8b890812">&lt;PSZ&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PSZ&gt;to_string_fn</strong>)(const &lt;PSZ&gt;enum, char(*)[12]);</code>

[to\_string\.h](to_string.h): responsible for turning the argument into a 12\-`char` null\-terminated output string\. `<PSZ>type` is contracted to be an internal iteration type of the box\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-9e3caf18" name = "user-content-tag-9e3caf18">&lt;B&gt;tree_entry</a> ###

<code>struct <strong>&lt;B&gt;tree_entry</strong> { &lt;PB&gt;type x; &lt;PB&gt;value *value; };</code>

On `TREE_VALUE`, creates a map from type to pointer\-to\-value\.



### <a id = "user-content-tag-a36433e3" name = "user-content-tag-a36433e3">&lt;B&gt;tree</a> ###

<code>struct <strong>&lt;B&gt;tree</strong>;</code>

To initialize it to an idle state, see [&lt;U&gt;tree](#user-content-fn-55cb0b0c), `TRIE_IDLE`, `{0}` \(`C99`\), or being `static`\.

![States.](../doc/states.png)



### <a id = "user-content-tag-18b8c30e" name = "user-content-tag-18b8c30e">&lt;B&gt;tree_iterator</a> ###

<code>struct <strong>&lt;B&gt;tree_iterator</strong> { struct &lt;PB&gt;iterator it; };</code>

Stores an iteration in a tree\. Generally, changes in the topology of the tree invalidate it\.



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static struct &lt;B&gt;tree</td><td><a href = "#user-content-fn-a36433e3">&lt;B&gt;tree</a></td><td></td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-f2bd70f4">&lt;B&gt;tree_</a></td><td>tree</td></tr>

<tr><td align = right>static &lt;PB&gt;entry</td><td><a href = "#user-content-fn-6828a06d">&lt;B&gt;tree_next</a></td><td>it</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-c0bc2f01">&lt;B&gt;trie_size</a></td><td>it</td></tr>

<tr><td align = right>static struct &lt;B&gt;tree_iterator</td><td><a href = "#user-content-fn-78d7d6e1">&lt;B&gt;tree_lower</a></td><td>tree, x</td></tr>

<tr><td align = right>static &lt;PB&gt;entry</td><td><a href = "#user-content-fn-2e61c7b0">&lt;B&gt;tree_get</a></td><td>tree, x</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-523ce581">&lt;B&gt;trie_put</a></td><td>trie, x, /*const fixme*/eject</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-dd88b41a">&lt;B&gt;trie_policy</a></td><td>trie, x, eject, replace</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-b11709d3">&lt;SZ&gt;to_string</a></td><td>box</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-a36433e3" name = "user-content-fn-a36433e3">&lt;B&gt;tree</a> ###

<code>static struct &lt;B&gt;tree <strong>&lt;B&gt;tree</strong>(void)</code>

Initializes `tree` to idle\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-f2bd70f4" name = "user-content-fn-f2bd70f4">&lt;B&gt;tree_</a> ###

<code>static void <strong>&lt;B&gt;tree_</strong>(struct &lt;B&gt;tree *const <em>tree</em>)</code>

Returns an initialized `tree` to idle, `tree` can be null\.



### <a id = "user-content-fn-6828a06d" name = "user-content-fn-6828a06d">&lt;B&gt;tree_next</a> ###

<code>static &lt;PB&gt;entry <strong>&lt;B&gt;tree_next</strong>(struct &lt;B&gt;tree_iterator *const <em>it</em>)</code>

Advances `it`\.

 * Return:  
   If the iteration is finished, null, otherwise, if `TREE_VALUE`, a static [&lt;B&gt;tree_entry](#user-content-typedef-9e3caf18) of a copy of the key and pointer to the value, otherwise, a pointer to key\.




### <a id = "user-content-fn-c0bc2f01" name = "user-content-fn-c0bc2f01">&lt;B&gt;trie_size</a> ###

<code>static size_t <strong>&lt;B&gt;trie_size</strong>(const struct &lt;B&gt;trie_iterator *const <em>it</em>)</code>

Counts the of the items in initialized `it`\.

 * Order:  
   &#927;\(|`it`|\)




### <a id = "user-content-fn-78d7d6e1" name = "user-content-fn-78d7d6e1">&lt;B&gt;tree_lower</a> ###

<code>static struct &lt;B&gt;tree_iterator <strong>&lt;B&gt;tree_lower</strong>(const struct &lt;B&gt;tree *const <em>tree</em>, const &lt;PB&gt;type <em>x</em>)</code>

 * Parameter: _tree_  
   Can be null\.
 * Return:  
   Finds the smallest entry in `tree` that is not less than `x`\. If `x` is higher than any of `tree`, it will be placed just passed the end\.
 * Order:  
   &#927;\(\\log |`tree`|\)




### <a id = "user-content-fn-2e61c7b0" name = "user-content-fn-2e61c7b0">&lt;B&gt;tree_get</a> ###

<code>static &lt;PB&gt;entry <strong>&lt;B&gt;tree_get</strong>(const struct &lt;B&gt;tree *const <em>tree</em>, const &lt;PB&gt;type <em>x</em>)</code>

 * Return:  
   Lowest match for `x` in `tree` or null no such item exists\.
 * Order:  
   &#927;\(\\log |`tree`|\)




### <a id = "user-content-fn-523ce581" name = "user-content-fn-523ce581">&lt;B&gt;trie_put</a> ###

<code>static int <strong>&lt;B&gt;trie_put</strong>(struct &lt;B&gt;trie *const <em>trie</em>, const &lt;PB&gt;entry <em>x</em>, &lt;PB&gt;entry *<em>/*const fixme*/eject</em>)</code>

Updates or adds a pointer to `x` into `trie`\.

 * Return:  
   Success\.
 * Exceptional return: realloc, ERANGE  
 * Order:  
   &#927;\(|`key`|\)




### <a id = "user-content-fn-dd88b41a" name = "user-content-fn-dd88b41a">&lt;B&gt;trie_policy</a> ###

<code>static int <strong>&lt;B&gt;trie_policy</strong>(struct &lt;B&gt;trie *const <em>trie</em>, const &lt;PB&gt;entry <em>x</em>, &lt;PB&gt;entry *<em>eject</em>, const &lt;PB&gt;replace_fn <em>replace</em>)</code>

Adds a pointer to `x` to `trie` only if the entry is absent or if calling `replace` returns true or is null\.

 * Parameter: _eject_  
   If not null, on success it will hold the overwritten value or a pointer\-to\-null if it did not overwrite any value\. If a collision occurs and `replace` does not return true, this will be a pointer to `x`\.
 * Parameter: _replace_  
   Called on collision and only replaces it if the function returns true\. If null, it is semantically equivalent to [&lt;T&gt;trie_put](#user-content-fn-bd93d12b)\.
 * Return:  
   Success\.
 * Exceptional return: realloc, ERANGE  
 * Order:  
   &#927;\(|`key`|\)




### <a id = "user-content-fn-b11709d3" name = "user-content-fn-b11709d3">&lt;SZ&gt;to_string</a> ###

<code>static const char *<strong>&lt;SZ&gt;to_string</strong>(const &lt;PSZ&gt;box *const <em>box</em>)</code>

[src/to\_string\.h](src/to_string.h): print the contents of `box` in a static string buffer of 256 bytes, with limitations of only printing 4 things at a time\. `<PSZ>box` is contracted to be the box itself\. `<SZ>` is loosely contracted to be a name `<X>box[<X_TO_STRING_NAME>]`\.

 * Return:  
   Address of the static buffer\.
 * Order:  
   &#920;\(1\)






## <a id = "user-content-license" name = "user-content-license">License</a> ##

2022 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



