# Overview

This repository contains the source code for a version of
KinslayerMUD from November 2004. The game was originally
founded in September 2002, and this version comes shortly
after the first major player file wipe that occurred in
the summer of 2004 when the game transitioned from a personal
computer hosted in Maryland, to a server managed by the user
named Void.

This repository comes with barebones library files required to
boot up fully and log in. Included are two zones and necessary
mobs, objects, rooms, etc. The first character to register will
be promoted to the maximum level of 105.

The source code has been modified slightly to achieve the following:
1) Bug fixes to support 64-bit CPU architecture. One bug in particular in buffer.cpp tried to round the buffer size to the nearest power of 2, but did not work properly using the 64-bit unsigned integer `size_t`.
2) Compiling using C++ 20x
3) Fixes for some memory leaks & corruption reported by Valgrind
4) Resolving some compiler warnings. As of now many more still remain but are mostly benign in its current state.
5) Parsing library files wither either \r\n or \n line endings. This version of the game ran primarily on Linux and did not need to support carriage returns

# Running the game

## Using CLion

Included is a `CMakeList.txt` file that can be used to build &
run the game with the CLion IDE:

1) Open CLion
2) Select `File -> Open`
3) Navigate to the root repository directory
4) Build the game by selecting `Build -> Build Project`
5) Open the configuration for the project by selecting `Run -> Edit Configurations...`
6) Under `"Program arguments:"`, enter `-d ../lib`
7) Click `Apply` & `OK`
8) Run the game by selecting `Run -> Run 'kinslayer-mud-2004-11-10'`

## Using Docker

From a command line terminal:

1) Navigate to the repository root directory
2) To build the image, enter `docker-compose build game`
3) To run the container, enter `docker-compose up game`