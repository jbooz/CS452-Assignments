# Homework #2: Memory Hole

* Author: Jonathan Boozel
* Class: CS452 Section #2
* Semester: Fall

## Overview

The goal of this program is to implement a user-space memory allocator using the buddy system algorithm.
The allocator uses the buddy system to manage memory regions obtained directly from the operating system
via functions like mmap() (which uses functions such as "malloc()" for memory management). All of these
functions work in tandem with one another to create a fluent memory management system that works seamlessly
to allocate memory correctly across the operating system.

The main.c file is used to test the functionality of everything, as it runs a few tests and sees if they pass
to determine if the program is running as intended.

## Steps to Compile

To properly compile this program, you must first ensure that you have opened a terminal
that is running within the "Source Files" folder so that everything works as intended.
To build all the files type "make" into the command line, and all the files and their
dependencies will build so they can be run. To run the "main.c" file, type "make run" into
the command line. This will run "main.c" which contains all the tests and is the extent
of this program.

## Results 

After compiling "main.c", the output showed that all of my tests passed. This confirms that
every method works as intended and nothing differed from its expected value.

I also ran the program through valgrind and it relayed that I had 0 memory leaks because every
malloc had a free attatched to it.

----------
