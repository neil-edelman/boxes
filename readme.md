# tree\.h #

Stand\-alone header [src/tree\.h](src/tree.h); examples [test/test\_tree\.c](test/test_tree.c)\. On a compatible workstation, `make` creates the test suite of the examples\.

## Ordered key\-tree ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PB&gt;key](#user-content-typedef-9d1494bc), [&lt;PB&gt;value](#user-content-typedef-1740653a), [&lt;PB&gt;compare_fn](#user-content-typedef-35616b31), [&lt;PB&gt;entry](#user-content-typedef-8e330c63), [&lt;PSTR&gt;to_string_fn](#user-content-typedef-8a8349ca)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [tree_result](#user-content-tag-9c3f99d7), [&lt;B&gt;tree](#user-content-tag-a36433e3), [&lt;B&gt;tree_entry](#user-content-tag-9e3caf18), [&lt;B&gt;tree_iterator](#user-content-tag-18b8c30e)
 * [General Declarations](#user-content-data): [e](#user-content-data-e00c22e0)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

A [&lt;B&gt;tree](#user-content-tag-a36433e3) is an ordered set or map contained in a tree\. For memory locality, this is implemented B\-tree, described in [Bayer, McCreight, 1972, Large](https://scholar.google.ca/scholar?q=Bayer%2C+McCreight%2C+1972%2C+Large)\.

 * Parameter: TREE\_NAME, TREE\_KEY  
   `<B>` that satisfies `C` naming conventions when mangled, required, and `TREE_KEY`, a comparable type, [&lt;PB&gt;key](#user-content-typedef-9d1494bc), whose default is `unsigned int`\. `<PB>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: TREE\_VALUE  
   `TRIE_VALUE` is an optional payload to go with the type, [&lt;PB&gt;value](#user-content-typedef-1740653a)\. The makes it a map of [&lt;B&gt;tree_entry](#user-content-tag-9e3caf18) instead of a set\.
 * Parameter: TREE\_COMPARE  
   A function satisfying [&lt;PB&gt;compare_fn](#user-content-typedef-35616b31)\. Defaults to ascending order\. Required if `TREE_KEY` is changed to an incomparable type\.
 * Parameter: TREE\_EXPECT\_TRAIT  
   Do not un\-define certain variables for subsequent inclusion in a parameterized trait\.
 * Parameter: TREE\_TO\_STRING\_NAME, TREE\_TO\_STRING  
   To string trait contained in [to\_string\.h](to_string.h); an optional unique `<SZ>` that satisfies `C` naming conventions when mangled and function implementing [&lt;PSTR&gt;to_string_fn](#user-content-typedef-8a8349ca)\.
 * Standard:  
   C89
 * Caveat:  
   multi\-key; implementation of order statistic tree? merge, difference




## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-9d1494bc" name = "user-content-typedef-9d1494bc">&lt;PB&gt;key</a> ###

<code>typedef TREE_KEY <strong>&lt;PB&gt;key</strong>;</code>

A comparable type, defaults to `unsigned`\.



### <a id = "user-content-typedef-1740653a" name = "user-content-typedef-1740653a">&lt;PB&gt;value</a> ###

<code>typedef TREE_VALUE <strong>&lt;PB&gt;value</strong>;</code>

On `TREE_VALUE`, otherwise just a set of [&lt;PB&gt;key](#user-content-typedef-9d1494bc)\.



### <a id = "user-content-typedef-35616b31" name = "user-content-typedef-35616b31">&lt;PB&gt;compare_fn</a> ###

<code>typedef int(*<strong>&lt;PB&gt;compare_fn</strong>)(&lt;PB&gt;key_c a, &lt;PB&gt;key_c b);</code>

Returns a positive result if `a` is out\-of\-order with respect to `b`, inducing a strict weak order\. This is compatible, but less strict then the comparators from `bsearch` and `qsort`; it only needs to divide entries into two instead of three categories\.



### <a id = "user-content-typedef-8e330c63" name = "user-content-typedef-8e330c63">&lt;PB&gt;entry</a> ###

<code>typedef struct &lt;B&gt;tree_entry <strong>&lt;PB&gt;entry</strong>;</code>

On `TREE_VALUE`, otherwise it's just an alias for pointer\-to\-[&lt;PB&gt;key](#user-content-typedef-9d1494bc)\.



### <a id = "user-content-typedef-8a8349ca" name = "user-content-typedef-8a8349ca">&lt;PSTR&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PSTR&gt;to_string_fn</strong>)(&lt;PSTR&gt;element_c, char(*)[12]);</code>

[src/to\_string\.h](src/to_string.h): responsible for turning the argument into a 12\-`char` null\-terminated output string\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-9c3f99d7" name = "user-content-tag-9c3f99d7">tree_result</a> ###

<code>enum <strong>tree_result</strong> { TREE_RESULT };</code>

A result of modifying the tree, of which `TREE_ERROR` is false\. ![A diagram of the result states.](../doc/put.png)



### <a id = "user-content-tag-a36433e3" name = "user-content-tag-a36433e3">&lt;B&gt;tree</a> ###

<code>struct <strong>&lt;B&gt;tree</strong>;</code>

To initialize it to an idle state, see [&lt;B&gt;tree](#user-content-fn-a36433e3), `TRIE_IDLE`, `{0}` \(`C99`\), or being `static`\.

![States.](../doc/states.png)



### <a id = "user-content-tag-9e3caf18" name = "user-content-tag-9e3caf18">&lt;B&gt;tree_entry</a> ###

<code>struct <strong>&lt;B&gt;tree_entry</strong> { &lt;PB&gt;key *key; &lt;PB&gt;value *value; };</code>

On `TREE_VALUE`, creates a map from pointer\-to\-[&lt;PB&gt;key](#user-content-typedef-9d1494bc) to pointer\-to\-[&lt;PB&gt;value](#user-content-typedef-1740653a)\. The reason these are pointers is because it is not connected in memory\. \(Does `key` still have to be?\)



### <a id = "user-content-tag-18b8c30e" name = "user-content-tag-18b8c30e">&lt;B&gt;tree_iterator</a> ###

<code>struct <strong>&lt;B&gt;tree_iterator</strong> { struct &lt;PB&gt;iterator _; };</code>

Stores an iteration in a tree\. Generally, changes in the topology of the tree invalidate it\. \(Future: have insert and delete with iterators\.\)



## <a id = "user-content-data" name = "user-content-data">General Declarations</a> ##

### <a id = "user-content-data-e00c22e0" name = "user-content-data-e00c22e0">e</a> ###

<code>TREE_TO_SUCCESSOR(to_successor, ref)TREE_TO_SUCCESSOR(to_successor_c, ref_c)static int &lt;PB&gt;is_element_c(&lt;PB&gt;entry_c <strong>e</strong>){ return !!e .key; return !!e; } struct &lt;PB&gt;forward { const struct &lt;PB&gt;tree *root; struct &lt;PB&gt;ref_c next; };</code>

Is `e` not null?

 * Implements:  
   `is_element_c`




## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static struct &lt;B&gt;tree</td><td><a href = "#user-content-fn-a36433e3">&lt;B&gt;tree</a></td><td></td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-f2bd70f4">&lt;B&gt;tree_</a></td><td>tree</td></tr>

<tr><td align = right>static struct &lt;B&gt;tree_iterator</td><td><a href = "#user-content-fn-18b8c30e">&lt;B&gt;tree_iterator</a></td><td>tree</td></tr>

<tr><td align = right>static &lt;PB&gt;entry</td><td><a href = "#user-content-fn-6828a06d">&lt;B&gt;tree_next</a></td><td>it</td></tr>

<tr><td align = right>static struct &lt;B&gt;tree_iterator</td><td><a href = "#user-content-fn-9cc261e0">&lt;B&gt;tree_lower_iterator</a></td><td>tree, x</td></tr>

<tr><td align = right>static &lt;PB&gt;value *</td><td><a href = "#user-content-fn-867e2fe3">&lt;B&gt;tree_lower_value</a></td><td>tree, x</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-b3495ae9">&lt;B&gt;tree_clone</a></td><td>tree, source</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-751c6337">&lt;STR&gt;to_string</a></td><td>box</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-a36433e3" name = "user-content-fn-a36433e3">&lt;B&gt;tree</a> ###

<code>static struct &lt;B&gt;tree <strong>&lt;B&gt;tree</strong>(void)</code>

 * Return:  
   Initializes `tree` to idle\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-f2bd70f4" name = "user-content-fn-f2bd70f4">&lt;B&gt;tree_</a> ###

<code>static void <strong>&lt;B&gt;tree_</strong>(struct &lt;B&gt;tree *const <em>tree</em>)</code>

Returns an initialized `tree` to idle, `tree` can be null\.



### <a id = "user-content-fn-18b8c30e" name = "user-content-fn-18b8c30e">&lt;B&gt;tree_iterator</a> ###

<code>static struct &lt;B&gt;tree_iterator <strong>&lt;B&gt;tree_iterator</strong>(struct &lt;B&gt;tree *const <em>tree</em>)</code>

 * Return:  
   An iterator before the first element of `tree`\. Can be null\.




### <a id = "user-content-fn-6828a06d" name = "user-content-fn-6828a06d">&lt;B&gt;tree_next</a> ###

<code>static &lt;PB&gt;entry <strong>&lt;B&gt;tree_next</strong>(struct &lt;B&gt;tree_iterator *const <em>it</em>)</code>

Advances `it` to the next element\.

 * Return:  
   A pointer to the current element or null\.




### <a id = "user-content-fn-9cc261e0" name = "user-content-fn-9cc261e0">&lt;B&gt;tree_lower_iterator</a> ###

<code>static struct &lt;B&gt;tree_iterator <strong>&lt;B&gt;tree_lower_iterator</strong>(struct &lt;B&gt;tree *const <em>tree</em>, const &lt;PB&gt;key <em>x</em>)</code>

 * Parameter: _tree_  
   Can be null\.
 * Return:  
   Finds the smallest entry in `tree` that is at the lower bound of `x`\. If `x` is higher than any of `tree`, it will be placed just passed the end\.
 * Order:  
   &#927;\(\\log |`tree`|\)




### <a id = "user-content-fn-867e2fe3" name = "user-content-fn-867e2fe3">&lt;B&gt;tree_lower_value</a> ###

<code>static &lt;PB&gt;value *<strong>&lt;B&gt;tree_lower_value</strong>(struct &lt;B&gt;tree *const <em>tree</em>, const &lt;PB&gt;key <em>x</em>)</code>

For example, `tree = { 10 }`, `x = 5 -> 10`, `x = 10 -> 10`, `x = 11 -> null`\.

 * Return:  
   Lower\-bound value match for `x` in `tree` or null if `x` is greater than all in `tree`\.
 * Order:  
   &#927;\(\\log |`tree`|\)




### <a id = "user-content-fn-b3495ae9" name = "user-content-fn-b3495ae9">&lt;B&gt;tree_clone</a> ###

<code>static int <strong>&lt;B&gt;tree_clone</strong>(struct &lt;B&gt;tree *const <em>tree</em>, const struct &lt;B&gt;tree *const <em>source</em>)</code>

`source` is copied to, and overwrites, `tree`\.

 * Parameter: _source_  
   In the case where it's null or idle, if `tree` is empty, then it continues to be\.
 * Return:  
   Success, otherwise `tree` is not modified\.
 * Exceptional return: malloc  
 * Exceptional return: EDOM  
   `tree` is null\.
 * Exceptional return: ERANGE  
   The size of `source` doesn't fit into `size_t`\.




### <a id = "user-content-fn-751c6337" name = "user-content-fn-751c6337">&lt;STR&gt;to_string</a> ###

<code>static const char *<strong>&lt;STR&gt;to_string</strong>(const &lt;PSTR&gt;box *const <em>box</em>)</code>

[src/to\_string\.h](src/to_string.h): print the contents of `box` in a static string buffer of 256 bytes, with limitations of only printing 4 things at a time\. `<STR>` is loosely contracted to be a name `<X>box[<X_TO_STRING_NAME>]`\.

 * Return:  
   Address of the static buffer\.
 * Order:  
   &#920;\(1\)






## <a id = "user-content-license" name = "user-content-license">License</a> ##

2022 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



