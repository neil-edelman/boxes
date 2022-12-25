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

* `pool` compares different objects' addresses; in `C89` that
behaviour is undefined. With `C99`, a `uintptr_t`-cast makes it
implementation-defined.
* `to_string.h` contains UTF-8 literals, (for convenience.)
* `trie` conforms to ISO/IEC 9899/AMD1:1995 because it uses `EILSEQ`.
* The testing framework uses `C13` in places, assumes a `union` is
in the usual order, uses non-standard `pragma` (in `test_trie.h`.)
The build files compatible with `gcc` and use `clang`.

## License ##

2016 Neil Edelman, distributed under the terms of the [MIT
License](https://opensource.org/licenses/MIT).
