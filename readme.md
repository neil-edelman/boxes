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
	<td>iterate, to_string</td>
	<td>bmp</td>
</tr><tr>
	<td>disjoint</td>
	<td></td>
	<td>thinking of doing</td>
	<td></td>
</tr></table>

These `C89` data structure headers use compile-time polymorphism
to generate lightweight and statically type-safe containers.  The
documented parameters in each project are pre-processor macros.
`boxes` is a simple automated dependancy and build system, ensuring
these independent but related projects all work together during
development.

## Details ##

Some code, called traits, may be shared between boxes on some extra
preprocessor commands for more functionality. This is contained in
headers that are meant to be further included internally by the box
itself. The documentation for each box explains more. One can have
multiple named traits by including the file again.

Throughout these headers, errors are returned with `errno`. Assertions
are used at runtime; to stop them, define `#define NDEBUG` before
`assert.h`. The source files are `UTF-8` and may contain multi-byte
literals. No effort has been made to synchronize for multi-threaded
execution. One does not need to download `boxes` to use the individual
projects: one can pick which ones are appropriate. The `sh`-script
`autoclone` downloads them all.

## License ##

2016 Neil Edelman, distributed under the terms of the [MIT
License](https://opensource.org/licenses/MIT).
