This is the compiled low-level version of Zura.

To start make sure you have the following installed:
- [cmake](https://cmake.org/)
- [make](https://www.gnu.org/software/make/)
- [gcc](https://gcc.gnu.org/)
- [gcc-multilib](https://packages.ubuntu.com/jammy/gcc-multilib)

Commands to install the above packages on Windows:
```bash
# Install chocolatey
Set-ExecutionPolicy Bypass -Scope Process -Force; `
  iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))

# Install packages
choco install cmake make mingw
```

Commands to install the above packages on Ubuntu and Debian: 
```bash
sudo apt install cmake make gcc gcc-multilib
```
For Arch Linux:
```bash
sudo pacman -S cmake make gcc gcc-multilib
```

Run the following commands if on Windows:
```bash
mkdir build
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

Run the following commands if on Linux:
```bash
mkdir build
cmake -S . -B build
cmake --build build
```

This will create a `zura` executable in the `build` directory.

## Usage
```bash
./build/zura <filename> -o <output name>
```

if you wish to save the c code to a file, use the `-s` flag:
```bash
./build/zura <filename> -o <output name> -s
```

if you want to clean up the build directory, use the `-c` flag:
```bash
./build/zura -c <exactable name>
```

Then run the output file with:
```bash
./<output name>
```

## Examples of Zura code
```bash
pkg main;

x int8 := 10;
fn main() -> int8 {
    info "age: %d", x;
    exit 0;
}
```
As you can see, Zura is a statically typed language. The type of a variable is specified after the variable name. The `:=` operator is used to assign a value to a variable. The `info` function is used to print to the console. The `exit` function is used to exit the program. The `main` function is the entry point of the program. The `->` operator is used to specify the return type of a function.

```bash
pkg main;

include "std";

fn main() -> int {
    x []int8 := [1, 5, 8, 10];

    loop (i int8 := 0; i < length(x)) : (i++) {
        info "%d\n", x[i];
    }

    exit 0;
}
```
This is an example of looping through an array and printing each element. The `include` keyword is used to include a file. The `loop` keyword is used to create a loop. The `length` function is used to get the length of an array. The `[]` operator is used to access an element in an array.