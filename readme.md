# boxes #

<table><tr>
	<th>link</th>
	<th>description</th>
	<th>possible traits</th>
	<th>required dependancies</th>
</tr><tr>
	<td><a href="https://github.com/neil-edelman/array">array</a></td>
	<td>contiguous dynamic array</td>
	<td>array_coda, to_string</td>
	<td></td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/bmp">bmp</a></td>
	<td>fixed bit-field</td>
	<td></td>
	<td></td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/heap">heap</a></td>
	<td>priority-queue</td>
	<td>to_string</td>
	<td>array</td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/list">list</a></td>
	<td>doubly-linked component</td>
	<td>list_coda, to_string</td>
	<td></td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/orcish">orcish</a></td>
	<td>name generator</td>
	<td></td>
	<td></td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/pool">pool</a></td>
	<td>stable pool</td>
	<td>to_string</td>
	<td>array, heap</td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/table">table</a></td>
	<td>set or map (associative array)</td>
	<td>default, to_string</td>
	<td></td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/trie">trie</a></td>
	<td>prefix tree</td>
	<td>to_string</td>
	<td></td>
</tr><tr>
	<td>disjoint</td>
	<td>upcoming</td>
	<td>to_string?</td>
	<td>array?</td>
</tr></table>

These `C89` data structure projects use compile-time polymorphism
to generate lightweight and statically type-safe containers.  The
documented parameters in each project are pre-processor macros.
`boxes` is a simple automated dependancy and build system, ensuring
these independent but related projects all work together during
development.  The `sh`-script `autoclone`, downloads them all.

## Details ##

Errors are returned with `errno`. Assertions are used at runtime;
to stop them, define `#define NDEBUG` before `assert.h`. The source
files are `UTF-8` and may contain multi-byte literals. No effort
has been made to synchronize for multi-threaded execution. In a
project, one can pick and choose which ones are appropriate.

## Internal Interace ##

The `BOX_(n)` macro is used to define `<X>n` and `<PX>n`, public
and private names for the trait labeled `X`. With these definitions,

<table>
	<tr><td>typedef BOX &lt;PX&gt;box</td></tr>
	<tr><td>typedef BOX_CONTENT &lt;PX&gt;element</td></tr>
	<tr><td>typedef const BOX_CONTENT &lt;PX&gt;element_c</td></tr>
</table>

Possible interfaces include,

<table><tr>
	<td>int &lt;BOX&gt;is_content(const &lt;PX&gt;element_c)</td>
	<td>Must have some element that is false.</td>
	<td>BOX_CONTENT</td>
</tr><tr>
	<td>struct &lt;BOX&gt;forward</td>
	<td>A forward constant iterator, (input_or_output_iterator.)</td>
	<td>BOX_CONTENT</td>
</tr><tr>
	<td>struct &lt;BOX&gt;forward &lt;PX&gt;forward_begin(const &lt;PX&gt;box *)</td>
	<td>Initializes to before the elements.</td>
	<td>BOX_CONTENT</td>
</tr></table>

## License ##

2016 Neil Edelman, distributed under the terms of the [MIT
License](https://opensource.org/licenses/MIT).

## Todo ##

Compiling with all warnings turned on in some compilers is an endless
race with the `lint`-like warnings. Most of them are super-useful,
but some are not. One may have issues with, for example,
`_CRT_SECURE_NO_WARNINGS`, `-Wno-comma`, `-Wno-logical-op-parentheses`,
`-Wno-parentheses`, `-Wno-shift-op-parentheses`.
