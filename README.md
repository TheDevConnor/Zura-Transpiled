# `Zura`: a low-level compiled alternative to `C` and `C++`

<p align="center">
  <a href="#why"> Why? </a> |
  <a href="#language-goals"> Goals </a> |
  <a href="#Project-Status"> Status </a> |
  <a href="#Getting-Started"> Getting Started </a> |
  <a href="#usage"> Usage </a> |
  <a href="#join-us"> Join Us </a>
</p>

## Introduction

Zura is a statically typed, compiled, low-level programming language. It is designed to be simple and easy to use. It is inspired by C and Go. It is currently in development and is not ready for production use.

Feel free to go take a look at the documentation to see how to use the language!
[Documentation](sample/SAMPLE.md)

## Why

## Language Goals

## Project Status

Zura Language is currently an experimental project. There is no working
compiler or toolchain. The demo interpreter for Zura is coming soon

I want to better understand whether I can build a language that meets a
successor language criteria, and whether the resulting language can gather a
critical mass of interest within the larger C++ industry and community.

Currently, I have fleshed out several core aspects of both Zura the project
and the language:

- The strategy of the Zura Language and project.
- An open-source project structure, governance model, and evolution process.

## Getting Started

To start make sure you have the following installed:

- [cmake](https://cmake.org/)
- [make](https://www.gnu.org/software/make/)
- [gcc](https://gcc.gnu.org/)
- [ninja](https://ninja-build.org/)

#### Windows

Commands to install the above packages on Windows:

```powershell
# Install chocolatey
> Set-ExecutionPolicy Bypass -Scope Process -Force; `
  iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
# Install packages
> choco install cmake make mingw ninja
```

#### Linux

Commands to install the above packages on Ubuntu and Debian:

```console
sudo apt-get install cmake make gcc ninja-build valgrind
```

For Arch Linux:

```console
sudo pacman -S cmake make gcc ninja valgrind 
```

##### Building

Now if you want to build zura for the debug mode do

```console
mkdir debug
cmake -G Ninja -DCMAKE_BUILD_TYPE=debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B debug -S .
ninja -C debug
```

and now the exacutable will be available for you to use to debug with

Or if you want the release version do this

```console
mkdir release
cmake -G Ninja -DCMAKE_BUILD_TYPE=release -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B release -S .
ninja -C release
```

This will create a `zura` executable in the `build` directory.

Or you can alternativly use the build.sh file to do this for you

```console
chmod +x build.sh
./build.sh debug or ./build.sh release
```

Or you can download the latest release from [here](https://github.com/TheDevConnor/Zura-Transpiled/releases/tag/pre-release) and add either the `zura.exe` (For Windows) or `zura` (For Linux) executable to your path.
Eventually, I will add a script to automate this process.

You can also copy the 'installer.sh' file and run it to install the latest release of Zura to your system.
Instead of building the project yourself.

```console
chmod +x installer.sh
./installer.sh
```

<!-- sample/SAMPLE.md -->
Now feel free to go take a look at the documentation to see how to use the language.
[Documentation](sample/SAMPLE.md)

## Usage

```console
./build/zura <filename> -o <output name>
```

if you wish to save the c code to a file, use the `-s` flag:

```console
./build/zura <filename> -o <output name> -s
```

if you want to clean up the build directory, use the `-c` flag:

```console
./build/zura -c <exactable name>
```

Then run the output file with:

```console
./<output name>
```

### Command that you can run

```console
zura --help
```

This command will show you all of the commands that you can run in the zura compiler.

## Join Us

[Zura Website](https://zuralang.co/)
