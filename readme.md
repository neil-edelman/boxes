# bmp\.h #

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [bmpchunk](#user-content-typedef-1f500d15)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;B&gt;bmp](#user-content-tag-c8c9536a)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

`<B>bmp` is a bit\-field of `BMP_BITS` bits\.

 * Parameter: BMP\_NAME, BMP\_BITS  
   `<B>` that satisfies `C` naming conventions when mangled and a number of bits associated therewith, which must be positive; required\. `<PB>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Parameter: BMP\_TEST  
   Optional unit testing framework using `assert`\. Testing contained in [\.\./test/test\_bmp\.h](../test/test_bmp.h)\.
 * Standard:  
   C89/90




## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-1f500d15" name = "user-content-typedef-1f500d15">bmpchunk</a> ###

<code>typedef unsigned <strong>bmpchunk</strong>;</code>

The underlying array type\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-c8c9536a" name = "user-content-tag-c8c9536a">&lt;B&gt;bmp</a> ###

<code>struct <strong>&lt;B&gt;bmp</strong> { bmpchunk chunk[BMP_CHUNKS]; };</code>

An array of `BMP_BITS` bits, \(taking up the next multiple of `sizeof(bmpchunk) &#215; CHARBIT`\.\)



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-58d2565c">&lt;B&gt;bmp_clear_all</a></td><td>a</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-3c840533">&lt;B&gt;bmp_invert_all</a></td><td>a</td></tr>

<tr><td align = right>static unsigned</td><td><a href = "#user-content-fn-ef4c7bb">&lt;B&gt;bmp_test</a></td><td>a, x</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-dc0e70b5">&lt;B&gt;bmp_set</a></td><td>a, x</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-edce4d60">&lt;B&gt;bmp_clear</a></td><td>a, x</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-e3ad5ae3">&lt;B&gt;bmp_toggle</a></td><td>a, x</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-f8fa8ce6">&lt;B&gt;bmp_insert</a></td><td>a, x, n</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-9af1d333">&lt;B&gt;bmp_remove</a></td><td>a, x, n</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-53567fd8">&lt;B&gt;bmp_tests</a></td><td></td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-58d2565c" name = "user-content-fn-58d2565c">&lt;B&gt;bmp_clear_all</a> ###

<code>static void <strong>&lt;B&gt;bmp_clear_all</strong>(struct &lt;B&gt;bmp *const <em>a</em>)</code>

Sets `a` to all false\.



### <a id = "user-content-fn-3c840533" name = "user-content-fn-3c840533">&lt;B&gt;bmp_invert_all</a> ###

<code>static void <strong>&lt;B&gt;bmp_invert_all</strong>(struct &lt;B&gt;bmp *const <em>a</em>)</code>

Inverts all entries of `a`\.



### <a id = "user-content-fn-ef4c7bb" name = "user-content-fn-ef4c7bb">&lt;B&gt;bmp_test</a> ###

<code>static unsigned <strong>&lt;B&gt;bmp_test</strong>(const struct &lt;B&gt;bmp *const <em>a</em>, const unsigned <em>x</em>)</code>

 * Return:  
   Projects the eigenvalue of bit `x` of `a`\. Either zero of non\-zero\.




### <a id = "user-content-fn-dc0e70b5" name = "user-content-fn-dc0e70b5">&lt;B&gt;bmp_set</a> ###

<code>static void <strong>&lt;B&gt;bmp_set</strong>(struct &lt;B&gt;bmp *const <em>a</em>, const unsigned <em>x</em>)</code>

Sets bit `x` in `a`\.



### <a id = "user-content-fn-edce4d60" name = "user-content-fn-edce4d60">&lt;B&gt;bmp_clear</a> ###

<code>static void <strong>&lt;B&gt;bmp_clear</strong>(struct &lt;B&gt;bmp *const <em>a</em>, const unsigned <em>x</em>)</code>

Clears bit `x` in `a`\.



### <a id = "user-content-fn-e3ad5ae3" name = "user-content-fn-e3ad5ae3">&lt;B&gt;bmp_toggle</a> ###

<code>static void <strong>&lt;B&gt;bmp_toggle</strong>(struct &lt;B&gt;bmp *const <em>a</em>, const unsigned <em>x</em>)</code>

Toggles bit `x` in `a`\.



### <a id = "user-content-fn-f8fa8ce6" name = "user-content-fn-f8fa8ce6">&lt;B&gt;bmp_insert</a> ###

<code>static void <strong>&lt;B&gt;bmp_insert</strong>(struct &lt;B&gt;bmp *const <em>a</em>, const unsigned <em>x</em>, const unsigned <em>n</em>)</code>

Inserts `n` zeros at `x` in `a`\. The `n` right bits are discarded\.



### <a id = "user-content-fn-9af1d333" name = "user-content-fn-9af1d333">&lt;B&gt;bmp_remove</a> ###

<code>static void <strong>&lt;B&gt;bmp_remove</strong>(struct &lt;B&gt;bmp *const <em>a</em>, const unsigned <em>x</em>, const unsigned <em>n</em>)</code>

Removes `n` at `x` in `a`\. The `n` bits coming from the right are zero\.



### <a id = "user-content-fn-53567fd8" name = "user-content-fn-53567fd8">&lt;B&gt;bmp_tests</a> ###

<code>static void <strong>&lt;B&gt;bmp_tests</strong>(void)</code>

Will be tested on stdout\. Requires `BMP_TEST`, and not `NDEBUG` while defining `assert`\.





## <a id = "user-content-license" name = "user-content-license">License</a> ##

2021 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



