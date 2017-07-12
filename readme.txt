Copyright (C) 2017 Neil Edelman.

neil dot edelman each mail dot mcgill dot ca

Defining T by LIST_TYPE, src/List.h makes available a generic,
type-safe, <T>List: the head of linked-list(s) of <T>ListNode. These
can potentially be part different structures. Makefile links it
with the tests. See doc/List.html for documentation. This is C89+
code; C++ has templates, and there is little value in using it in
a C++ project.

License:

The MIT License (MIT)

Copyright (c) 2017 Neil Edelman

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject
to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

The software is provided "as is," without warranty of any kind,
express or implied, including but not limited to the warranties of
merchantability, fitness for a particular purpose and noninfringement.
In no event shall the authors or copyright holders be liable for
any claim, damages or other liability, whether in an action of
contract, tort or otherwise, arising from, out of or in connection
with the software or the use or other dealings in the software.
