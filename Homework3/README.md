# Homework #3: Execution Integrity and Do Process

* Author: Jonathan Boozel
* Class: CS452 Section #2
* Semester: Fall

## Overview

The goal of this program is to implement my own working shell in C. The shell is capable of parsing 
command lines, managing execution paths, executing shell builtins (`pwd`, `cd`, `exit`, `history`), 
supporting standard input/output redirection (`<`, `>`), and handling command sequences and pipelines.

The custom test suite that I have built is used to verify the correctness of the shell's parsing,
builtin handling, and execution logic.

## Steps to Compile

To properly compile this program, you must first ensure that you have opened a terminal
that is running within the source files directory. 
* To build the shell program, type **`make`** into the command line.
* To run the regression test suite, type **`make test`**.
* To check for memory leaks, type **`make valgrind`**.
* To clean up compiled binaries and object files, type **`make clean`**.

## Results 

After running 'make test', all test cases passed successfully, confirming that command execution,
redirection, sequences, and builtins behave as intended.

Running the program through Valgrind confirmed that it has zero definite memory leaks, with any remaining 
allocated blocks consisting solely of standard reachable memory from the GNU Readline library.

----------
