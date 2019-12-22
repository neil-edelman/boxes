# List\.h #

## Parameterised Closed Doubly\-Linked List ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PL&gt;ToString](#user-content-typedef-10241527), [&lt;PL&gt;Action](#user-content-typedef-40925d79), [&lt;PL&gt;Predicate](#user-content-typedef-6fadfe58), [&lt;PL&gt;Compare](#user-content-typedef-c749dcca)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;L&gt;ListLink](#user-content-tag-16c786c1), [&lt;L&gt;List](#user-content-tag-68dde1fd)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

[&lt;L&gt;List](#user-content-tag-68dde1fd) is a list of [&lt;L&gt;ListLink](#user-content-tag-16c786c1); it may be supplied a total\-order function, `LIST_COMPARE` [&lt;PL&gt;Compare](#user-content-typedef-c749dcca)\.

Internally, `<L>ListLink` is a doubly\-linked node with sentinels residing in `<L>List`\. It only provides an order, but `<L>ListLink` may be enclosed in another `struct`\. While in a list, adding to another list destroys the integrity of the original list, see [&lt;L&gt;ListRemove](#user-content-fn-86ea1635)\.

`<L>Link` is not synchronised\. Errors are returned with `errno`\. The parameters are `#define` preprocessor macros, and are all undefined at the end of the file for convenience\. To stop assertions, use `#define NDEBUG` before inclusion of `assert.h`, \(which is used in this file\.\)



 * Parameter: LIST\_NAME  
   `<L>` that satisfies `C` naming conventions when mangled; required\. `<PL>` is private, whose names are prefixed in a manner to avoid collisions; any should be re\-defined prior to use elsewhere\.
 * Parameter: LIST\_COMPARE  
   Optional total\-order function satisfying [&lt;PL&gt;Compare](#user-content-typedef-c749dcca)\.
 * Parameter: LIST\_TO\_STRING  
   Optional print function implementing [&lt;PL&gt;ToString](#user-content-typedef-10241527); makes available [&lt;L&gt;ListToString](#user-content-fn-cb2d798d)\.
 * Parameter: LIST\_TEST  
   Unit testing framework [&lt;L&gt;ListTest](#user-content-fn-cbfce6bd), included in a separate header, [\.\./test/TestList\.h](../test/TestList.h)\. Must be defined equal to a random filler function, satisfying [&lt;PL&gt;Action](#user-content-typedef-40925d79)\. Requires `LIST_TO_STRING` and not `NDEBUG`\.
 * Standard:  
   C89
 * See also:  
   [Array](https://github.com/neil-edelman/Array); [Orcish](https://github.com/neil-edelman/Orcish); [Pool](https://github.com/neil-edelman/Pool); [Set](https://github.com/neil-edelman/Set)


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-10241527" name = "user-content-typedef-10241527">&lt;PL&gt;ToString</a> ###

<code>typedef void(*<strong>&lt;PL&gt;ToString</strong>)(const struct &lt;L&gt;ListLink *, char(*)[12]);</code>

Responsible for turning [&lt;L&gt;ListLink](#user-content-tag-16c786c1) \(the first argument\) into a maximum 11\-`char` string \(the second\.\)



### <a id = "user-content-typedef-40925d79" name = "user-content-typedef-40925d79">&lt;PL&gt;Action</a> ###

<code>typedef void(*<strong>&lt;PL&gt;Action</strong>)(struct &lt;L&gt;ListLink *);</code>

Operates by side\-effects on the link\.



### <a id = "user-content-typedef-6fadfe58" name = "user-content-typedef-6fadfe58">&lt;PL&gt;Predicate</a> ###

<code>typedef int(*<strong>&lt;PL&gt;Predicate</strong>)(const struct &lt;L&gt;ListLink *);</code>

Returns \(Non\-zero\) true or \(zero\) false when given a link\.



### <a id = "user-content-typedef-c749dcca" name = "user-content-typedef-c749dcca">&lt;PL&gt;Compare</a> ###

<code>typedef int(*<strong>&lt;PL&gt;Compare</strong>)(const struct &lt;L&gt;ListLink *a, const struct &lt;L&gt;ListLink *b);</code>

Returns less then, equal to, or greater then zero, forming an equivalence relation between `a` as compared to `b`\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-16c786c1" name = "user-content-tag-16c786c1">&lt;L&gt;ListLink</a> ###

<code>struct <strong>&lt;L&gt;ListLink</strong>;</code>

Storage of this structure is the responsibility of the caller\.



### <a id = "user-content-tag-68dde1fd" name = "user-content-tag-68dde1fd">&lt;L&gt;List</a> ###

<code>struct <strong>&lt;L&gt;List</strong>;</code>

Serves as head and tail for linked\-list of [&lt;L&gt;ListLink](#user-content-tag-16c786c1)\. Use [&lt;L&gt;ListClear](#user-content-fn-5a31ef1a) or statically initialise using the macro `LIST_IDLE(<list>)`\. Because this list is closed; that is, given a valid pointer to an element, one can determine all others, null values are not allowed and it is _not_ the same as `{0}`\.



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-5a31ef1a">&lt;L&gt;ListClear</a></td><td>list</td></tr>

<tr><td align = right>static struct &lt;L&gt;ListLink *</td><td><a href = "#user-content-fn-99a48f99">&lt;L&gt;ListFirst</a></td><td>list</td></tr>

<tr><td align = right>static struct &lt;L&gt;ListLink *</td><td><a href = "#user-content-fn-aeda30a1">&lt;L&gt;ListLast</a></td><td>list</td></tr>

<tr><td align = right>static struct &lt;L&gt;ListLink *</td><td><a href = "#user-content-fn-1f929a80">&lt;L&gt;ListPrevious</a></td><td>link</td></tr>

<tr><td align = right>static struct &lt;L&gt;ListLink *</td><td><a href = "#user-content-fn-87d43080">&lt;L&gt;ListNext</a></td><td>link</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-86be42ba">&lt;L&gt;ListUnshift</a></td><td>list, add</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-f851aa45">&lt;L&gt;ListPush</a></td><td>list, add</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-8f34cfa7">&lt;L&gt;ListAddBefore</a></td><td>anchor, add</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-efcf7496">&lt;L&gt;ListAddAfter</a></td><td>anchor, add</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-86ea1635">&lt;L&gt;ListRemove</a></td><td>link</td></tr>

<tr><td align = right>static struct &lt;L&gt;ListLink *</td><td><a href = "#user-content-fn-5fc0880f">&lt;L&gt;ListShift</a></td><td>list</td></tr>

<tr><td align = right>static struct &lt;L&gt;ListLink *</td><td><a href = "#user-content-fn-1aee8d88">&lt;L&gt;ListPop</a></td><td>list</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-686e23e">&lt;L&gt;ListTake</a></td><td>list, from</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-422d6349">&lt;L&gt;ListTakeIf</a></td><td>list, from, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-75eb7639">&lt;L&gt;ListTakeBefore</a></td><td>anchor, from</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-b6bffcc1">&lt;L&gt;ListForEach</a></td><td>list, action</td></tr>

<tr><td align = right>static struct &lt;L&gt;ListLink *</td><td><a href = "#user-content-fn-80e00435">&lt;L&gt;ListAny</a></td><td>list, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-baec140b">&lt;L&gt;ListSelfCorrect</a></td><td>list</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-49acd799">&lt;L&gt;ListSort</a></td><td>list</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-cd5530ff">&lt;L&gt;ListMerge</a></td><td>list, from</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-a794b6dc">&lt;L&gt;ListCompare</a></td><td>alist, blist</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-ca63a904">&lt;L&gt;ListTakeSubtraction</a></td><td>list, a, b</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-9e709f35">&lt;L&gt;ListTakeUnion</a></td><td>list, a, b</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-fcd532e3">&lt;L&gt;ListTakeIntersection</a></td><td>list, a, b</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-267eb36f">&lt;L&gt;ListTakeXor</a></td><td>list, a, b</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-cb2d798d">&lt;L&gt;ListToString</a></td><td>list</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-cbfce6bd">&lt;L&gt;ListTest</a></td><td>parent_new, parent</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-5a31ef1a" name = "user-content-fn-5a31ef1a">&lt;L&gt;ListClear</a> ###

<code>static void <strong>&lt;L&gt;ListClear</strong>(struct &lt;L&gt;List *const <em>list</em>)</code>

Clears and removes all values from `list`, thereby initialising it\. All previous values are un\-associated\.

 * Parameter: _list_  
   if null, does nothing\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-99a48f99" name = "user-content-fn-99a48f99">&lt;L&gt;ListFirst</a> ###

<code>static struct &lt;L&gt;ListLink *<strong>&lt;L&gt;ListFirst</strong>(const struct &lt;L&gt;List *const <em>list</em>)</code>

 * Parameter: _list_  
   If null, returns null\.
 * Return:  
   A pointer to the first element of `list`, if it exists\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-aeda30a1" name = "user-content-fn-aeda30a1">&lt;L&gt;ListLast</a> ###

<code>static struct &lt;L&gt;ListLink *<strong>&lt;L&gt;ListLast</strong>(const struct &lt;L&gt;List *const <em>list</em>)</code>

 * Parameter: _list_  
   If null, returns null\.
 * Return:  
   A pointer to the last element of `list`, if it exists\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-1f929a80" name = "user-content-fn-1f929a80">&lt;L&gt;ListPrevious</a> ###

<code>static struct &lt;L&gt;ListLink *<strong>&lt;L&gt;ListPrevious</strong>(struct &lt;L&gt;ListLink *<em>link</em>)</code>

 * Parameter: _link_  
   If null, returns null, otherwise must be part of a list\.
 * Return:  
   The previous element\. When `link` is the first element, returns null\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-87d43080" name = "user-content-fn-87d43080">&lt;L&gt;ListNext</a> ###

<code>static struct &lt;L&gt;ListLink *<strong>&lt;L&gt;ListNext</strong>(struct &lt;L&gt;ListLink *<em>link</em>)</code>

 * Parameter: _link_  
   If null, returns null, otherwise must be part of a list\.
 * Return:  
   The next element\. When `link` is the last element, returns null\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-86be42ba" name = "user-content-fn-86be42ba">&lt;L&gt;ListUnshift</a> ###

<code>static void <strong>&lt;L&gt;ListUnshift</strong>(struct &lt;L&gt;List *const <em>list</em>, struct &lt;L&gt;ListLink *const <em>add</em>)</code>

Adds `add` to the beginning of `list`\.

 * Parameter: _list_  
   If null, does nothing\.
 * Parameter: _add_  
   If null, does nothing\. Should not associated to any list\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-f851aa45" name = "user-content-fn-f851aa45">&lt;L&gt;ListPush</a> ###

<code>static void <strong>&lt;L&gt;ListPush</strong>(struct &lt;L&gt;List *const <em>list</em>, struct &lt;L&gt;ListLink *const <em>add</em>)</code>

Adds `add` to the end of `list`\.

 * Parameter: _list_  
   If null, does nothing\.
 * Parameter: _add_  
   If null, does nothing\. Should not associated to any list\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-8f34cfa7" name = "user-content-fn-8f34cfa7">&lt;L&gt;ListAddBefore</a> ###

<code>static void <strong>&lt;L&gt;ListAddBefore</strong>(struct &lt;L&gt;ListLink *const <em>anchor</em>, struct &lt;L&gt;ListLink *const <em>add</em>)</code>

Adds `add` immediately before `anchor`\.

 * Parameter: _anchor_  
   If null, does nothing\. Must be part of a list\.
 * Parameter: _add_  
   If null, does nothing\. Should not be part of any list\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-efcf7496" name = "user-content-fn-efcf7496">&lt;L&gt;ListAddAfter</a> ###

<code>static void <strong>&lt;L&gt;ListAddAfter</strong>(struct &lt;L&gt;ListLink *const <em>anchor</em>, struct &lt;L&gt;ListLink *const <em>add</em>)</code>

Adds `add` immediately after `anchor`\.

 * Parameter: _anchor_  
   If null, does nothing\. Must be part of a list\.
 * Parameter: _add_  
   If null, does nothing\. Should not be part of any list\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-86ea1635" name = "user-content-fn-86ea1635">&lt;L&gt;ListRemove</a> ###

<code>static void <strong>&lt;L&gt;ListRemove</strong>(struct &lt;L&gt;ListLink *const <em>link</em>)</code>

Un\-associates `link` from the list; consequently, the `link` is free to add to another list\. Removing an element that was not added to a list results in undefined behaviour\.

 * Parameter: _link_  
   If null, does nothing\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-5fc0880f" name = "user-content-fn-5fc0880f">&lt;L&gt;ListShift</a> ###

<code>static struct &lt;L&gt;ListLink *<strong>&lt;L&gt;ListShift</strong>(struct &lt;L&gt;List *const <em>list</em>)</code>

Un\-associates the first element of `list`\.

 * Parameter: _list_  
   If null, returns null\.
 * Return:  
   The erstwhile first element or null if the list was empty\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-1aee8d88" name = "user-content-fn-1aee8d88">&lt;L&gt;ListPop</a> ###

<code>static struct &lt;L&gt;ListLink *<strong>&lt;L&gt;ListPop</strong>(struct &lt;L&gt;List *const <em>list</em>)</code>

Un\-associates the last element of `list`\.

 * Parameter: _list_  
   If null, returns null\.
 * Return:  
   The erstwhile last element or null if the list was empty\.




### <a id = "user-content-fn-686e23e" name = "user-content-fn-686e23e">&lt;L&gt;ListTake</a> ###

<code>static void <strong>&lt;L&gt;ListTake</strong>(struct &lt;L&gt;List *const <em>list</em>, struct &lt;L&gt;List *const <em>from</em>)</code>

Moves the elements `from` onto `list` at the end\.

 * Parameter: _list_  
   If null, then it removes elements from `from`\.
 * Parameter: _from_  
   If null, it does nothing, otherwise this list will be empty on return\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-422d6349" name = "user-content-fn-422d6349">&lt;L&gt;ListTakeIf</a> ###

<code>static void <strong>&lt;L&gt;ListTakeIf</strong>(struct &lt;L&gt;List *const <em>list</em>, struct &lt;L&gt;List *const <em>from</em>, const &lt;PL&gt;Predicate <em>predicate</em>)</code>

Moves all elements `from` onto `list` at the end if `predicate` is null or true\.

 * Parameter: _list_  
   If null, then it removes elements\.
 * Parameter: _from_  
   If null, does nothing\.
 * Order:  
   &#920;\(|`list`| &#215; &#927;\(`predicate`\)




### <a id = "user-content-fn-75eb7639" name = "user-content-fn-75eb7639">&lt;L&gt;ListTakeBefore</a> ###

<code>static void <strong>&lt;L&gt;ListTakeBefore</strong>(struct &lt;L&gt;ListLink *const <em>anchor</em>, struct &lt;L&gt;List *const <em>from</em>)</code>

Moves the elements `from` immediately before `anchor`\.

 * Parameter: _anchor_  
   If null, does nothing\. Must be part of a vild list\.
 * Parameter: _from_  
   If null, does nothing\. This list will be empty on return\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-b6bffcc1" name = "user-content-fn-b6bffcc1">&lt;L&gt;ListForEach</a> ###

<code>static void <strong>&lt;L&gt;ListForEach</strong>(struct &lt;L&gt;List *const <em>list</em>, const &lt;PL&gt;Action <em>action</em>)</code>

Performs `action` for each element in `list` in order\. `action` can be to delete the element\.

 * Parameter: _list_  
   If null, does nothing\.
 * Parameter: _action_  
   If null, does nothing\.
 * Order:  
   &#920;\(|`list`|\) &#215; O\(`action`\)




### <a id = "user-content-fn-80e00435" name = "user-content-fn-80e00435">&lt;L&gt;ListAny</a> ###

<code>static struct &lt;L&gt;ListLink *<strong>&lt;L&gt;ListAny</strong>(const struct &lt;L&gt;List *const <em>list</em>, const &lt;PL&gt;Predicate <em>predicate</em>)</code>

Iterates through `list` and calls `predicate` until it returns true\.

 * Parameter: _list_  
   If null, returns null\.
 * Parameter: _predicate_  
   If null, returns null\.
 * Return:  
   The first `predicate` that returned true, or, if the statement is false on all, null\.
 * Order:  
   &#927;\(|`list`| &#215; `predicate`\)




### <a id = "user-content-fn-baec140b" name = "user-content-fn-baec140b">&lt;L&gt;ListSelfCorrect</a> ###

<code>static void <strong>&lt;L&gt;ListSelfCorrect</strong>(struct &lt;L&gt;List *const <em>list</em>)</code>

Usually [&lt;L&gt;List](#user-content-tag-68dde1fd) doesn't change memory locations, but when it does, this corrects `list`'s two ends, \(not the nodes, which must be fixed\.\) Note that the two ends become invalid even when it's empty\.

 * Parameter: _list_  
   If null, does nothing\.
 * Order:  
   &#927;\(1\)




### <a id = "user-content-fn-49acd799" name = "user-content-fn-49acd799">&lt;L&gt;ListSort</a> ###

<code>static void <strong>&lt;L&gt;ListSort</strong>(struct &lt;L&gt;List *const <em>list</em>)</code>

Performs a stable, adaptive sort of `list` according to `compare`\. Requires `LIST_COMPARE`\. This does natural merge sort; <Peters 2002, Timsort>, _via_ <McIlroy 1993, Optimistic>, does long merges by galloping, but we don't have random access to the data because we are in a linked\-list\.

 * Parameter: _list_  
   If null, does nothing\.
 * Order:  
   &#937;\(|`list`|\), &#927;\(|`list`| log |`list`|\)




### <a id = "user-content-fn-cd5530ff" name = "user-content-fn-cd5530ff">&lt;L&gt;ListMerge</a> ###

<code>static void <strong>&lt;L&gt;ListMerge</strong>(struct &lt;L&gt;List *const <em>list</em>, struct &lt;L&gt;List *const <em>from</em>)</code>

Merges from `from` into `list` according to `compare`\. If the elements are sorted in both lists, \(see [&lt;L&gt;ListSort](#user-content-fn-49acd799),\) then the elements of `list` will be sorted, too\. Requires `LIST_COMPARE`\.

 * Parameter: _list_  
   If null, then it removes elements\.
 * Parameter: _from_  
   If null, does nothing, otherwise this list will be empty on return\.
 * Order:  
   &#927;\(|`list`| \+ |`from`|\)




### <a id = "user-content-fn-a794b6dc" name = "user-content-fn-a794b6dc">&lt;L&gt;ListCompare</a> ###

<code>static int <strong>&lt;L&gt;ListCompare</strong>(const struct &lt;L&gt;List *const <em>alist</em>, const struct &lt;L&gt;List *const <em>blist</em>)</code>

Compares `alist` to `blist` as sequences\. Requires `LIST_COMPARE`\.

 * Return:  
   The first `LIST_COMPARE` that is not equal to zero, or 0 if they are equal\. Null is considered as before everything else; two null pointers are considered equal\.
 * Implements:  
   [&lt;PL&gt;Compare](#user-content-typedef-c749dcca) as <<PL>List>Compare
 * Order:  
   &#920;\(min\(|`alist`|, |`blist`|\)\)




### <a id = "user-content-fn-ca63a904" name = "user-content-fn-ca63a904">&lt;L&gt;ListTakeSubtraction</a> ###

<code>static void <strong>&lt;L&gt;ListTakeSubtraction</strong>(struct &lt;L&gt;List *const <em>list</em>, struct &lt;L&gt;List *const <em>a</em>, struct &lt;L&gt;List *const <em>b</em>)</code>

Appends `list` with `b` subtracted from `a`\. Requires `LIST_COMPARE`\.

 * Parameter: _list_  
   If null, then it removes elements\.
 * Parameter: _a_  
   Sorted lists\.
 * Parameter: _b_  
   Sorted lists\.
 * Order:  
   &#927;\(|`a`| \+ |`b`|\)




### <a id = "user-content-fn-9e709f35" name = "user-content-fn-9e709f35">&lt;L&gt;ListTakeUnion</a> ###

<code>static void <strong>&lt;L&gt;ListTakeUnion</strong>(struct &lt;L&gt;List *const <em>list</em>, struct &lt;L&gt;List *const <em>a</em>, struct &lt;L&gt;List *const <em>b</em>)</code>

Appends `list` with the union of `a` and `b`\. Equal elements are moved from `a`\. Requires `LIST_COMPARE`\.

 * Parameter: _list_  
   If null, then it removes elements\.
 * Parameter: _a_  
   Sorted lists\.
 * Parameter: _b_  
   Sorted lists\.
 * Order:  
   &#927;\(|`a`| \+ |`b`|\)




### <a id = "user-content-fn-fcd532e3" name = "user-content-fn-fcd532e3">&lt;L&gt;ListTakeIntersection</a> ###

<code>static void <strong>&lt;L&gt;ListTakeIntersection</strong>(struct &lt;L&gt;List *const <em>list</em>, struct &lt;L&gt;List *const <em>a</em>, struct &lt;L&gt;List *const <em>b</em>)</code>

Appends `list` with the intersection of `a` and `b`\. Equal elements are moved from `a`\. Requires `LIST_COMPARE`\.

 * Parameter: _list_  
   If null, then it removes elements\.
 * Parameter: _a_  
   Sorted lists\.
 * Parameter: _b_  
   Sorted lists\.
 * Order:  
   &#927;\(|`a`| \+ |`b`|\)




### <a id = "user-content-fn-267eb36f" name = "user-content-fn-267eb36f">&lt;L&gt;ListTakeXor</a> ###

<code>static void <strong>&lt;L&gt;ListTakeXor</strong>(struct &lt;L&gt;List *const <em>list</em>, struct &lt;L&gt;List *const <em>a</em>, struct &lt;L&gt;List *const <em>b</em>)</code>

Appends `list` with `a` exclusive\-or `b`\. Equal elements are moved from `a`\. Requires `LIST_COMPARE`\.

 * Parameter: _list_  
   If null, then it removes elements\.
 * Parameter: _a_  
   Sorted lists\.
 * Parameter: _b_  
   Sorted lists\.
 * Order:  
   O\(|`a`| \+ |`b`|\)




### <a id = "user-content-fn-cb2d798d" name = "user-content-fn-cb2d798d">&lt;L&gt;ListToString</a> ###

<code>static const char *<strong>&lt;L&gt;ListToString</strong>(const struct &lt;L&gt;List *const <em>list</em>)</code>

Can print 2 things at once before it overwrites\. One must set `LIST_TO_STRING` to a function implementing [&lt;PL&gt;ToString](#user-content-typedef-10241527) to get this functionality\.

 * Return:  
   Prints `list` in a static buffer\.
 * Order:  
   &#920;\(1\); it has a 1024 character limit; every element takes some\.




### <a id = "user-content-fn-cbfce6bd" name = "user-content-fn-cbfce6bd">&lt;L&gt;ListTest</a> ###

<code>static void <strong>&lt;L&gt;ListTest</strong>(struct &lt;L&gt;ListLink *(*const <em>parent_new</em>)(void *), void *const <em>parent</em>)</code>

The linked\-list will be tested on stdout\. `LIST_TEST` has to be set\.





## <a id = "user-content-license" name = "user-content-license">License</a> ##

2017 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



