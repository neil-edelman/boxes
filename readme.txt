Copyright (C) 2018 Neil Edelman, see copying.txt.  neil dot edelman
each mail dot mcgill dot ca; distributed under the terms of the MIT
License; \url{ https://opensource.org/licenses/MIT }.

This is an example programme showing one way to do static object
oriented things with C. To do this, it separates the storage,
({Pool},) from the data structure, ({List}). Pool and List are
generics controlled by the pre-processor. We have a virtual table
for all virtual functions. All this is wrapped up in {Animals} and
tested.
