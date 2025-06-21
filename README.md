# TUI-Sandbox 

Learn about how Terminal-UI Applications work and how to develop one.

## Analyzer

Intercepts stdin and stdout of a program and prints info about the Escape Expressions. 

Note: Many programs will notice that the input/output does not come directly from a terminal and behave differently / not work at all.  
For example, **less** behaves the same as **cat** in that case; 
**Vim** on the other hand still works as normal, it only gives a warning.

*Usage:*  
''' 
build/bin/analyzer -o <file> <program> <program args>
'''

While the analyzer prints to a file, we can also print directly to another terminal by passing the **tty** output (which will be /dev/pts/XX) of that terminal as the file argument. This also works with **tmux**, so we can have two terminals open next to each other, where one executes the program and we see the analyzer output in the other one.

(TODO screenshot)

*Examples:*  
''' 
build/bin/analyzer -o /dev/pts/1 vim file.txt 
build/bin/analyzer -o analyzer_output.txt build/bin/samples_ncurses/basic
'''


## Raw-TUI

Simple framework that uses ANSI Escape Expressions to control the terminal, to write own simple TUI applications. 



## Build  

# Repository
