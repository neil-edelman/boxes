Unlike languages that have built-in support for dynamic-typing,
there's no one obvious one way to do object-oriented things in
<em>C</em>. We focus on the case where you have some collection of
objects of different types and you want to iterate and do something,
often depending on the type.  </p>

<p> <img src = "Animals.png" width = 736 height = 738> </p>

<p> In this example, it separates the storage, most of the time
<em>Pool</em>, from the data structure, <em>List</em>. <em>Pool</em>
and <em>List</em> are generics controlled by the pre-processor. The
inheritance, denoted by a clear arrow, is manifest in the code as
nested <em>struct</em>s. By doing it this way, for example, you
cannot refer to <em>bad_emu->name</em>, but
<em>bad_emu->emu.animal.data.name</em>.  This explicitness favours
shallow polymorphism.  </p>

<p> <em>Pool</em> is a dynamic list, and may move elements around
in memory; any dependencies must be updated.  All the composition
links, denoted by a black diamond, from a <em>Pool</em> or a
descendant of a <em>Pool</em> must have a memory move function that
corrects the pointers. In this case, <em>emu%()</em>, says that in
<em>emu_migrate()</em>, linked to <em>struct Emu</em> by, (<em>#define
POOL_TYPE struct Emu</em>, <em>#define POOL_MIGRATE_EACH
&emu_migrate</em>, #include "Pool.h"</em>,) there is a case handing
the pointed-at structure.  </p>

<p> We have a virtual table for all virtual functions, (not shown,)
one for each <em>Animal</em>. If we were doing this in some other
language, we might have <em>MountInfo</em> be an interface.
