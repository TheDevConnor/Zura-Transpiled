# Zura Programming Language

Zura is a statically typed language that blends familiar syntax with modern constructs such as templates, type casting, and high-level control structures. This documentation serves as an introduction to the syntax, constructs, and idioms of Zura.

## Table of Contents

1. [Basic Syntax](#basic-syntax)
2. [Data Types](#data-types)
3. [Variables](#variables)
4. [Functions](#functions)
5. [Loops and Conditionals](#loops-and-conditionals)
6. [Structures](#structures)
7. [Enums](#enums)
8. [Templates](#templates)
9. [Casting](#casting)
10. [Built-in Functions](#built-in-functions)
11. [Debugging Zura](#debug-mode)
<!-- 12. [Pointers](#pointer) -->

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
- `float`: 64-bit floating-point number
- `bool`: Boolean value
- `string`: String value
- `char`: Character value (Not implemented yet)

```cpp
have x: int = 10;
have y: float = 10.5;
have z: bool = true;
have s: str = "Hello, World!";
have c: char = 'A';
```

## Variables

Variables in Zura are defined using the `have` keyword. Variables can be assigned a value at the time of declaration or later in the program.

```cpp
have x: int = 10;
have y: int;
y = 20;
```

You can also use the `auto` keyword to let the compiler infer the type of the variable.

```cpp
auto z = 30;
```

where `z` is inferred to be of type `int`.

## Functions

Functions in Zura are defined using the `fn` keyword. Functions can take arguments and return values.

```cpp
const add := fn (x: int, y: int) int {
   return x + y;
};
```

## Loops and Conditionals

Zura supports `if`, `else`, `while`, and `for` control structures, but the difference is that we use the `loop` keyword instead of `for` and `while`.

```cpp
# for loop
loop (i := 0; i < 10) : (i++) {
   dis(i, "\n");
}

# while loop
loop (i := 0; i < 10) {
   dis(i, "\n");
}
```

You can also have optional conditions in the while loop.

```cpp
have x: int = 0;
loop (x < 10) : (x++) {
   dis(x, "\n");
}
```

The `if` and `else` statements are similar to other languages.

```cpp
have x: int = 10;
if (x > 10) {
   dis("x is greater than 10\n");
} else {
   dis("x is less than or equal to 10\n");
}
```

## Structures

Zura supports structures which allow you to define custom data types.

```cpp
const Point := struct {
   have x: int;
   have y: int;
};

have p: Point;

p.x = 10;
p.y = 20;

dis("Point: (", p.x, ", ", p.y, ")\n");
```

## Enums

Zura supports enums which allow you to define a set of named constants.

```cpp
const Color := enum {
   Red,
   Green,
   Blue
};

have c: Color = Color.Red;
dis("Color: ", c, "\n");
```

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

Zura supports casting between different types using the `cast` keyword.
The syntax is `@cast<type>(value)`.

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

   dis("Integer values: 'x=", x, "' and 'y=", y, "'\n");

   have fx: float = @cast<float>(x);
   have fy: float = @cast<float>(y);

   dis("Float values: 'fx=", fx, "' and 'fy=", fy, "'\n");

   have avg: float = calculate_average(fx, fy);
   dis("Average of float values: 'avg=", avg, "'\n");

   return 0;
};
```

## Built-in Functions

Zura provides a set of built-in functions for common operations such as input/output, string manipulation, and math functions.

```cpp
have x: int = 10;
dis("Value of x: ", x, "\n");

have s: string = "Hello, World!";
dis("String value: ", s, "\n");

have y: int = @input<int>("Enter a number: ");
dis("You entered: ", y, "\n");

have x: int = @cast<int>(10.5);
dis("Casting float to int: ", x, "\n");
```

## Debug mode

You can add the `-debug` flag to the Zura binary to compile your code in debug mode. This adds debugging information (like line/column locations and expression watching) to the output assembly/executable. You can try it out GCC. (For the nerds, this uses DWARF v5.)
