# array\.h #

Stand\-alone header [\.\./src/array\.h](../src/array.h); examples [\.\./test/test\_array\.c](../test/test_array.c)\.

## Contiguous dynamic array ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;pT&gt;type](#user-content-typedef-9b5be28b), [&lt;pTR&gt;action_fn](#user-content-typedef-93439a66), [&lt;pTR&gt;predicate_fn](#user-content-typedef-6293cba5), [&lt;pTR&gt;to_string_fn](#user-content-typedef-d00960b3), [&lt;pTR&gt;bipredicate_fn](#user-content-typedef-cec76a40), [&lt;pTR&gt;compare_fn](#user-content-typedef-a1dda3af), [&lt;pTR&gt;biaction_fn](#user-content-typedef-c699afa9)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;t&gt;array](#user-content-tag-9c4cf562)
 * [General Declarations](#user-content-data): [a](#user-content-data-e40c292c)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of array.](../doc/array/array.png)

[&lt;t&gt;array](#user-content-tag-9c4cf562) is a dynamic array that stores contiguous [&lt;pT&gt;type](#user-content-typedef-9b5be28b)\. Resizing may be necessary when increasing the size of the array; this incurs amortised cost\. As such, the contents are not stable\.

\* [src/iterate\.h](src/iterate.h): defining `HAVE_ITERATE_H` supplies functions\.

 * Parameter: ARRAY\_NAME, ARRAY\_TYPE  
   `<t>` that satisfies `C` naming conventions when mangled and a valid tag\-type, [&lt;pT&gt;type](#user-content-typedef-9b5be28b), associated therewith; required\.
 * Parameter: ARRAY\_COMPARE, ARRAY\_IS\_EQUAL  
   Compare trait contained in [src/compare\.h](src/compare.h)\. Requires `<name>[<trait>]compare` to be declared as [&lt;pTR&gt;compare_fn](#user-content-typedef-a1dda3af) or `<name>[<trait>]is_equal` to be declared as [&lt;pTR&gt;bipredicate_fn](#user-content-typedef-cec76a40), respectfully, \(but not both\.\)
 * Parameter: ARRAY\_TO\_STRING  
   To string trait contained in [src/to\_string\.h](src/to_string.h)\. Requires `<name>[<trait>]to_string` be declared as [&lt;pTR&gt;to_string_fn](#user-content-typedef-d00960b3)\.
 * Parameter: ARRAY\_EXPECT\_TRAIT, ARRAY\_TRAIT  
   Named traits are obtained by including `array.h` multiple times with `ARRAY_EXPECT_TRAIT` and then subsequently including the name in `ARRAY_TRAIT`\.
 * Parameter: ARRAY\_DECLARE\_ONLY  
   For headers in different compilation units\.
 * Standard:  
   C89


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-9b5be28b" name = "user-content-typedef-9b5be28b">&lt;pT&gt;type</a> ###

<code>typedef ARRAY_TYPE <strong>&lt;pT&gt;type</strong>;</code>

A valid tag type set by `ARRAY_TYPE`\.



### <a id = "user-content-typedef-93439a66" name = "user-content-typedef-93439a66">&lt;pTR&gt;action_fn</a> ###

<code>typedef void(*<strong>&lt;pTR&gt;action_fn</strong>)(&lt;pT&gt;type *);</code>

[src/iterate\.h](src/iterate.h): Operates by side\-effects\.



### <a id = "user-content-typedef-6293cba5" name = "user-content-typedef-6293cba5">&lt;pTR&gt;predicate_fn</a> ###

<code>typedef int(*<strong>&lt;pTR&gt;predicate_fn</strong>)(const &lt;pT&gt;type *);</code>

[src/iterate\.h](src/iterate.h): Returns a boolean given read\-only\.



### <a id = "user-content-typedef-d00960b3" name = "user-content-typedef-d00960b3">&lt;pTR&gt;to_string_fn</a> ###

<code>typedef void(*<strong>&lt;pTR&gt;to_string_fn</strong>)(const &lt;pT&gt;type *, char(*)[12]);</code>

Type of `ARRAY_TO_STRING` needed function `<tr>to_string`\. Responsible for turning the read\-only argument into a 12\-max\-`char` output string\.



### <a id = "user-content-typedef-cec76a40" name = "user-content-typedef-cec76a40">&lt;pTR&gt;bipredicate_fn</a> ###

<code>typedef int(*<strong>&lt;pTR&gt;bipredicate_fn</strong>)(&lt;pT&gt;type *restrict, &lt;pT&gt;type *restrict);</code>

[src/compare\.h](src/compare.h): Returns a boolean given two read\-only elements\.



### <a id = "user-content-typedef-a1dda3af" name = "user-content-typedef-a1dda3af">&lt;pTR&gt;compare_fn</a> ###

<code>typedef int(*<strong>&lt;pTR&gt;compare_fn</strong>)(const &lt;pT&gt;type *restrict a, const &lt;pT&gt;type *restrict b);</code>

[src/compare\.h](src/compare.h): Three\-way comparison on a totally order set; returns an integer value less than, equal to, greater than zero, if `a < b`, `a == b`, `a > b`, respectively\.



### <a id = "user-content-typedef-c699afa9" name = "user-content-typedef-c699afa9">&lt;pTR&gt;biaction_fn</a> ###

<code>typedef int(*<strong>&lt;pTR&gt;biaction_fn</strong>)(&lt;pT&gt;type *restrict, &lt;pT&gt;type *restrict);</code>

[src/compare\.h](src/compare.h): Returns a boolean given two modifiable arguments\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-9c4cf562" name = "user-content-tag-9c4cf562">&lt;t&gt;array</a> ###

<code>struct <strong>&lt;t&gt;array</strong> { size_t size, capacity; &lt;pT&gt;type *data; };</code>

Manages the array field `data` which has `size` elements\. The space is indexed up to `capacity`, which is at least `size`\.

![States.](../doc/array/states.png)



## <a id = "user-content-data" name = "user-content-data">General Declarations</a> ##

### <a id = "user-content-data-e40c292c" name = "user-content-data-e40c292c">a</a> ###

<code>static void &lt;t&gt;array_(struct &lt;t&gt;array *const <strong>a</strong>){ if(a)free(a -&gt;data), *a = &lt;t&gt;array(); } static int &lt;T&gt;reserve(struct &lt;t&gt;array *const a, const size_t min){ size_t c0; &lt;pT&gt;type *data; const size_t max_size =(size_t)~0 /sizeof *a -&gt;data; if(a -&gt;data){ assert(a -&gt;size &lt;=a -&gt;capacity); if(min &lt;=a -&gt;capacity)return 1; c0 = a -&gt;capacity &lt;ARRAY_MIN_CAPACITY ?ARRAY_MIN_CAPACITY :a -&gt;capacity; } else { assert(!a -&gt;size &amp;&amp;!a -&gt;capacity); if(!min)return 1; c0 = ARRAY_MIN_CAPACITY; } if(min &gt;max_size)return errno = ERANGE, 0; while(c0 &lt;min){ size_t c1 = c0 +(c0 &gt;&gt;1)+(c0 &gt;&gt;3); if(c0 &gt;=c1){ c0 = max_size; break; } c0 = c1; } if(!(data = realloc(a -&gt;data, sizeof *a -&gt;data *c0))){ if(!errno)errno = ERANGE; return 0; } a -&gt;data = data, a -&gt;capacity = c0; return 1; } static &lt;pT&gt;type *&lt;T&gt;buffer(struct &lt;t&gt;array *const a, const size_t n){ if(a -&gt;size &gt;(size_t)~0 -n){ errno = ERANGE; return 0; } return &lt;T&gt;reserve(a, a -&gt;size +n)&amp;&amp;a -&gt;data ?a -&gt;data +a -&gt;size :0; } static &lt;pT&gt;type *&lt;T&gt;append(struct &lt;t&gt;array *const a, const size_t n){ &lt;pT&gt;type *b; if(!(b = &lt;T&gt;buffer(a, n)))return 0; assert(n &lt;=a -&gt;capacity &amp;&amp;a -&gt;size &lt;=a -&gt;capacity -n); return a -&gt;size +=n, b; } static &lt;pT&gt;type *&lt;T&gt;insert(struct &lt;t&gt;array *const a, const size_t n, const size_t at){ const size_t old_size = a -&gt;size; &lt;pT&gt;type *const b = &lt;T&gt;append(a, n); assert(a &amp;&amp;at &lt;=old_size); if(!b)return 0; memmove(a -&gt;data +at +n, a -&gt;data +at, sizeof *a -&gt;data *(old_size -at)); return a -&gt;data +at; } static &lt;pT&gt;type *&lt;T&gt;new(struct &lt;t&gt;array *const a){ return &lt;T&gt;append(a, 1); } static int &lt;T&gt;shrink(struct &lt;t&gt;array *const a){ &lt;pT&gt;type *data; size_t c; assert(a &amp;&amp;a -&gt;capacity &gt;=a -&gt;size); if(!a -&gt;data)return assert(!a -&gt;size &amp;&amp;!a -&gt;capacity), 1; c = a -&gt;size &amp;&amp;a -&gt;size &gt;ARRAY_MIN_CAPACITY ?a -&gt;size :ARRAY_MIN_CAPACITY; if(!(data = realloc(a -&gt;data, sizeof *a -&gt;data *c))){ if(!errno)errno = ERANGE; return 0; } a -&gt;data = data, a -&gt;capacity = c; return 1; } static void &lt;T&gt;remove(struct &lt;t&gt;array *const a, &lt;pT&gt;type *const element){ const size_t n =(size_t)(element -a -&gt;data); assert(a &amp;&amp;element &amp;&amp;element &gt;=a -&gt;data &amp;&amp;element &lt;a -&gt;data +a -&gt;size); memmove(element, element +1, sizeof *element *(--a -&gt;size -n)); } static void &lt;T&gt;lazy_remove(struct &lt;t&gt;array *const a, &lt;pT&gt;type *const datum){ size_t n =(size_t)(datum -a -&gt;data); assert(a &amp;&amp;datum &amp;&amp;datum &gt;=a -&gt;data &amp;&amp;datum &lt;a -&gt;data +a -&gt;size); if(--a -&gt;size !=n)memcpy(datum, a -&gt;data +a -&gt;size, sizeof *datum); } static void &lt;T&gt;clear(struct &lt;t&gt;array *const a){ assert(a), a -&gt;size = 0; } static &lt;pT&gt;type *&lt;T&gt;peek(const struct &lt;t&gt;array *const a){ return assert(a), a -&gt;size ?a -&gt;data +a -&gt;size -1 :0; } static &lt;pT&gt;type *&lt;T&gt;pop(struct &lt;t&gt;array *const a){ return assert(a), a -&gt;size ?a -&gt;data +--a -&gt;size :0; } static int &lt;T&gt;splice(struct &lt;t&gt;array *restrict const a, const struct &lt;t&gt;array *restrict const b, const size_t i0, const size_t i1){ const size_t a_range = i1 -i0, b_range = b ?b -&gt;size :0; assert(a &amp;&amp;a !=b &amp;&amp;i0 &lt;=i1 &amp;&amp;i1 &lt;=a -&gt;size); if(a_range &lt;b_range){ const size_t diff = b_range -a_range; if(!&lt;T&gt;buffer(a, diff))return 0; memmove(a -&gt;data +i1 +diff, a -&gt;data +i1,(a -&gt;size -i1)*sizeof *a -&gt;data); a -&gt;size +=diff; } else if(b_range &lt;a_range){ memmove(a -&gt;data +i0 +b_range, a -&gt;data +i1,(a -&gt;size -i1)*sizeof *a -&gt;data); a -&gt;size -=a_range -b_range; } if(b)memcpy(a -&gt;data +i0, b -&gt;data, b -&gt;size *sizeof *a -&gt;data); return 1; }</code>

If `a` is not null, destroys and returns it to idle\. Ensures `min` capacity of `a`\. Invalidates pointers in `a`\. The capacity of `a` will be increased to at least `n` elements beyond the size\. Invalidates any pointers in `a`\. Adds `n` elements to the back of `a`\. It will invalidate pointers in `a` if `n` is greater than the buffer space\. Adds `n` un\-initialised elements at position `at` in `a`\. It will invalidate any pointers in `a` if the buffer holds too few elements\. Shrinks the capacity `a` to the size, freeing unused memory\. If the size is zero, it will be in an idle state\. Invalidates pointers in `a`\. Removes `element` from `a`\. Do not attempt to remove an element that is not in `a`\. Removes `datum` from `a` and replaces it with the tail\. Do not attempt to remove an element that is not in `a`\. Sets `a` to be empty\. That is, the size of `a` will be zero, but if it was previously in an active non\-idle state, it continues to be\. Indices \[`i0`, `i1`\) of `a` will be replaced with a copy of `b`\.

 * Implements:  
   `append` from `BOX_CONTIGUOUS`




## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static struct &lt;t&gt;array</td><td><a href = "#user-content-fn-9c4cf562">&lt;t&gt;array</a></td><td></td></tr>

<tr><td align = right>static &lt;pT&gt;type *</td><td><a href = "#user-content-fn-443f2b31">&lt;TR&gt;any</a></td><td>box, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-51d87ca4">&lt;TR&gt;each</a></td><td>box, action</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-21ef106e">&lt;TR&gt;if_each</a></td><td>box, predicate, action</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-f61ec8de">&lt;TR&gt;copy_if</a></td><td>dst, src, copy</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-8bb1c0a2">&lt;TR&gt;keep_if</a></td><td>box, keep, destruct</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-a76df7bd">&lt;TR&gt;trim</a></td><td>box, predicate</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-260f8348">&lt;TR&gt;to_string</a></td><td>box</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-aa7d8478">&lt;TR&gt;compare</a></td><td>a, b</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-b6d70ac1">&lt;TR&gt;lower_bound</a></td><td>box, element</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-bbcea84">&lt;TR&gt;upper_bound</a></td><td>box, element</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-3ff5a0f5">&lt;TR&gt;insert_after</a></td><td>box, element</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-17397135">&lt;TR&gt;sort</a></td><td>box</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-d9028091">&lt;TR&gt;reverse</a></td><td>box</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-72cedc06">&lt;TR&gt;is_equal</a></td><td>a, b</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-24266775">&lt;TR&gt;unique_merge</a></td><td>box, merge</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-2527597a">&lt;TR&gt;unique</a></td><td>box</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-9c4cf562" name = "user-content-fn-9c4cf562">&lt;t&gt;array</a> ###

<code>static struct &lt;t&gt;array <strong>&lt;t&gt;array</strong>(void)</code>

Zeroed data \(not all\-bits\-zero\) is initialized\.

 * Return:  
   An idle array\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-443f2b31" name = "user-content-fn-443f2b31">&lt;TR&gt;any</a> ###

<code>static &lt;pT&gt;type *<strong>&lt;TR&gt;any</strong>(const &lt;pT&gt;box *const <em>box</em>, const &lt;pTR&gt;predicate_fn <em>predicate</em>)</code>

[src/iterate\.h](src/iterate.h): Iterates through `box` and calls `predicate` until it returns true\.

 * Return:  
   The first `predicate` that returned true, or, if the statement is false on all, null\.
 * Order:  
   &#927;\(`box.size`\) &#215; &#927;\(`predicate`\)




### <a id = "user-content-fn-51d87ca4" name = "user-content-fn-51d87ca4">&lt;TR&gt;each</a> ###

<code>static void <strong>&lt;TR&gt;each</strong>(&lt;pT&gt;box *const <em>box</em>, const &lt;pTR&gt;action_fn <em>action</em>)</code>

[src/iterate\.h](src/iterate.h): Iterates through `box` and calls `action` on all the elements\. Differs calling `action` until the iterator is one\-ahead, so can delete elements as long as it doesn't affect the next, \(specifically, a linked\-list\.\)

 * Order:  
   &#927;\(|`box`|\) &#215; &#927;\(`action`\)




### <a id = "user-content-fn-21ef106e" name = "user-content-fn-21ef106e">&lt;TR&gt;if_each</a> ###

<code>static void <strong>&lt;TR&gt;if_each</strong>(&lt;pT&gt;box *const <em>box</em>, const &lt;pTR&gt;predicate_fn <em>predicate</em>, const &lt;pTR&gt;action_fn <em>action</em>)</code>

[src/iterate\.h](src/iterate.h): Iterates through `box` and calls `action` on all the elements for which `predicate` returns true\.

 * Order:  
   &#927;\(`box.size`\) &#215; \(&#927;\(`predicate`\) \+ &#927;\(`action`\)\)




### <a id = "user-content-fn-f61ec8de" name = "user-content-fn-f61ec8de">&lt;TR&gt;copy_if</a> ###

<code>static int <strong>&lt;TR&gt;copy_if</strong>(&lt;pT&gt;box *restrict const <em>dst</em>, const &lt;pTR&gt;box *restrict const <em>src</em>, const &lt;pTR&gt;predicate_fn <em>copy</em>)</code>

[src/iterate\.h](src/iterate.h), `pT_CONTIGUOUS`: For all elements of `src`, calls `copy`, and if true, lazily copies the elements to `dst`\. `dst` and `src` can not be the same but `src` can be null, \(in which case, it does nothing\.\)

 * Exceptional return: realloc  
 * Order:  
   &#927;\(|`src`|\) &#215; &#927;\(`copy`\)




### <a id = "user-content-fn-8bb1c0a2" name = "user-content-fn-8bb1c0a2">&lt;TR&gt;keep_if</a> ###

<code>static void <strong>&lt;TR&gt;keep_if</strong>(&lt;pT&gt;box *const <em>box</em>, const &lt;pTR&gt;predicate_fn <em>keep</em>, const &lt;pTR&gt;action_fn <em>destruct</em>)</code>

[src/iterate\.h](src/iterate.h): For all elements of `box`, calls `keep`, and if false, if contiguous, lazy deletes that item, if not, eagerly\. Calls `destruct` if not\-null before deleting\.

 * Order:  
   &#927;\(|`box`|\) \(&#215; O\(`keep`\) \+ O\(`destruct`\)\)




### <a id = "user-content-fn-a76df7bd" name = "user-content-fn-a76df7bd">&lt;TR&gt;trim</a> ###

<code>static void <strong>&lt;TR&gt;trim</strong>(&lt;pT&gt;box *const <em>box</em>, const &lt;pTR&gt;predicate_fn <em>predicate</em>)</code>

[src/iterate\.h](src/iterate.h), `pT_CONTIGUOUS`: Removes at either end of `box` the things that `predicate`, if it exists, returns true\.

 * Order:  
   &#927;\(`box.size`\) &#215; &#927;\(`predicate`\)




### <a id = "user-content-fn-260f8348" name = "user-content-fn-260f8348">&lt;TR&gt;to_string</a> ###

<code>static const char *<strong>&lt;TR&gt;to_string</strong>(const &lt;pT&gt;box *const <em>box</em>)</code>

[src/to\_string\.h](src/to_string.h): print the contents of `box` in a static string buffer of 256 bytes, with limitations of only printing 4 things in a single sequence point\.

 * Return:  
   Address of the static buffer\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-aa7d8478" name = "user-content-fn-aa7d8478">&lt;TR&gt;compare</a> ###

<code>static int <strong>&lt;TR&gt;compare</strong>(const &lt;pT&gt;box *restrict const <em>a</em>, const &lt;pT&gt;box *restrict const <em>b</em>)</code>

[src/compare\.h](src/compare.h), `COMPARE`: Lexicographically compares `a` to `b`\. Both can be null, with null values before everything\.

 * Return:  
   `a < b`: negative; `a == b`: zero; `a > b`: positive\.
 * Order:  
   &#927;\(`|a|` & `|b|`\)




### <a id = "user-content-fn-b6d70ac1" name = "user-content-fn-b6d70ac1">&lt;TR&gt;lower_bound</a> ###

<code>static size_t <strong>&lt;TR&gt;lower_bound</strong>(const &lt;pT&gt;box *const <em>box</em>, const &lt;pT&gt;type *const <em>element</em>)</code>

[src/compare\.h](src/compare.h), `COMPARE`, `BOX_ACCESS`: `box` should be partitioned true/false with less\-then `element`\.

 * Return:  
   The first index of `a` that is not less than `cursor`\.
 * Order:  
   &#927;\(log `a.size`\)




### <a id = "user-content-fn-bbcea84" name = "user-content-fn-bbcea84">&lt;TR&gt;upper_bound</a> ###

<code>static size_t <strong>&lt;TR&gt;upper_bound</strong>(const &lt;pT&gt;box *const <em>box</em>, const &lt;pT&gt;type *const <em>element</em>)</code>

[src/compare\.h](src/compare.h), `COMPARE`, `BOX_ACCESS`: `box` should be partitioned false/true with greater\-than or equal\-to `element`\.

 * Return:  
   The first index of `box` that is greater than `element`\.
 * Order:  
   &#927;\(log |`box`|\)




### <a id = "user-content-fn-3ff5a0f5" name = "user-content-fn-3ff5a0f5">&lt;TR&gt;insert_after</a> ###

<code>static int <strong>&lt;TR&gt;insert_after</strong>(&lt;pT&gt;box *const <em>box</em>, const &lt;pT&gt;type *const <em>element</em>)</code>

[src/compare\.h](src/compare.h), `COMPARE`, `BOX_CONTIGUOUS`: Copies `element` at the upper bound of a sorted `box`\.

 * Return:  
   Success\.
 * Exceptional return: realloc, ERANGE  
 * Order:  
   &#927;\(`a.size`\)




### <a id = "user-content-fn-17397135" name = "user-content-fn-17397135">&lt;TR&gt;sort</a> ###

<code>static void <strong>&lt;TR&gt;sort</strong>(&lt;pT&gt;box *const <em>box</em>)</code>

[src/compare\.h](src/compare.h), `COMPARE`, `BOX_CONTIGUOUS`: Sorts `box` by `qsort`, \(which has a high\-context\-switching cost, but is easy\.\)

 * Order:  
   &#927;\(|`box`| log |`box`|\)




### <a id = "user-content-fn-d9028091" name = "user-content-fn-d9028091">&lt;TR&gt;reverse</a> ###

<code>static void <strong>&lt;TR&gt;reverse</strong>(&lt;pT&gt;box *const <em>box</em>)</code>

[src/compare\.h](src/compare.h), `COMPARE`, `BOX_CONTIGUOUS`: Sorts `box` in reverse by `qsort`\.

 * Order:  
   &#927;\(|`box`| log |`box`|\)




### <a id = "user-content-fn-72cedc06" name = "user-content-fn-72cedc06">&lt;TR&gt;is_equal</a> ###

<code>static int <strong>&lt;TR&gt;is_equal</strong>(const &lt;pT&gt;box *restrict const <em>a</em>, const &lt;pT&gt;box *restrict const <em>b</em>)</code>

[src/compare\.h](src/compare.h)

 * Return:  
   If `a` piecewise equals `b`, which both can be null\.
 * Order:  
   &#927;\(|`a`| & |`b`|\)




### <a id = "user-content-fn-24266775" name = "user-content-fn-24266775">&lt;TR&gt;unique_merge</a> ###

<code>static void <strong>&lt;TR&gt;unique_merge</strong>(&lt;pT&gt;box *const <em>box</em>, const &lt;pTR&gt;biaction_fn <em>merge</em>)</code>

[src/compare\.h](src/compare.h), `BOX_CONTIGUOUS`: Removes consecutive duplicate elements in `box` lazily\.

 * Parameter: _merge_  
   Controls surjection\. Called with duplicate elements, if false `(x, y)->(x)`, if true `(x,y)->(y)`\. More complex functions, `(x, y)->(x+y)` can be simulated by mixing the two in the value returned\. Can be null: behaves like false, always deleting the second element\.
 * Order:  
   &#927;\(|`box`|\) &#215; &#927;\(`merge`\)




### <a id = "user-content-fn-2527597a" name = "user-content-fn-2527597a">&lt;TR&gt;unique</a> ###

<code>static void <strong>&lt;TR&gt;unique</strong>(&lt;pT&gt;box *const <em>box</em>)</code>

[src/compare\.h](src/compare.h), `BOX_CONTIGUOUS`: Removes consecutive duplicate elements in `box`\.

 * Order:  
   &#927;\(|`box`|\)






## <a id = "user-content-license" name = "user-content-license">License</a> ##

2016 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



