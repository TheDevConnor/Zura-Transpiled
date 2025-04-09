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
9. [Templates](#templates)
10. [Casting](#casting)
11. [Pointers](#pointers)
12. [Alloc and Free](#memory)
13. [Built-in Functions](#built-in-functions)
14. [Debugging Zura](#debug-mode)

## Basic Syntax

Zura uses a unique combination of operators and control structures for defining variables, functions, and loops. All Zura programs must define a `main` function which serves as the entry point.

```cpp
const main := fn () int {
   return 0;
};
```

## Data Types

Zura supports the following data types:

- `int`: 32-bit signed integer
- `float`: 32-bit floating-point number
- `bool`: 8-bit boolean value (Technically 1-bit types cannot exist.)
- `string`: 64-bit char* (pointer to the first character of the string.)
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
   @output(i, "\n");
}

# C while loop
#    iterator  cond     (no post-loop operator)
loop (i := 0; i < 10) {
   @output(i, "\n");
}
```

You are allowed to have a post-loop operator in the loop syntax without the inline variable declaration.

```cpp
have x: int = 0;
loop (x < 10) : (x++) {
   @output(x, "\n");
}
```

The `if` and `else` statements are similar to other languages.

```cpp
have x: int = 10;
if (x > 10) {
   @output("x > 10\n");
} else {
   @output("x <= 10\n");
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

There is more to come like; push, pop, and other array operations.

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

@output("Point: (", p.x, ", ", p.y, ")\n");
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
@output("Color: ", c, "\n");
```

Each member of the enum is treated as the C-like equivalent of a `unsigned long`.

## Templates

Zura supports templates which allow you to define generic functions and data structures.
These are very similar to C++ templates.

```cpp
@template <typename T>
const swap := fn (a: T, b: T) T {
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

   @output("Float values: 'fx=", fx, "' and 'fy=", fy, "'\n");

   have avg: float = calculate_average(fx, fy);
   @output("Average of float values: 'avg=", avg, "'\n");

   return 0;
};
```

## Built-in Functions

Zura provides a set of built-in functions that do not require the importing of standard libraries, like the most common ones that handle I/O.

```cpp
have x: int = 10;
@output("Value of x: ", x, "\n");

have s: string = "Hello, World!";
@output("String value: ", s, "\n");

have y: int = @input<int>("Enter a number: ");
@output("You entered: ", y, "\n");

have x: int = @cast<int>(10.5);
@output("Casting float to int: ", x, "\n");

have z: []char;
@output("Enter a string: ");
@input(z, 10); # z is the value that the input will be stored in; 10 is the maximum number of bytes to write into the buffer
@output("You entered: ", z, "\n");
```

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

Docs on `@alloc` and `@free` are coming soon!

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
