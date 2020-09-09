# RDDLAdapter

Adapt the work from Keller et al. (PROST) and Sanner (RiDDLeSim) to a 
deep reinforcement learning tool developed at Saarland University (in process).

For this purpose, I implement a python-scipt to merge the starting of RiDDLeSim 
and Prost in one script executing all together. 

The communication is implemented via "named pipes" according to the scheme of the learner.

## Prerequisites

### General

In general you need to have the following:

 * git in order to clone this repository

### PROST

To run prost, you need to install or fulfill the following requirements:

 * Linux is you operating system
 * flex and bison are installed to parse the input
 * cmake and c++ compiler (recommended is gcc)
 * the BDD library [BuDDy](http://sourceforge.net/projects/buddy)

You can install most of the dependencies with the following command

`sudo apt install git g++ cmake bison flex libbdd-dev`

If BuDDy is not found automatically, you can additionally set an
environment variable `$BDD_ROOT` which points to your BuDDY installation,
or you can adapt the file `src/cmake_modules/FindBDD.cmake`.

### RiDDLeSim

Prost interacts with a server called RiDDLeSim that simulates an environment 
based on the specification of a .rddl-file. To be able to run this Server you 
need need to install java:

`sudo apt install default-jre` (install Java runtime environment)

## Installation

###RiDDLeSim

To install RiDDLeSim you need to do the following (without any guarantee for success):

 * `cd /path/to/rddl/rddlsim` (switch to directory within this repo)
 * `./compile` (compile rddlsim)

Because of the execution of this repo, it is required that you set an environment
variable `$RDDLSIM_ROOT` that points to /path/to/rddl/rddlsim, e.g. by adding

`export RDDLSIM_ROOT=/path/to/rddl/rddlsim`

to the file ~/.bashrc. It has to point to the base directoy of the rddlsim subdirectory.

### PROST

To finally install PROST, you need to do the following:

 * `cd /path/to/rddl/prost` (switch to directory within this repo)
 * `./build.py` (compile the planner)

## Execution

Both, RiDDLeSim and PROST, can be executed on its own as given in the PROST-README.md 
or by the combined script which is recommended as it is easier for this task.

As we use PROST only as the parser and interacting environment for a learner and 
not as a planner, you only have to give the instance to the Parser. So, the arguments 
are mainly the same as for RiDDLeSim and can be seen in the help of the script `run.py`.

The main arguments you have to provide are :
 * -b: the folder containing the environment and its problem instances (located in prost/testbed/benchmarks)
 * -i: the concrete instance to solve (locaed in the folder given above)

An exemplary prompt to execute the program would be

`./run.py -b ./prost/testbed/benchmarks/elevator-2011/ -i ./prost/testbed/benchmarls/elevator-2011/elevators_inst_mdp__1.rddl`

## Contributors

Roman Joeres

based his work thankfully on the work of the PROST core developers 

 * [Thomas Keller](mailto:tho.keller@unibas.ch?subject=[Prost])
 * [Florian Geißer](mailto:florian.geisser@anu.edu.au?subject=[Prost])
 * [David Speck](mailto:speckd@informatik.uni-freiburg.de?subject=[Prost)

and 

 * Augusto Blaas Correa
 * Dorde Relic
 * Florent Teichteil-Koenigsbuch
 * Jendrik Seipp
 * Joris Scharpff
 * Max Grüner
 * Patrick Eyerich
 * Scott Sanner

and for the RiDDLeSim-Server 

 * Scott Sanner