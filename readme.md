# boxes #

<table><tr>
	<th>link</th>
	<th>description</th>
	<th>possible traits, notes</th>
	<th>dependancies</th>
</tr><tr>
	<td><a href="https://github.com/neil-edelman/array">array</a></td>
	<td>contiguous dynamic array</td>
	<td>compare, iterate, to_string</td>
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
	<td>compare, iterate, to_string</td>
	<td></td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/orcish">orcish</a></td>
	<td>name generator</td>
	<td></td>
	<td></td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/pool">pool</a></td>
	<td>stable memory pool</td>
	<td>to_string</td>
	<td>array, heap</td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/table">table</a></td>
	<td>hash table implementing set or map</td>
	<td>iterate, to_string</td>
	<td></td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/tree">tree</a></td>
	<td>B-tree implementing set or map</td>
	<td>cursor, to_string</td>
	<td></td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/trie">trie</a></td>
	<td>prefix tree implementing string set or map</td>
	<td>under active development</td>
	<td></td>
</tr><tr>
	<td>disjoint</td>
	<td></td>
	<td>would be nice</td>
	<td></td>
</tr></table>

These `C89` data structure headers use compile-time polymorphism
to generate lightweight and statically type-safe containers.  The
documented parameters in each project are pre-processor macros.
`boxes` is a simple automated dependancy and build system, ensuring
these independent but related projects all work together during
development.

## Details ##

Throughout these headers, errors are returned with `errno`. Assertions
are used at runtime; to stop them, define `#define NDEBUG` before
`assert.h`. The source files are `UTF-8` and may contain multi-byte
literals. No effort has been made to synchronize for multi-threaded
execution. One does not need to download `boxes` to use the individual
projects: one can pick which ones are appropriate. The `sh`-script
`autoclone` downloads them all.

## Internal Interace ##

The `BOX_(n)` macro is used to define `<X>n` and `<PX>n`, public
and private names for the trait labeled `X`. With these definitions,

<table>
	<tr><td>typedef BOX &lt;PX&gt;box</td></tr>
	<tr><td>typedef BOX_CONTENT &lt;PX&gt;element</td></tr>
	<tr><td>typedef const BOX_CONTENT &lt;PX&gt;element_c</td></tr>
</table>

possible interfaces include, (not all used,)

<table><tr>
	<td>int &lt;BOX&gt;is_content_c(const &lt;PX&gt;element_c)</td>
	<td>Must have some an out-of-band element that is null.</td>
	<td>BOX_CONTENT BOX_ITERATOR BOX_ACCESS</td>
</tr><tr>
	<td>struct &lt;BOX&gt;forward</td>
	<td>A forward constant circular iterator, that has one more than the cardinality
	of the box to represent null.</td>
	<td>BOX_CONTENT</td>
</tr><tr>
	<td>struct &lt;BOX&gt;forward &lt;BOX&gt;forward(const &lt;PX&gt;box *)</td>
	<td>Initializes the iterator to return null.</td>
	<td>BOX_CONTENT</td>
</tr><tr>
	<td>&lt;PX&gt;element_c &lt;BOX&gt;next_c(&lt;BOX&gt;forward *)</td>
	<td>Moves the element to the next iteration and returns that element,
	(null if there are no more.)</td>
	<td>BOX_CONTENT</td>
</tr><tr>
	<td>struct &lt;BOX&gt;iterator</td>
	<td>A bi-directional circular iterator that may support addition and removal. It has a
	direction, and a null.</td>
	<td>BOX_ITERATOR BOX_ACCESS</td>
</tr><tr>
	<td>struct &lt;BOX&gt;iterator &lt;BOX&gt;iterator(const &lt;PX&gt;box *)</td>
	<td>Initializes to null.</td>
	<td>BOX_ITERATOR</td>
</tr><tr>
	<td>&lt;PX&gt;element &lt;BOX&gt;next(&lt;BOX&gt;iterator *)</td>
	<td>Moves the element to the next and returns that element,
	(null if there are no more.)</td>
	<td>BOX_ITERATOR</td>
</tr><tr>
	<td>&lt;PX&gt;element &lt;BOX&gt;previous(&lt;BOX&gt;iterator *)</td>
	<td>Moves the element to the previous and returns that element,
	(null if there are no more.)</td>
	<td>BOX_ITERATOR</td>
</tr><tr>
	<td>int &lt;BOX&gt;remove(&lt;BOX&gt;iterator *)</td>
	<td>Returns whether the element just returned has been removed.</td>
	<td>BOX_ITERATOR</td>
</tr><tr>
	<td>size_t &lt;BOX&gt;size(&lt;PX&gt;box *)</td>
	<td>Returns the size, or, passed null, 0.</td>
	<td>BOX_ACCESS</td>
</tr><tr>
	<td>&lt;BOX&gt;iterator &lt;BOX&gt;before(const &lt;PX&gt;box *, size_t)</td>
	<td>Iterator at the subscripted element, if the element exists.</td>
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
	<td>True if the uninitialized elements have been added to the back.</td>
	<td>BOX_CONTIGUOUS</td>
</tr></table>

## License ##

2016 Neil Edelman, distributed under the terms of the [MIT
License](https://opensource.org/licenses/MIT).
