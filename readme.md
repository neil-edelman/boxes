 # Set\.h #

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef):  [&lt;PK&gt;Hash](#user-content-typedef-3763afc0), [&lt;PK&gt;IsEqual](#user-content-typedef-f1be3090), [&lt;PK&gt;ToString](#user-content-typedef-77ec3874), [&lt;PK&gt;Action](#user-content-typedef-46e4e58a)
 * [Struct, Union, and Enum Definitions](#user-content-tag):  [&lt;K&gt;SetItem](#user-content-tag-505abce1), [&lt;K&gt;Set](#user-content-tag-2f49050a)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

 ## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

`&lt;K&gt;Set` is a collection of objects with a hash function and equality function that doesn't allow duplication\. Collisions are handled by separate chaining\. The maximum load factor is `ln 2` \. While in the set, the values cannot change in way that affects their hash\.

 - Parameter: SET\_NAME, SET\_TYPE  
   `K` that satisfies `C` naming conventions when mangled; required\.
 - Parameter: SET\_HASH  
   A function satisfying [&lt;PK&gt;Hash](#user-content--3763afc0); required\.
 - Parameter: SET\_IS\_EQUAL  
   A function satisfying [&lt;PK&gt;IsEqual](#user-content--f1be3090); required\.
 - Parameter: SET\_TO\_STRING  
   Optional print function implementing [&lt;PK&gt;ToString](#user-content--77ec3874); makes available [&lt;K&gt;SetToString](#user-content-(null)-1b39893a)\.
 - Parameter: SET\_TEST  
   Unit testing framework, included in a separate header, [\.\./test/SetTest\.h](../test/SetTest.h)\. Must be defined equal to a random filler function, satisfying [&lt;PV&gt;Action](#user-content--4585d713)\. Requires `SET_TO_STRING` \.
 * Standard:  
   C89/90
 * Caveat:  
   Implement tests\.




 ## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

 ### <a id = "user-content-typedef-3763afc0" name = "user-content-typedef-3763afc0"><PK>Hash</a> ###

<code>typedef unsigned(*<strong>&lt;PK&gt;Hash</strong>)(const K);</code>

A map from `K` onto `unsigned int` \. Should be as close as possible to a discrete uniform distribution for maximum performance and, when computing, take all of `K` into account\.



 ### <a id = "user-content-typedef-f1be3090" name = "user-content-typedef-f1be3090"><PK>IsEqual</a> ###

<code>typedef int(*<strong>&lt;PK&gt;IsEqual</strong>)(const K, const K);</code>

A constant equivalence relation between `K` that satisfies `&lt;PK&gt;IsEqual(a, b) -&gt; &lt;PK&gt;Hash(a) == &lt;PK&gt;Hash(b)` \.



 ### <a id = "user-content-typedef-77ec3874" name = "user-content-typedef-77ec3874"><PK>ToString</a> ###

<code>typedef void(*<strong>&lt;PK&gt;ToString</strong>)(const K *const, char(*const)[12]);</code>

Responsible for turning `K` \(the first argument\) into a 12 `char` string \(the second\.\)



 ### <a id = "user-content-typedef-46e4e58a" name = "user-content-typedef-46e4e58a"><PK>Action</a> ###

<code>typedef void(*<strong>&lt;PK&gt;Action</strong>)(const K *const);</code>

Used for `SET_TEST` \.



 ## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

 ### <a id = "user-content-tag-505abce1" name = "user-content-tag-505abce1"><K>SetItem</a> ###

<code>struct <strong>&lt;K&gt;SetItem</strong>;</code>

Contains `K` and more internal to the working of the hash\. Storage of the `&lt;K&gt;SetItem` structure is the responsibility of the caller; it could be one part of a complicated structure\.



 ### <a id = "user-content-tag-2f49050a" name = "user-content-tag-2f49050a"><K>Set</a> ###

<code>struct <strong>&lt;K&gt;Set</strong>;</code>

A `&lt;K&gt;Set` \.



 ## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-c4f364cf">&lt;K&gt;Set_</a></td><td>set</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-2f49050a">&lt;K&gt;Set</a></td><td>set</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-6cbfd773">&lt;K&gt;SetClear</a></td><td>set</td></tr>

<tr><td align = right>static const K *</td><td><a href = "#user-content-fn-a08ac546">&lt;K&gt;SetGet</a></td><td>set, key</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-266df0ef">&lt;K&gt;SetPut</a></td><td>set, item, p_eject</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-2956c7e9">&lt;K&gt;SetPutIfAbsent</a></td><td>set, item, p_is_absent</td></tr>

<tr><td align = right>static struct &lt;K&gt;SetItem *</td><td><a href = "#user-content-fn-257b1a0e">&lt;K&gt;SetRemove</a></td><td>set, key</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-1b39893a">&lt;K&gt;SetToString</a></td><td>set</td></tr>

</table>



 ## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

 ### <a id = "user-content-fn-c4f364cf" name = "user-content-fn-c4f364cf"><K>Set_</a> ###

<code>static void <strong>&lt;K&gt;Set_</strong>(struct &lt;K&gt;Set *const <em>set</em>)</code>

Destructor for `set` \. After, it takes no memory and is in an empty state\.



 ### <a id = "user-content-fn-2f49050a" name = "user-content-fn-2f49050a"><K>Set</a> ###

<code>static void <strong>&lt;K&gt;Set</strong>(struct &lt;K&gt;Set *const <em>set</em>)</code>

Initialises `set` to be take no memory and be in an empty state\. If it is `static` data, then it is initialised by default\. Alternatively, assigning `{0}` \(`C99+` \) or `SET_ZERO` as the initialiser also puts it in an empty state\. Calling this on an active set will cause memory leaks\.

 - Parameter: _set_  
   If null, does nothing\.
 - Order:  
   &#920;\(1\)




 ### <a id = "user-content-fn-6cbfd773" name = "user-content-fn-6cbfd773"><K>SetClear</a> ###

<code>static void <strong>&lt;K&gt;SetClear</strong>(struct &lt;K&gt;Set *const <em>set</em>)</code>

Clears and removes all entries from `set` \. The capacity and memory of the hash table is preserved, but all previous values are un\-associated\. Until the previous size is obtained, the load factor will be less\.

 - Parameter: _set_  
   If null, does nothing\.
 - Order:  
   &#920;\(`set.buckets` \)




 ### <a id = "user-content-fn-a08ac546" name = "user-content-fn-a08ac546"><K>SetGet</a> ###

<code>static const K *<strong>&lt;K&gt;SetGet</strong>(struct &lt;K&gt;Set *const <em>set</em>, const K <em>key</em>)</code>

Gets `item` from `set` \.

 - Return:  
   The value which [&lt;PK&gt;IsEqual](#user-content--f1be3090) the `item` , or, if no such value exists, null\.
 - Order:  
   Constant time assuming the hash function is uniform; worst &#927;\(n\)\.




 ### <a id = "user-content-fn-266df0ef" name = "user-content-fn-266df0ef"><K>SetPut</a> ###

<code>static int <strong>&lt;K&gt;SetPut</strong>(struct &lt;K&gt;Set *const <em>set</em>, struct &lt;K&gt;SetItem *const <em>item</em>, const struct &lt;K&gt;SetItem **const <em>p_eject</em>)</code>

Puts the `item` in `set` \. Adding an element with the same `K` , according to [&lt;PK&gt;IsEqual](#user-content--f1be3090) `SET_IS_EQUAL` , causes the old data to be ejected\.

 - Parameter: _set_  
   If null, returns false\.
 - Parameter: _item_  
   If null, returns false\. Must not be part this `set` or any other, because the integrety of the other set will be compromised\.
 - Parameter: _p\_eject_  
   If not\-null, this address of a variable that will store the `K` that was replaced, if any\. If null, does nothing\.
 - Return:  
   Success\.
 - Exceptional Return: realloc  
 - Order:  
   Constant time assuming the hash function is uniform; worst &#927;\(n\)\.




 ### <a id = "user-content-fn-2956c7e9" name = "user-content-fn-2956c7e9"><K>SetPutIfAbsent</a> ###

<code>static int <strong>&lt;K&gt;SetPutIfAbsent</strong>(struct &lt;K&gt;Set *const <em>set</em>, struct &lt;K&gt;SetItem *const <em>item</em>, int *const <em>p_is_absent</em>)</code>

Puts the `item` in `set` only if the entry is absent\.

 - Parameter: _set_  
   If null, returns false\.
 - Parameter: _item_  
   If null, returns false\. Must not be part this `set` or any other\.
 - Parameter: _p\_is\_absent_  
   If not\-null, it will signal the successful placement\.
 - Return:  
   Successful operation, including doing nothing because the entry is already in the set\.
 - Exceptional Return: realloc  
 - Order:  
   Constant time assuming the hash function is uniform; worst &#927;\(n\)\.




 ### <a id = "user-content-fn-257b1a0e" name = "user-content-fn-257b1a0e"><K>SetRemove</a> ###

<code>static struct &lt;K&gt;SetItem *<strong>&lt;K&gt;SetRemove</strong>(struct &lt;K&gt;Set *const <em>set</em>, const K <em>key</em>)</code>

Removes an element specified by `key` from `set` \.

 - Return:  
   Successfully removed an element or null\.
 - Order:  
   Constant time assuming the hash function is uniform; worst &#927;\(n\)\.




 ### <a id = "user-content-fn-1b39893a" name = "user-content-fn-1b39893a"><K>SetToString</a> ###

<code>static const char *<strong>&lt;K&gt;SetToString</strong>(const struct &lt;K&gt;Set *const <em>set</em>)</code>

Can print 2 things at once before it overwrites\. One must set `SET_TO_STRING` to a function implementing [&lt;PK&gt;ToString](#user-content--77ec3874) to get this functionality\.

 - Return:  
   Prints `set` in a static buffer\.
 - Order:  
   &#920;\(1\); it has a 1024 character limit; every element takes some of it\.






 ## <a id = "user-content-license" name = "user-content-license">License</a> ##

2019 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT) \.



