This is a small implementation of base64 encode and decode functions designed for embedded systems.
No dynamic memory is needed. However, a fair amount of static memory is needed for the lookup tables.

Storage of the lookup tables can be adapted using the ``BASE64_LOOKUP_TABLE_SECTION`` define.
If it is set, the lookup table will be moved to the section specified by the define. Example:

``#define BASE64_LOOKUP_TABLE_SECTION ".bss"``

This project is licensed under GPLv2.

## Build Instructions
* Go to build directory and execute ``cmake <path to this library>``
* Type ``make`` to build the static library object.
* Type ``make execute-base64-test`` to build and run the tests
