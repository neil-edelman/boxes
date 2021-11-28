# boxes #

`boxes` is an automated dependancy and build system for a `C` data structure
collection using the standard library. The `C89` code that is part of the individual
projects are separate, independent, and should work on most systems and
compilers. The build system (this) requires a shell that understands `sh`-scripts
and `Makefile`; it is useful in automated developing of the code. If one has
`git` set up, the script `autoclone` downloads them all.

## Examples ##

<table><tr>
	<th>link</th>
	<th>possible traits</th>
	<th>required dependancies</th>
</tr><tr>
	<td>https://github.com/neil-edelman/array</td>
	<td>compare, contiguous, to string</td>
	<td></td>
</tr><tr>
	<td>https://github.com/neil-edelman/bmp</td>
	<td></td>
</tr><tr>
	<td>https://github.com/neil-edelman/heap</td>
	<td></td>
</tr></table>

## Why boxes? ##

> [C sucks because](https://wiki.theory.org/index.php/YourLanguageSucks#C_sucks_because)
> You get nothing else that's truly portable. Rolling your own containers (likely
> inferior to a standard, highly-optimised version) is something you better get
> used to doing for every new application, or using overly complicated existing
> frameworks that take over your application (nspr, apr, glib...).

This is a middle ground. No libraries. Each stand-alone `C89` code. It's like
rolling your own containers, but all the work of testing and documenting has
already been done, (mostly, it is ongoing.) One can pick and choose which
ones are appropriate for ones project. Where appropriate, the headers require
`#define` parameters; see each for usage.

## Interface ##

Internal interface is projects are subdirectories of `boxes` and have separate
`git` repositories. Each one must have `test` and `src`; the the `src` has `C`
file(s) which are the same name as the project; this is the one-source-of-truth.
`make` creates a test in `bin`. Traits are the same for every project and are
stored globally in `boxes` under `traits`.

The interface could change in future versions.

## Details ##

The documented parameters are preprocessor macros defined before
including the file, and they are generally undefined automatically before
the box is complete for convenience. See each project's `test` section
for examples.

Since `C` doesn't have interfaces, one can include anonymous and
named traits with the `EXPECT_TRAIT`. This returns in a state of
incompletion until one includes it again.

Assertions are used to ensure data integrity at runtime; to stop them,
define `#define NDEBUG` before `assert.h`, included in the files.

Errors are returned with the standard `errno`: `EDOM`, `ERANGE`, `EISEQ`
(1994 Amendment 1 to `C90`); standard library functions provide their own
values, which are passed on.

The source files are `UTF-8` and may contain multi-byte literals. Some
terminals don't have this as a default.

No effort has been made to synchronize for multi-threaded execution.

## License ##

2016 Neil Edelman, distributed under the terms of the
[MIT License](https://opensource.org/licenses/MIT).

## Todo ##

Compiling with all warnings turned on in some compilers is an endless race
with the `lint`-like warnings. Most of them are super-useful, but some are
not. You may have issues with, for example, `_CRT_SECURE_NO_WARNINGS`,
`-Wno-comma`, `-Wno-logical-op-parentheses`, `-Wno-parentheses`,
`-Wno-shift-op-parentheses`.
