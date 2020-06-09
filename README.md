# TSF - Transaction Serialization Format

## Contents

* <a href="#intro">Introduction</a>
* <a href="#ccode">C Code Description and Build</a>
* <a href="#tfiles">Performance Test Files</a>
* <a href="#jcode">Java Code</a>

<a id="intro"></a>
### Introduction ###

This is the source repository for TSF code.  Java code is in the Java directory.  C code is in the C directory.  The Performance test files are in the test-files directory.

The Transaction Serial Format (TSF) and the Transaction Array Model (TAM) provide full, efficient, transaction serialization facilities for devices with limited onboard energy, such as those in an Internet of Things (IoT) network.  TSF provides a compact, non-parsed, format that requires minimal processing for transaction deserialization.  TAM provides an internal data structure that needs minimal dynamic storage and directly uses the elements of TSF.  The simple lexical units of TSF do not require parsing. The lexical units contain enough information to allocate the internal TAM data structure efficiently.  TSF generality is equivalent to XML and JSON.  TSF represents any XML document or JSON object without loss of information, including whitespace.  A performance comparison of a C reference implementation of TSF and TAM to the popular Expat XML library, also written in C, shows that TSF reduces deserialization processor time by more than 80%

<a id="ccode"></a>
### C Code Description and Build ###

The performance measurements for TSF were all done with C programs, because the benchmark library, lib_expat, was written in C.  The C code is written using an idiomatic C which follows object-oriented techniques, in effect object-oriented C.  It is strict ANSI C and does not require a C++ compiler.  I feel that OO techniques are easily adapted to C code and provide the same advantages as do OO languages.

There are two versions of the TSF code, an ASCII version the uses 8-bit characters internnaly, and a UTF-8 (UCS-4 internal) version.  Since lib_expat handled UTF-8, direct comparisons to lib_expat performance were done using the URTF-8 version.  The TSF ASCII version performs better, and there are other advantages to using it, described in detail in the dissertation.  The primary difference is the use of the type `wchar_t`, `typedef`'ed as `Char`, and its corresponding functions, instead of the basic character type `char`.  In a few sources cases, the version differences are minimal and handled with conditional compiles, but in some cases, I felt that they would be easier to read and understand without conditional compiles.  Any program whose name contains the characters `UC` is a wide character (UTF-8, UCS-4 internal) version of the corresponding 8-bit source code.

### Lib_expat Build ###

Lib_expat can be acquired in either source form or pre-built.  For the TSF comparisons, lib_expat was built from source on a Mac Pro with OS X.  The same compiler was used to build the TSF code in order to have an apples-to-apples comparison.  Lib_expat build instructions are part of the lib_expat package and are omitted here.  The following programs are the minimal amount of code required to use the lib_expat package as an XML deserializer (parser).  This is the code used to collect performance numbers for lib_expat.

### Performance Comparision Source ###

**Npx.c**, **NpxUC.c** - The primary comparison driver, which will use Expat to parse an XML file, or TSF code to parse a TSF file.  It uses a number of command line switches to determine what specific function will be done for any given run.  It will run using a single file as input or will search a directory for all the files with a specific suffix.

**ExpCharacterData.c**, **ExpCharacterData.h** - A minimal character data object invoked when Expat encounters CDATA or PCDATA in the input

**ExpElement.c**, **ExpElement.h** - A minimal XML element object invoked when Expat encounters an XML element in he input

**expat-text.c** - Verify the operation of Expat and options used for the performance comparison.  This code is linked in the Npx.c test driver

**Npx2XML.c** - Convert TSF files to XML files, used for lossless conversion verification.

**NpxDeserial.c** - The serialization and deserialization conditionally compile either 8-bit or UCS-4 (`wchar\_t`) functions, and are linked in the test driver

### Common Functions ###

The TSF programs use three custom packages that contain low-level processing that is common to many C programs. I wrote these many years ago for generic C programming.

**Memory.c**, **Memory.h** - These functions are an interface to the C memory allocation functions.  They provide an out-of-memory error handling capability based on `setjmp.h`, a debugging capability that tracks memory allocations to avoid memory leaks, and a fence around allocation blocks that can detect some writes that exceed the boundary of an allocated block of memory.

**Sstring.c**, **Sstring.h** - These functions manage C-string assignments, concatenations, and substrings, and the memory allocations that are needed by these operations.  These functions depend upon `Memory.h`.

**StringBufffer.c**, **StringBufffer.h** - These functions provide string creation that does not require prior knowledge of the length of a string.  String buffers are chunks of string data that are linked together internally.  The functions provide an interface that hides the fact that the data is not stored contiguously. These functions depend upon `Memory.h`.

**UCStringBufffer.c**, **UCStringBufffer.h** - Wide character versions.

### TSF Programs ###

Some programs have wide character (UTF-8) and 8-bit versions.  In other cases, the changes between versions are minimal, and are handled by conditional compiles.

**NpxRoot.h** - A header for common code ans definitions.

**NpxUCElement.h** - Declaration of the method table for the NpxUCElement object.  This is the only "public" member of the NpxUCElement object.

**NpxUCElement.c** - The `NpxElement.c` file is the implementation of the NpxElement object, and contains all the methods referenced in the method table.  It also declares the NpxElementI structure which is the "private" implementation of the properties of the NpxElement object.   The code uses an interface (Memory.h) to the C library dynamic memory management functions.

**NpxElement.c**, **NpxElement.h** - 8-bit version.

### Miscellaneous Programs ###

**utf8-1.c** - Display the compiler size of the character types.

### CMakelists ###

The following commands comprise a CMakelists file which describes the C program build and can be used directly by any IDE that recognizes CMakelists.TheyIt can also be converted to build files for other IDE's.

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

<a id="tfiles"></a>
### Test File Archive
These are the test files used in the performance comparison.  The characteristics of all files are described in the dissertation.  Each file has a TSF format version and a corresponding XML formal version.  The performance test simply deserializes each version, captures, and displays the results.  The XML files are described in the following table.

File Name | File Size | Description
----------|-----------|------------
future001.xml | 70358 | Scenario file from the Mana Game Series
bpmnxpdl\_40a.xsd.xml | 160946 | XSD file for XPDL 2.0
eric.map.osm.xml | 218015 | OpenStreetMap export from northern WVa
cshl.map.osm.xml | 298233 | OSM export of a research laboratory
sccc.map.osm.xml | 404977 | OSM export of a community college
British-Royals.xhtml | 482666 | British Royalty Lineage from Alfred the Great
csh\_lirr\_osm.xml | 712661 | OSM export of a train station
exoplanet-catalog.xml | 2147926 | NASA Kepler Exoplanet Catalog
LARGEbasicXML.xml | 3420388 | Military Strategy Game Unit Order of Battle

<a id="jcode"></a>
### Java Source

The Java code is not part of the TSF perfvormance comparison.  It provides some conversion utilities and generally demonstrates the the TSF format is easily handled in Java as well as C.

