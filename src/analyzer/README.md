# Programs

Recommended to use: use\_tee\_cmd

- **use\_tee\_cmd**:   Executes  *tee <file> | <program> | tee <file>* with /bin/sh within a child of the process

- **use\_tee\_cmd\_manual**:   User must paste  *tee <file> | <program> | tee <file>* command manually in a second terminal. Inconvenient to use, was just created for initial test purposes.

- **use\_pipe\_function**: Uses pipe() to forward stdin and stdout to and from the program, which is executed as a child within the process.


Note: The interception only works with programs who don't check if they are connected directly to a terminal; 
e.g. when the program looks up the terminal size it will file. Maybe this is possible e.g. the same way as ssh does
