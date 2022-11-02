# kinslayer-mud-2004-11-10

## Overview

The game comes with barebones library files required to run.
Included are two zones and necessary mobs, objects, rooms, etc.
The first character to register will be promoted to the maximum
level ofa level 105.

## Running the game

### Using CLion

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

### Using Docker

From a command line terminal:

1) Navigate to the repository root directory
2) To build the image, enter `docker-compose build game`
3) To run the container, enter `docker-compose up game`