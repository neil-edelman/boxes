# list\.h #

## Doubly\-linked component ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PL&gt;action_fn](#user-content-typedef-5aae0d96), [&lt;PL&gt;predicate_fn](#user-content-typedef-9bb522f5), [&lt;PL&gt;compare_fn](#user-content-typedef-a22f279f), [&lt;PSZ&gt;box](#user-content-typedef-ace240bb), [&lt;PSZ&gt;type](#user-content-typedef-d1a7c35e), [&lt;PSZ&gt;to_string_fn](#user-content-typedef-8b890812)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;L&gt;listlink](#user-content-tag-15769e01), [&lt;L&gt;list](#user-content-tag-eb84971d), [&lt;PL&gt;iterator](#user-content-tag-fe8215f1)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of a stochastic skip-list.](web/list.png)

In parlance of <Thareja 2014, Data Structures>, [&lt;L&gt;list](#user-content-tag-eb84971d) is a circular header doubly\-linked list of [&lt;L&gt;listlink](#user-content-tag-15769e01)\. The header, or sentinel, resides in `<L>list`\. This allows it to benefit from being closed structure, such that with with a pointer to any element, it is possible to extract the entire list in &#927;\(`size`\)\. It only provides an order component, and is not very useful without enclosing `<L>listlink` in another `struct`; this is useful for multi\-linked elements\.

FIXME: don't duplicate; this should be the private implementation of the functions above [&lt;L&gt;list_next](#user-content-fn-40b28b9b), _etc_\.

 * Parameter: LIST\_NAME  
   `<L>` that satisfies `C` naming conventions when mangled; required\. `<PL>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: LIST\_COMPARE  
   Optional total\-order function satisfying [&lt;PL&gt;compare_fn](#user-content-typedef-a22f279f)\. \(fixme: move to trait\.\)
 * Parameter: LIST\_EXPECT\_TRAIT  
   Do not un\-define certain variables for subsequent inclusion in a trait\.
 * Parameter: LIST\_TO\_STRING\_NAME, LIST\_TO\_STRING  
   To string trait contained in [to\_string\.h](to_string.h); requires `ARRAY_ITERATE` and goes forwards\. An optional mangled name for uniqueness and function implementing [&lt;PSZ&gt;to_string_fn](#user-content-typedef-8b890812)\.
 * Parameter: LIST\_TEST  
   To string trait contained in [\.\./test/test\_list\.h](../test/test_list.h); optional unit testing framework using `assert`\. Can only be defined once _per_ `Array`\. Must be defined equal to a \(random\) filler function, satisfying [&lt;PL&gt;action_fn](#user-content-typedef-5aae0d96)\. Output will be shown with the to string trait in which it's defined; provides tests for the base code and all later traits\.
 * Standard:  
   C89


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-5aae0d96" name = "user-content-typedef-5aae0d96">&lt;PL&gt;action_fn</a> ###

<code>typedef void(*<strong>&lt;PL&gt;action_fn</strong>)(struct &lt;L&gt;listlink *);</code>

Operates by side\-effects on the node\.



### <a id = "user-content-typedef-9bb522f5" name = "user-content-typedef-9bb522f5">&lt;PL&gt;predicate_fn</a> ###

<code>typedef int(*<strong>&lt;PL&gt;predicate_fn</strong>)(const struct &lt;L&gt;listlink *);</code>

Returns \(Non\-zero\) true or \(zero\) false when given a node\.



### <a id = "user-content-typedef-a22f279f" name = "user-content-typedef-a22f279f">&lt;PL&gt;compare_fn</a> ###

<code>typedef int(*<strong>&lt;PL&gt;compare_fn</strong>)(const struct &lt;L&gt;listlink *a, const struct &lt;L&gt;listlink *b);</code>

Returns less then, equal to, or greater then zero, inducing an ordering between `a` and `b`\.



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

### <a id = "user-content-tag-15769e01" name = "user-content-tag-15769e01">&lt;L&gt;listlink</a> ###

<code>struct <strong>&lt;L&gt;listlink</strong> { struct &lt;L&gt;listlink *prev, *next; };</code>

Storage of this structure is the responsibility of the caller\. Generally, one encloses this in a host `struct`\. Multiple independent lists can be in the same host structure, however one link can can only be a part of one list at a time\.

![States.](web/node-states.png)



### <a id = "user-content-tag-eb84971d" name = "user-content-tag-eb84971d">&lt;L&gt;list</a> ###

<code>struct <strong>&lt;L&gt;list</strong> { struct &lt;L&gt;listlink head, tail; };</code>

Serves as head and tail for linked\-list of [&lt;L&gt;listlink](#user-content-tag-15769e01)\. Use [&lt;L&gt;list_clear](#user-content-fn-f965b937) to initialize the list\. Because this list is closed; that is, given a valid pointer to an element, one can determine all others, null values are not allowed and it is _not_ the same as `{0}`\. These are sentinels such that `head.prev` and `tail.next` are always and the only ones to be null in a valid list\.

![States.](web/states.png)



### <a id = "user-content-tag-fe8215f1" name = "user-content-tag-fe8215f1">&lt;PL&gt;iterator</a> ###

<code>struct <strong>&lt;PL&gt;iterator</strong> { struct &lt;L&gt;listlink *node; };</code>

Contains all iteration parameters\.



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static struct &lt;L&gt;listlink *</td><td><a href = "#user-content-fn-44386a44">&lt;L&gt;list_first</a></td><td>list</td></tr>

<tr><td align = right>static struct &lt;L&gt;listlink *</td><td><a href = "#user-content-fn-caeebaea">&lt;L&gt;list_last</a></td><td>list</td></tr>

<tr><td align = right>static struct &lt;L&gt;listlink *</td><td><a href = "#user-content-fn-93616d3b">&lt;L&gt;list_previous</a></td><td>link</td></tr>

<tr><td align = right>static struct &lt;L&gt;listlink *</td><td><a href = "#user-content-fn-40b28b9b">&lt;L&gt;list_next</a></td><td>link</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-f965b937">&lt;L&gt;list_clear</a></td><td>list</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-b26385b9">&lt;L&gt;list_add_before</a></td><td>anchor, add</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-fee45e54">&lt;L&gt;list_add_after</a></td><td>anchor, add</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-bb35ae87">&lt;L&gt;list_unshift</a></td><td>list, add</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-8dc506e">&lt;L&gt;list_push</a></td><td>list, add</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-dfcb43e2">&lt;L&gt;list_remove</a></td><td>node</td></tr>

<tr><td align = right>static struct &lt;L&gt;listlink *</td><td><a href = "#user-content-fn-1c88ead6">&lt;L&gt;list_shift</a></td><td>list</td></tr>

<tr><td align = right>static struct &lt;L&gt;listlink *</td><td><a href = "#user-content-fn-aeb1eac5">&lt;L&gt;list_pop</a></td><td>list</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-1171e3b">&lt;L&gt;list_to</a></td><td>from, to</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-727db3d7">&lt;L&gt;list_to_before</a></td><td>from, anchor</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-90de573b">&lt;L&gt;list_to_if</a></td><td>from, to, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-42fb011b">&lt;L&gt;list_for_each</a></td><td>list, action</td></tr>

<tr><td align = right>static struct &lt;L&gt;listlink *</td><td><a href = "#user-content-fn-48bee118">&lt;L&gt;list_any</a></td><td>list, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-1ce0c229">&lt;L&gt;list_self_correct</a></td><td>list</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-257babc6">&lt;L&gt;list_sort</a></td><td>list</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-6df7d15d">&lt;L&gt;list_compare</a></td><td>alist, blist</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-e9e9fc48">&lt;L&gt;list_duplicates_to</a></td><td>from, to</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-2d73ee58">&lt;L&gt;list_subtraction_to</a></td><td>a, b, result</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-2eaa814b">&lt;L&gt;list_union_to</a></td><td>a, b, result</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-2cced3a5">&lt;L&gt;list_intersection_to</a></td><td>a, b, result</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-51bed9c1">&lt;L&gt;list_xor_to</a></td><td>a, b, result</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-b11709d3">&lt;SZ&gt;to_string</a></td><td>box</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-44386a44" name = "user-content-fn-44386a44">&lt;L&gt;list_first</a> ###

<code>static struct &lt;L&gt;listlink *<strong>&lt;L&gt;list_first</strong>(const struct &lt;L&gt;list *const <em>list</em>)</code>

 * Return:  
   A pointer to the first element of `list`, if it exists\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-caeebaea" name = "user-content-fn-caeebaea">&lt;L&gt;list_last</a> ###

<code>static struct &lt;L&gt;listlink *<strong>&lt;L&gt;list_last</strong>(const struct &lt;L&gt;list *const <em>list</em>)</code>

 * Return:  
   A pointer to the last element of `list`, if it exists\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-93616d3b" name = "user-content-fn-93616d3b">&lt;L&gt;list_previous</a> ###

<code>static struct &lt;L&gt;listlink *<strong>&lt;L&gt;list_previous</strong>(struct &lt;L&gt;listlink *<em>link</em>)</code>

 * Return:  
   The previous element\. When `link` is the first element, returns null\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-40b28b9b" name = "user-content-fn-40b28b9b">&lt;L&gt;list_next</a> ###

<code>static struct &lt;L&gt;listlink *<strong>&lt;L&gt;list_next</strong>(struct &lt;L&gt;listlink *<em>link</em>)</code>

 * Return:  
   The next element\. When `link` is the last element, returns null\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-f965b937" name = "user-content-fn-f965b937">&lt;L&gt;list_clear</a> ###

<code>static void <strong>&lt;L&gt;list_clear</strong>(struct &lt;L&gt;list *const <em>list</em>)</code>

Clears and initializes `list`\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-b26385b9" name = "user-content-fn-b26385b9">&lt;L&gt;list_add_before</a> ###

<code>static void <strong>&lt;L&gt;list_add_before</strong>(struct &lt;L&gt;listlink *const <em>anchor</em>, struct &lt;L&gt;listlink *const <em>add</em>)</code>

`add` before `anchor`\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-fee45e54" name = "user-content-fn-fee45e54">&lt;L&gt;list_add_after</a> ###

<code>static void <strong>&lt;L&gt;list_add_after</strong>(struct &lt;L&gt;listlink *const <em>anchor</em>, struct &lt;L&gt;listlink *const <em>add</em>)</code>

`add` after `anchor`\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-bb35ae87" name = "user-content-fn-bb35ae87">&lt;L&gt;list_unshift</a> ###

<code>static void <strong>&lt;L&gt;list_unshift</strong>(struct &lt;L&gt;list *const <em>list</em>, struct &lt;L&gt;listlink *const <em>add</em>)</code>

Adds `add` to the beginning of `list`\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-8dc506e" name = "user-content-fn-8dc506e">&lt;L&gt;list_push</a> ###

<code>static void <strong>&lt;L&gt;list_push</strong>(struct &lt;L&gt;list *const <em>list</em>, struct &lt;L&gt;listlink *const <em>add</em>)</code>

Adds `add` to the end of `list`\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-dfcb43e2" name = "user-content-fn-dfcb43e2">&lt;L&gt;list_remove</a> ###

<code>static void <strong>&lt;L&gt;list_remove</strong>(struct &lt;L&gt;listlink *const <em>node</em>)</code>

Remove `node`\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-1c88ead6" name = "user-content-fn-1c88ead6">&lt;L&gt;list_shift</a> ###

<code>static struct &lt;L&gt;listlink *<strong>&lt;L&gt;list_shift</strong>(struct &lt;L&gt;list *const <em>list</em>)</code>

Removes the first element of `list` and returns it, if any\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-aeb1eac5" name = "user-content-fn-aeb1eac5">&lt;L&gt;list_pop</a> ###

<code>static struct &lt;L&gt;listlink *<strong>&lt;L&gt;list_pop</strong>(struct &lt;L&gt;list *const <em>list</em>)</code>

Removes the last element of `list` and returns it, if any\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-1171e3b" name = "user-content-fn-1171e3b">&lt;L&gt;list_to</a> ###

<code>static void <strong>&lt;L&gt;list_to</strong>(struct &lt;L&gt;list *const <em>from</em>, struct &lt;L&gt;list *const <em>to</em>)</code>

Moves the elements `from` onto `to` at the end\.

 * Parameter: _to_  
   If null, then it removes elements from `from`\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-727db3d7" name = "user-content-fn-727db3d7">&lt;L&gt;list_to_before</a> ###

<code>static void <strong>&lt;L&gt;list_to_before</strong>(struct &lt;L&gt;list *const <em>from</em>, struct &lt;L&gt;listlink *const <em>anchor</em>)</code>

Moves the elements `from` immediately before `anchor`\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-90de573b" name = "user-content-fn-90de573b">&lt;L&gt;list_to_if</a> ###

<code>static void <strong>&lt;L&gt;list_to_if</strong>(struct &lt;L&gt;list *const <em>from</em>, struct &lt;L&gt;list *const <em>to</em>, const &lt;PL&gt;predicate_fn <em>predicate</em>)</code>

Moves all elements `from` onto `to` at the end if `predicate` is true\.

 * Parameter: _to_  
   If null, then it removes elements\.
 * Order:  
   &#920;\(|`from`|\) &#215; &#927;\(`predicate`\)




### <a id = "user-content-fn-42fb011b" name = "user-content-fn-42fb011b">&lt;L&gt;list_for_each</a> ###

<code>static void <strong>&lt;L&gt;list_for_each</strong>(struct &lt;L&gt;list *const <em>list</em>, const &lt;PL&gt;action_fn <em>action</em>)</code>

Performs `action` for each element in `list` in order\.

 * Parameter: _action_  
   Can be to delete the element\.
 * Order:  
   &#920;\(|`list`|\) &#215; O\(`action`\)




### <a id = "user-content-fn-48bee118" name = "user-content-fn-48bee118">&lt;L&gt;list_any</a> ###

<code>static struct &lt;L&gt;listlink *<strong>&lt;L&gt;list_any</strong>(const struct &lt;L&gt;list *const <em>list</em>, const &lt;PL&gt;predicate_fn <em>predicate</em>)</code>

Iterates through `list` and calls `predicate` until it returns true\.

 * Return:  
   The first `predicate` that returned true, or, if the statement is false on all, null\.
 * Order:  
   &#927;\(|`list`|\) &#215; &#927;\(`predicate`\)




### <a id = "user-content-fn-1ce0c229" name = "user-content-fn-1ce0c229">&lt;L&gt;list_self_correct</a> ###

<code>static void <strong>&lt;L&gt;list_self_correct</strong>(struct &lt;L&gt;list *const <em>list</em>)</code>

Corrects `list` ends to compensate for memory relocation of the list itself\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-257babc6" name = "user-content-fn-257babc6">&lt;L&gt;list_sort</a> ###

<code>static void <strong>&lt;L&gt;list_sort</strong>(struct &lt;L&gt;list *const <em>list</em>)</code>

Performs a stable, adaptive sort of `list` according to `compare`\. Requires `LIST_COMPARE`\. [Peters 2002, Timsort](https://scholar.google.ca/scholar?q=Peters+2002%2C+Timsort), _via_ [McIlroy 1993, Optimistic](https://scholar.google.ca/scholar?q=McIlroy+1993%2C+Optimistic), does long merges by galloping, but we don't have random access to the data because we are in a linked\-list; this does natural merge sort\.

 * Order:  
   &#937;\(|`list`|\), &#927;\(|`list`| log |`list`|\)




### <a id = "user-content-fn-6df7d15d" name = "user-content-fn-6df7d15d">&lt;L&gt;list_compare</a> ###

<code>static int <strong>&lt;L&gt;list_compare</strong>(const struct &lt;L&gt;list *const <em>alist</em>, const struct &lt;L&gt;list *const <em>blist</em>)</code>

Compares `alist` to `blist` as sequences\.

 * Return:  
   The first `LIST_COMPARE` that is not equal to zero, or 0 if they are equal\. Null is considered as before everything else; two null pointers are considered equal\.
 * Implements:  
   [&lt;PL&gt;compare_fn](#user-content-typedef-a22f279f)
 * Order:  
   &#920;\(min\(|`alist`|, |`blist`|\)\)




### <a id = "user-content-fn-e9e9fc48" name = "user-content-fn-e9e9fc48">&lt;L&gt;list_duplicates_to</a> ###

<code>static void <strong>&lt;L&gt;list_duplicates_to</strong>(struct &lt;L&gt;list *const <em>from</em>, struct &lt;L&gt;list *const <em>to</em>)</code>

Moves all local\-duplicates of `from` to the end of `to`\.

For example, if `from` is `(A, B, B, A)`, it would concatenate `(B)` to `to` and leave `(A, B, A)` in `from`\. If one [&lt;L&gt;list_sort](#user-content-fn-257babc6) `from` first, `(A, A, B, B)`, the global duplicates will be transferred, `(A, B)`\.



 * Order:  
   &#927;\(|`from`|\)




### <a id = "user-content-fn-2d73ee58" name = "user-content-fn-2d73ee58">&lt;L&gt;list_subtraction_to</a> ###

<code>static void <strong>&lt;L&gt;list_subtraction_to</strong>(struct &lt;L&gt;list *const <em>a</em>, struct &lt;L&gt;list *const <em>b</em>, struct &lt;L&gt;list *const <em>result</em>)</code>

Subtracts `a` from `b`, as sequential sorted individual elements, and moves it to `result`\. All elements are removed from `a`\. All parameters must be unique or can be null\.

For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then `(a:A, a:D)` would be moved to `result`\.



 * Order:  
   &#927;\(|`a`| \+ |`b`|\)


### <a id = "user-content-fn-2eaa814b" name = "user-content-fn-2eaa814b">&lt;L&gt;list_union_to</a> ###

<code>static void <strong>&lt;L&gt;list_union_to</strong>(struct &lt;L&gt;list *const <em>a</em>, struct &lt;L&gt;list *const <em>b</em>, struct &lt;L&gt;list *const <em>result</em>)</code>

Moves the union of `a` and `b` as sequential sorted individual elements to `result`\. Equal elements are moved from `a`\. All parameters must be unique or can be null\.

For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then `(a:A, a:B, b:C, a:D)` would be moved to `result`\.



 * Order:  
   &#927;\(|`a`| \+ |`b`|\)


### <a id = "user-content-fn-2cced3a5" name = "user-content-fn-2cced3a5">&lt;L&gt;list_intersection_to</a> ###

<code>static void <strong>&lt;L&gt;list_intersection_to</strong>(struct &lt;L&gt;list *const <em>a</em>, struct &lt;L&gt;list *const <em>b</em>, struct &lt;L&gt;list *const <em>result</em>)</code>

Moves the intersection of `a` and `b` as sequential sorted individual elements to `result`\. Equal elements are moved from `a`\. All parameters must be unique or can be null\.

For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then `(a:B)` would be moved to `result`\.



 * Order:  
   &#927;\(|`a`| \+ |`b`|\)


### <a id = "user-content-fn-51bed9c1" name = "user-content-fn-51bed9c1">&lt;L&gt;list_xor_to</a> ###

<code>static void <strong>&lt;L&gt;list_xor_to</strong>(struct &lt;L&gt;list *const <em>a</em>, struct &lt;L&gt;list *const <em>b</em>, struct &lt;L&gt;list *const <em>result</em>)</code>

Moves `a` exclusive\-or `b` as sequential sorted individual elements to `result`\. Equal elements are moved from `a`\. All parameters must be unique or can be null\.

For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then `(a:A, b:C, a:D)` would be moved to `result`\.



 * Order:  
   O\(|`a`| \+ |`b`|\)


### <a id = "user-content-fn-b11709d3" name = "user-content-fn-b11709d3">&lt;SZ&gt;to_string</a> ###

<code>static const char *<strong>&lt;SZ&gt;to_string</strong>(const &lt;PSZ&gt;box *const <em>box</em>)</code>

 * Return:  
   Print the contents of [&lt;PSZ&gt;box](#user-content-typedef-ace240bb) `box` in a static string buffer of 256 bytes with limitations of only printing 4 things at a time\.
 * Order:  
   &#920;\(1\)




## <a id = "user-content-license" name = "user-content-license">License</a> ##

2017 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



