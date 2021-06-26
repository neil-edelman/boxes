# boxes #

`boxes` is an automated dependancy and build system for a `C` data structure
collection using the standard library. The individual projects are separate and
independent. It is therefore unnecessary to _use_ `boxes` in any of the individual
projects, but useful in automated developing.

## _Cf_ Some Implementations ##

* [array](https://github.com/neil-edelman/array);
* [heap](https://github.com/neil-edelman/heap);
* [list](https://github.com/neil-edelman/list);
* [orcish](https://github.com/neil-edelman/orcish);
* [pool](https://github.com/neil-edelman/pool);
* [set](https://github.com/neil-edelman/set);
* [trie](https://github.com/neil-edelman/trie).

## Interface ##

Projects are subdirectories of `boxes` and have separate `git` repositories.
Each one must have `test` and `src`; the the `src` has `C` file(s) which are
the same name as the project; this is the one-source-of-truth. `make` creates
a test in `bin`. Traits are the same for every project and are stored globally
in `boxes` under `traits`.

## Details ##

No effort has been made to synchronize for multi-threaded execution.

Assertions are used to ensure data integrity at runtime; to stop them,
define `#define NDEBUG` before `assert.h`, included in the files, (see
`man assert`.)

Errors are returned with `errno`: `EDOM`, `ERANGE`, `EISEQ`, (1994
Amendment 1 to `C89`); standard library functions provide their own
values which are passed on.

The source files are `UTF-8`.

The documented parameters are preprocessor macros defined before
including the file, and they are generally undefined automatically before
the box is complete for convenience. (Except one can include traits by
including the expect trait define and re-including.) See each project's
`test` section for examples.

There are optional shared files in `interfaces` and `traits`. Some boxes
support different interfaces, and you add an interface by defining the
macro and having the interface file. You can then `EXPECT_TRAIT`, include
the header, and continue defining a trait, and then include the header
again.

## License ##

2016 Neil Edelman, distributed under the terms of the
[MIT License](https://opensource.org/licenses/MIT).

## Todo ##

More traits instead of just `to_string`.
Have `algorithm`, `functional`, `compare`?

TEST depends on RAND, but should probably be different for timing.

Add support for custom allocators. Shouldn't #define malloc(n) cust(n)?
