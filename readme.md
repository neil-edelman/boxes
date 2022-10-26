# list\.h #

Stand\-alone header [src/list\.h](src/list.h); examples [test/test\_list\.c](test/test_list.c); on a compatible workstation, `make` creates the test suite of the examples\.

## Doubly\-linked component ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PITR&gt;action_fn](#user-content-typedef-49d9168b), [&lt;PITR&gt;predicate_fn](#user-content-typedef-c5016dba), [&lt;PSTR&gt;to_string_fn](#user-content-typedef-8a8349ca), [&lt;PCMP&gt;bipredicate_fn](#user-content-typedef-82edbc04), [&lt;PCMP&gt;compare_fn](#user-content-typedef-2c6ed2db), [&lt;PCMP&gt;biaction_fn](#user-content-typedef-f8efb17d)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;L&gt;listlink](#user-content-tag-15769e01), [&lt;L&gt;list](#user-content-tag-eb84971d)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of a stochastic skip-list.](doc/list.png)

In parlance of [Thareja 2014, Structures](https://scholar.google.ca/scholar?q=Thareja+2014%2C+Structures), [&lt;L&gt;list](#user-content-tag-eb84971d) is a circular header, or sentinel, to a doubly\-linked list of [&lt;L&gt;listlink](#user-content-tag-15769e01)\. This is a closed structure, such that with with a pointer to any element, it is possible to extract the entire list\. The links will be generally in a larger container type\.

[src/iterate\.h](src/iterate.h): defining `HAVE_ITERATE_H` supplies `<ITR>` functions for all boxes that support them\. Is not a trait, adds a fixed amount of functions for all boxes\.[src/to\_string\.h](src/to_string.h): `<STR>` trait functions require `<name>[<trait>]to_string` be declared as [&lt;PSTR&gt;to_string_fn](#user-content-typedef-8a8349ca)\.[src/compare\.h](src/compare.h): `<CMP>` trait functions require `<name>[<trait>]compare` to be declared as [&lt;PCMP&gt;compare_fn](#user-content-typedef-2c6ed2db) or `<name>[<trait>]is_equal` to be declared as [&lt;PCMP&gt;bipredicate_fn](#user-content-typedef-82edbc04), respectfully, \(but not both\.\)

 * Parameter: LIST\_NAME  
   `<L>` that satisfies `C` naming conventions when mangled; required\. `<PL>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: LIST\_COMPARE, LIST\_IS\_EQUAL  
   Compare trait contained in [src/compare\.h](src/compare.h)\.
 * Parameter: LIST\_TO\_STRING  
   To string trait contained in [src/to\_string\.h](src/to_string.h)\.
 * Parameter: LIST\_EXPECT\_TRAIT, LIST\_TRAIT  
   Named traits are obtained by including `array.h` multiple times with `LIST_EXPECT_TRAIT` and then subsequently including the name in `LIST_TRAIT`\.
 * Standard:  
   C89


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-49d9168b" name = "user-content-typedef-49d9168b">&lt;PITR&gt;action_fn</a> ###

<code>typedef void(*<strong>&lt;PITR&gt;action_fn</strong>)(&lt;PITR&gt;element);</code>

[src/iterate\.h](src/iterate.h): Operates by side\-effects\.



### <a id = "user-content-typedef-c5016dba" name = "user-content-typedef-c5016dba">&lt;PITR&gt;predicate_fn</a> ###

<code>typedef int(*<strong>&lt;PITR&gt;predicate_fn</strong>)(const &lt;PITR&gt;element_c);</code>

[src/iterate\.h](src/iterate.h): Returns a boolean given read\-only\.



### <a id = "user-content-typedef-8a8349ca" name = "user-content-typedef-8a8349ca">&lt;PSTR&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;PSTR&gt;to_string_fn</strong>)(const &lt;PSTR&gt;element, char(*)[12]);</code>

[src/to\_string\.h](src/to_string.h): responsible for turning the read\-only argument into a 12\-`char` null\-terminated output string\. The first argument should be a read\-only reference to an element and the second a pointer to the bytes\.



### <a id = "user-content-typedef-82edbc04" name = "user-content-typedef-82edbc04">&lt;PCMP&gt;bipredicate_fn</a> ###

<code>typedef int(*<strong>&lt;PCMP&gt;bipredicate_fn</strong>)(&lt;PCMP&gt;element_c restrict, &lt;PCMP&gt;element_c restrict);</code>

[src/compare\.h](src/compare.h): Returns a boolean given two read\-only elements\.



### <a id = "user-content-typedef-2c6ed2db" name = "user-content-typedef-2c6ed2db">&lt;PCMP&gt;compare_fn</a> ###

<code>typedef int(*<strong>&lt;PCMP&gt;compare_fn</strong>)(const &lt;PCMP&gt;element_c restrict a, const &lt;PCMP&gt;element_c restrict b);</code>

[src/compare\.h](src/compare.h): Three\-way comparison on a totally order set; returns an integer value less than, equal to, greater than zero, if `a < b`, `a == b`, `a > b`, respectively\.



### <a id = "user-content-typedef-f8efb17d" name = "user-content-typedef-f8efb17d">&lt;PCMP&gt;biaction_fn</a> ###

<code>typedef int(*<strong>&lt;PCMP&gt;biaction_fn</strong>)(&lt;PCMP&gt;element restrict, &lt;PCMP&gt;element restrict);</code>

[src/compare\.h](src/compare.h): Returns a boolean given two modifiable arguments\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-15769e01" name = "user-content-tag-15769e01">&lt;L&gt;listlink</a> ###

<code>struct <strong>&lt;L&gt;listlink</strong> { struct &lt;L&gt;listlink *next, *prev; };</code>

Storage of this structure is the responsibility of the caller, who must provide a stable pointer while in a list\. Generally, one encloses this in a host `struct` or `union`\.

![States.](doc/node-states.png)



### <a id = "user-content-tag-eb84971d" name = "user-content-tag-eb84971d">&lt;L&gt;list</a> ###

<code>struct <strong>&lt;L&gt;list</strong>;</code>

Serves as head and tail sentinel for a linked\-list of [&lt;L&gt;listlink](#user-content-tag-15769e01)\.

![States.](doc/states.png)



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-f965b937">&lt;L&gt;list_clear</a></td><td>list</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-b26385b9">&lt;L&gt;list_add_before</a></td><td>anchor, add</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-fee45e54">&lt;L&gt;list_add_after</a></td><td>anchor, add</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-8dc506e">&lt;L&gt;list_push</a></td><td>list, add</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-bb35ae87">&lt;L&gt;list_unshift</a></td><td>list, add</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-dfcb43e2">&lt;L&gt;list_remove</a></td><td>node</td></tr>

<tr><td align = right>static struct &lt;L&gt;listlink *</td><td><a href = "#user-content-fn-1c88ead6">&lt;L&gt;list_shift</a></td><td>list</td></tr>

<tr><td align = right>static struct &lt;L&gt;listlink *</td><td><a href = "#user-content-fn-aeb1eac5">&lt;L&gt;list_pop</a></td><td>list</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-1171e3b">&lt;L&gt;list_to</a></td><td>from, to</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-727db3d7">&lt;L&gt;list_to_before</a></td><td>from, anchor</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-1ce0c229">&lt;L&gt;list_self_correct</a></td><td>list</td></tr>

<tr><td align = right>static &lt;PITR&gt;element</td><td><a href = "#user-content-fn-73c52918">&lt;ITR&gt;any</a></td><td>box, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-96abfbdb">&lt;ITR&gt;each</a></td><td>box, action</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-6e4cf157">&lt;ITR&gt;if_each</a></td><td>box, predicate, action</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-4b2c205b">&lt;ITR&gt;copy_if</a></td><td>dst, src, copy</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d816173b">&lt;ITR&gt;keep_if</a></td><td>box, keep, destruct</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-108e9df6">&lt;ITR&gt;trim</a></td><td>box, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-c4c1df3b">&lt;ITR&gt;to_if</a></td><td>from, to, predicate</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-751c6337">&lt;STR&gt;to_string</a></td><td>box</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-c2fff878">&lt;CMP&gt;compare</a></td><td>a, b</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-620cbec1">&lt;CMP&gt;lower_bound</a></td><td>box, element</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-b6f29e84">&lt;CMP&gt;upper_bound</a></td><td>box, element</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-c57ffcf5">&lt;CMP&gt;insert_after</a></td><td>box, element</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-a7c44d35">&lt;CMP&gt;sort</a></td><td>box</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-f184f491">&lt;CMP&gt;reverse</a></td><td>box</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-82b7806">&lt;CMP&gt;is_equal</a></td><td>a, b</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-a9b0c375">&lt;CMP&gt;unique_merge</a></td><td>box, merge</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-52f3957a">&lt;CMP&gt;unique</a></td><td>box</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d9a682b3">&lt;CMP&gt;merge</a></td><td>to, from</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-a7c44d35">&lt;CMP&gt;sort</a></td><td>list</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-7d4d7583">&lt;CMP&gt;subtraction_to</a></td><td>a, b, result</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-54db3a7c">&lt;CMP&gt;union_to</a></td><td>a, b, result</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-ce5608a0">&lt;CMP&gt;intersection_to</a></td><td>a, b, result</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-33635302">&lt;CMP&gt;xor_to</a></td><td>a, b, result</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-8ff3b955">&lt;CMP&gt;duplicates_to</a></td><td>from, to</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-f965b937" name = "user-content-fn-f965b937">&lt;L&gt;list_clear</a> ###

<code>static void <strong>&lt;L&gt;list_clear</strong>(struct &lt;L&gt;list *const <em>list</em>)</code>

Clears and initializes `list`\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-b26385b9" name = "user-content-fn-b26385b9">&lt;L&gt;list_add_before</a> ###

<code>static void <strong>&lt;L&gt;list_add_before</strong>(struct &lt;L&gt;listlink *restrict const <em>anchor</em>, struct &lt;L&gt;listlink *restrict const <em>add</em>)</code>

`add` before `anchor`\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-fee45e54" name = "user-content-fn-fee45e54">&lt;L&gt;list_add_after</a> ###

<code>static void <strong>&lt;L&gt;list_add_after</strong>(struct &lt;L&gt;listlink *const <em>anchor</em>, struct &lt;L&gt;listlink *const <em>add</em>)</code>

`add` after `anchor`\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-8dc506e" name = "user-content-fn-8dc506e">&lt;L&gt;list_push</a> ###

<code>static void <strong>&lt;L&gt;list_push</strong>(struct &lt;L&gt;list *const <em>list</em>, struct &lt;L&gt;listlink *const <em>add</em>)</code>

Adds `add` to the end of `list`\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-bb35ae87" name = "user-content-fn-bb35ae87">&lt;L&gt;list_unshift</a> ###

<code>static void <strong>&lt;L&gt;list_unshift</strong>(struct &lt;L&gt;list *const <em>list</em>, struct &lt;L&gt;listlink *const <em>add</em>)</code>

Adds `add` to the beginning of `list`\.

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

<code>static void <strong>&lt;L&gt;list_to</strong>(struct &lt;L&gt;list *restrict const <em>from</em>, struct &lt;L&gt;list *restrict const <em>to</em>)</code>

Moves the elements `from` onto `to` at the end\.

 * Parameter: _to_  
   If null, then it removes elements from `from`\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-727db3d7" name = "user-content-fn-727db3d7">&lt;L&gt;list_to_before</a> ###

<code>static void <strong>&lt;L&gt;list_to_before</strong>(struct &lt;L&gt;list *restrict const <em>from</em>, struct &lt;L&gt;listlink *restrict const <em>anchor</em>)</code>

Moves the elements `from` immediately before `anchor`, which can not be in the same list\.

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-1ce0c229" name = "user-content-fn-1ce0c229">&lt;L&gt;list_self_correct</a> ###

<code>static void <strong>&lt;L&gt;list_self_correct</strong>(struct &lt;L&gt;list *const <em>list</em>)</code>

Corrects `list` ends to compensate for memory relocation of the list head itself\. \(Can only have one copy of the list, this will invalidate all other copies\.\)

 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-73c52918" name = "user-content-fn-73c52918">&lt;ITR&gt;any</a> ###

<code>static &lt;PITR&gt;element <strong>&lt;ITR&gt;any</strong>(const &lt;PITR&gt;box *const <em>box</em>, const &lt;PITR&gt;predicate_fn <em>predicate</em>)</code>

[src/iterate\.h](src/iterate.h): Iterates through `box` and calls `predicate` until it returns true\.

 * Return:  
   The first `predicate` that returned true, or, if the statement is false on all, null\.
 * Order:  
   &#927;\(`box.size`\) &#215; &#927;\(`predicate`\)




### <a id = "user-content-fn-96abfbdb" name = "user-content-fn-96abfbdb">&lt;ITR&gt;each</a> ###

<code>static void <strong>&lt;ITR&gt;each</strong>(&lt;PITR&gt;box *const <em>box</em>, const &lt;PITR&gt;action_fn <em>action</em>)</code>

[src/iterate\.h](src/iterate.h): Iterates through `box` and calls `action` on all the elements\.

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

[src/iterate\.h](src/iterate.h), `BOX_CONTIGUOUS`: For all elements of `box`, calls `keep`, and if false, lazy deletes that item\. Calls `destruct` if not\-null before deleting\.

 * Order:  
   &#927;\(|`box`|\) \(&#215; O\(`keep`\) \+ O\(`destruct`\)\)




### <a id = "user-content-fn-108e9df6" name = "user-content-fn-108e9df6">&lt;ITR&gt;trim</a> ###

<code>static void <strong>&lt;ITR&gt;trim</strong>(&lt;PITR&gt;box *const <em>box</em>, const &lt;PITR&gt;predicate_fn <em>predicate</em>)</code>

[src/iterate\.h](src/iterate.h), `BOX_CONTIGUOUS`: Removes at either end of `box` the things that `predicate`, if it exists, returns true\.

 * Order:  
   &#927;\(`box.size`\) &#215; &#927;\(`predicate`\)




### <a id = "user-content-fn-c4c1df3b" name = "user-content-fn-c4c1df3b">&lt;ITR&gt;to_if</a> ###

<code>static void <strong>&lt;ITR&gt;to_if</strong>(struct &lt;L&gt;list *restrict const <em>from</em>, struct &lt;L&gt;list *restrict const <em>to</em>, const &lt;PITR&gt;predicate_fn <em>predicate</em>)</code>

HAVE_ITERATE_H: Moves all elements `from` onto the tail of `to` if `predicate` is true\.

 * Parameter: _to_  
   If null, then it removes elements\.
 * Order:  
   &#920;\(|`from`|\) &#215; &#927;\(`predicate`\)




### <a id = "user-content-fn-751c6337" name = "user-content-fn-751c6337">&lt;STR&gt;to_string</a> ###

<code>static const char *<strong>&lt;STR&gt;to_string</strong>(const &lt;PSTR&gt;box *const <em>box</em>)</code>

[src/to\_string\.h](src/to_string.h): print the contents of `box` in a static string buffer of 256 bytes, with limitations of only printing 4 things at a time\.

 * Return:  
   Address of the static buffer\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-c2fff878" name = "user-content-fn-c2fff878">&lt;CMP&gt;compare</a> ###

<code>static int <strong>&lt;CMP&gt;compare</strong>(const &lt;PCMP&gt;box *restrict const <em>a</em>, const &lt;PCMP&gt;box *restrict const <em>b</em>)</code>

[src/compare\.h](src/compare.h), `COMPARE`: Lexicographically compares `a` to `b`\. Both can be null, with null values before everything\.

 * Return:  
   `a < b`: negative; `a == b`: zero; `a > b`: positive\.
 * Order:  
   &#927;\(`|a|` & `|b|`\)




### <a id = "user-content-fn-620cbec1" name = "user-content-fn-620cbec1">&lt;CMP&gt;lower_bound</a> ###

<code>static size_t <strong>&lt;CMP&gt;lower_bound</strong>(const &lt;PCMP&gt;box *const <em>box</em>, &lt;PCMP&gt;element_c <em>element</em>)</code>

[src/compare\.h](src/compare.h), `COMPARE`, `BOX_ACCESS`: `box` should be partitioned true/false with less\-then `element`\.

 * Return:  
   The first index of `a` that is not less than `cursor`\.
 * Order:  
   &#927;\(log `a.size`\)




### <a id = "user-content-fn-b6f29e84" name = "user-content-fn-b6f29e84">&lt;CMP&gt;upper_bound</a> ###

<code>static size_t <strong>&lt;CMP&gt;upper_bound</strong>(const &lt;PCMP&gt;box *const <em>box</em>, &lt;PCMP&gt;element_c <em>element</em>)</code>

[src/compare\.h](src/compare.h), `COMPARE`, `BOX_ACCESS`: `box` should be partitioned false/true with greater\-than or equal\-to `element`\.

 * Return:  
   The first index of `box` that is greater than `element`\.
 * Order:  
   &#927;\(log |`box`|\)




### <a id = "user-content-fn-c57ffcf5" name = "user-content-fn-c57ffcf5">&lt;CMP&gt;insert_after</a> ###

<code>static int <strong>&lt;CMP&gt;insert_after</strong>(&lt;PCMP&gt;box *const <em>box</em>, const &lt;PCMP&gt;element_c <em>element</em>)</code>

[src/compare\.h](src/compare.h), `COMPARE`, `BOX_CONTIGUOUS`: Copies `element` at the upper bound of a sorted `box`\.

 * Return:  
   Success\.
 * Exceptional return: realloc, ERANGE  
 * Order:  
   &#927;\(`a.size`\)




### <a id = "user-content-fn-a7c44d35" name = "user-content-fn-a7c44d35">&lt;CMP&gt;sort</a> ###

<code>static void <strong>&lt;CMP&gt;sort</strong>(&lt;PCMP&gt;box *const <em>box</em>)</code>

[src/compare\.h](src/compare.h), `COMPARE`, `BOX_CONTIGUOUS`: Sorts `box` by `qsort`, \(which has a high\-context\-switching cost, but is easy\.\)

 * Order:  
   &#927;\(|`box`| log |`box`|\)




### <a id = "user-content-fn-f184f491" name = "user-content-fn-f184f491">&lt;CMP&gt;reverse</a> ###

<code>static void <strong>&lt;CMP&gt;reverse</strong>(&lt;PCMP&gt;box *const <em>box</em>)</code>

[src/compare\.h](src/compare.h), `COMPARE`, `BOX_CONTIGUOUS`: Sorts `box` in reverse by `qsort`\.

 * Order:  
   &#927;\(|`box`| log |`box`|\)




### <a id = "user-content-fn-82b7806" name = "user-content-fn-82b7806">&lt;CMP&gt;is_equal</a> ###

<code>static int <strong>&lt;CMP&gt;is_equal</strong>(const &lt;PCMP&gt;box *restrict const <em>a</em>, const &lt;PCMP&gt;box *restrict const <em>b</em>)</code>

[src/compare\.h](src/compare.h)

 * Return:  
   If `a` piecewise equals `b`, which both can be null\.
 * Order:  
   &#927;\(|`a`| & |`b`|\)




### <a id = "user-content-fn-a9b0c375" name = "user-content-fn-a9b0c375">&lt;CMP&gt;unique_merge</a> ###

<code>static void <strong>&lt;CMP&gt;unique_merge</strong>(&lt;PCMP&gt;box *const <em>box</em>, const &lt;PCMP&gt;biaction_fn <em>merge</em>)</code>

[src/compare\.h](src/compare.h), `BOX_CONTIGUOUS`: Removes consecutive duplicate elements in `box` lazily\.

 * Parameter: _merge_  
   Controls surjection\. Called with duplicate elements, if false `(x, y)->(x)`, if true `(x,y)->(y)`\. More complex functions, `(x, y)->(x+y)` can be simulated by mixing the two in the value returned\. Can be null: behaves like false, always deleting the second element\.
 * Order:  
   &#927;\(|`box`|\) &#215; &#927;\(`merge`\)




### <a id = "user-content-fn-52f3957a" name = "user-content-fn-52f3957a">&lt;CMP&gt;unique</a> ###

<code>static void <strong>&lt;CMP&gt;unique</strong>(&lt;PCMP&gt;box *const <em>box</em>)</code>

[src/compare\.h](src/compare.h), `BOX_CONTIGUOUS`: Removes consecutive duplicate elements in `box`\.

 * Order:  
   &#927;\(|`box`|\)




### <a id = "user-content-fn-d9a682b3" name = "user-content-fn-d9a682b3">&lt;CMP&gt;merge</a> ###

<code>static void <strong>&lt;CMP&gt;merge</strong>(struct &lt;L&gt;list *restrict const <em>to</em>, struct &lt;L&gt;list *restrict const <em>from</em>)</code>

Merges `from` into `to`, preferring elements from `to` go in the front\.

 * Order:  
   &#927;\(|`from`| \+ |`to`|\)\.




### <a id = "user-content-fn-a7c44d35" name = "user-content-fn-a7c44d35">&lt;CMP&gt;sort</a> ###

<code>static void <strong>&lt;CMP&gt;sort</strong>(struct &lt;L&gt;list *const <em>list</em>)</code>

`LIST_COMPARE`: Natural merge sort `list`, a stable, adaptive sort, according to `compare`\. This list\-only version is slower then `qsort`\.

 * Order:  
   &#937;\(|`list`|\), &#927;\(|`list`| log |`list`|\)




### <a id = "user-content-fn-7d4d7583" name = "user-content-fn-7d4d7583">&lt;CMP&gt;subtraction_to</a> ###

<code>static void <strong>&lt;CMP&gt;subtraction_to</strong>(struct &lt;L&gt;list *restrict const <em>a</em>, struct &lt;L&gt;list *restrict const <em>b</em>, struct &lt;L&gt;list *restrict const <em>result</em>)</code>

Subtracts `a` from `b`, as sequential sorted individual elements, and moves it to `result`\. All elements are removed from `a`\. All parameters must be unique or can be null\.

For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then `(a:A, a:D)` would be moved to `result`\.



 * Order:  
   &#927;\(|`a`| \+ |`b`|\)




### <a id = "user-content-fn-54db3a7c" name = "user-content-fn-54db3a7c">&lt;CMP&gt;union_to</a> ###

<code>static void <strong>&lt;CMP&gt;union_to</strong>(struct &lt;L&gt;list *restrict const <em>a</em>, struct &lt;L&gt;list *restrict const <em>b</em>, struct &lt;L&gt;list *restrict const <em>result</em>)</code>

Moves the union of `a` and `b` as sequential sorted individual elements to `result`\. Equal elements are moved from `a`\. All parameters must be unique or can be null\.

For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then `(a:A, a:B, b:C, a:D)` would be moved to `result`\.



 * Order:  
   &#927;\(|`a`| \+ |`b`|\)


### <a id = "user-content-fn-ce5608a0" name = "user-content-fn-ce5608a0">&lt;CMP&gt;intersection_to</a> ###

<code>static void <strong>&lt;CMP&gt;intersection_to</strong>(struct &lt;L&gt;list *restrict const <em>a</em>, struct &lt;L&gt;list *restrict const <em>b</em>, struct &lt;L&gt;list *restrict const <em>result</em>)</code>

Moves the intersection of `a` and `b` as sequential sorted individual elements to `result`\. Equal elements are moved from `a`\. All parameters must be unique or can be null\.

For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then `(a:B)` would be moved to `result`\.



 * Order:  
   &#927;\(|`a`| \+ |`b`|\)


### <a id = "user-content-fn-33635302" name = "user-content-fn-33635302">&lt;CMP&gt;xor_to</a> ###

<code>static void <strong>&lt;CMP&gt;xor_to</strong>(struct &lt;L&gt;list *restrict const <em>a</em>, struct &lt;L&gt;list *restrict const <em>b</em>, struct &lt;L&gt;list *restrict const <em>result</em>)</code>

Moves `a` exclusive\-or `b` as sequential sorted individual elements to `result`\. Equal elements are moved from `a`\. All parameters must be unique or can be null\.

For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then `(a:A, b:C, a:D)` would be moved to `result`\.



 * Order:  
   O\(|`a`| \+ |`b`|\)


### <a id = "user-content-fn-8ff3b955" name = "user-content-fn-8ff3b955">&lt;CMP&gt;duplicates_to</a> ###

<code>static void <strong>&lt;CMP&gt;duplicates_to</strong>(struct &lt;L&gt;list *restrict const <em>from</em>, struct &lt;L&gt;list *restrict const <em>to</em>)</code>

Moves all local\-duplicates of `from` to the end of `to`\.

For example, if `from` is `(A, B, B, A)`, it would concatenate the second `(B)` to `to` and leave `(A, B, A)` in `from`\. If one [&lt;CMP&gt;sort](#user-content-fn-a7c44d35) `from` first, `(A, A, B, B)`, the global duplicates will be transferred, `(A, B)`\.



 * Order:  
   &#927;\(|`from`|\)


## <a id = "user-content-license" name = "user-content-license">License</a> ##

2017 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



