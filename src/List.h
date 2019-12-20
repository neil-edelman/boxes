/** @license 2017 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Parameterised List

 <tag:<L>List> is a list of <tag:<L>ListLink>; it may be supplied a total-order
 function, `LIST_COMPARE` <typedef:<PI>Compare>.

 Internally, `<L>ListLink` is a doubly-linked node with sentinels residing in
 `<L>List`. It only provides an order, but `<L>ListLink` may be enclosed in
 another `struct`. While in the list, the links should not be added to another
 list.

 `<L>Link` is not synchronised. Errors are returned with `errno`. The
 parameters are `#define` preprocessor macros, and are all undefined at the end
 of the file for convenience. `assert.h` is used; to stop assertions, use
 `#define NDEBUG` before inclusion.

 @param[LIST_NAME]
 `<L>` that satisfies `C` naming conventions when mangled; required. `<PL>` is
 private, whose names are prefixed in a manner to avoid collisions; any should
 be re-defined prior to use elsewhere.

 @param[LIST_COMPARE]
 Optional total-order function satisfying <typedef:<PL>Compare>.

 @param[LIST_TO_STRING]
 Optional print function implementing <typedef:<PL>ToString>; makes available
 <fn:<L>ListToString>.

 @param[LIST_TEST]
 Unit testing framework <fn:<L>ListTest>, included in a separate header,
 <../test/TestList.h>. Must be defined equal to a random filler function,
 satisfying <typedef:<PL>Action>. Requires `LIST_TO_STRING` and not `NDEBUG`.

 @std C89
 @cf [Array](https://github.com/neil-edelman/Array)
 @cf [Orcish](https://github.com/neil-edelman/Orcish)
 @cf [Pool](https://github.com/neil-edelman/Pool)
 @cf [Set](https://github.com/neil-edelman/Set) */

/* Check defines. */
#ifndef LIST_NAME
#error Generic LIST_NAME undefined.
#endif
#if defined(LIST_TEST) && !defined(LIST_TO_STRING)
#error LIST_TEST requires LIST_TO_STRING.
#endif


