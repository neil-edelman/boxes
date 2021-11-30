# boxes #

<table><tr>
	<th>link</th>
	<th>description</th>
	<th>possible traits</th>
	<th>required dependancies</th>
</tr><tr>
	<td><a href="https://github.com/neil-edelman/array">array</a></td>
	<td>contiguous dynamic array</td>
	<td>compare, contiguous, to_string</td>
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
	<td>to_string</td>
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
	<td><a href = "https://github.com/neil-edelman/set">set</a></td>
	<td>unordered associative array</td>
	<td>to_string, compare?</td>
	<td></td>
</tr><tr>
	<td><a href = "https://github.com/neil-edelman/trie">trie</a></td>
	<td>prefix tree</td>
	<td>to_string, compare?</td>
	<td>array?</td>
</tr><tr>
	<td>disjoint</td>
	<td>upcoming</td>
	<td>to_string?, compare?</td>
	<td>array?</td>
</tr></table>

`boxes` is a very simple automated dependancy and build system for
separate `C89` data structure projects that are intended to generate
lightweight, independent, and type-safe containers for existing
code. If one has `git` configured, the `sh`-script, `autoclone`,
downloads them all, or download each project or file individually.
See each for usage, and each project's `test` section for examples.

## Why boxes? ##

> [C sucks because](https://wiki.theory.org/index.php/YourLanguageSucks#C_sucks_because)
> You get nothing else that's truly portable. Rolling your own containers (likely
> inferior to a standard, highly-optimised version) is something you better get
> used to doing for every new application, or using overly complicated existing
> frameworks that take over your application (nspr, apr, glib...).

This aims to be a middle ground. No libraries or frameworks, each
fairly small, simple, stand-alone `C89` code. It's like rolling
your own containers, but aims to have work of testing and documenting
already done. In a project, one can pick and choose which ones are
appropriate.

## Internal interface ##

Most of the `C` code makes extensive use of the pre-processor for
code generation. So much so, that traditional parsers get hopelessly
confused; by default, a custom parser is used for documentation:
[cdoc](https://github.com/neil-edelman/cdoc).

The code in the individual projects should work on most systems and
compilers. The build system (this) requires a shell that understands
`sh`-scripts and GNU `Makefile`; it is useful in automated developing
of these containers.

Projects are subdirectories of `boxes` and have separate `git`
repositories.  Each one must have `test` and `src`. Files could be
duplicated, but the `C` one-source-of-truth files are: any files
the `src` has which are the same name as the project, and files in
`traits`. `make` creates a test in `bin` using `gcc` or `clang`.

## Details ##

The documented parameters are preprocessor macros defined before
including the file, and they are generally undefined automatically
before the box is complete for convenience. Since `C` doesn't have
interfaces, one can include anonymous and named traits with the
`EXPECT_TRAIT`.  This returns in a state of incompletion until one
includes it again with the trait's parameters defined.

Assertions are used to ensure data integrity at runtime; to stop
them, define `#define NDEBUG` before `assert.h`, included in the
files.

Errors are returned with the standard `errno`. Since this is portable
`C89` code, we have limited options for returning our own errors,
namely, `EDOM`, `ERANGE`, `EISEQ` (1994 Amendment 1 to `C90`);
standard library functions may provide their own values, which are
passed on.

The source files are `UTF-8` and may contain multi-byte literals.
Some specific terminals don't have this as a default.

No effort has been made to synchronize for multi-threaded execution.

## License ##

2016 Neil Edelman, distributed under the terms of the [MIT
License](https://opensource.org/licenses/MIT).

## Todo ##

Compiling with all warnings turned on in some compilers is an endless
race with the `lint`-like warnings. Most of them are super-useful,
but some are not. You may have issues with, for example,
`_CRT_SECURE_NO_WARNINGS`, `-Wno-comma`, `-Wno-logical-op-parentheses`,
`-Wno-parentheses`, `-Wno-shift-op-parentheses`.
