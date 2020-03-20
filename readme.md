# Set\.h #

## Parameterised Hash Set ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [&lt;PE&gt;UInt](#user-content-typedef-54b8b39a), [&lt;PE&gt;Type](#user-content-typedef-11e62996), [&lt;PE&gt;MType](#user-content-typedef-7d6f0919), [&lt;PE&gt;Hash](#user-content-typedef-812e78a), [&lt;PE&gt;IsEqual](#user-content-typedef-c1486ede), [&lt;PE&gt;Replace](#user-content-typedef-a4aa6992), [&lt;PE&gt;Action](#user-content-typedef-9c0e506c), [&lt;PE&gt;ToString](#user-content-typedef-a5b40ebe)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;E&gt;SetElement](#user-content-tag-8952cfcc), [&lt;E&gt;Set](#user-content-tag-c69e9d84)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

![Example of &lt;String&gt;Set.](web/set.png)

[&lt;E&gt;Set](#user-content-tag-c69e9d84) is a collection of elements of [&lt;E&gt;SetElement](#user-content-tag-8952cfcc) that doesn't allow duplication; it must be supplied an equality function, `SET_IS_EQUAL` [&lt;PE&gt;IsEqual](#user-content-typedef-c1486ede), and a hash function, `SET_HASH` [&lt;PE&gt;Hash](#user-content-typedef-812e78a)\.

Internally, it is a separately chained, hash set with a maximum load factor of `ln 2`, and power\-of\-two resizes, with buckets as pointers\. This offers some independence of sets from set elements, but cache performance is left up to the caller\. It can be expanded to a hash map or associative array by enclosing the `<E>SetElement` in another `struct`, as appropriate\. While in a set, the elements should not change in a way that affects their hash values\.

`<E>Set` is not synchronised\. Errors are returned with `errno`\. The parameters are `#define` preprocessor macros, and are all undefined at the end of the file for convenience\. `assert.h` is used; to stop assertions, use `#define NDEBUG` before inclusion\.



 * Parameter: SET\_NAME, SET\_TYPE  
   `<E>` that satisfies `C` naming conventions when mangled and a valid [&lt;PE&gt;Type](#user-content-typedef-11e62996) associated therewith; required\. `<PE>` is private, whose names are prefixed in a manner to avoid collisions; any should be re\-defined prior to use elsewhere\.
 * Parameter: SET\_HASH  
   A function satisfying [&lt;PE&gt;Hash](#user-content-typedef-812e78a); required\.
 * Parameter: SET\_IS\_EQUAL  
   A function satisfying [&lt;PE&gt;IsEqual](#user-content-typedef-c1486ede); required\.
 * Parameter: SET\_TO\_STRING  
   Optional print function implementing [&lt;PE&gt;ToString](#user-content-typedef-a5b40ebe); makes available [&lt;E&gt;SetToString](#user-content-fn-b4e4b20)\.
 * Parameter: SET\_POINTER\_GET  
   Usually [&lt;PE&gt;MType](#user-content-typedef-7d6f0919) in the same as [&lt;PE&gt;Type](#user-content-typedef-11e62996); this flag makes `<PE>MType` be a pointer\-to\-`<PE>Type`\. Should be used when the copying of `<PE>Type` into functions is a performance issue\. As well as [&lt;E&gt;SetGet](#user-content-fn-8d1390a0), affects the definition of [&lt;PE&gt;Hash](#user-content-typedef-812e78a) and [&lt;PE&gt;IsEqual](#user-content-typedef-c1486ede)\.
 * Parameter: SET\_NO\_CACHE  
   Calculates the hash every time and discards it; should be used when the hash calculation is trivial to avoid storing duplicate [&lt;PE&gt;UInt](#user-content-typedef-54b8b39a) _per_ datum\.
 * Parameter: SET\_UINT  
   This is [&lt;PE&gt;UInt](#user-content-typedef-54b8b39a) and defaults to `unsigned int`\.
 * Parameter: SET\_TEST  
   Unit testing framework [&lt;E&gt;SetTest](#user-content-fn-382b20c0), included in a separate header, [\.\./test/TestSet\.h](../test/TestSet.h)\. Must be defined equal to a random filler function, satisfying [&lt;PE&gt;Action](#user-content-typedef-9c0e506c)\. Requires `SET_TO_STRING` and not `NDEBUG`\.
 * Standard:  
   C89
 * See also:  
   [Array](https://github.com/neil-edelman/Array); [Heap](https://github.com/neil-edelman/Heap); [List](https://github.com/neil-edelman/List); [Orcish](https://github.com/neil-edelman/Orcish); [Pool](https://github.com/neil-edelman/Pool); [Trie](https://github.com/neil-edelman/Trie)


## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-54b8b39a" name = "user-content-typedef-54b8b39a">&lt;PE&gt;UInt</a> ###

<code>typedef SET_UINT <strong>&lt;PE&gt;UInt</strong>;</code>

Valid unsigned integer type used for hash values\. The hash map will saturate at `min(((ln 2)/2) &#183; range(<PE>UInt), (1/8) &#183; range(size_t))`, at which point no new buckets can be added and the load factor will increase over the maximum\.



### <a id = "user-content-typedef-11e62996" name = "user-content-typedef-11e62996">&lt;PE&gt;Type</a> ###

<code>typedef SET_TYPE <strong>&lt;PE&gt;Type</strong>;</code>

Valid tag type defined by `SET_TYPE`\. Included in [&lt;E&gt;SetElement](#user-content-tag-8952cfcc)\.



### <a id = "user-content-typedef-7d6f0919" name = "user-content-typedef-7d6f0919">&lt;PE&gt;MType</a> ###

<code>typedef const &lt;PE&gt;Type *<strong>&lt;PE&gt;MType</strong>;</code>

`SET_POINTER_GET` modifies `<PE>MType` to be a pointer, otherwise it's the same as [&lt;PE&gt;Type](#user-content-typedef-11e62996)\.



### <a id = "user-content-typedef-812e78a" name = "user-content-typedef-812e78a">&lt;PE&gt;Hash</a> ###

<code>typedef &lt;PE&gt;UInt(*<strong>&lt;PE&gt;Hash</strong>)(const &lt;PE&gt;MType);</code>

A map from [&lt;PE&gt;MType](#user-content-typedef-7d6f0919) onto [&lt;PE&gt;UInt](#user-content-typedef-54b8b39a)\. Should be as close as possible to a discrete uniform distribution for maximum performance\.



### <a id = "user-content-typedef-c1486ede" name = "user-content-typedef-c1486ede">&lt;PE&gt;IsEqual</a> ###

<code>typedef int(*<strong>&lt;PE&gt;IsEqual</strong>)(const &lt;PE&gt;MType a, const &lt;PE&gt;MType b);</code>

Equivalence relation between [&lt;PE&gt;MType](#user-content-typedef-7d6f0919) that satisfies `<PE>IsEqual(a, b) -> <PE>Hash(a) == <PE>Hash(b)`\.



### <a id = "user-content-typedef-a4aa6992" name = "user-content-typedef-a4aa6992">&lt;PE&gt;Replace</a> ###

<code>typedef int(*<strong>&lt;PE&gt;Replace</strong>)(&lt;PE&gt;Type *original, &lt;PE&gt;Type *replace);</code>

A di\-predicate; returns true if the `replace` replaces the `original`; used in [&lt;E&gt;SetPolicyPut](#user-content-fn-2ceb4efb)\.



### <a id = "user-content-typedef-9c0e506c" name = "user-content-typedef-9c0e506c">&lt;PE&gt;Action</a> ###

<code>typedef void(*<strong>&lt;PE&gt;Action</strong>)(&lt;PE&gt;Type *);</code>

Operates by side\-effects\. Used for `SET_TEST`\.



### <a id = "user-content-typedef-a5b40ebe" name = "user-content-typedef-a5b40ebe">&lt;PE&gt;ToString</a> ###

<code>typedef void(*<strong>&lt;PE&gt;ToString</strong>)(const &lt;PE&gt;Type *, char(*)[12]);</code>

Responsible for turning [&lt;PE&gt;Type](#user-content-typedef-11e62996) into a 12\-`char` null\-terminated output string\. Used for `SET_TO_STRING`\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-8952cfcc" name = "user-content-tag-8952cfcc">&lt;E&gt;SetElement</a> ###

<code>struct <strong>&lt;E&gt;SetElement</strong>;</code>

Contains [&lt;PE&gt;Type](#user-content-typedef-11e62996) as the first element `key`, along with data internal to the set\. Storage of the `<E>SetElement` structure is the responsibility of the caller\.



### <a id = "user-content-tag-c69e9d84" name = "user-content-tag-c69e9d84">&lt;E&gt;Set</a> ###

<code>struct <strong>&lt;E&gt;Set</strong>;</code>

To initialise, see [&lt;E&gt;Set](#user-content-fn-c69e9d84)\. Assigning `{0}` \(`C99`\+\) or `SET_IDLE` as the initialiser, or being part of `static` data, also puts it in an idle state, \(no dynamic memory allocated\.\)

![States.](web/states.png)



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-86b27fc1">&lt;E&gt;Set_</a></td><td>set</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-c69e9d84">&lt;E&gt;Set</a></td><td>set</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-66181859">&lt;E&gt;SetClear</a></td><td>set</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#user-content-fn-2dff525d">&lt;E&gt;SetSize</a></td><td>set</td></tr>

<tr><td align = right>static struct &lt;E&gt;SetElement *</td><td><a href = "#user-content-fn-8d1390a0">&lt;E&gt;SetGet</a></td><td>set, data</td></tr>

<tr><td align = right>static int</td><td><a href = "#user-content-fn-33c00814">&lt;E&gt;SetReserve</a></td><td>set, reserve</td></tr>

<tr><td align = right>static struct &lt;E&gt;SetElement *</td><td><a href = "#user-content-fn-df6b38cd">&lt;E&gt;SetPut</a></td><td>set, element</td></tr>

<tr><td align = right>static struct &lt;E&gt;SetElement *</td><td><a href = "#user-content-fn-2ceb4efb">&lt;E&gt;SetPolicyPut</a></td><td>set, element, replace</td></tr>

<tr><td align = right>static struct &lt;E&gt;SetElement *</td><td><a href = "#user-content-fn-21a4ad4">&lt;E&gt;SetRemove</a></td><td>set, data</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#user-content-fn-b4e4b20">&lt;E&gt;SetToString</a></td><td>set</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-382b20c0">&lt;E&gt;SetTest</a></td><td>parent_new, parent</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-86b27fc1" name = "user-content-fn-86b27fc1">&lt;E&gt;Set_</a> ###

<code>static void <strong>&lt;E&gt;Set_</strong>(struct &lt;E&gt;Set *const <em>set</em>)</code>

Destructor for active `set`\. After, it takes no memory and is in an idle state\. If idle, does nothing\.



### <a id = "user-content-fn-c69e9d84" name = "user-content-fn-c69e9d84">&lt;E&gt;Set</a> ###

<code>static void <strong>&lt;E&gt;Set</strong>(struct &lt;E&gt;Set *const <em>set</em>)</code>

Initialises `set` to be take no memory and be in an idle state\. Calling this on an active set will cause memory leaks\.

 * Parameter: _set_  
   If null, does nothing\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-66181859" name = "user-content-fn-66181859">&lt;E&gt;SetClear</a> ###

<code>static void <strong>&lt;E&gt;SetClear</strong>(struct &lt;E&gt;Set *const <em>set</em>)</code>

Clears and removes all entries from `set`\. The capacity and memory of the hash table is preserved, but all previous values are un\-associated\. The load factor will be less until it reaches it's previous size\.

 * Parameter: _set_  
   If null, does nothing\.
 * Order:  
   &#920;\(`set.buckets`\)




### <a id = "user-content-fn-2dff525d" name = "user-content-fn-2dff525d">&lt;E&gt;SetSize</a> ###

<code>static size_t <strong>&lt;E&gt;SetSize</strong>(const struct &lt;E&gt;Set *const <em>set</em>)</code>

 * Parameter: _set_  
   If null, returns 0\.
 * Return:  
   The number of entries in the `set`\.
 * Order:  
   &#920;\(1\)




### <a id = "user-content-fn-8d1390a0" name = "user-content-fn-8d1390a0">&lt;E&gt;SetGet</a> ###

<code>static struct &lt;E&gt;SetElement *<strong>&lt;E&gt;SetGet</strong>(struct &lt;E&gt;Set *const <em>set</em>, const &lt;PE&gt;MType <em>data</em>)</code>

Queries whether `data` is is `set`\.

 * Parameter: _set_  
   If null, returns null\.
 * Return:  
   The value which [&lt;PE&gt;IsEqual](#user-content-typedef-c1486ede) `data`, or, if no such value exists, null\.
 * Order:  
   Average &#927;\(1\), \(hash distributes elements uniformly\); worst &#927;\(n\)\.




### <a id = "user-content-fn-33c00814" name = "user-content-fn-33c00814">&lt;E&gt;SetReserve</a> ###

<code>static int <strong>&lt;E&gt;SetReserve</strong>(struct &lt;E&gt;Set *const <em>set</em>, const size_t <em>reserve</em>)</code>

Reserve at least `reserve`, divided by the maximum load factor, space in the buckets of `set`\.

 * Return:  
   Success\.
 * Exceptional return: ERANGE  
   `reserve` plus the size would take a bigger number then could fit in a `size_t`\.
 * Exceptional return: realloc  




### <a id = "user-content-fn-df6b38cd" name = "user-content-fn-df6b38cd">&lt;E&gt;SetPut</a> ###

<code>static struct &lt;E&gt;SetElement *<strong>&lt;E&gt;SetPut</strong>(struct &lt;E&gt;Set *const <em>set</em>, struct &lt;E&gt;SetElement *const <em>element</em>)</code>

Puts the `element` in `set`\.

 * Parameter: _set_  
   If null, returns null\.
 * Parameter: _element_  
   If null, returns null\. Should not be of a set because the integrity of that set will be compromised\.
 * Return:  
   Any ejected element or null\. \(An ejected element has [&lt;PE&gt;IsEqual](#user-content-typedef-c1486ede) `SET_IS_EQUAL` the `element`\.\)
 * Exceptional return: realloc, ERANGE  
   There was an error with a re\-sizing\. Successfully calling [&lt;E&gt;SetReserve](#user-content-fn-33c00814) with at least one before ensures that this does not happen\.
 * Order:  
   Average amortised &#927;\(1\), \(hash distributes elements uniformly\); worst &#927;\(n\)\.




### <a id = "user-content-fn-2ceb4efb" name = "user-content-fn-2ceb4efb">&lt;E&gt;SetPolicyPut</a> ###

<code>static struct &lt;E&gt;SetElement *<strong>&lt;E&gt;SetPolicyPut</strong>(struct &lt;E&gt;Set *const <em>set</em>, struct &lt;E&gt;SetElement *const <em>element</em>, const &lt;PE&gt;Replace <em>replace</em>)</code>

Puts the `element` in `set` only if the entry is absent or if calling `replace` returns true\.

 * Parameter: _set_  
   If null, returns null\.
 * Parameter: _element_  
   If null, returns null\. Should not be of a set because the integrity of that set will be compromised\.
 * Parameter: _replace_  
   Called on collision and only replaces it if the function returns true\. If null, doesn't do any replacement on collision\.
 * Return:  
   Any ejected element or null\. On collision, if `replace` returns false or `replace` is null, returns `element` and leaves the other element in the set\.
 * Exceptional return: realloc, ERANGE  
   There was an error with a re\-sizing\. Successfully calling [&lt;E&gt;SetReserve](#user-content-fn-33c00814) with at least one before ensures that this does not happen\.
 * Order:  
   Average amortised &#927;\(1\), \(hash distributes elements uniformly\); worst &#927;\(n\)\.




### <a id = "user-content-fn-21a4ad4" name = "user-content-fn-21a4ad4">&lt;E&gt;SetRemove</a> ###

<code>static struct &lt;E&gt;SetElement *<strong>&lt;E&gt;SetRemove</strong>(struct &lt;E&gt;Set *const <em>set</em>, const &lt;PE&gt;MType <em>data</em>)</code>

Removes an element `data` from `set`\.

 * Return:  
   Successfully ejected element or null\. This element is free to be put into another set or modify it's hash values\.
 * Order:  
   Average &#927;\(1\), \(hash distributes elements uniformly\); worst &#927;\(n\)\.




### <a id = "user-content-fn-b4e4b20" name = "user-content-fn-b4e4b20">&lt;E&gt;SetToString</a> ###

<code>static const char *<strong>&lt;E&gt;SetToString</strong>(const struct &lt;E&gt;Set *const <em>set</em>)</code>

Can print 2 things at once before it overwrites\. One must set `SET_TO_STRING` to a function implementing [&lt;PE&gt;ToString](#user-content-typedef-a5b40ebe) to get this functionality\.

 * Return:  
   Prints `set` in a static buffer in order by bucket\.
 * Order:  
   &#920;\(1\); it has a 1024 character limit; every element takes some\.




### <a id = "user-content-fn-382b20c0" name = "user-content-fn-382b20c0">&lt;E&gt;SetTest</a> ###

<code>static void <strong>&lt;E&gt;SetTest</strong>(struct &lt;E&gt;SetElement *(*const <em>parent_new</em>)(void *), void *const <em>parent</em>)</code>

The list will be tested on `stdout`\. Requires `SET_TEST` to be a [&lt;PE&gt;Action](#user-content-typedef-9c0e506c) and `SET_TO_STRING`\.

 * Parameter: _parent\_new_  
   Specifies the dynamic up\-level creator of the parent `struct`\. Could be null; then testing will be done statically on an array of [&lt;E&gt;SetElement](#user-content-tag-8952cfcc) and `SET_TEST` is not allowed to go over the limits of the data type\.
 * Parameter: _parent_  
   The parameter passed to `parent_new`\. Ignored if `parent_new` is null\.






## <a id = "user-content-license" name = "user-content-license">License</a> ##

2019 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



