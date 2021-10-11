# bmp\.h #

 * [Description](#user-content-preamble)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;B&gt;bmp](#user-content-tag-c8c9536a)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

`<B>bmp` is a bit\-field of `BMP_BITS` bits\.

 * Parameter: BMP\_NAME, BMP\_BITS  
   `<B>` that satisfies `C` naming conventions when mangled and a number of bits associated therewith; required\. `<PB>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: BMP\_TEST  
   Optional unit testing framework using `assert`\. Testing contained in [\.\./test/test\_bmp\.h](../test/test_bmp.h)\.
 * Standard:  
   C89/90




## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-c8c9536a" name = "user-content-tag-c8c9536a">&lt;B&gt;bmp</a> ###

<code>struct <strong>&lt;B&gt;bmp</strong> { &lt;PB&gt;chunk chunk[BMP_CHUNKS]; };</code>

An array of `BMP_BITS` bits, taking up the next multiple of `BMP_TYPE` size\.



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-53567fd8">&lt;B&gt;bmp_tests</a></td><td></td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-53567fd8" name = "user-content-fn-53567fd8">&lt;B&gt;bmp_tests</a> ###

<code>static void <strong>&lt;B&gt;bmp_tests</strong>(void)</code>

Will be tested on stdout\. Requires `BMP_TEST`, and not `NDEBUG` while defining `assert`\.





## <a id = "user-content-license" name = "user-content-license">License</a> ##

2021 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



