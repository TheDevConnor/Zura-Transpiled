Examples for Zura!

## Table of Contents
- [Hello World](#hello-world)
- [Comments](#comments)
- [Binary Operators](#binary-operators)
- [Variables](#variables)
- [Functions](#functions)
    - [Nested Functions](#nested-functions)

## Hello World
```zura
fn main(argc: i8, argv: str[]) <i8> {
    info "Hello World!";
    exit 0;
}
```
BOOM! You just wrote your first Zura program! Now let's break it down.

```zura
fn main(argc: i8, argv: str[]) <i8> {
```
This marks the main function, a mandatory element in every Zura program. The main function must yield an integer and accepts two arguments: argc, denoting the argument count, and argv, an array of strings containing program arguments.

```zura
    info "Hello World!";
```
This line calls the info function, printing the supplied string to the console.

```zura
    exit 0;
```
Here, the exit function concludes the program with the provided exit code.

```zura
}
```
This denotes the conclusion of the main function, a standard requirement for all functions in Zura.

## Comments
```zura
fn main(argc: i8, argv: str[]) <i8> {
    // This is a single line comment
    /*
        :( Not supported yet
    */
    info "Hello World!";
    exit 0;
}
```
Comments are very simple in Zura. Single line comments are denoted by two forward slashes, and multiline comments are denoted by a forward slash followed by an asterisk, and concluded by an asterisk followed by a forward slash. Nothing more to it.

## Binary Operators
In Zura there are 6 binary operators: +, -, *, /, %, and ^ by default. These operators are used to perform arithmetic operations on numbers. Here is an examples:
```zura
fn main(argc: i8, argv: str[]) <i8> {
    have x: i8 = 1 + (2 * 3) / 4 % 5; // Which returns 2
    info "%d", x;
    exit 0;
}
```

The '^' is not supported yet. It will be added in the future.

## Variables
```zura
fn main(argc: i8, argv: str[]) <i8> {
    have x: i8 = 0; // staticly typed variable
    auto y := 0;     // dynamically typed variable
    info "%d + %d = %d", x, y, x + y;
    exit 0;
}
```
This program outputs 0 + 0 = 0 to the console. Let's delve into it.

```zura
    have x: i8 = 0; // staticly typed variable
```
Here, have introduces a variable declaration. x is the variable name, i8 denotes its type, and = 0 assigns an initial value. Also node 
that here we are using a static type, meaning that the type of the variable is known at compile time. Because of this, we can use the have keyword to declare the variable.

```zura
    auto y := 0;     // dynamically typed variable
```
Whereas the previous variable was statically typed, this one is dynamically typed. This means that the type of the variable is not known at compile time. Because of this, we must use the auto keyword to declare the variable. Meaning that the type of the variable is inferred from the value assigned to it.

```zura
    info "%d + %d = %d", x, y, x + y;
```
A call to the info function outputs formatted text, showcasing the values of x, y, and their sum.

The rest of the program is the same as the previous example. With the exit function concluding the program. And the closing brace denoting the end of the main function.

## Functions
```zura
fn add(x: i8, y: i8) <i8> {
    return x + y;
}

fn main(argc: i8, argv: str[]) <i8> {
    info "%d + %d = %d", 1, 2, add(1, 2);
    exit 0;
}
```
This simple example introduces a function, akin to prior examples, but with a return type and statement.

```zura
fn add(x: i8, y: i8) <i8> {
    return x + y;
}
```
This declares the add function, specifying its name, arguments, return type, and body containing a return statement. To write a function, we must first use the fn keyword, followed by the function name, then the arguments, then the return type, and finally the body of the function. The body of the function is denoted by the opening and closing braces. The return statement returns the value of the expression following it. In this case, the sum of x and y.

```zura
fn main(argc: i8, argv: str[]) <i8> {
    info "%d + %d = %d", 1, 2, add(1, 2);
    exit 0;
}
```
This declares the add function, specifying its name, arguments, return type, and body containing a return statement. Except this time the print function is being called, showcasing the sum of 1 and 2. The rest of the program is the same as the previous example.

### Nested Functions
```zura
fn main(argc: i8, argv: str[]) <i8> {
    fn add(x: i8, y: i8) <i8> {
        return x + y;
    }
    info "%d + %d = %d", 1, 2, add(1, 2);
    exit 0;
}
```
This snippet showcases nested functions, functions declared within other functions.