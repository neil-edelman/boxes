# bmp\.h #

Header [\.\./\.\./src/bmp\.h](../../src/bmp.h); examples [\.\./\.\./test/test\_bmp\.c](../../test/test_bmp.c)\.

## Fixed bit\-field ##

 * [Description](#user-content-preamble)
 * [Typedef Aliases](#user-content-typedef): [bmpchunk](#user-content-typedef-1f500d15)
 * [Struct, Union, and Enum Definitions](#user-content-tag): [&lt;t&gt;bmp](#user-content-tag-7a20983c)
 * [Function Summary](#user-content-summary)
 * [Function Definitions](#user-content-fn)
 * [License](#user-content-license)

## <a id = "user-content-preamble" name = "user-content-preamble">Description</a> ##

`<t>bmp` is a bit\-field of `BMP_BITS` bits\. The representation in memory is most\-signifiant bit first\.

 * Parameter: BMP\_NAME, BMP\_BITS  
   `<t>` that satisfies `C` naming conventions when mangled and a number of bits associated therewith, which must be positive; required\.
 * Standard:  
   C89/90
 * Dependancies:  
   [box](../box.h)




## <a id = "user-content-typedef" name = "user-content-typedef">Typedef Aliases</a> ##

### <a id = "user-content-typedef-1f500d15" name = "user-content-typedef-1f500d15">bmpchunk</a> ###

<code>typedef unsigned <strong>bmpchunk</strong>;</code>

The underlying array type\.



## <a id = "user-content-tag" name = "user-content-tag">Struct, Union, and Enum Definitions</a> ##

### <a id = "user-content-tag-7a20983c" name = "user-content-tag-7a20983c">&lt;t&gt;bmp</a> ###

<code>struct <strong>&lt;t&gt;bmp</strong> { bmpchunk chunk[BMP_CHUNKS]; };</code>

An array of `BMP_BITS` bits, \(taking up the next multiple of `sizeof(bmpchunk)` &#215; `CHARBIT`\.\)



## <a id = "user-content-summary" name = "user-content-summary">Function Summary</a> ##

<table>

<tr><th>Modifiers</th><th>Function Name</th><th>Argument List</th></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-7b089536">&lt;T&gt;clear_all</a></td><td>a</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-40a9d9d">&lt;T&gt;invert_all</a></td><td>a</td></tr>

<tr><td align = right>static unsigned</td><td><a href = "#user-content-fn-1deccc11">&lt;T&gt;test</a></td><td>a, x</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-997e5bef">&lt;T&gt;set</a></td><td>a, x</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-7f4a964e">&lt;T&gt;clear</a></td><td>a, x</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-378030f9">&lt;T&gt;toggle</a></td><td>a, x</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-e80ff7d4">&lt;T&gt;insert</a></td><td>a, x, n</td></tr>

<tr><td align = right>static void</td><td><a href = "#user-content-fn-56806709">&lt;T&gt;remove</a></td><td>a, x, n</td></tr>

</table>



## <a id = "user-content-fn" name = "user-content-fn">Function Definitions</a> ##

### <a id = "user-content-fn-7b089536" name = "user-content-fn-7b089536">&lt;T&gt;clear_all</a> ###

<code>static void <strong>&lt;T&gt;clear_all</strong>(struct &lt;t&gt;bmp *const <em>a</em>)</code>

Sets `a` to all false\.



### <a id = "user-content-fn-40a9d9d" name = "user-content-fn-40a9d9d">&lt;T&gt;invert_all</a> ###

<code>static void <strong>&lt;T&gt;invert_all</strong>(struct &lt;t&gt;bmp *const <em>a</em>)</code>

Inverts all entries of `a`\.



### <a id = "user-content-fn-1deccc11" name = "user-content-fn-1deccc11">&lt;T&gt;test</a> ###

<code>static unsigned <strong>&lt;T&gt;test</strong>(const struct &lt;t&gt;bmp *const <em>a</em>, const unsigned <em>x</em>)</code>

 * Return:  
   Projects the eigenvalue of bit `x` of `a`\. Either zero of non\-zero, but not necessarily one\.




### <a id = "user-content-fn-997e5bef" name = "user-content-fn-997e5bef">&lt;T&gt;set</a> ###

<code>static void <strong>&lt;T&gt;set</strong>(struct &lt;t&gt;bmp *const <em>a</em>, const unsigned <em>x</em>)</code>

Sets bit `x` in `a`\.



### <a id = "user-content-fn-7f4a964e" name = "user-content-fn-7f4a964e">&lt;T&gt;clear</a> ###

<code>static void <strong>&lt;T&gt;clear</strong>(struct &lt;t&gt;bmp *const <em>a</em>, const unsigned <em>x</em>)</code>

Clears bit `x` in `a`\.



### <a id = "user-content-fn-378030f9" name = "user-content-fn-378030f9">&lt;T&gt;toggle</a> ###

<code>static void <strong>&lt;T&gt;toggle</strong>(struct &lt;t&gt;bmp *const <em>a</em>, const unsigned <em>x</em>)</code>

Toggles bit `x` in `a`\.



### <a id = "user-content-fn-e80ff7d4" name = "user-content-fn-e80ff7d4">&lt;T&gt;insert</a> ###

<code>static void <strong>&lt;T&gt;insert</strong>(struct &lt;t&gt;bmp *const <em>a</em>, const unsigned <em>x</em>, const unsigned <em>n</em>)</code>

Inserts `n` zeros at `x` in `a`\. The `n` right bits are discarded\.



### <a id = "user-content-fn-56806709" name = "user-content-fn-56806709">&lt;T&gt;remove</a> ###

<code>static void <strong>&lt;T&gt;remove</strong>(struct &lt;t&gt;bmp *const <em>a</em>, const unsigned <em>x</em>, const unsigned <em>n</em>)</code>

Removes `n` at `x` in `a`\. The `n` bits coming from the right are zero\.





## <a id = "user-content-license" name = "user-content-license">License</a> ##

2021 Neil Edelman, distributed under the terms of the [MIT License](https://opensource.org/licenses/MIT)\.



