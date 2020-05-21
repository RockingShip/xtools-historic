# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]

```
2020-05-21 15:33:57 Modified copy of `ctype[]` for `xcc`
                    `symfirst/symnext` is a complext `if()`.
                    The linker orders undefs based on hash value and broke stage3 after ctypes were moved to the library.  
2020-05-21 11:48:43 Allow mixing of statements and declarations.
                    Not being able was only to conform towards traditional C.
2020-05-21 01:11:19 Fixed `ostest.c`.
                    Oops.
2020-05-21 00:17:11 Remove macro `DYNAMIC`.
                    `malloc()` was never supported.
2020-05-20 23:21:31 Rename identifiers.
                    Code uses symbol as reference to identifiers.  
                    Weed out legacy and bad choices.
                    Minor fixes as a consequence.
2020-05-20 21:10:04 Reformat sources. No code change.
                    Finally.
2020-05-20 20:43:30 Support for and convert to enums
                    Drops need for a preprocessor.
2020-05-20 10:16:04 Support for and convert to function prototypes.
                    Makes so many thing easier and IDE chokes on traditional.
2020-05-19 23:09:16 Merge `declloc()/declgbl()` into `declvar()`.
                    Simplify near duplicate code.
2020-05-19 16:42:39 `lval[LPTR]` now consistent.
                    `LPTR` is an addressing mode to indicate `EA` needs to be deferenced.
                    `FUNCTION/ARRAY` are addresss.
2020-05-19 16:42:39 Bugfix: "!" in combination with BRANCH.
                    The "!" only inverted the last operator.
2020-05-18 18:59:57 Merged `glbsym[]` and `locsym[]`.
                    Two identifier pools with duplicated code.
                    Also, identifier scoping is now more managable. 
2020-05-17 23:16:04 Support for and convert to double-slash comments.
                    Implementation in `xcc1.c:preprocess()`
2020-05-17 23:16:04 Added `unsigned_GT()`.
                    `xasm/xlnk` need unsigned compare for large storage.
```

## Release [1.1.0] 2020-05-17 15:06:26

XTools/XLibrary can compile itself and demo `cattle` under 16 and 64 bits architecture.

```
2020-05-17 14:48:16 Got `cattle` working.
                    "make cattle; ./xemu cattle" as described in `README.md`.
                    Slight tweaking to the original.
2020-05-17 14:38:30 Fixed `"for(;;)"` and `"a?b,c:d"`.
                    There were never tested.
2020-05-17 14:33:28 Handle large data.
                    Large arrays or sections greater than 32k require special handling because of their negative sizes.
2020-05-16 19:41:57 `xlnk` can now handle libraries.
                    Libraries handling was not endian aware.
2020-05-16 19:34:24 Fixed emulator file handling.
                    Correct return value (NULL) when opening non-existing files for reading.
                    File offsets and read/write sizes are unsigned.
                    `pso` now opens at invocation stdin/stdout and stderr using the new call `XFILE::fdopen()`.
                    `fdopen()` does not pre-load internal buffers or it will block waiting for input on stdin.
2020-05-16 19:30:40 Make `xar` endian aware.
                    `xar` was one of the last components and received less attention.
2020-05-16 19:03:54 Completed build scripts.
                    Added scripts for `lib` and seperated `getversion()` for native and sandbox.
2020-05-15 20:59:00 Got `xcc,xasm,xlnk` to stage 3.
                    These finally seem to be fully working.
2020-05-15 20:56:35 Sandbox exit code.
                    Allow `make` to detect unexpected situations. 
2020-05-15 20:38:02 Be less verbose and get version from autoconf.
                    Make `make` less noisy. Renavle with `-v`.
2020-05-15 13:49:08 Reduce xcc symbol table size to avoid stack overflow.
                    Late stage of the school project raised the symbol table size to `2003`.
                    This caused  `___END` to `0xf803` and stack grew down to `0xf3cd`.
2020-05-15 00:16:20 Fixed issues that broke `xcc` and `xlnk`.
                    fgets() with long lines.
                    Literal pool dump as words was broken.
                    `REL` command `PUSHB` was broken.
2020-05-15 00:32:37 Fixed and added missing I/O library routines.
                    School project was msdos with text/binary mode.
                    Problem was that text was being processed using calls for binary files.
                    Now, fread()/fwrite() are binary and fgets()/fputs() are text with obsoleded '\r' handling.  
2020-05-14 14:00:56 Sandbox argc/argv.
                    Converted xtools to argc/argv convention. 
2020-05-13 22:32:58 Stage1 compiles and runs sandboxed `ostest.c`. 
                    Synced and tested system call use and implementation.
2020-05-13 17:52:37 Use file extensions `.xs .xo .xa` to not conflict with `.s .o .a .obj .lib`.
                    Use a different set of file extensions to differentiate from ELF archecture.
2020-05-12 23:09:16 Got "-Dint=long" working (Yay!).
                    Xtools assumes "sizeof(int)==sizeof(char*)" which gives problems on 64 bit archecture.
                    Fix sources to run on 16/64 bits by manually sign extending function argumens and some variables. 
2020-05-12 22:42:37 Fixed argc/argv.
                    School project used a single commandline string.
                    Converted to argc/argv convention. 
2020-05-12 22:06:26 Fixed+synced `usage()` and friends.
                    Tune usage/invocation to gcc invocation.
2020-05-12 21:09:06 Fixed name hashing.
                    Late bug found in xtools. 
                    Name chains can be zero terminated if the namech[0] is non-zero. 
2020-05-12 21:01:46 Conform printf format.
                    School assignment had different view on field justification.
2020-05-12 20:19:53 Split `gencode()` to avoid stdargs.
                    Variadic function have always been a pain because arguments are pushed in order of occurance. 
                    native/xtools have contradicting assumptions of this.
                    Solve by splitting the function into the different Variadic flavours.
2020-05-12 20:19:53 Conform fopen/fread/fwrite/fseek and endianness.
                    xtools is a MSB architecture and school assignment had different arguments. 
2020-05-12 19:40:32 Reordered blocks to avoid forward references.
                    xcc will choke on forward references functions because it treat undeclared identifiers as ints.
2020-05-09 23:21:52 Reformat native programs, only critical code changes.
2020-05-12 12:13:53 Add build scripts.
```

## Release 1.0.0 2020-05-09 11:26:17

Original 1991 submitted school assignment.

```
2020-05-09 11:26:17 Changed license and '\n' line separators. 
```

[Unreleased]: /RockingShip/xtools/compare/v1.1.0...HEAD
[1.1.0]: /RockingShip/xtools/compare/v1.0.0...v1.1.0
