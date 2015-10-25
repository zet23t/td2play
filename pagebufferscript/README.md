# Page Buffer Script

This is still a very early idea I am working on: A scripting language that's targeting the arduino environment (low memory needs) while trying to do things as efficient as possible for an interpreted language. 

In order to achieve those two goals, I'll try keep things minimally simple:

* Variables are statically typed
* Static program structure
* No runtime type information
* No garbage collection, not even dynamic memory management (at least not now)

The syntax I'm aiming at is strongly inspired by Lua, a scripting language I've worked for years with. 

The compiler is written in Lua as well wheras the interpreter is written in C++. 

