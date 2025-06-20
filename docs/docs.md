# Zura Programming Language

Zura is a statically typed language that blends familiar syntax with modern constructs such as templates, type casting, and high-level control structures. This documentation serves as an introduction to the syntax, constructs, and idioms of Zura.

## Table of Contents

1. [Basic Syntax](#basic-syntax)
2. [Data Types](#data-types)
3. [Variables](#variables)
4. [Functions](#functions)
5. [Loops and Conditionals](#loops-and-conditionals)
6. [Arrays](#arrays)
7. [Structures](#structures)
8. [Enums](#enums)
9. [Pointers](#pointers)
10. [Alloc, Free, Dereference](#memory)
11. [Templates](#templates)
12. [Casting](#casting)
13. [Built-in Functions](#built-in-functions)
14. [Getting CMDLine Args from the user](#Cmd-line-args) 
15. [Debugging Zura](#debug-mode)

## Basic Syntax

Zura uses a unique combination of operators and control structures for defining variables, functions, and loops. All Zura programs must define a `main` function which serves as the entry point.

```cpp
const main := fn () int! {
   return 0;
};
```

## Data Types

Zura supports the following data types:

- `int?`: 32-bit signed integer
- `int!`: 32-bit unsigned integer
- `float`: 32-bit floating-point number
- `bool`: 8-bit boolean value (Technically 1-bit types cannot exist.)
- `str`: 64-bit char* (pointer to the first character of the string.)
- `char`: 8-bit signed integer

```cpp
have x: int = 10;     # 0b00000000000000000000000000001010
have y: float = 10.5; # 0b01000001001010000000000000000000
have z: bool = true;  # 0b00000001

# (because this is a pointer, the binary value is system-dependent)
have s: str = "Hello, World!";
have c: char = 'A';   # 0b0101001
```

## Variables

Variables in Zura are defined using the `have` keyword. Variables can be assigned a value at the time of declaration or later in the program.

```cpp
have x: int = 10;
have y: int;
y = 20;
```

You can also use the `auto` keyword to let the compiler infer the type of the variable based on the type of its value.

```cpp
auto z = 30;
```

`z` is inferred to be of type `int`.

**NOTE**:
Please try to avoid using the `auto` keyword simply to avoid writing out types in your code. Use it responsibly!

## Functions

Functions in Zura are defined using the `fn` keyword. Functions can take arguments and return values.

```cpp
const add := fn (x: int, y: int) int {
   return x + y;
};
```

## Loops and Conditionals

Zura supports the `if`, `else`, `while`, and `for` control structures that you may see in C-style languages. In Zura, however, we do not use `for` and `while`, but use the `loop` syntax with an optional iterator.

```cpp
# C for loop
#    iterator  cond    postfix
loop (i := 0; i < 10) : (i++) {
   @output(1, i, "\n");
}

# C while loop
#    iterator  cond     (no post-loop operator)
loop (i := 0; i < 10) {
   @output(1, i, "\n");
}
```

You are allowed to have a post-loop operator in the loop syntax without the inline variable declaration.

```cpp
have x: int = 0;
loop (x < 10) : (x++) {
   @output(1, x, "\n");
}
```

The `if` and `else` statements are similar to other languages.

```cpp
have x: int = 10;
if (x > 10) {
   @output(1, "x > 10\n");
} else {
   @output(1, "x <= 10\n");
}
```

## Arrays

Zura supports arrays which are fixed-size collections of elements of the same type. Arrays are defined using square brackets `[]`.

```cpp
have arr: [5]int = [1, 2, 3, 4, 5];
```

You can also define arrays with a specific size and fill them with a default value.

```cpp
have arr: [5]int = [0];
```

You can access elements of an array using the square bracket operator `[]`.

```cpp
have x: int = arr[0];
```

## Structures

Zura supports structures which allow you to define custom data types. Similar to C-family languages, structs are simply a way to manage multiple variables in one easy-to-access place.

```cpp
const Point := struct {
   have x: int;
   have y: int;
};

have p: Point;

p.x = 10;
p.y = 20;

@output(1, "Point: (", p.x, ", ", p.y, ")\n");
```

Structs can also be defined in one big expression, rather than defining each member individually like the example above.

```cpp
have p: Point = {
  x: 10;
  y: 20;
}
```

This is functionally identical to first example.

## Enums

Zura supports enums which allow you to define a set of named constants.

```cpp
const Color := enum {
   Red,   # 1
   Green, # 2
   Blue   # 3
};

have c: Color = Color.Red;
@output(1, "Color: ", c, "\n");
```

Each member of the enum is treated as the C-like equivalent of a `unsigned long` and is auto-incrementing.

## Pointers

In Zura, we try to avoid the use of pointer arithmetic and other fancy operations as those often lead to segmentation faults and headaches (speaking from experience. This compiler was written in C++!).

The data type of a pointer is defined with an asterisk `*`. A point-to operator that returns the address of the right-hand side is done with the `&` character.

```cpp
have i: int = 43;
have j: *int = &i; # Contains the address of i.

# Goes the address stored in j and sets that to 3.
# The address stored in j (&i) is not affected.
*j = 3;
```

When dereferencing a struct pointer, we use the same `.` operator that we would for regular structs, unlike the `->` in C-family languages.

```cpp
const Point := struct {
  x: int;
  y: int;
};

have p: Point = {
  x: 10,
  y: 20
};

have pP: *Point = &p;
pP.x = 35; # Sets p.x = 35.
```

Pointers are also allowed to be the members of a struct.

```cpp
const Ray := struct {
  endpoint: *Point;
  angle: int;
};

have p: Point = {
  x: 10,
  y: 20
};

have r: Ray = {
  endpoint: &p, # The memory address of p.
  angle: 180;
};
```

## Memory

```cpp
const main := fn () int! {
  have nums: *int = @alloc(1);

  nums& = 42;
  @outputln(1, "num: ", nums&);

  @free(nums, 1);
  return 0;
};
```
Write docs for this.

## Templates

Zura supports templates which allow you to define generic functions and data structures.
These are very similar to rust style inline generics.

```cpp
const swap := fn <T> (a: T, b: T) T {
   have temp: T = a;
   a = b;
   b = temp;

   return a;
};
```

## Casting

Zura supports casting between different types using the `cast` keyword. This is a "functional" cast rather than a static cast like in C++. This will convert between data types rather than simply changing the type associated with the bytes.

```cpp
have x: float = 10.5;
have y: int = @cast<int>(x);
```

Here is a more complex example:

```cpp
const calculate_average := fn (a: float, b: float) float {
   return (a + b) / 2.0;
};

const main := fn () int { 
   have x: int = 10;
   have y: int = 20;

   @output("Integer values: 'x=", x, "' and 'y=", y, "'\n");

   have fx: float = @cast<float>(x);
   have fy: float = @cast<float>(y);

   @output(1, "Float values: 'fx=", fx, "' and 'fy=", fy, "'\n");

   have avg: float = calculate_average(fx, fy);
   @output(1, "Average of float values: 'avg=", avg, "'\n");

   return 0;
};
```

## Built-in Functions

Zura provides a set of built-in functions that do not require the importing of standard libraries, like the most common ones that handle I/O.
```cpp
# Outputing text to a terminal
have x: int! = 10;
@output(1, "Value of x: ", x, "\n");
# Where ouput takes in the file descriptor (1 || 0) then the args that you want to pass in
# Where 0 is for read in and 1 is for read out.
# You also have the option of having zura handle the '\0\n' when outputing.
@outputln(1, "Value of x: ", x);
```

```cpp
# Importing a file is pretty straight forward
@import "path_to_file";
```

Add in docs for @open, @close, @input

## Cmd-line-args
Zura provides built-in functions to access command-line arguments passed to the program.

Here is an example of how to retrieve and use them:
```cpp
const ArgsList := struct {
  argv: *[]str,
  size: int!,
};

const getCMDArgs := fn () ArgsList {
  have argc: int! = @getArgc();    # Get the number of args
  have argv: *[]str = @getArgv();  # Pointer to array of str

  have argsList: ArgsList = {
    argv: argv, 
    size: argc, 
  }; 

  return argsList;
};

const main := fn () int! {
  have args: ArgsList = getCMDArgs();
  
  @outputln(1, "Number of arguments: ", args.size);
  loop (i=0; i < args.size) : (i++) {
    @outputln(1, args.argv&[i]);
  }
  
  return 0;
};
```
This code defines a simple structure ArgsList that stores the pointer to the array of arguments and the argument count. The function `getCMDArgs()` 
retrieves this information using the built-in `@getArgc()` and `@getArgv()` functions. The main function then prints out the argument count and each argument.

## Debug mode

You can add the `-debug` flag to the Zura binary to compile your code in debug mode. This adds debugging information (like line/column locations and expression watching) to the output assembly/executable. You can try it out in GDB. (For the nerds, this uses DWARF v5.)

```cpp
1. const main := fn () int {
2.   have i: int = 45;
4.   # gdb: "Old value = 0, New value = 45"
3.   i = 3;
5.   # gdb: "Old value = 45, New value = 3"
6.   return i;
7. };
```
