Mostly independent `C89` data structure headers that use compile-time
polymorphism to generate lightweight and statically type-safe
containers.  The documented parameters in each project are pre-processor
macros. All the public functions are documented in `doc/<box>.md`.

## Design papers ##

The implementation of the boxes is fairly standard. Some rationales
of the more complex containers documented here.

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
a `union` is in the usual order, uses non-standard `pragma` (in
`test_trie.h`.) The `Makefile` provided assumes the `gcc` suite and
uses `clang`.

## License ##

2016 Neil Edelman, distributed under the terms of the [MIT
License](https://opensource.org/licenses/MIT).
