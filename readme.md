# boxes #

`boxes` is an automated dependancy and build system for a `C` data structure
collection using the standard library. The `C89` code that is part of the individual
projects are separate, independent, fairly standard, and should work on most
systems and compilers. The build system (this) requires a shell that understands
`sh`-scripts; it is useful in automated developing.

## Interface ##

Projects are subdirectories of `boxes` and have separate `git` repositories.
Each one must have `test` and `src`; the the `src` has `C` file(s) which are
the same name as the project; this is the one-source-of-truth. `make` creates
a test in `bin`. Traits are the same for every project and are stored globally
in `boxes` under `traits`. The script `./autoclone` downloads them all, if
one has `git` set up.

## Why boxes? ##

> [C sucks because](https://wiki.theory.org/index.php/YourLanguageSucks#C_sucks_because)
> You get nothing else that's truly portable. Rolling your own containers (likely
> inferior to a standard, highly-optimised version) is something you better get
> used to doing for every new application, or using overly complicated existing
> frameworks that take over your application (nspr, apr, glib...).

This is a middle ground. No libraries. Each stand-alone `C89` code. It's like
rolling your own containers, but all the work of testing and documenting has
already been done, (mostly, it is ongoing.) Where appropriate, the headers
require `#define` parameters; see each for usage.

## Details ##

No effort has been made to synchronize for multi-threaded execution.

Assertions are used to ensure data integrity at runtime; to stop them,
define `#define NDEBUG` before `assert.h`, included in the files, (see
`man assert`.)

Errors are returned with the standard `errno`: `EDOM`, `ERANGE`, `EISEQ`
(1994 Amendment 1 to `C90`); standard library functions provide their own
values, which are passed on.

The source files are `UTF-8`. Some terminals don't have this as a default
(_eg_ `code::blocks`.)

The documented parameters are preprocessor macros defined before
including the file, and they are generally undefined automatically before
the box is complete for convenience. (Except one can include traits by
including the expect trait define and re-including.) See each project's
`test` section for examples.

There are optional shared files in `traits`. Some boxes support different
named traits with the `EXPECT_TRAIT`, include the header, and continue
defining a trait, and then include the header again.

## License ##

2016 Neil Edelman, distributed under the terms of the
[MIT License](https://opensource.org/licenses/MIT).

## Todo ##

 `PERL` if one wants endless parentheses. `-Weverything` but without
 _eg_ `-Wno-comma`, `-Wno-logical-op-parentheses`,
 `-Wno-parentheses`, `-Wno-shift-op-parentheses`.

TEST depends on RAND, but should probably be different for timing.

Add support for custom allocators. Shouldn't #define malloc(n) cust(n)?

Support multithreading where appropriate.
