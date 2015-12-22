# TinyDuino 2 Play Lib

This is my 2nd library for the TinyDuino that is specifically targeting
the TinyScreen+ which is a much more potent processor with more RAM and
processing power.

## Project state

This project is still in a very early stage! I'm new to C++ and I make lots of
mistakes. Implementations change and there's right now only one branch (master) - 
which happens to be unstable. Once the core functionality has reached a certain 
level, I intend to work on a develop branch and keeping the master branch stable.
Right now this doesn't make sense.

## Project structure

* ./srclib: The library files (headers) to be included in the arduino files
* ./arduino: The arduino projects to test with
* ./simulator: A simple simulator that enables compilation on PC for testing
* ./screenshots: A collection of screenshots I'm making over the course of development
* ./tools: Tools that may be used during development
* ./pagebufferscript: A scripting language for TD environment - concept only so far

For more information, please read the readme files in the respective directories.

## Requirements

* The simulator depends on GLFW3 which is crossplatform
* Codeblocks project files are provided to compile the simulator and projects on windows - this could be adapted quite easily however
* The image converter is written in HTML/JavaScript and allows converting images to C code quite easily
* Other scripts and tools use Lua in combination with the gd library. You'd need to install a Lua interpreter together with the required libraries in order to run these tools

## Feature list

* (stable) Rendering via command buffer list, which is less memory consuming than offscreen buffering
* (stable) Color depth support for 8bit and 16bit coloring
* (stable) Fixed point math library
* (stable) Textures (must have power of two sizes, can be non-square)
* (stable/not all modes) Textures with transparency mask
* (experimental/not all modes) blend modes for sprite drawings
* (work in progress) Tilemap rendering
* (work in progress) Tiled .tmx import script
* (stable) Javascript image conversion tool

## Regarding tests

There are a few tests around the math libraries that can be compiled and run
quite easily (no fancy frameworks needed, just a bunch of asserts that crash
the test if the outcome is wrong).

Next to that there are some basic testing projects that demonstrate one or two
functionalities that can be compiled and run via simulator or tiny screen. Sadly
I don't know how to automate testing the functionalities in a better way yet.

## Regarding code quality

I deem this project's code quality as low by my own standards:

* Naming of files is inconsistent
* Naming of classes is inconsistent
* No meaningful namespacing
* Bad architecture / organization

Here are my excuses:

* I'm really new to C++
* I'm quite new to Arduino & Embedded programming
* The Arduino IDE (compiler) doesn't allow subdirectories in a project to be used. This makes organization difficult. 
* I have very little time to work out things, let alone denting out previously made decissions.

Please share your thoughts of concern how to improve this. 