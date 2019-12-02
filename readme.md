 # Set\.h #

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef):  [&lt;PE&gt;Hash](#user-content-typedef-812e78a), [&lt;PE&gt;IsEqual](#user-content-typedef-c1486ede), [&lt;PE&gt;ToString](#user-content-typedef-a5b40ebe), [&lt;PE&gt;Action](#user-content-typedef-9c0e506c)
 * [Struct, Union, and Enum Definitions](#user-content-tag):  [&lt;E&gt;SetItem](#user-content-tag-f1847bfb), [&lt;E&gt;Set](#user-content-tag-c69e9d84)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

 ## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

`<E>Set` is a collection of elements of type `E` , along with a hash function and equality function, that doesn't allow duplication\. Internally, it is a hash set, and collisions are handled by separate chaining\. It requires the storage of [&lt;E&gt;SetItem](#user-content-raw_p-f1847bfb), which is `E` along with data internal to the set; one can get the `E` by doing [&lt;E&gt;SetItem](#user-content-(null)-f1847bfb)\. The maximum load factor is `ln 2` \. While in the set, the values cannot change\. One can use this as the key in an associative array\.

 - Parameter: SET\_NAME, SET\_TYPE  
   `E` that satisfies `C` naming conventions when mangled; required\.
 - Parameter: SET\_HASH  
   A function satisfying [&lt;PE&gt;Hash](#user-content--812e78a); required\.
 - Parameter: SET\_IS\_EQUAL  
   A function satisfying [&lt;PE&gt;IsEqual](#user-content--c1486ede); required\.
 - Parameter: SET\_NO\_CACHE  
   Always calculates the hash every time and don't store it _per_ datum\. Best used when the data to be hashed is very small, \(_viz_ , the hash calculation is trivial\.\)
 - Parameter: SET\_TO\_STRING  
   Optional print function implementing [&lt;PE&gt;ToString](#user-content--a5b40ebe); makes available [&lt;E&gt;SetToString](#user-content-(null)-b4e4b20)\.
 - Parameter: SET\_TEST  
   Unit testing framework, included in a separate header, [\.\./test/SetTest\.h](../test/SetTest.h)\. Must be defined equal to a random filler function, satisfying [&lt;PV&gt;Action](#user-content--4585d713)\. Requires `SET\_TO\_STRING` \.
 * Standard:  
   C89/90
 * Caveat:  
   Implement tests\.




 ## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

 ### <a id = "user-content-typedef-812e78a" name = "user-content-typedef-812e78a"><PE>Hash</a> ###

<code>typedef unsigned(*<strong>&lt;PE&gt;Hash</strong>)(const E);</code>

A map from `E` onto `unsigned int` \. Should be as close as possible to a discrete uniform distribution for maximum performance and, when computing, take all of `E` into account\.



 ### <a id = "user-content-typedef-c1486ede" name = "user-content-typedef-c1486ede"><PE>IsEqual</a> ###

<code>typedef int(*<strong>&lt;PE&gt;IsEqual</strong>)(const E, const E);</code>

A constant equivalence relation between `E` that satisfies `<PE>IsEqual\(a, b\) \-> <PE>Hash\(a\) == <PE>Hash\(b\)` \.



 ### <a id = "user-content-typedef-a5b40ebe" name = "user-content-typedef-a5b40ebe"><PE>ToString</a> ###

<code>typedef void(*<strong>&lt;PE&gt;ToString</strong>)(const E *const, char(*const)[12]);</code>

Responsible for turning `E` \(the first argument\) into a 12 `char` string \(the second\.\)



 ### <a id = "user-content-typedef-9c0e506c" name = "user-content-typedef-9c0e506c"><PE>Action</a> ###

<code>typedef void(*<strong>&lt;PE&gt;Action</strong>)(const E *const);</code>

Used for `SET\_TEST` \.



 ## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

 ### <a id = "user-content-tag-f1847bfb" name = "user-content-tag-f1847bfb"><E>SetItem</a> ###

<code>struct <strong>&lt;E&gt;SetItem</strong>;</code>

Contains `E` and more internal to the working of the hash\. Storage of the `<E>SetItem` structure is the responsibility of the caller; it could be one part of a complicated structure\.



 ### <a id = "user-content-tag-c69e9d84" name = "user-content-tag-c69e9d84"><E>Set</a> ###

<code>struct <strong>&lt;E&gt;Set</strong>;</code>

A `<E>Set` \. To initianise, see [&lt;E&gt;Set](#user-content-(null)-c69e9d84)\.



 ## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-86b27fc1">&lt;E&gt;Set_</a></td><td>set</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-c69e9d84">&lt;E&gt;Set</a></td><td>set</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-66181859">&lt;E&gt;SetClear</a></td><td>set</td></tr>

<tr><td align = right>static const E *</td><td><a href = "#user-content-fn-8d1390a0">&lt;E&gt;SetGet</a></td><td>set, key</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-df6b38cd">&lt;E&gt;SetPut</a></td><td>set, item, p_eject</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-c6ea7aa7">&lt;E&gt;SetPutIfAbsent</a></td><td>set, item, p_is_absent</td></tr>

<tr><td align = right>static struct &lt;E&gt;SetItem *</td><td><a href = "#user-content-fn-21a4ad4">&lt;E&gt;SetRemove</a></td><td>set, key</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-b4e4b20">&lt;E&gt;SetToString</a></td><td>set</td></tr>

</table>



 ## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

 ### <a id = "user-content-fn-86b27fc1" name = "user-content-fn-86b27fc1"><E>Set_</a> ###

<code>static void <strong>&lt;E&gt;Set_</strong>(struct &lt;E&gt;Set *const <em>set</em>)</code>

Destructor for `set` \. After, it takes no memory and is in an empty state\.



 ### <a id = "user-content-fn-c69e9d84" name = "user-content-fn-c69e9d84"><E>Set</a> ###

<code>static void <strong>&lt;E&gt;Set</strong>(struct &lt;E&gt;Set *const <em>set</em>)</code>

Initialises `set` to be take no memory and be in an empty state\. If it is `static` data, then it is initialised by default\. Alternatively, assigning `\{0\}` \(`C99` \+\) or `SET\_ZERO` as the initialiser also puts it in an empty state\. Calling this on an active set will cause memory leaks\.

 - Parameter: _set_  
   If null, does nothing\.
 - Order:  
   &#920;\(1\)




 ### <a id = "user-content-fn-66181859" name = "user-content-fn-66181859"><E>SetClear</a> ###

<code>static void <strong>&lt;E&gt;SetClear</strong>(struct &lt;E&gt;Set *const <em>set</em>)</code>

Clears and removes all entries from `set` \. The capacity and memory of the hash table is preserved, but all previous values are un\-associated\. The load factor will be less until it reaches it's previous size\.

 - Parameter: _set_  
   If null, does nothing\.
 - Order:  
   &#920;\(`set\.buckets` \)




 ### <a id = "user-content-fn-8d1390a0" name = "user-content-fn-8d1390a0"><E>SetGet</a> ###

<code>static const E *<strong>&lt;E&gt;SetGet</strong>(struct &lt;E&gt;Set *const <em>set</em>, const E <em>key</em>)</code>

Gets `item` from `set` \.

 - Return:  
   The value which [&lt;PE&gt;IsEqual](#user-content--c1486ede) the `item` , or, if no such value exists, null\.
 - Order:  
   Constant time assuming the hash function is uniform; worst &#927;\(n\)\.




 ### <a id = "user-content-fn-df6b38cd" name = "user-content-fn-df6b38cd"><E>SetPut</a> ###

<code>static int <strong>&lt;E&gt;SetPut</strong>(struct &lt;E&gt;Set *const <em>set</em>, struct &lt;E&gt;SetItem *const <em>item</em>, const struct &lt;E&gt;SetItem **const <em>p_eject</em>)</code>

Puts the `item` in `set` \. Adding an element with the same `E` , according to [&lt;PE&gt;IsEqual](#user-content--c1486ede) `SET\_IS\_EQUAL` , causes the old data to be ejected\.

 - Parameter: _set_  
   If null, returns false\.
 - Parameter: _item_  
   If null, returns false\. Must not be part this `set` or any other, because the integrety of the other set will be compromised\.
 - Parameter: _p\_eject_  
   If not\-null, this address of a variable that will store the `E` that was replaced, if any\. If null, does nothing\.
 - Return:  
   Success\.
 - Exceptional Return: realloc  
 - Order:  
   Constant time assuming the hash function is uniform; worst &#927;\(n\)\.




 ### <a id = "user-content-fn-c6ea7aa7" name = "user-content-fn-c6ea7aa7"><E>SetPutIfAbsent</a> ###

<code>static int <strong>&lt;E&gt;SetPutIfAbsent</strong>(struct &lt;E&gt;Set *const <em>set</em>, struct &lt;E&gt;SetItem *const <em>item</em>, int *const <em>p_is_absent</em>)</code>

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




 ### <a id = "user-content-fn-21a4ad4" name = "user-content-fn-21a4ad4"><E>SetRemove</a> ###

<code>static struct &lt;E&gt;SetItem *<strong>&lt;E&gt;SetRemove</strong>(struct &lt;E&gt;Set *const <em>set</em>, const E <em>key</em>)</code>

Removes an element specified by `key` from `set` \.

 - Return:  
   Successfully removed an element or null\.
 - Order:  
   Constant time assuming the hash function is uniform; worst &#927;\(n\)\.




 ### <a id = "user-content-fn-b4e4b20" name = "user-content-fn-b4e4b20"><E>SetToString</a> ###

<code>static const char *<strong>&lt;E&gt;SetToString</strong>(const struct &lt;E&gt;Set *const <em>set</em>)</code>

Can print 2 things at once before it overwrites\. One must set `SET\_TO\_STRING` to a function implementing [&lt;PE&gt;ToString](#user-content--a5b40ebe) to get this functionality\.

 - Return:  
   Prints `set` in a static buffer\.
 - Order:  
   &#920;\(1\); it has a 1024 character limit; every element takes some of it\.






 ## <a id = "user-content-license" name = "user-content-license">License</a> ##

2019 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT) \.



