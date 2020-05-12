# XTools

X-C-Compiler/Assembler/Linker/Archiver. 
A remastered 1991 school project intended for supporting the [untangle](/RockingShip/untangle) fractal engine.

## About the project

In my early education days of computer science there was a "PSO" (practical system design) assignment which required the 
design, implementation and testing of a emulated CPU and MMU unit. 
Personally not challenging so to up the ante and in agreement with school staff I received permission to
extend the testing by implementing a turing-complete compiler/assembler/linker/archiver toolchain.
I was allowed to extend the CPU instruction as long as changes were backwards compatible. 
The biggest challenge was fitting the toolchain in 64k memory with most instructions being 5-6 bytes.

```xcc``` was heavily inspired and partly based on  ```Small-C``` and Digital-Equipment's TOPS-20 ```"REL blocks"```.
[```Small-C```](https://en.wikipedia.org/wiki/Small-C) is a minimalistic ```C``` compiler for resource-limited environments and 
[```"REL blocks"```](https://www.livingcomputers.org/UI/UserDocs/TOPS-20-v7-1/3_LINK_Reference_Manual.pdf) (chapter 1 and appendix A) which are basically a mix of code and post-fix operators to evaluate and resolve symbol references.

```xcc``` language features:

 - Preprocessor ```#include```, ```#define``` and ```#asm```
 - No structures (use arrays with named offsets)
 - No ```typedef```
 - Datatypes ```char```, ```int``` and "(single) pointer to"
 - Signed ints only
 - ```"sizeof(int)==sizeof(char*)"``` (allows for de-referencing of ints)
 - No heap or ```malloc()``` (pointer stack based variables might be an alternative)

Project layout:
```
    <root>
    +-- pso/     # school assigned emulator
    +-- source/  # xcc,xasm,xlnk,xar 
    +-- lib/     # X-library 
    \-- xemu.c   # native emulator
```

`xcc` assumes 16-bit/MSB architecture.
The project was originally developed using the 16-bits Zortech C/C++ development environment.

The function `disp_opc()` in `xemu.c` is an opcode cheat sheet.

## Getting Started

These instructions will get the project up and running on your local machine for development and testing purposes.

### Prerequisites

The school assignment is written in traditional ```C++``` and ```xcc``` written in minimalistic ```C```.

Both assume that ```"sizeof(int)==sizeof(char*)"```. 
Pointers are stored in ints which need need some creativity to work with 64-bit architecture. 

This requires the gcc compiler flags ```"-traditional-cpp -fpermissive -Dint=long"``` and ```libc``` replacement. 

### Installing

The makefile supports a number of targets:

```
    make pso     # emulator of school assignment
    make stage1  # build 'xcc' with native compiler
    make stage2  # build 'xcc' with emulated stage1 compiler 
    make stage3  # build 'xcc' with emulated stage2 compiler and compare result with stage2
```

Included example is my 1996 IOCCC submission: [Bulls and Cows](https://en.wikipedia.org/wiki/Bulls_and_Cows),
where you think of a secret number and the program tries to guess.
It's originally a one-liner, extra whitespace included for readability.
Anything can be programmed into a for-loop.

Compile test program. 
With gcc you would issue: "gcc cattle.c -traditional-cpp"

```
    # compile
    ./xemu xcc cattle
    # assemble
    ./xemu xasm cattle
    # link
    ./xemu xlnk cattle -l xlib
```

Run test program. 

```
    # run (think of secret number 0291)
    ./xemu cattle
    1123  # program displays first guess
    02    # you answer with "02". No digits at correct position, '2' and '1' present
    7912  # program displays second guess
    03    # you answer with "03". No digits at correct position, '2', '9' and '1' present
    9237  # program displays third guess
    11    # you answer with "11". The '2' is correct, '1' is present
    0291  # program displays fourth guess
    40    # you answer with "40". The guess is correct
```




## Versioning

We use [SemVer](http://semver.org/) for versioning.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Ron Cain, Jim E. Hendrix and many others for Small-C. Hat tip for placing in public domain!
* Digital Equipment for amazingly life path inspiring hardware and software
