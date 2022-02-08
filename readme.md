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
	<td>bmp (maybe)</td>
</tr><tr>
	<td>disjoint</td>
	<td>upcoming</td>
	<td>to_string?</td>
	<td>array?</td>
</tr></table>

`boxes` is a very simple automated dependancy and build system for
separate `C89` data structure projects. These projects are intended
to generate lightweight, independent, and statically type-safe
containers as drop-ins for new and existing code. The `sh`-script
`autoclone`, downloads them all.

Most of the projects make extensive use of code generation: in the
background, it uses the `C89` pre-processor for template meta-programming
to do compile-time polymorphism.  The documented parameters are
pre-processor macros defined before including the file, and they
are generally undefined automatically before the box is complete
for convenience.  In a project, one can pick and choose which ones
are appropriate.

## Details ##

Assertions are used to ensure data integrity at runtime; to stop
them, define `#define NDEBUG` before `assert.h`, included in the
files. Errors are returned with the standard `errno`. Since this
is portable `C89` code, we have limited options for returning our
own errors, namely, `EDOM`, `ERANGE`, `EISEQ` (1994 Amendment 1 to
`C90`); standard library functions may provide their own values,
which are passed on.

The source files are `UTF-8` and may contain multi-byte literals.
Some specific terminals don't have this as a default.

No effort has been made to synchronize for multi-threaded execution.

## License ##

2016 Neil Edelman, distributed under the terms of the [MIT
License](https://opensource.org/licenses/MIT).

## Todo ##

Compiling with all warnings turned on in some compilers is an endless
race with the `lint`-like warnings. Most of them are super-useful,
but some are not. One may have issues with, for example,
`_CRT_SECURE_NO_WARNINGS`, `-Wno-comma`, `-Wno-logical-op-parentheses`,
`-Wno-parentheses`, `-Wno-shift-op-parentheses`.
