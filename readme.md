# List\.h #

## Parameterised List ##

 * [Description](#user-content-preamble)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

[&lt;L&gt;List](#user-content-tag-68dde1fd) is a list of [&lt;L&gt;ListLink](#user-content-tag-16c786c1); it may be supplied a total\-order function, `LIST_COMPARE` [&lt;PI&gt;Compare](#user-content-typedef-2fbbef99)\.

Internally, `<L>ListLink` is a doubly\-linked node with sentinels residing in `<L>List`\. It only provides an order, but `<L>ListLink` may be enclosed in another `struct`\. While in the list, the links should not be added to another list\.

`<L>Link` is not synchronised\. Errors are returned with `errno`\. The parameters are `#define` preprocessor macros, and are all undefined at the end of the file for convenience\. `assert.h` is used; to stop assertions, use `#define NDEBUG` before inclusion\.



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


## <a id = "user-content-license" name = "user-content-license">License</a> ##

2017 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



