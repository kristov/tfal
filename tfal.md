# TFAL

The rule of TFAL is: Code is data and data is code.

# TFAL opcodes

All TFAL programs are sets of TFAL opcodes. Each opcode is a set consisting of two items:

* The opcode
* A set that is used as input to opcode processing

# Function calling in TFAL:

Defining functions and calling them are done using several opcode structures:

* Function definition
* Function calls
* Function return

## Function definition:

This opcode defines a new function. The set that defines the function is made up of two items. The first item is a set that stores empty (or pre-initialised) values for funcion arguments, scope storage for local function variables and space allocated for return data. This set will be copied onto the stack and the arg values populated by calling code. The second set is a list of expressions (opcodes) that makes up the body.

    [
      i8:DEFUN
      [
        [
          [arg space]
          [scope space]
          [return space]
        ]
        [body]
      ]
    ]

## Function calls

The first item is a reference to the DEFUN where the function definition can be found. The third item is a list of references to the callers scope where argument values can be copied from into the call stack space.

    [
      i8:CALFUN
      [
        R: function reference
        [references to args]
      ]
    ]

## Function return

This opcode contains a list of references in the function scope where to find data to copy into the return space before restoring the program counter.

    [
      i8:RETURN
      [return space references]
    ]

The return space references are references to arguments, local scope values or global variables.

# Operation of functions

## Compile-time checks

Checks that the editor does to make sure the program is valid:

* Does the function definition comply with the structure of a function?
* Are all empty arg space vars referenced in the function?
* If there is return space, does every return statement correctly fill the return space?
* For every function call the references the function, does it populate arguments correctly?

## The function define function

The DEFUN opcode takes the function definition structure and stores it somewhere. This could be in some global lookup table or a variable.

## The function call function

If calling a function is itself a function, what are the arguments and what is the body?

* Get item 1 and look up the function definition (it should be a reference to a function structure)
* Get item 2 from the function call and copy that set onto stack (flattened?)
* Get item 2 from the function definition (scope space) and copy that to the stack
* Set the instruction pointer to the function body

## The function return function

* Get item 1 and for each ref item copy the value into the return space on the stack

## What is a closure?

A closure is where values from the current scope are copied into the scope space of the function definition when the function definition is created.

# Code within a function

Copying the concept from LLVM, "blocks" of code are used. Each block must end with a change of control (a jump or return).

## If-then-else

This is a block terminator which conditionally jumps to another block.

# Structures

Structures are sets containing types or references to sub-structures. The definition of a structure is a template for further uses of it. When structures are created they are copied from the definition. Internal references to sub-structures are resolved at the time of copy.

A structure containing a 32 bit unsigned integer and a string:

    [
      i32:0
      s:4 test

A structure containing a 16 bit signed integer and a reference to a previous structure:

    [
      I16:0
      R: 0

If that second structure is passed as an argument to a function call, the structure is copied from that definition onto the stack. At the time of the copy the reference is resolved and the copied structure ends up like this:

    [
      I16:0
      [
        i32:0
        s:4 test

Because the first structure contains a populated string it behaves like a default value that can be referenced in the copy. That can also be altered without affecting the original value in the structure definition.

Structure definitions will probably be wrapped in some form of structure that includes the name of the structure and documentation so editors can display those to the user.

## Calling functions with structures

A function definition may contain a structure as one of the arguments. This can be defined using a reference:

    [
      i8:DEFUN
      [
        [
          R: 1
        ]
        []
        [
          i8:0
        ]
      ]
      [
        ...
      ]

This function refers to something else for it's first argument, which for the sake of this example is the second struct from above. It does not use any locally scoped storage (empty set) and it returns an unsigned integer type. Here the function body can refer to any of the elements in the structure as the reference is expanded accordingly.

# Registration of objects in a module

Function definitions, structure definitions (templates) and global variables need to be stored somewhere. A code module is a big set made up of all these things at the root level. This root level set could also contain the name of the module and documentation 
