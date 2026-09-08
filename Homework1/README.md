# Homework #1: Queue<Anon>

* Author: Jonathan Boozel
* Class: CS452 Section #2
* Semester: Fall

## Overview

The goal of this program is to create a double-ended doubly-linked list that acts as a
queue in the C language. It has Nodes that point to next/previous Nodes, and Nodes can
be added to either end. The queue operates under FIFO, which is typical for this type
of data structure. The queue has 4 main methods that manipulate its data. The 4 methods,
put, get, ith, and rem all attempt to alter the contents or retrieve data from the queue.

The "main.c" file is the main program, which is meant to test functionality of each of the
methods described above. The program will run to check that every method works as intended.

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
