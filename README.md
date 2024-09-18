# Texted
This is a text editor (hence the name) that is aimed to read and edit large text files easier – inspired from a fustration of using Vim to 
open and edit multi gigabtye files. Editing text requires insertion and deletion of data at arbitary offsets in the file – a task that 
file I/O is unsutited for. This leads to the design decision of loading the whole file in to memory using a linked-list like structure, causing
the memory usage to be atleast as large as a the size of the file file being opend. This projects aims to overcome this requirement by dynamicaly loading
portions of a file into memory based on if data from that file portion is being used or not. 

This project should run anywhere POSIX system calls and a terminal emulator that supports xterm control seqeunces. Because of the use of 
the `<stdatomic.h>` header file for atomic operations, it requires C11 to compile. 

### Status
I currently have the design of the high-level abstractions for various parts finished and have the unit tests that cover them. 
Current work is focused on two branches. One branch on making texted function as a text editor, and another working on the 
backend store that dynamically loads and saves data from memory. 
