# bmp\.h #

Stand\-alone header [\.\./src/bmp\.h](../src/bmp.h); examples [\.\./test/test\_bmp\.c](../test/test_bmp.c)\.

## Fixed bit\-field ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [bmpchunk](#user-content-typedef-1f500d15)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;M&gt;bmp](#user-content-tag-45973bd7)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

`<M>bmp` is a bit\-field of `BMP_BITS` bits\. The representation in memory is most\-signifiant bit first\.

 * Parameter: BMP\_NAME, BMP\_BITS  
   `<M>` that satisfies `C` naming conventions when mangled and a number of bits associated therewith, which must be positive; required\. `<PM>` is private, whose names are prefixed in a manner to avoid collisions\.
 * Standard:  
   C89/90




## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-1f500d15" name = "user-content-typedef-1f500d15">bmpchunk</a> ###

<code>typedef unsigned <strong>bmpchunk</strong>;</code>

The underlying array type\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-45973bd7" name = "user-content-tag-45973bd7">&lt;M&gt;bmp</a> ###

<code>struct <strong>&lt;M&gt;bmp</strong> { bmpchunk chunk[BMP_CHUNKS]; };</code>

An array of `BMP_BITS` bits, \(taking up the next multiple of `sizeof(bmpchunk)` &#215; `CHARBIT`\.\)



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-c9cf907d">&lt;M&gt;bmp_clear_all</a></td><td>a</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-f9b9cdb0">&lt;M&gt;bmp_invert_all</a></td><td>a</td></tr>

<tr><td align = right>static unsigned</td><td><a href = "#user-content-fn-3e108dc">&lt;M&gt;bmp_test</a></td><td>a, x</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-b3f65e98">&lt;M&gt;bmp_set</a></td><td>a, x</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-3441b8e5">&lt;M&gt;bmp_clear</a></td><td>a, x</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-27f85a04">&lt;M&gt;bmp_toggle</a></td><td>a, x</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-422a6f9d">&lt;M&gt;bmp_insert</a></td><td>a, x, n</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-530e280">&lt;M&gt;bmp_remove</a></td><td>a, x, n</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-c9cf907d" name = "user-content-fn-c9cf907d">&lt;M&gt;bmp_clear_all</a> ###

<code>static void <strong>&lt;M&gt;bmp_clear_all</strong>(struct &lt;M&gt;bmp *const <em>a</em>)</code>

Sets `a` to all false\.



### <a id = "user-content-fn-f9b9cdb0" name = "user-content-fn-f9b9cdb0">&lt;M&gt;bmp_invert_all</a> ###

<code>static void <strong>&lt;M&gt;bmp_invert_all</strong>(struct &lt;M&gt;bmp *const <em>a</em>)</code>

Inverts all entries of `a`\.



### <a id = "user-content-fn-3e108dc" name = "user-content-fn-3e108dc">&lt;M&gt;bmp_test</a> ###

<code>static unsigned <strong>&lt;M&gt;bmp_test</strong>(const struct &lt;M&gt;bmp *const <em>a</em>, const unsigned <em>x</em>)</code>

 * Return:  
   Projects the eigenvalue of bit `x` of `a`\. Either zero of non\-zero, but not necessarily one\.




### <a id = "user-content-fn-b3f65e98" name = "user-content-fn-b3f65e98">&lt;M&gt;bmp_set</a> ###

<code>static void <strong>&lt;M&gt;bmp_set</strong>(struct &lt;M&gt;bmp *const <em>a</em>, const unsigned <em>x</em>)</code>

Sets bit `x` in `a`\.



### <a id = "user-content-fn-3441b8e5" name = "user-content-fn-3441b8e5">&lt;M&gt;bmp_clear</a> ###

<code>static void <strong>&lt;M&gt;bmp_clear</strong>(struct &lt;M&gt;bmp *const <em>a</em>, const unsigned <em>x</em>)</code>

Clears bit `x` in `a`\.



### <a id = "user-content-fn-27f85a04" name = "user-content-fn-27f85a04">&lt;M&gt;bmp_toggle</a> ###

<code>static void <strong>&lt;M&gt;bmp_toggle</strong>(struct &lt;M&gt;bmp *const <em>a</em>, const unsigned <em>x</em>)</code>

Toggles bit `x` in `a`\.



### <a id = "user-content-fn-422a6f9d" name = "user-content-fn-422a6f9d">&lt;M&gt;bmp_insert</a> ###

<code>static void <strong>&lt;M&gt;bmp_insert</strong>(struct &lt;M&gt;bmp *const <em>a</em>, const unsigned <em>x</em>, const unsigned <em>n</em>)</code>

Inserts `n` zeros at `x` in `a`\. The `n` right bits are discarded\.



### <a id = "user-content-fn-530e280" name = "user-content-fn-530e280">&lt;M&gt;bmp_remove</a> ###

<code>static void <strong>&lt;M&gt;bmp_remove</strong>(struct &lt;M&gt;bmp *const <em>a</em>, const unsigned <em>x</em>, const unsigned <em>n</em>)</code>

Removes `n` at `x` in `a`\. The `n` bits coming from the right are zero\.





## <a id = "user-content-license" name = "user-content-license">License</a> ##

2021 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



