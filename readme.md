# boxes #

<table><tr>
	<th>link</th>
	<th>description</th>
	<th>paper</th>
</tr><tr>
	<td><a href="https://github.com/neil-edelman/array">array</a></td>
	<td>dynamic array</td>
	<td></td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/bmp">bmp</a></td>
	<td>fixed bit-field</td>
	<td></td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/heap">heap</a></td>
	<td>priority-queue</td>
	<td></td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/list">list</a></td>
	<td>doubly-linked component</td>
	<td></td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/orcish">orcish</a></td>
	<td>name generator</td>
	<td></td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/pool">pool</a></td>
	<td>stable memory pool</td>
	<td><a href = "https://github.com/neil-edelman/pool/blob/master/doc/pool.pdf">A slab-allocator for similar objects</a></td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/table">table</a></td>
	<td>hash table implementing set or map</td>
	<td><a href = "https://github.com/neil-edelman/table/blob/master/doc/table.pdf">Allocation-conscious chained hash-table</a></td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/tree">tree</a></td>
	<td>B-tree implementing set or map</td>
	<td><a href = "https://github.com/neil-edelman/tree/blob/master/doc/tree.pdf">Practical in-memory B-tree design</a></td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/trie">trie</a></td>
	<td>prefix tree implementing string set or map</td>
	<td><a href = "https://github.com/neil-edelman/trie/blob/master/doc/trie.pdf">Compact binary prefix trees</a></td>
</tr></table>

These `C89` data structure headers use compile-time polymorphism
to generate lightweight and statically type-safe containers.  The
documented parameters in each project are pre-processor macros.
`boxes` is a simple automated dependancy and build system, ensuring
these independent but related projects all work together during
development; it is unneeded to use the individual projects: one can
pick which ones are appropriate. However, the handy `sh`-script
`autoclone` downloads them all.

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
execution.

## License ##

2016 Neil Edelman, distributed under the terms of the [MIT
License](https://opensource.org/licenses/MIT).
