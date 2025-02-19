`C89` data structure headers that use compile-time polymorphism to
generate lightweight and statically type-safe containers.

## Downloading ##

This is not a library, just headers where needed. Pick and choose
from `src` and use them. The dependencies are edges in the following
diagram. Or just download the entire `src` directory and put it
where it's convenient to reference from one's code.

![Dependencies](dependencies.svg)

Dotted edges means it's an optional dependency. The dashed containers,
`deque` and `pool` are stable containers. A dotted box—`array`,
`heap`, `table`, `tree`, and `trie`, means unstable. Depending on
one's use case, it may be good to have one or more stable containers
as a backing for one or more unstable pointer containers.

## Documentation ##

Documented parameters in each project are pre-processor macros.
Examples are in [text/](]test/) test_array.c _etc_.

<table><tr>
<th>documentation</th>
<th>design paper</th>
</tr>
<tr><td><a href = "doc/array/">array</a></td><td></td></tr>
<tr><td><a href = "doc/deque/">deque</a></td><td></td></tr>
<tr><td><a href = "doc/heap/">heap</a></td><td></td></tr>
<tr><td><a href = "doc/list/">list</a></td><td></td></tr>
<tr>
<td><a href = "doc/pool/">memory pool</a></td>
<td><a href = "doc/pool/pool.pdf">A slab-allocator for similar objects</a></td>
</tr><tr>
<td><a href = "doc/table/">hash table</a></td>
<td><a href = "doc/table/table.pdf">Allocation-conscious chained hash-table</a></td>
</tr><tr>
<td><a href = "doc/tree/">B-tree</a></td>
<td><a href = "doc/tree/tree.pdf">Practical in-memory B-tree design</a></td>
</tr><tr>
<td><a href = "doc/trie/">trie</a></td>
<td><a href = "doc/trie/trie.pdf">Compact binary prefix trees</a></td>
</tr></table>

## Compatibility ##

Uses `C++17` feature `__has_include` in `box.h` to decide to include
optional features so that part of the header files work without the
full thing. If one's compiler is does not support `C++17` (especially)
and one needs these features, try uncommenting the appropriate
lines.

The testing framework in `test/` is much less multi-platform then
the headers in `src/`, requiring `C13` anonymous unions, assuming
a `union` is in the usual order. The `Makefile` provided assumes
the `gcc` suite and uses `clang`.

On a segmented memory model, `pool` probably won't work reliably,
especially when one compiles with `C90`.

(Especially) `trie` hasn't been tested when `CHAR_BIT ≠ 8`, as it
could be in `C90`.

`C90` requires identifiers to be a minimum of 31 significant initial
characters in an internal identifier or a macro name. If one's
compiler does the very minimum, it probably won't be enough to
differentiate all the private symbols. One may have to edit the
code to abbreviate.

Utf-8 everywhere. There are few places where this makes a difference,
but one may have to edit some files if working with ASCII-only
compilers. (Specifically, "…" in `to_string`.)

## License ##

2016 Neil Edelman, distributed under the terms of the [MIT
License](https://opensource.org/licenses/MIT).
