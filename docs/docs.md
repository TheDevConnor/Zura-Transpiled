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
  @outputln(1, "Hello, World!");
  return 0;
};
```

## Data Types

Zura supports the following data types:

- `int?`: 64-bit signed integer (9,223,372,036,854,775,807)
- `int!`: 64-bit unsigned integer (18,446,744,073,709,551,615)
- `float`: 64-bit floating-point number
- `bool`: 8-bit (1 byte) boolean (true or false)
- `str`: 64-bit char* (pointer to the first character of the string.)
- `char`: 8-bit signed integer
- `void`: Represents no value or a null pointer.
- `nil`: Represents a null pointer (equivalent to `nullptr` in C++ or `NULL` in C).

## Variables

Variables in Zura are defined using the `have` keyword. Variables can be assigned a value at the time of declaration or later in the program.

```cpp
have x: int! = 10;
have y: int!;
y = 20;
```

## Functions

Functions in Zura are defined using the `fn` keyword. Functions can take arguments and return values.

```cpp
const add := fn (x: int!, y: int!) int! {
   return x + y;
};
```

## Loops and Conditionals

Zura supports the `if`, `else`, `while`, and `for` control structures that you may see in C-style languages. In Zura, however, we do not use `for` and `while`, but use the `loop` syntax with an optional iterator.

```cpp
#    iterator  cond    postfix
loop (i = 0; i < 10) : (i++) {
   @output(1, i, "\n");
}

#    iterator  cond     (no post-loop operator)
loop (i = 0; i < 10) {
   @output(1, i, "\n");
   i++; # You must increment the iterator manually; or else you will have an infinite loop.
}
```

You are allowed to have a post-loop operator in the loop syntax without the inline variable declaration. Aka a `while` loop.

```cpp
have x: int! = 0;
loop (x < 10) : (x++) {
   @output(1, x, "\n");
}
```

Which can also be written as:

```cpp
have x: int! = 0;
loop (x < 10) {
   @output(1, x, "\n");
   x++;
}
```

The `break` and `continue` statements are also supported in Zura.

```cpp
loop (i = 0; i < 10) : (i++) {
   if (i == 5) {
      continue; # Skip the rest of the loop when i is 5.
   }
   if (i == 8) {
      break; # Exit the loop when i is 8.
   }
   @output(1, i, "\n");
}
```

The `if` and `else` statements are similar to other languages.

```cpp
have x: int! = 10;
if (x > 10) {
   @output(1, "x > 10\n");
} else {
   @output(1, "x <= 10\n");
}
```

## Arrays
Zura supports arrays which are fixed-size collections of elements of the same type. Arrays are defined using square brackets `[]`.

```cpp
have arr: [5]int! = [1, 2, 3, 4, 5];
```

You can also define arrays with a specific size and auto initialize them to zero.

```cpp
have arr: [5]int! = [0];
```

You can access elements of an array using the square bracket operator `[]`.

```cpp
have x: int! = arr[0];
```

## Structures
Zura supports structures which allow you to define custom data types. Similar to C-family languages, structs are simply a way to manage multiple variables in one easy-to-access place.

```cpp
const Point := struct {
   have x: int!;
   have y: int!;
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
```

When dereferencing a pointer, you use the `&` operator on the right-hand side to access the value at the address stored in the pointer.

```cpp
j& = 100; # Here we are dereferencing j and giving it a new value of 100.
@output(1, "Value of i: ", i, "\n"); # This will output "Value of i: 100"
```

Pointers are also allowed to be the members of a struct.

```cpp
const Point := struct {
  x: int!,
  y: int!
};

const Ray := struct {
  endpoint: *Point,
  angle: int!
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
Zura provides built-in functions for memory management, such as `@alloc`, `@free`, and `@memcpy`.

The `@alloc` will allocate a piece of memory of the given size (in bytes) and return the pointer to it. The @alloc function returns a void pointer that can later be type-casted for use of any type. Additionally, if the allocation fails, the returned pointer will be `nil` (0).
```cpp
have buffer: *char = @cast<*char>(@alloc(100)); # Allocates 100 bytes of memory for a char buffer.
if (buffer == nil) {
    @outputln(1, "Memory allocation failed");
}
@output(1, "Buffer allocated at address: ", buffer, "\n");
```

The `@free` function will free the memory allocated by `@alloc`. It takes a pointer to the memory and the size of the memory to be freed.

```cpp
@free(buffer, 100); # Free the memory allocated for the buffer.
```

`@memcpy` is used to copy a specified number of bytes from one memory location to another. It is useful for copying or transferring data from one buffer or array to the next. The function takes three arguments: the source pointer, the destination pointer, and the number of bytes to copy.
> [!WARNING]
> It is very important that the destination has been fully allocated and has enough space to hold the copied data.
> If you did not do this, then the program will not work as intended, or, more likely, crash altogether.
> Additionally, the type of the pointers used does not matter, whether they match or differ. A byte-for-byte copy from the source to the destination will be performed.

```cpp
have source: *char = "Hello, World!";
have des: *char =@cast<*char>(@alloc(50)); # Allocate 50 bytes for the destination buffer.
if (des == nil) {
    @outputln(1, "Memory allocation failed for destination buffer");
}
@memcpy(source, des, 13); # Copy 13 bytes from source to destination
@output(1, "Copied string: ", des, "\n");
@free(des, 50); # Free the memory allocated for the destination buffer.
```

## Templates 
Zura does not yet support templates.

## Casting
Zura supports casting between different types using the `cast` keyword. This is a "functional" cast rather than a static cast like in C++. This will convert between data types rather than simply changing the type associated with the bytes.

```cpp
have x: float = 10.5;
have y: int = @cast<int!>(x);
```

Here is a more complex example:

```cpp
const calculate_average := fn (a: float, b: float) float {
   return (a + b) / 2.0;
};

const main := fn () int! { 
   have x: int! = 10;
   have y: int! = 20;

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
Zura provides a set of built-in functions that do not require the importing of standard libraries. Here is a list of the built-in functions:

`@import ""` - (str) - Imports another Zura file and makes its functions available in the current file.

`@output` - (int, ...) - Outputs the given arguments to the console. The first argument is the file descriptor (1 for stdout, 2 for stderr).

`@outputln` - (int, ...) - Similar to `@output`, but adds a newline at the end of the output.

`@input` - (int, *void, int) - Reads input from the console. The first argument is the file descriptor (0 for stdin), the second argument is a pointer to the buffer where the input will be stored, and the third argument is the size of the buffer.

`@alloc` - (int) - Allocates a block of memory of the specified size in bytes and returns a pointer to it.

`@free` - (*void, int) - Frees the memory allocated by `@alloc`.

`@memcpy` - (*void, *void, int) - Copies a specified number of bytes from one memory location to another.

`@sizeof` - (type) - Returns the size of the specified type in bytes.

`@cast<>()` - <type>(value) - Casts the value to the specified type.

`@getArgc` - () - Returns the number of command-line arguments passed to the program.

`@getArgv` - () - Returns a pointer to an array of strings containing the command-line arguments.

`@open` - (path, (3 bools)(read, write, create)) This function takes in a path, either relative or absolute, and returns a file descriptor opened from that path. It is useful for opening files used in I/O. However, it is not designed for use in sockets. That is the job of the `@socket` function. On an error or unsuccessful opening, the function returns a negative number (USUALLY -1, but can sometimes be different...)

`@close` - (fd) - Closes the file descriptor specified by `fd`. This is useful for cleaning up resources after you are done using a file.

`@streq` - (str1, str2) - Compares two strings for equality. Returns `true` if they are equal, `false` otherwise.

## Cmd-line-args
Zura provides built-in functions to access command-line arguments passed to the program.

Here is an example of how to retrieve and use them:
```cpp
const main := fn () int! {
  have argc: int! = @getArgc();    # Get the number of args
  have argv: *[]str = @getArgv();  # Pointer to array of str

  @outputln(1, "Number of arguments: ", argc);
  loop (i=0; i < argc) : (i++) {
    @outputln(1, argv&[i]);
  }
  
  # Note: The `&` operator is used to dereference the pointer to the array of strings.
  # This allows us to access each argument as a string.
  # The `argv&[i]` syntax is used to access the i-th argument
  return 0;
};
```
This code defines a variable that stores the pointer to the array of arguments and the argument count.

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
