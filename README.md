# vm_test
Here is an attempt to write your own virtual machine.

    /compiler - compiler of a Go-like language
    /vm - VM implementation for debugging.
    example.ss - example programm

## Go-like language (currently supported)
    
### Arithmetic Operators
	+
	-
	*
	/
    
### Relational Operators
	==
	!=
	>
	<
	>=
	<=    
    
### Logical Operators
	&&
	||
	!
    
### Definition of variables
	CONST
	BYTE - only array of bytes
	VAR - general type of variable (fixed point)
	
### Functions
	FUNC - definition
    RETURN
 
### Other keywords
	IF
	GOTO
	WHILE
	BREAK - break loop
	BRKPNT - set break point

### Built-in procedures
	PRINT
	PRINT_LN - with line feed
	
### Built-in functions
    IN - Read data from port. Params: port number
    RAND - Get random number, from 0 to top. Params: top
    INKEY - Get keypress. Params: wait (sec), (<=0) for no wait
    LEN - Get array (string) size. Params: pointer to array
    SCR_W - Get screen width.
    SCR_H - Get screen height.
    PUTS - Print . Params: pointer to array
    PUTN - Print number. Params: number
    PUTI - Print integer part of number. Params: number
    BEEP - Beep. Params: length, frequency
    OUT - Write data to port. Params: port number, data
    CLS - Clear of screen
    CURS - Set cursor position. Params: x, y
