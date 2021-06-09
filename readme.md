# list\.h #

## Doubly\-Linked List ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PN&gt;action_fn](#user-content-typedef-7ef2f840), [&lt;PN&gt;predicate_fn](#user-content-typedef-22328bf), [&lt;PN&gt;compare_fn](#user-content-typedef-237a6b85), [&lt;PA&gt;to_string_fn](#user-content-typedef-a933c596)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;N&gt;list_node](#user-content-tag-49d3e78), [&lt;N&gt;list](#user-content-tag-b7f8a30b), [&lt;PN&gt;iterator](#user-content-tag-f9929923)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of a stochastic skip-list.](web/list.png)

In parlance of <Thareja 2014, Data Structures>, [&lt;N&gt;list](#user-content-tag-b7f8a30b) is a circular header doubly\-linked list of [&lt;N&gt;list_node](#user-content-tag-49d3e78)\. The header, or sentinel, resides in `<N>list`\. This is a closed structure, such that with with a pointer to any element, it is possible to extract the entire list in &#927;\(`size`\)\. It only provides an order, and is not very useful without enclosing `<N>list_node` in another `struct`; this is useful for multi\-linked elements\.

`<N>list` is not synchronised\. Errors are returned with `errno`\. The parameters are preprocessor macros, and are all undefined at the end of the file for convenience\. Assertions are used in this file; to stop them, define `NDEBUG` before `assert.h`\.



 * Parameter: LIST\_NAME  
   `<N>` that satisfies `C` naming conventions when mangled; required\. `<PN>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: LIST\_COMPARE  
   Optional total\-order function satisfying [&lt;PN&gt;compare_fn](#user-content-typedef-237a6b85)\. \(fixme: move to trait\.\)
 * Parameter: LIST\_EXPECT\_TRAIT  
   Do not un\-define certain variables for subsequent inclusion in a trait\.
 * Parameter: LIST\_TO\_STRING\_NAME, LIST\_TO\_STRING  
   To string trait contained in [ToString\.h](ToString.h); `<A>` that satisfies `C` naming conventions when mangled and function implementing [&lt;PA&gt;to_string_fn](#user-content-typedef-a933c596)\. There can be multiple to string traits, but only one can omit `LIST_TO_STRING_NAME`\.
 * Parameter: LIST\_TEST  
   To string trait contained in [\.\./test/TestList\.h](../test/TestList.h); optional unit testing framework using `assert`\. Can only be defined once _per_ `Array`\. Must be defined equal to a \(random\) filler function, satisfying [&lt;PN&gt;action_fn](#user-content-typedef-7ef2f840)\. Output will be shown with the to string trait in which it's defined; provides tests for the base code and all later traits\.
 * Standard:  
   C89
 * See also:  
   [Array](https://github.com/neil-edelman/Array); [Heap](https://github.com/neil-edelman/Heap); [Orcish](https://github.com/neil-edelman/Orcish); [Pool](https://github.com/neil-edelman/Pool); [Set](https://github.com/neil-edelman/Set); [Trie](https://github.com/neil-edelman/Trie)


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-7ef2f840" name = "user-content-typedef-7ef2f840">&lt;PN&gt;action_fn</a> ###

<code>typedef void(*<strong>&lt;PN&gt;action_fn</strong>)(struct &lt;N&gt;list_node *);</code>

Operates by side\-effects on the node\.



### <a id = "user-content-typedef-22328bf" name = "user-content-typedef-22328bf">&lt;PN&gt;predicate_fn</a> ###

<code>typedef int(*<strong>&lt;PN&gt;predicate_fn</strong>)(const struct &lt;N&gt;list_node *);</code>

Returns \(Non\-zero\) true or \(zero\) false when given a node\.



### <a id = "user-content-typedef-237a6b85" name = "user-content-typedef-237a6b85">&lt;PN&gt;compare_fn</a> ###

<code>typedef int(*<strong>&lt;PN&gt;compare_fn</strong>)(const struct &lt;N&gt;list_node *a, const struct &lt;N&gt;list_node *b);</code>

Returns less then, equal to, or greater then zero, inducing an ordering between `a` and `b`\.



### <a id = "user-content-typedef-a933c596" name = "user-content-typedef-a933c596">&lt;PA&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PA&gt;to_string_fn</strong>)(const &lt;PA&gt;type *, char(*)[12]);</code>

Responsible for turning the first argument into a 12\-`char` null\-terminated output string\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-49d3e78" name = "user-content-tag-49d3e78">&lt;N&gt;list_node</a> ###

<code>struct <strong>&lt;N&gt;list_node</strong>;</code>

Storage of this structure is the responsibility of the caller\. One can only be in one list at a time; adding to another list while in a list destroys the integrity of the original list, see [&lt;N&gt;list_remove](#user-content-fn-325a120c)\.

![States.](web/node-states.png)



### <a id = "user-content-tag-b7f8a30b" name = "user-content-tag-b7f8a30b">&lt;N&gt;list</a> ###

<code>struct <strong>&lt;N&gt;list</strong>;</code>

Serves as head and tail for linked\-list of [&lt;N&gt;list_node](#user-content-tag-49d3e78)\. Use [&lt;N&gt;list_clear](#user-content-fn-49539c91) to initialise the list\. Because this list is closed; that is, given a valid pointer to an element, one can determine all others, null values are not allowed and it is _not_ the same as `{0}`\.

![States.](web/states.png)



### <a id = "user-content-tag-f9929923" name = "user-content-tag-f9929923">&lt;PN&gt;iterator</a> ###

<code>struct <strong>&lt;PN&gt;iterator</strong>;</code>

Contains all iteration parameters\.



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static struct &lt;N&gt;list_node *</td><td><a href = "#user-content-fn-c768eb92">&lt;N&gt;list_first</a></td><td>list</td></tr>

<tr><td align = right>static struct &lt;N&gt;list_node *</td><td><a href = "#user-content-fn-92687ec">&lt;N&gt;list_last</a></td><td>list</td></tr>

<tr><td align = right>static struct &lt;N&gt;list_node *</td><td><a href = "#user-content-fn-17162a51">&lt;N&gt;list_previous</a></td><td>link</td></tr>

<tr><td align = right>static struct &lt;N&gt;list_node *</td><td><a href = "#user-content-fn-7e1a85b5">&lt;N&gt;list_next</a></td><td>link</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-49539c91">&lt;N&gt;list_clear</a></td><td>list</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-8f48c713">&lt;N&gt;list_add_before</a></td><td>anchor, add</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-64250d2">&lt;N&gt;list_add_after</a></td><td>anchor, add</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d200fa75">&lt;N&gt;list_unshift</a></td><td>list, add</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-c3dc2328">&lt;N&gt;list_push</a></td><td>list, add</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-325a120c">&lt;N&gt;list_remove</a></td><td>node</td></tr>

<tr><td align = right>static struct &lt;N&gt;list_node *</td><td><a href = "#user-content-fn-43482e98">&lt;N&gt;list_shift</a></td><td>list</td></tr>

<tr><td align = right>static struct &lt;N&gt;list_node *</td><td><a href = "#user-content-fn-c2f0dd33">&lt;N&gt;list_pop</a></td><td>list</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-7ca11e95">&lt;N&gt;list_to</a></td><td>from, to</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-7928d819">&lt;N&gt;list_to_before</a></td><td>from, anchor</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-94383229">&lt;N&gt;list_to_if</a></td><td>from, to, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-b7e8c585">&lt;N&gt;list_for_each</a></td><td>list, action</td></tr>

<tr><td align = right>static struct &lt;N&gt;list_node *</td><td><a href = "#user-content-fn-7f8ec16">&lt;N&gt;list_any</a></td><td>list, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-a060b4cf">&lt;N&gt;list_self_correct</a></td><td>list</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-7e28a168">&lt;N&gt;list_sort</a></td><td>list</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-3739cccb">&lt;N&gt;list_compare</a></td><td>alist, blist</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-6af4b05a">&lt;N&gt;list_duplicates_to</a></td><td>from, to</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-1b4b3d66">&lt;N&gt;list_subtraction_to</a></td><td>a, b, result</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-192deea5">&lt;N&gt;list_union_to</a></td><td>a, b, result</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-a03954e7">&lt;N&gt;list_intersection_to</a></td><td>a, b, result</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d46d188f">&lt;N&gt;list_xor_to</a></td><td>a, b, result</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-6fb489ab">&lt;A&gt;to_string</a></td><td>box</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-36495738">&lt;N&gt;list_test</a></td><td>parent_new, parent</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-c768eb92" name = "user-content-fn-c768eb92">&lt;N&gt;list_first</a> ###

<code>static struct &lt;N&gt;list_node *<strong>&lt;N&gt;list_first</strong>(const struct &lt;N&gt;list *const <em>list</em>)</code>

 * Return:  
   A pointer to the first element of `list`, if it exists\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-92687ec" name = "user-content-fn-92687ec">&lt;N&gt;list_last</a> ###

<code>static struct &lt;N&gt;list_node *<strong>&lt;N&gt;list_last</strong>(const struct &lt;N&gt;list *const <em>list</em>)</code>

 * Return:  
   A pointer to the last element of `list`, if it exists\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-17162a51" name = "user-content-fn-17162a51">&lt;N&gt;list_previous</a> ###

<code>static struct &lt;N&gt;list_node *<strong>&lt;N&gt;list_previous</strong>(struct &lt;N&gt;list_node *<em>link</em>)</code>

 * Return:  
   The previous element\. When `link` is the first element, returns null\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-7e1a85b5" name = "user-content-fn-7e1a85b5">&lt;N&gt;list_next</a> ###

<code>static struct &lt;N&gt;list_node *<strong>&lt;N&gt;list_next</strong>(struct &lt;N&gt;list_node *<em>link</em>)</code>

 * Return:  
   The next element\. When `link` is the last element, returns null\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-49539c91" name = "user-content-fn-49539c91">&lt;N&gt;list_clear</a> ###

<code>static void <strong>&lt;N&gt;list_clear</strong>(struct &lt;N&gt;list *const <em>list</em>)</code>

Clears and initialises `list`\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-8f48c713" name = "user-content-fn-8f48c713">&lt;N&gt;list_add_before</a> ###

<code>static void <strong>&lt;N&gt;list_add_before</strong>(struct &lt;N&gt;list_node *const <em>anchor</em>, struct &lt;N&gt;list_node *const <em>add</em>)</code>

`add` before `anchor`\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-64250d2" name = "user-content-fn-64250d2">&lt;N&gt;list_add_after</a> ###

<code>static void <strong>&lt;N&gt;list_add_after</strong>(struct &lt;N&gt;list_node *const <em>anchor</em>, struct &lt;N&gt;list_node *const <em>add</em>)</code>

`add` after `anchor`\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-d200fa75" name = "user-content-fn-d200fa75">&lt;N&gt;list_unshift</a> ###

<code>static void <strong>&lt;N&gt;list_unshift</strong>(struct &lt;N&gt;list *const <em>list</em>, struct &lt;N&gt;list_node *const <em>add</em>)</code>

Adds `add` to the beginning of `list`\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-c3dc2328" name = "user-content-fn-c3dc2328">&lt;N&gt;list_push</a> ###

<code>static void <strong>&lt;N&gt;list_push</strong>(struct &lt;N&gt;list *const <em>list</em>, struct &lt;N&gt;list_node *const <em>add</em>)</code>

Adds `add` to the end of `list`\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-325a120c" name = "user-content-fn-325a120c">&lt;N&gt;list_remove</a> ###

<code>static void <strong>&lt;N&gt;list_remove</strong>(struct &lt;N&gt;list_node *const <em>node</em>)</code>

Remove `node`\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-43482e98" name = "user-content-fn-43482e98">&lt;N&gt;list_shift</a> ###

<code>static struct &lt;N&gt;list_node *<strong>&lt;N&gt;list_shift</strong>(struct &lt;N&gt;list *const <em>list</em>)</code>

Removes the first element of `list` and returns it, if any\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-c2f0dd33" name = "user-content-fn-c2f0dd33">&lt;N&gt;list_pop</a> ###

<code>static struct &lt;N&gt;list_node *<strong>&lt;N&gt;list_pop</strong>(struct &lt;N&gt;list *const <em>list</em>)</code>

Removes the last element of `list` and returns it, if any\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-7ca11e95" name = "user-content-fn-7ca11e95">&lt;N&gt;list_to</a> ###

<code>static void <strong>&lt;N&gt;list_to</strong>(struct &lt;N&gt;list *const <em>from</em>, struct &lt;N&gt;list *const <em>to</em>)</code>

Moves the elements `from` onto `to` at the end\.

 * Parameter: _to_  
   If null, then it removes elements from `from`\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-7928d819" name = "user-content-fn-7928d819">&lt;N&gt;list_to_before</a> ###

<code>static void <strong>&lt;N&gt;list_to_before</strong>(struct &lt;N&gt;list *const <em>from</em>, struct &lt;N&gt;list_node *const <em>anchor</em>)</code>

Moves the elements `from` immediately before `anchor`\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-94383229" name = "user-content-fn-94383229">&lt;N&gt;list_to_if</a> ###

<code>static void <strong>&lt;N&gt;list_to_if</strong>(struct &lt;N&gt;list *const <em>from</em>, struct &lt;N&gt;list *const <em>to</em>, const &lt;PN&gt;predicate_fn <em>predicate</em>)</code>

Moves all elements `from` onto `to` at the end if `predicate` is true\.

 * Parameter: _to_  
   If null, then it removes elements\.
 * Order:  
   &#920;\(|`from`|\) &#215; &#927;\(`predicate`\)




### <a id = "user-content-fn-b7e8c585" name = "user-content-fn-b7e8c585">&lt;N&gt;list_for_each</a> ###

<code>static void <strong>&lt;N&gt;list_for_each</strong>(struct &lt;N&gt;list *const <em>list</em>, const &lt;PN&gt;action_fn <em>action</em>)</code>

Performs `action` for each element in `list` in order\.

 * Parameter: _action_  
   Can be to delete the element\.
 * Order:  
   &#920;\(|`list`|\) &#215; O\(`action`\)




### <a id = "user-content-fn-7f8ec16" name = "user-content-fn-7f8ec16">&lt;N&gt;list_any</a> ###

<code>static struct &lt;N&gt;list_node *<strong>&lt;N&gt;list_any</strong>(const struct &lt;N&gt;list *const <em>list</em>, const &lt;PN&gt;predicate_fn <em>predicate</em>)</code>

Iterates through `list` and calls `predicate` until it returns true\.

 * Return:  
   The first `predicate` that returned true, or, if the statement is false on all, null\.
 * Order:  
   &#927;\(|`list`|\) &#215; &#927;\(`predicate`\)




### <a id = "user-content-fn-a060b4cf" name = "user-content-fn-a060b4cf">&lt;N&gt;list_self_correct</a> ###

<code>static void <strong>&lt;N&gt;list_self_correct</strong>(struct &lt;N&gt;list *const <em>list</em>)</code>

Corrects `list` ends to compensate for memory relocation of the list itself\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-7e28a168" name = "user-content-fn-7e28a168">&lt;N&gt;list_sort</a> ###

<code>static void <strong>&lt;N&gt;list_sort</strong>(struct &lt;N&gt;list *const <em>list</em>)</code>

Performs a stable, adaptive sort of `list` according to `compare`\. Requires `LIST_COMPARE`\. [Peters 2002, Timsort](https://scholar.google.ca/scholar?q=Peters+2002%2C+Timsort), _via_ [McIlroy 1993, Optimistic](https://scholar.google.ca/scholar?q=McIlroy+1993%2C+Optimistic), does long merges by galloping, but we don't have random access to the data because we are in a linked\-list; this does natural merge sort\.

 * Order:  
   &#937;\(|`list`|\), &#927;\(|`list`| log |`list`|\)




### <a id = "user-content-fn-3739cccb" name = "user-content-fn-3739cccb">&lt;N&gt;list_compare</a> ###

<code>static int <strong>&lt;N&gt;list_compare</strong>(const struct &lt;N&gt;list *const <em>alist</em>, const struct &lt;N&gt;list *const <em>blist</em>)</code>

Compares `alist` to `blist` as sequences\.

 * Return:  
   The first `LIST_COMPARE` that is not equal to zero, or 0 if they are equal\. Null is considered as before everything else; two null pointers are considered equal\.
 * Implements:  
   [&lt;PN&gt;compare_fn](#user-content-typedef-237a6b85)
 * Order:  
   &#920;\(min\(|`alist`|, |`blist`|\)\)




### <a id = "user-content-fn-6af4b05a" name = "user-content-fn-6af4b05a">&lt;N&gt;list_duplicates_to</a> ###

<code>static void <strong>&lt;N&gt;list_duplicates_to</strong>(struct &lt;N&gt;list *const <em>from</em>, struct &lt;N&gt;list *const <em>to</em>)</code>

Moves all local\-duplicates of `from` to the end of `to`\.

For example, if `from` is `(A, B, B, A)`, it would concatenate `(B)` to `to` and leave `(A, B, A)` in `from`\. If one [&lt;N&gt;list_sort](#user-content-fn-7e28a168) `from` first, `(A, A, B, B)`, the global duplicates will be transferred, `(A, B)`\.



 * Order:  
   &#927;\(|`from`|\)




### <a id = "user-content-fn-1b4b3d66" name = "user-content-fn-1b4b3d66">&lt;N&gt;list_subtraction_to</a> ###

<code>static void <strong>&lt;N&gt;list_subtraction_to</strong>(struct &lt;N&gt;list *const <em>a</em>, struct &lt;N&gt;list *const <em>b</em>, struct &lt;N&gt;list *const <em>result</em>)</code>

Subtracts `a` from `b`, as sequential sorted individual elements, and moves it to `result`\. All elements are removed from `a`\. All parameters must be unique or can be null\.

For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then `(a:A, a:D)` would be moved to `result`\.



 * Order:  
   &#927;\(|`a`| \+ |`b`|\)


### <a id = "user-content-fn-192deea5" name = "user-content-fn-192deea5">&lt;N&gt;list_union_to</a> ###

<code>static void <strong>&lt;N&gt;list_union_to</strong>(struct &lt;N&gt;list *const <em>a</em>, struct &lt;N&gt;list *const <em>b</em>, struct &lt;N&gt;list *const <em>result</em>)</code>

Moves the union of `a` and `b` as sequential sorted individual elements to `result`\. Equal elements are moved from `a`\. All parameters must be unique or can be null\.

For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then `(a:A, a:B, b:C, a:D)` would be moved to `result`\.



 * Order:  
   &#927;\(|`a`| \+ |`b`|\)


### <a id = "user-content-fn-a03954e7" name = "user-content-fn-a03954e7">&lt;N&gt;list_intersection_to</a> ###

<code>static void <strong>&lt;N&gt;list_intersection_to</strong>(struct &lt;N&gt;list *const <em>a</em>, struct &lt;N&gt;list *const <em>b</em>, struct &lt;N&gt;list *const <em>result</em>)</code>

Moves the intersection of `a` and `b` as sequential sorted individual elements to `result`\. Equal elements are moved from `a`\. All parameters must be unique or can be null\.

For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then `(a:B)` would be moved to `result`\.



 * Order:  
   &#927;\(|`a`| \+ |`b`|\)


### <a id = "user-content-fn-d46d188f" name = "user-content-fn-d46d188f">&lt;N&gt;list_xor_to</a> ###

<code>static void <strong>&lt;N&gt;list_xor_to</strong>(struct &lt;N&gt;list *const <em>a</em>, struct &lt;N&gt;list *const <em>b</em>, struct &lt;N&gt;list *const <em>result</em>)</code>

Moves `a` exclusive\-or `b` as sequential sorted individual elements to `result`\. Equal elements are moved from `a`\. All parameters must be unique or can be null\.

For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then `(a:A, b:C, a:D)` would be moved to `result`\.



 * Order:  
   O\(|`a`| \+ |`b`|\)


### <a id = "user-content-fn-6fb489ab" name = "user-content-fn-6fb489ab">&lt;A&gt;to_string</a> ###

<code>static const char *<strong>&lt;A&gt;to_string</strong>(const &lt;PA&gt;box *const <em>box</em>)</code>

 * Return:  
   Print the contents of `box` in a static string buffer of 256 bytes with limitations of only printing 4 things at a time\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-36495738" name = "user-content-fn-36495738">&lt;N&gt;list_test</a> ###

<code>static void <strong>&lt;N&gt;list_test</strong>(struct &lt;N&gt;list_node *(*const <em>parent_new</em>)(void *), void *const <em>parent</em>)</code>

The linked\-list will be tested on stdout\. `LIST_TEST` has to be set\.

 * Parameter: _parent\_new_  
   Responsible for creating new objects and returning the list\.
 * Parameter: _parent_  
   Responsible for creating new objects and returning the list\.




## <a id = "user-content-license" name = "user-content-license">License</a> ##

2017 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



