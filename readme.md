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

possible interfaces include,

<table><tr>
	<td>int &lt;BOX&gt;is_content(const &lt;PX&gt;element_c)</td>
	<td>Must have some elements that are false; that is null.</td>
	<td>BOX_CONTENT BOX_ITERATOR BOX_ACCESS</td>
</tr><tr>
	<td>struct &lt;BOX&gt;forward</td>
	<td>A forward constant iterator, (input_or_output_iterator.) Must return null when past the end.</td>
	<td>BOX_CONTENT</td>
</tr><tr>
	<td>struct &lt;BOX&gt;forward &lt;BOX&gt;forward_begin(const &lt;PX&gt;box *)</td>
	<td>Initializes to before the elements.</td>
	<td>BOX_CONTENT</td>
</tr><tr>
	<td>&lt;PX&gt;element_c &lt;BOX&gt;forward_next(&lt;BOX&gt;forward *)</td>
	<td>Returns the element passed-though to get to the next, or null if there are no more.</td>
	<td>BOX_CONTENT</td>
</tr><tr>
	<td>struct &lt;BOX&gt;iterator</td>
	<td>A bi-directional iterator that supports (at least symbolic) removal. It has a direction, and the state
	can be one-off the ends at either side.</td>
	<td>BOX_ITERATOR BOX_ACCESS</td>
</tr><tr>
	<td>struct &lt;BOX&gt;iterator &lt;BOX&gt;begin(const &lt;PX&gt;box *)</td>
	<td>Initializes to before the elements.</td>
	<td>BOX_ITERATOR</td>
</tr><tr>
	<td>struct &lt;BOX&gt;iterator &lt;BOX&gt;end(const &lt;PX&gt;box *)</td>
	<td>Initializes to after the elements.</td>
	<td>BOX_ITERATOR</td>
</tr><tr>
	<td>&lt;PX&gt;element &lt;BOX&gt;next(&lt;BOX&gt;iterator *)</td>
	<td>Returns the element passed-though to get to the next, or null if there are no more.</td>
	<td>BOX_ITERATOR</td>
</tr><tr>
	<td>&lt;PX&gt;element &lt;BOX&gt;previous(&lt;BOX&gt;iterator *)</td>
	<td>Returns the element passed-though to get to the previous, or null if there are no more.</td>
	<td>BOX_ITERATOR</td>
</tr><tr>
	<td>int &lt;BOX&gt;remove(&lt;BOX&gt;iterator *)</td>
	<td>Returns whether the element is and has been removed.</td>
	<td>BOX_ITERATOR</td>
</tr><tr>
	<td>size_t &lt;BOX&gt;size(&lt;PX&gt;box *)</td>
	<td>Returns the size, or, passed null, 0.</td>
	<td>BOX_ACCESS</td>
</tr><tr>
	<td>&lt;BOX&gt;iterator &lt;BOX&gt;index(const &lt;PX&gt;box *, size_t)</td>
	<td>Iterator immediately before subscripted element.</td>
	<td>BOX_ACCESS</td>
</tr><tr>
	<td>&lt;PX&gt;element &lt;BOX&gt;at(const &lt;PX&gt;box *, size_t)</td>
	<td>The subscripted element of the box.</td>
	<td>BOX_ACCESS</td>
</tr><tr>
	<td>void &lt;BOX&gt;tell_size(&lt;PX&gt;box *, size_t)</td>
	<td>Sets size.</td>
	<td>BOX_CONTIGUOUS</td>
</tr><tr>
	<td>&lt;PX&gt;element &lt;BOX&gt;append(&lt;PX&gt;box *, size_t)</td>
	<td>True if the unintialized elements have been added to the back.</td>
	<td>BOX_CONTIGUOUS</td>
</tr></table>

## License ##

2016 Neil Edelman, distributed under the terms of the [MIT
License](https://opensource.org/licenses/MIT).
