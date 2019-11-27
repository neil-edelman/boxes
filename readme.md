 # Array\.h #

 * [Desciption](#preamble-)
 * [Typedef Aliases](#typedef-): <a href = "#typedef-<PT>ToString"><PT>ToString</a>, <a href = "#typedef-<PT>Action"><PT>Action</a>, <a href = "#typedef-<PT>Predicate"><PT>Predicate</a>
 * [Struct, Union, and Enum Definitions](#tag-): <a href = "#tag-<T>Array"><T>Array</a>
 * [Function Summary](#summary-)
 * [Function Definitions](#fn-)
 * [License](#license-)

 ## <a name = "preamble-">Description</a> ##

`<T>Array` is a dynamic contiguous array that stores `<T>` , which must be set using `ARRAY_TYPE` \. To ensure that the capacity is greater then or equal to the size, resizing may be necessary and incurs amortised cost\. When adding new elements, the elements may change memory location to fit\. It is therefore unstable; any pointers to this memory may become stale and unusable when expanding\.

`<T>Array` is not synchronised\. Errors are returned with `errno` \. The parameters are preprocessor macros, and are all undefined at the end of the file for convenience\. `assert.h` is included in this file; to stop the debug assertions, use `#define NDEBUG` before inclusion\.

![States](web/states.png)

@title Contiguous Dynamic Parameterised Array

 - Parameter: ARRAY\_NAME, ARRAY\_TYPE  
   The name that literally becomes `<T>` , and a valid type associated therewith, accessible to the compiler at the time of inclusion; should be conformant to naming and to the maximum available length of identifiers\. Must each be present before including\.
 - Parameter: ARRAY\_STACK  
   Doesn't define removal functions except [<T>ArrayPop](#fn-<T>ArrayPop), making it a stack\.
 - Parameter: ARRAY\_TO\_STRING  
   Optional print function implementing [<PT>ToString](#typedef-<PT>ToString); makes available [<T>ArrayToString](#fn-<T>ArrayToString)\.
 - Parameter: ARRAY\_TEST  
   Unit testing framework using `<T>ArrayTest` , included in a separate header, `../test/ArrayTest.h` \. Must be defined equal to a \(random\) filler function, satisfying [<PT>Action](#typedef-<PT>Action)\. Requires `ARRAY_TO_STRING` and not `NDEBUG` \.
 * Author:  
   Neil
 * Standard:  
   C89
 * Caveat:  
   (<a href = "#fn-<T>ArrayIndex"><T>ArrayIndex</a>, <a href = "#fn-<T>ArrayPeek"><T>ArrayPeek</a>, <a href = "#fn-<T>ArrayUpdateNew"><T>ArrayUpdateNew</a>, <a href = "#fn-<T>ArrayBuffer"><T>ArrayBuffer</a>, <a href = "#fn-<T>ArrayExpand"><T>ArrayExpand</a>, <a href = "#fn-<T>ArrayEach"><T>ArrayEach</a>, <a href = "#fn-<T>ArrayEach"><T>ArrayEach</a>, <a href = "#fn-<T>ArrayIfEach"><T>ArrayIfEach</a>, <a href = "#fn-<T>ArrayIfEach"><T>ArrayIfEach</a>, <a href = "#fn-<T>ArrayAny"><T>ArrayAny</a>, <a href = "#fn-<T>ArrayAny"><T>ArrayAny</a>, <a href = "#fn-<T>ArrayToString"><T>ArrayToString</a>)




 ## <a name = "typedef-">Typedef Aliases</a> ##

 ### <a name = "typedef-<PT>ToString" id = "typedef-<PT>ToString"><PT>ToString</a> ###

`typedef void(*`**`<PT>ToString`**`)(const T *, char(*const)[12]);`

Responsible for turning `<T>` \(the first argument\) into a 12 `char` null\-terminated output string \(the second\.\) Private; must re\-declare\. Used for `ARRAY_TO_STRING` \.



 ### <a name = "typedef-<PT>Action" id = "typedef-<PT>Action"><PT>Action</a> ###

`typedef void(*`**`<PT>Action`**`)(T *const data);`

Operates by side\-effects on `data` only\. Private; must re\-declare\.



 ### <a name = "typedef-<PT>Predicate" id = "typedef-<PT>Predicate"><PT>Predicate</a> ###

`typedef int(*`**`<PT>Predicate`**`)(const T *const data);`

Given constant `data` , returns a boolean\. Private; must re\-declare\.





 ## <a name = "tag-">Struct, Union, and Enum Definitions</a> ##

 ### <a name = "tag-<T>Array" id = "tag-<T>Array"><T>Array</a> ###

`struct `**`<T>Array`**`;`

The array\. Zeroed data is a valid state\. To instantiate explicitly, see [<T>Array](#fn-<T>Array) or initialise it with `ARRAY_INIT` or `{0}` \(C99\.\)





 ## <a name = "summary-">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#fn-<T>Array_"><T>Array_</a></td><td>a</td></tr>

<tr><td align = right>static void</td><td><a href = "#fn-<T>Array"><T>Array</a></td><td>a</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#fn-<T>ArraySize"><T>ArraySize</a></td><td>a</td></tr>

<tr><td align = right>static int</td><td><a href = "#fn-<T>ArrayRemove"><T>ArrayRemove</a></td><td>a, data</td></tr>

<tr><td align = right>static int</td><td><a href = "#fn-<T>ArrayLazyRemove"><T>ArrayLazyRemove</a></td><td>a, data</td></tr>

<tr><td align = right>static void</td><td><a href = "#fn-<T>ArrayClear"><T>ArrayClear</a></td><td>a</td></tr>

<tr><td align = right>static T *</td><td><a href = "#fn-<T>ArrayGet"><T>ArrayGet</a></td><td>a</td></tr>

<tr><td align = right>static T *</td><td><a href = "#fn-<T>ArrayEnd"><T>ArrayEnd</a></td><td>a</td></tr>

<tr><td align = right>static size_t</td><td><a href = "#fn-<T>ArrayIndex"><T>ArrayIndex</a></td><td>a, data</td></tr>

<tr><td align = right>static T *</td><td><a href = "#fn-<T>ArrayPeek"><T>ArrayPeek</a></td><td>a</td></tr>

<tr><td align = right>static T *</td><td><a href = "#fn-<T>ArrayPop"><T>ArrayPop</a></td><td>a</td></tr>

<tr><td align = right>static T *</td><td><a href = "#fn-<T>ArrayBack"><T>ArrayBack</a></td><td>a, here</td></tr>

<tr><td align = right>static T *</td><td><a href = "#fn-<T>ArrayNext"><T>ArrayNext</a></td><td>a, here</td></tr>

<tr><td align = right>static T *</td><td><a href = "#fn-<T>ArrayNew"><T>ArrayNew</a></td><td>a</td></tr>

<tr><td align = right>static T *</td><td><a href = "#fn-<T>ArrayUpdateNew"><T>ArrayUpdateNew</a></td><td>a, update_ptr</td></tr>

<tr><td align = right>static T *</td><td><a href = "#fn-<T>ArrayBuffer"><T>ArrayBuffer</a></td><td>a, buffer</td></tr>

<tr><td align = right>static int</td><td><a href = "#fn-<T>ArrayExpand"><T>ArrayExpand</a></td><td>a, add</td></tr>

<tr><td align = right>static void</td><td><a href = "#fn-<T>ArrayEach"><T>ArrayEach</a></td><td>a, action</td></tr>

<tr><td align = right>static void</td><td><a href = "#fn-<T>ArrayIfEach"><T>ArrayIfEach</a></td><td>a, predicate, action</td></tr>

<tr><td align = right>static T *</td><td><a href = "#fn-<T>ArrayAny"><T>ArrayAny</a></td><td>a, predicate</td></tr>

<tr><td align = right>static void</td><td><a href = "#fn-<T>ArrayKeepIf"><T>ArrayKeepIf</a></td><td>a, keep</td></tr>

<tr><td align = right>static void</td><td><a href = "#fn-<T>ArrayTrim"><T>ArrayTrim</a></td><td>a, predicate</td></tr>

<tr><td align = right>static int</td><td><a href = "#fn-<T>ArraySplice"><T>ArraySplice</a></td><td>a, anchor, range, b</td></tr>

<tr><td align = right>static int</td><td><a href = "#fn-<T>ArrayIndexSplice"><T>ArrayIndexSplice</a></td><td>a, i0, i1, b</td></tr>

<tr><td align = right>static const char *</td><td><a href = "#fn-<T>ArrayToString"><T>ArrayToString</a></td><td>a</td></tr>

</table>



 ## <a name = "fn-">Function Definitions</a> ##

 ### <a name = "fn-<T>Array_" id = "fn-<T>Array_"><T>Array_</a> ###

`static void `**`<T>Array_`**`(struct <T>Array *const `_`a`_`)`

Destructor for `a` ; returns an initialised `a` to the empty state where it takes no dynamic memory\.

 - Parameter: _a_  
   If null, does nothing\.
 - Order:  
   &#920;\(1\)




 ### <a name = "fn-<T>Array" id = "fn-<T>Array"><T>Array</a> ###

`static void `**`<T>Array`**`(struct <T>Array *const `_`a`_`)`

Initialises `a` to be empty\.

 - Order:  
   &#920;\(1\)




 ### <a name = "fn-<T>ArraySize" id = "fn-<T>ArraySize"><T>ArraySize</a> ###

`static size_t `**`<T>ArraySize`**`(const struct <T>Array *const `_`a`_`)`

 - Parameter: _a_  
   If null, returns zero\.
 - Return:  
   The size of `a` \.
 - Order:  
   &#927;\(1\)




 ### <a name = "fn-<T>ArrayRemove" id = "fn-<T>ArrayRemove"><T>ArrayRemove</a> ###

`static int `**`<T>ArrayRemove`**`(struct <T>Array *const `_`a`_`, T *const `_`data`_`)`

Removes `data` from `a` \.

 - Parameter: _a_  
   If null, returns false\.
 - Parameter: _data_  
   If null, returns false\. Will be removed; data will remain the same but be updated to the next element, or if this was the last element, the pointer will be past the end\.
 - Return:  
   Success, otherwise `errno` will be set for valid input\.
 - Exceptional Return: EDOM  
   `data` is not part of `a` \.
 - Order:  
   &#927;\(n\)\.




 ### <a name = "fn-<T>ArrayLazyRemove" id = "fn-<T>ArrayLazyRemove"><T>ArrayLazyRemove</a> ###

`static int `**`<T>ArrayLazyRemove`**`(struct <T>Array *const `_`a`_`, T *const `_`data`_`)`

Removes `data` from `a` and replaces the spot it was in with the tail\.

 - Parameter: _a_  
   If null, returns false\.
 - Parameter: _data_  
   If null, returns false\. Will be removed; data will remain the same but be updated to the last element, or if this was the last element, the pointer will be past the end\.
 - Return:  
   Success, otherwise `errno` will be set for valid input\.
 - Exceptional Return: EDOM  
   `data` is not part of `a` \.
 - Order:  
   &#927;\(1\)\.




 ### <a name = "fn-<T>ArrayClear" id = "fn-<T>ArrayClear"><T>ArrayClear</a> ###

`static void `**`<T>ArrayClear`**`(struct <T>Array *const `_`a`_`)`

Sets the size of `a` to zero, effectively removing all the elements, but leaves the capacity alone, \(the only thing that will free memory allocation is [<T>Array\_](#fn-<T>Array\_)\.\)

 - Parameter: _a_  
   If null, does nothing\.
 - Order:  
   &#920;\(1\)




 ### <a name = "fn-<T>ArrayGet" id = "fn-<T>ArrayGet"><T>ArrayGet</a> ###

`static T *`**`<T>ArrayGet`**`(const struct <T>Array *const `_`a`_`)`

Causing something to be added to the `<T>Array` may invalidate this pointer, see [<T>ArrayUpdateNew](#fn-<T>ArrayUpdateNew)\.

 - Parameter: _a_  
   If null, returns null\.
 - Return:  
   A pointer to the `a` 's data, indexable up to the `a` 's size\.
 - Order:  
   &#920;\(1\)




 ### <a name = "fn-<T>ArrayEnd" id = "fn-<T>ArrayEnd"><T>ArrayEnd</a> ###

`static T *`**`<T>ArrayEnd`**`(const struct <T>Array *const `_`a`_`)`

Causing something to be added to the `<T>Array` may invalidate this pointer, see [<T>ArrayUpdateNew](#fn-<T>ArrayUpdateNew)\.

 - Parameter: _a_  
   If null, returns null\.
 - Return:  
   One past the end of the array; take care when dereferencing, as it is not part of the array\.
 - Order:  
   &#920;\(1\)




 ### <a name = "fn-<T>ArrayIndex" id = "fn-<T>ArrayIndex"><T>ArrayIndex</a> ###

`static size_t `**`<T>ArrayIndex`**`(const struct <T>Array *const `_`a`_`, const T *const `_`data`_`)`

Gets an index given `data` \.

 - Parameter: _a_  
   Must be a valid object that stores `data` \.
 - Parameter: _data_  
   If the element is not part of the `a` , behaviour is undefined\.
 - Return:  
   An index\.
 - Order:  
   &#920;\(1\)
 - Caveat:  
   Untested\.




 ### <a name = "fn-<T>ArrayPeek" id = "fn-<T>ArrayPeek"><T>ArrayPeek</a> ###

`static T *`**`<T>ArrayPeek`**`(const struct <T>Array *const `_`a`_`)`

 - Parameter: _a_  
   If null, returns null\.
 - Return:  
   The last element or null if the a is empty\. Causing something to be added to the `a` may invalidate this pointer\.
 - Order:  
   &#920;\(1\)
 - Caveat:  
   Untested\.




 ### <a name = "fn-<T>ArrayPop" id = "fn-<T>ArrayPop"><T>ArrayPop</a> ###

`static T *`**`<T>ArrayPop`**`(struct <T>Array *const `_`a`_`)`

The same value as [<T>ArrayPeek](#fn-<T>ArrayPeek)\.

 - Parameter: _a_  
   If null, returns null\.
 - Return:  
   Value from the the top of the `a` that is removed or null if the stack is empty\. Causing something to be added to the `a` may invalidate this pointer\. See [<T>ArrayUpdateNew](#fn-<T>ArrayUpdateNew)\.
 - Order:  
   &#920;\(1\)




 ### <a name = "fn-<T>ArrayBack" id = "fn-<T>ArrayBack"><T>ArrayBack</a> ###

`static T *`**`<T>ArrayBack`**`(const struct <T>Array *const `_`a`_`, const T *const `_`here`_`)`

Iterate through `a` backwards\.

 - Parameter: _a_  
   The array; if null, returns null\.
 - Parameter: _here_  
   Set it to null to get the last element, if it exists\.
 - Return:  
   A pointer to the previous element or null if it does not exist\.
 - Order:  
   &#920;\(1\)




 ### <a name = "fn-<T>ArrayNext" id = "fn-<T>ArrayNext"><T>ArrayNext</a> ###

`static T *`**`<T>ArrayNext`**`(const struct <T>Array *const `_`a`_`, const T *const `_`here`_`)`

Iterate through `a` \. It is safe to add using [<T>ArrayUpdateNew](#fn-<T>ArrayUpdateNew) with the return value as `update` \. Removing an element causes the pointer to go to the next element, if it exists\.

 - Parameter: _a_  
   The array; if null, returns null\.
 - Parameter: _here_  
   Set it to null to get the first element, if it exists\.
 - Return:  
   A pointer to the next element or null if there are no more\.
 - Order:  
   &#920;\(1\)




 ### <a name = "fn-<T>ArrayNew" id = "fn-<T>ArrayNew"><T>ArrayNew</a> ###

`static T *`**`<T>ArrayNew`**`(struct <T>Array *const `_`a`_`)`

Gets an uninitialised new element\. May move the `a` to a new memory location to fit the new size\.

 - Parameter: _a_  
   If is null, returns null\.
 - Return:  
   A new, un\-initialised, element, or null and `errno` may be set\.
 - Exceptional Return: ERANGE  
   Tried allocating more then can fit in `size_t` objects\.
 - Exceptional Return: realloc  
   [IEEE Std 1003\.1\-2001](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html) \.
 - Order:  
   Amortised &#927;\(1\)\.




 ### <a name = "fn-<T>ArrayUpdateNew" id = "fn-<T>ArrayUpdateNew"><T>ArrayUpdateNew</a> ###

`static T *`**`<T>ArrayUpdateNew`**`(struct <T>Array *const `_`a`_`, T **const `_`update_ptr`_`)`

Gets an uninitialised new element in `a` and updates the `update_ptr` if it is within the memory region that was changed to accomodate new space\.

 - Parameter: _a_  
   If null, returns null\.
 - Parameter: _update\_ptr_  
   Pointer to update on memory move\. If null, does nothing\.
 - Return:  
   A new, un\-initialised, element, or null and `errno` may be set\.
 - Exceptional Return: ERANGE  
   Tried allocating more then can fit in `size_t` \.
 - Exceptional Return: realloc  
   [IEEE Std 1003\.1\-2001](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html) \.
 - Order:  
   Amortised &#927;\(1\)\.
 - Caveat:  
   Untested\.




 ### <a name = "fn-<T>ArrayBuffer" id = "fn-<T>ArrayBuffer"><T>ArrayBuffer</a> ###

`static T *`**`<T>ArrayBuffer`**`(struct <T>Array *const `_`a`_`, const size_t `_`buffer`_`)`

Ensures that `a` array is `buffer` capacity beyond the elements in the array\.

 - Parameter: _a_  
   If is null, returns null\.
 - Parameter: _buffer_  
   If this is zero, returns null\.
 - Return:  
   One past the end of the array, or null and `errno` may be set\.
 - Exceptional Return: ERANGE  
   Tried allocating more then can fit in `size_t` \.
 - Exceptional Return: realloc  
   [IEEE Std 1003\.1\-2001](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html) \.
 - Order:  
   Amortised &#927;\(`buffer` \)\.
 - Caveat:  
   Test\.




 ### <a name = "fn-<T>ArrayExpand" id = "fn-<T>ArrayExpand"><T>ArrayExpand</a> ###

`static int `**`<T>ArrayExpand`**`(struct <T>Array *const `_`a`_`, const size_t `_`add`_`)`

Adds `add` to the size in `a` \.

 - Return:  
   Success\.
 - Exceptional Return: ERANGE  
   The size added is greater than the capacity\. To avoid this, call [<T>ArrayBuffer](#fn-<T>ArrayBuffer) before\.
 - Order:  
   &#927;\(1\)
 - Caveat:  
   Test\.




 ### <a name = "fn-<T>ArrayEach" id = "fn-<T>ArrayEach"><T>ArrayEach</a> ###

`static void `**`<T>ArrayEach`**`(struct <T>Array *const `_`a`_`, const <PT>Action `_`action`_`)`

Iterates through `a` and calls `action` on all the elements\. The topology of the list can not change while in this function\. That is, don't call [<T>ArrayNew](#fn-<T>ArrayNew), [<T>ArrayRemove](#fn-<T>ArrayRemove), _etc_ in `action` \.

 - Parameter: _a_  
   If null, does nothing\.
 - Parameter: _action_  
   If null, does nothing\.
 - Order:  
   &#927;\(`size` &#215; `action` \)
 - Caveat:  
   Untested\. Sequence interface\.




 ### <a name = "fn-<T>ArrayIfEach" id = "fn-<T>ArrayIfEach"><T>ArrayIfEach</a> ###

`static void `**`<T>ArrayIfEach`**`(struct <T>Array *const `_`a`_`, const <PT>Predicate `_`predicate`_`, const <PT>Action `_`action`_`)`

Iterates through `a` and calls `action` on all the elements for which `predicate` returns true\. The topology of the list can not change while in this function\.

 - Parameter: _a_  
   If null, does nothing\.
 - Parameter: _predicate_  
   If null, does nothing\.
 - Parameter: _action_  
   If null, does nothing\.
 - Order:  
   &#927;\(`size` &#215; `action` \)
 - Caveat:  
   Untested\. Sequence interface\.




 ### <a name = "fn-<T>ArrayAny" id = "fn-<T>ArrayAny"><T>ArrayAny</a> ###

`static T *`**`<T>ArrayAny`**`(const struct <T>Array *const `_`a`_`, const <PT>Predicate `_`predicate`_`)`

Iterates through `a` and calls `predicate` until it returns true\.

 - Parameter: _a_  
   If null, returns null\.
 - Parameter: _predicate_  
   If null, returns null\.
 - Return:  
   The first `predicate` that returned true, or, if the statement is false on all, null\.
 - Order:  
   &#927;\(`size` &#215; `action` \)
 - Caveat:  
   Untested\. Sequence interface\.




 ### <a name = "fn-<T>ArrayKeepIf" id = "fn-<T>ArrayKeepIf"><T>ArrayKeepIf</a> ###

`static void `**`<T>ArrayKeepIf`**`(struct <T>Array *const `_`a`_`, const <PT>Predicate `_`keep`_`)`

For all elements of `a` , calls `keep` , and for each element, if the return value is false, lazy deletes that item\.

 - Parameter: _a_  
   If null, does nothing\.
 - Parameter: _keep_  
   If null, does nothing\.
 - Order:  
   &#927;\(`size` \)




 ### <a name = "fn-<T>ArrayTrim" id = "fn-<T>ArrayTrim"><T>ArrayTrim</a> ###

`static void `**`<T>ArrayTrim`**`(struct <T>Array *const `_`a`_`, const <PT>Predicate `_`predicate`_`)`

Removes at either end of `a` of things that `predicate` returns true\.

 - Parameter: _a_  
   If null, does nothing\.
 - Parameter: _predicate_  
   If null, does nothing\.
 - Order:  
   &#927;\(`size` \)




 ### <a name = "fn-<T>ArraySplice" id = "fn-<T>ArraySplice"><T>ArraySplice</a> ###

`static int `**`<T>ArraySplice`**`(struct <T>Array *const `_`a`_`, const T *`_`anchor`_`, const long `_`range`_`, const struct <T>Array *const `_`b`_`)`

In `a` , replaces the elements from `anchor` up to `range` with a copy of `b` \.

 - Parameter: _a_  
   If null, returns zero\.
 - Parameter: _anchor_  
   Beginning of the replaced value, inclusive\. If null, appends to the end\.
 - Parameter: _range_  
   How many replaced values in the original; negative values are implicitly plus the length of the array; clamped at the minimum and maximum\.
 - Parameter: _b_  
   The replacement array\. If null, deletes without replacing\. It is more efficient than individual [<T>ArrayRemove](#fn-<T>ArrayRemove) to delete several consecutive values\.
 - Return:  
   Success\.
 - Exceptional Return: EDOM  
   `a` and `b` are not null and the same\.
 - Exceptional Return: ERANGE  
   `anchor` is not null and not in `a` \.
 - Exceptional Return: ERANGE  
   `range` is greater then 65535 or smaller then \-65534\.
 - Exceptional Return: ERANGE  
   `b` would cause the array to overflow\.
 - Exceptional Return: realloc  
   [IEEE Std 1003\.1\-2001](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html) \.
 - Order:  
   &#920;\(`b.size` \) if the elements have the same size, otherwise, amortised &#927;\(`a.size` \+ `b.size` \)\.




 ### <a name = "fn-<T>ArrayIndexSplice" id = "fn-<T>ArrayIndexSplice"><T>ArrayIndexSplice</a> ###

`static int `**`<T>ArrayIndexSplice`**`(struct <T>Array *const `_`a`_`, const size_t `_`i0`_`, const size_t `_`i1`_`, const struct <T>Array *const `_`b`_`)`

In `a` , replaces the elements from indices `i0` \(inclusive\) to `i1` \(exclusive\) with a copy of `b` \.

 - Parameter: _a_  
   If null, returns zero\.
 - Parameter: _i0_  
   The replacement indices, `[i0, i1)` , such that `0 <= i0 <= i1 <= a.size` \.
 - Parameter: _i1_  
   The replacement indices, `[i0, i1)` , such that `0 <= i0 <= i1 <= a.size` \.
 - Parameter: _b_  
   The replacement array\. If null, deletes without replacing\.
 - Return:  
   Success\.
 - Exceptional Return: EDOM  
   `a` and `b` are not null and the same\.
 - Exceptional Return: EDOM  
   `i0` or `i1` are out\-of\-bounds or `i0 > i1` \.
 - Exceptional Return: ERANGE  
   `b` would cause the array to overflow\.
 - Exceptional Return: realloc  
   [IEEE Std 1003\.1\-2001](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html) \.
 - Order:  
   &#920;\(`b.size` \) if the elements have the same size, otherwise, amortised &#927;\(`a.size` \+ `b.size` \)\.




 ### <a name = "fn-<T>ArrayToString" id = "fn-<T>ArrayToString"><T>ArrayToString</a> ###

`static const char *`**`<T>ArrayToString`**`(const struct <T>Array *const `_`a`_`)`

Can print 4 things at once before it overwrites\. One must a `ARRAY_TO_STRING` to a function implementing [<PT>ToString](#typedef-<PT>ToString) to get this functionality\.

 - Return:  
   Prints `a` in a static buffer\.
 - Order:  
   &#920;\(1\); it has a 255 character limit; every element takes some of it\.
 - Caveat:  
   ToString interface\.






 ## <a name = "license-">License</a> ##

2016 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



