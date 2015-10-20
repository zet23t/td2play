# Simulator

The simulator project should enable you to iterate and test your code faster.
It provides certain basic libraries that are used by the tiny screen library 
and the arduino projects are compiled against these.

It's using GLFW 3 for displaying the TinyScreen's content. 

The code is in a very very shameful state as it grew from a prove of concept 
codebase. As it already does everything that's needed, I am not sure if that'll ever change.

## Approach

The simulator mimics certain aspects of the TinyDuino Screen+ library and 
some other parts that come with Arduino. 

In order to compile an arduino project inside the ../arduino directory, a 
wrapper file must be created in the arduino_wrapper directory. I'd recommend
to maintain the same structure and file naming there as in the ../arduino 
directory (see examples).