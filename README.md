This is the compiled low-level version of Zura.

## Introduction
Zura is a statically typed, compiled, low-level programming language. It is designed to be simple and easy to use. It is inspired by C and Go. It is currently in development and is not ready for production use.

tabe of contents:
- [Installation](#installation)
- [Usage](#usage)
- [Examples of Zura code](sample/SAMPLE.MD)


## Installation
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

Or you can download the latest release from [here](https://github.com/TheDevConnor/Zura-Transpiled/releases/tag/pre-release) and add either the `zura.exe` (For Windows) or `zura` (For Linux) executable to your path.
Eventually, I will add a script to automate this process.

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