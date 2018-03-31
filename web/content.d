</p>

<h1>Dynamic <em>Animals</em> using <em>List</em> and <em>Pool</em>
in <em>C</em></h1>

<p> Using <em>&lt;Animal&gt;List</em> as the abstract data type,
and <em>Pool</em> of different types containing
<em>&lt;Animal&gt;ListNode</em> sets up a kind of static inheritance.
</p>

<p> <img src = "Animals.png" width = 693 height = 951> </p>

<p> <em>Pool</em> and <em>List</em> are kind of generics controlled
by the pre-processor, in the sense that you define something, and
include something, and the code for that one case is written
automatically.  These are arguably preferable to turning off static
type-checking by using pointers-to-void.</p>

<p> The inheritance, denoted by a clear arrow, is manifest in the
code as nested <em>struct</em>s. By doing it this way, for example,
you cannot refer to <em>bad_emu->name</em>, but
<em>bad_emu->emu.animal.data.name</em>.  </p>

<p> <em>Pool</em> is a dynamic list, and may move elements around
in memory; any dependencies must have a memory move function that
updates the pointers. These are the dashed dependancy lines. One
will notice that the dependancies flow out of each object which is
a <em>Pool</em> or which descends therefrom. For example, wlg, the
arrow between <em>Emu</em> and <em>Animal</em> labeled <em>emu%()</em>,
says that in <em>emu_migrate()</em>, linked to <em>struct Emu</em>
by, (<em>#define POOL_TYPE struct Emu</em>, <em>#define POOL_MIGRATE_EACH
&emu_migrate</em>, #include "Pool.h"</em>,) there is a case handing
<em>&lt;Animal&gt;List</em>, notably, <em>AnimalListNodeMigrate()</em>.
If we wanted a static array of animals, this would not be needed,
as in the case of <em>Bear</em>.</p>

<p> We have a static virtual table for all virtual functions,
<em>AnimalVt</em>, one for each type of <em>Animal</em>. Note that
the variables must be all private or all public, corresponding to
putting the <em>struct</em> in the <em>.c</em> file or exporting
it to the <em>.h</em> file.
