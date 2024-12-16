Mostly independent `C89` data structure headers that use compile-time
polymorphism to generate lightweight and statically type-safe
containers.

## Downloading ##

Pick and choose from `src` and use them. Here is the dependencies.

![Dependencies](dependencies.svg)

Or just download the entire `src` directory and put it where it's
convenient to reference from one's code. It is a static generator—it
won't include unnecessary code.  Documentation is in `doc/` and
examples are in `test/`. Parameters in each project are pre-processor
macros.

## Design papers ##

The implementation is fairly standard. Some rationales of the more
complex containers documented here.

<table><tr>
<td><a href = "doc/pool.md">memory pool</a></td>
<td><a href = "doc/pool/pool.pdf">A slab-allocator for similar objects</a></td>
</tr><tr>
<td><a href = "doc/table.md">hash table</a></td>
<td><a href = "doc/table/table.pdf">Allocation-conscious chained hash-table</a></td>
</tr><tr>
<td><a href = "doc/tree.md">B-tree</a></td>
<td><a href = "doc/tree/tree.pdf">Practical in-memory B-tree design</a></td>
</tr><tr>
<td><a href = "doc/trie.md">trie</a></td>
<td><a href = "doc/trie/trie.pdf">Compact binary prefix trees</a></td>
</tr></table>

## Compatibility ##

The testing framework in `test/` is much less multi-platform then
the headers in `src/`, requiring `C13` anonymous unions, assuming
a `union` is in the usual order. The `Makefile` provided assumes
the `gcc` suite and uses `clang`.

## Issues

On a segmented memory model, `pool` probably won't work reliably,
especially when one compiles with `C90`.

(Especially) `trie` hasn't been tested when `CHAR_BIT ≠ 8`, as it
could be in `C90`.

`C90` requires identifiers to be a minimum of 31 significant initial
characters in an internal identifier or a macro name. If one's
compiler does the very minimum, it probably won't be enough to
differentiate all the private symbols. One may have to edit the
code to abbreviate.

## License ##

2016 Neil Edelman, distributed under the terms of the [MIT
License](https://opensource.org/licenses/MIT).
