Copyright (C) 2018 Neil Edelman, see copying.txt.  neil dot edelman
each mail dot mcgill dot ca; distributed under the terms of the MIT
License; \url{ https://opensource.org/licenses/MIT }.

Unlike languages that have built-in support for dynamic-typing,
there's no one obvious way to do object-oriented things in {C}. We
focus on the case where you have some collection of objects of
different types and you want to iterate and do something depending
on the type. In this example, it separates the storage, ({Pool},)
from the data structure, ({List}.) Pool and List are generics
controlled by the pre-processor. We have a virtual table for all
virtual functions. All this is wrapped up in {src/Animal.c} and
tested in {test/Animals.c}.
