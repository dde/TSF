# TSF - Transaction Serialization Format

Source repository for TSF code.  Java code is in the Java directory.  C code is in the C directory.

The Transaction Serial Format (TSF) and the Transaction Array Model (TAM) provide full, efficient, transaction serialization facilities for devices with limited onboard energy, such as those in an Internet of Things (IoT) network.  TSF provides a compact, non-parsed, format that requires minimal processing for transaction deserialization.  TAM provides an internal data structure that needs minimal dynamic storage and directly uses the elements of TSF.  The simple lexical units of TSF do not require parsing. The lexical units contain enough information to allocate the internal TAM data structure efficiently.  TSF generality is equivalent to XML and JSON.  TSF represents any XML document or JSON object without loss of information, including whitespace.  A performance comparison of a C reference implementation of TSF and TAM to the popular Expat XML library, also written in C, shows that TSF reduces deserialization processor time by more than 80%

## C Code Description and Build

The performance measurements for TSF were all done with C programs, because the benchmark library, lib_expat, was written in C.  The C code is written using an idiomatic C which follows object-oriented techniques, in effect object-oriented C.  It is strict ANSI C and does not require a C++ compiler.  The author feels that OO techniques are easily adapted to C code and provide the same advantages as do OO languages.

There are two versions of the code, an ASCII version the uses 8-bit characters internnaly, and a UTF-8 (UCS-4 internal) version.  Since lib_expat handled UTF-8, direct comparisons to lib_expat performance were done using the URTF-8 version.  The ASCII version performs better, and there are other advantages to using it, described in detail in the dissertation.  The primary difference is the use of the type `wchar\_t`, `typedef`'ed as `Char`, and its corresponding functions, instead of the basic character type `char`.  These could have been coded as a single set of conditionally compiled programs, but it was felt that they would be easier to read an understand without conditional compiles.

### Lib_expat Build ###

Lib_expat can be acquired in either source form or pre-built.  For the TSF comparisons, lib_expat was built from source on a Mac Pro with OS X.  The same compiler was used to build the TSF code in order to have an apples-to-apples comparison.  Lib_expat build instructions are part of the lib_expat package and are omitted here.

### Common Functions ###

The programs use three custom packages that contain low-level processing that is common to many C programs. These were written by the author many years ago for generic C programming.

**Memory.h** - These functions are an interface to the C memory allocation functions.  They provide an out-of-memory error handling capability based on `setjmp.h`, a debugging capability that tracks memory allocations to avoid memory leaks, and a fence around allocation blocks that can detect some writes that exceed the boundary of an allocated block of memory.

**String.h** - These functions manage C-string assignments, concatenations, and substrings, and the memory allocations that are needed by these operations.  These functions depend upon `Memory.h`.

**StringBufffer.h** - These functions provide string creation that does not require prior knowledge of the length of a string.  String buffers are chunks of string data that are linked together internally.  The functions provide an interface that hides the fact that the data is not stored contiguously. These functions depend upon `Memory.h`.

### UTF-8 Objects are all named with UC ###

**NpxUCElement.h** - Declaration of the method table for the NpxUCElement object.  This is the only "public" member of the NpxUCElement object.

**NpxUCElement.c** - The `NpxElement.c` file is the implementation of the NpxElement object, and contains all the methods referenced in the method table.  It also declares the NpxElementI structure which is the "private" implementation of the properties of the NpxElement object.   The code uses an interface (Memory.h) to the C library dynamic memory management functions.

### Serialization/Deserialization Code ###

**NpxDeserial.c** - The serialization and deserialization conditionally compile either 8-bit or UCS-4 (`wchar\_t`) functions.

### CMakelists File ###

This file describes the C program build and can be used directly by any IDE that recognizes CMakelists.  It can also be converted to build files for other IDE's.

```
cmake_minimum_required(VERSION 3.12)
project(npx)

include_directories(/Users/danevans/Pace/research/dissertation/libexpat-R_2_2_6/expat)

link_directories(/Users/danevans/Pace/research/dissertation/libexpat-R_2_2_6/expat/cmake-build-debug)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES hello.cpp)

set(NPX_UTIL_HDR Memory.h Sstring.h StringBuffer.h UCStringBuffer.h)
set(NPX_UTIL_SRC Memory.c Sstring.c StringBuffer.c UCStringBuffer.c)
set(NPX_BASIC_HDR NpxElement.h NpxRoot.h)
set(NPX_BASIC_SRC NpxElement.c)
set(NPX_HDR ${NPX_BASIC_HDR} ${NPX_UTIL_HDR})
set(NPX_SRC ${NPX_BASIC_SRC} ${NPX_UTIL_SRC})
set(NPX_UCHDR NpxUCElement.h ${NPX_UTIL_HDR})
set(NPX_UCSRC NpxUCElement.c ${NPX_UTIL_SRC})

set(NPX_SOURCE Npx.c NpxDeserial.c Npx2XML.c expat-test.c ${NPX_SRC} ${NPX_HDR} ${NPX_UCSRC} ${NPX_UCHDR})
set(NPX_UCSOURCE NpxUC.c NpxDeserial.c Npx2XML.c expat-test.c ${NPX_UCSRC} ${NPX_UCHDR} ${NPX_SRC} ${NPX_HDR})

# add_library(expat SHARED STATIC ${EXPAT_LOCATION}/libexpat)
add_executable(npx ${NPX_SOURCE})
add_executable(npxuc ${NPX_UCSOURCE})
add_executable(utf8-1 utf8-1.c)
add_executable(ucsbuf UCStringBuffer.c Memory.c UCStringBuffer.h Memory.h)
target_link_libraries(npx libexpat.dylib)
target_link_libraries(npxuc libexpat.dylib)
```
