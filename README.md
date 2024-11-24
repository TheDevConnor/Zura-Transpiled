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

## Why

C++ remains the dominant programming language for performance-critical software,
with massive and growing codebases and investments. However, it is struggling to
improve and meet developers' needs, as outlined above, in no small part due to
accumulating decades of technical debt. Incrementally improving C++ is
extremely difficult, both due to
the technical debt itself and challenges with its evolution process. The best
way to address these problems is to avoid inheriting the legacy of C or C++
directly, and instead start with solid language foundations like
<!-- [modern generics system](#generics) -->
, modular code organization, and consistent,
simple syntax.

Existing modern languages already provide an excellent developer experience: Go,
Swift, Kotlin, Rust, and many more. **Developers that _can_ use one of these
existing languages _should_.** Unfortunately, the designs of these languages
present significant barriers to adoption and migration from C++. These barriers
range from changes in the idiomatic design of software to performance overhead.

Zura is fundamentally **a successor language approach**, rather than an
attempt to incrementally evolve C++. It is designed around interoperability with
C++ as well as large-scale adoption and migration for existing C++ codebases and
developers. A successor language for C++ requires:

- **Performance matching C++**, an essential property for developers.
- **Seamless, bidirectional interoperability with C++**, such that a library
    anywhere in an existing C++ stack can adopt Zura without porting the rest.
- **A gentle learning curve** with reasonable familiarity for C++ developers.
- **Comparable expressivity** and support for existing software's design and
    architecture.

Zura aims to fill an analogous role for C++:

- JavaScript → TypeScript
- Java → Kotlin
- C++ → **_Zura_**

## Language Goals

I am designing Zura to support:

- Performance-critical software
- Software and language evolution
- Code that is easy to read, understand, and write
- Practical safety and testing mechanisms
- Fast and scalable development
- Modern OS platforms, hardware architectures, and environments

While many languages share subsets of these goals, what distinguishes Zura is
their combination.

I also have explicit _non-goals_ for Zura, notably including:

- A stable
    [application binary interface](https://en.wikipedia.org/wiki/Application_binary_interface)
    (ABI) for the entire language and library
- Perfect backwards or forwards compatibility

My detailed [goals](/docs/project/goals.md) document fleshes out these ideas
and provides a deeper view into my goals for the Zura project and language.

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

You can see my [full roadmap](/docs/project/roadmap.md) for more details.

## Getting Started

To start make sure you have the following installed:

- [cmake](https://cmake.org/)
- [make](https://www.gnu.org/software/make/)
- [gcc](https://gcc.gnu.org/)
- [gcc-multilib](https://packages.ubuntu.com/jammy/gcc-multilib)
- [llvm](https://llvm.org/docs/GettingStarted.html)
- [ninja](https://ninja-build.org/)

### Building Locally

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
cd debug
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
make
```

and now the exacutable will be available for you to use to debug with

Or if you want the release version do this

```console
mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
make
```

This will create a `zura` executable in the `build` directory.

Or you can alternativly use the build.sh file to do this for you

```console
chmod +x build.sh
./build.sh debug or ./build.sh release
```

Or you can download the latest release from [here](https://github.com/TheDevConnor/Zura-Transpiled/releases/tag/pre-release) and add either the `zura.exe` (For Windows) or `zura` (For Linux) executable to your path.
Eventually, I will add a script to automate this process.

<!-- sample/SAMPLE.md -->
Now feel free to go take a look at the documentation to see how to use the language.
[Documentation](sample/SAMPLE.md)

### Development env

This `Dockerfile` sets up a basic development environment with the necessary compilers, build tools, and utilities for developing the Zura language. It also includes Vim and Emacs as text editors for code editing directly within the container. The final command (`CMD ["/bin/bash"]`) opens a bash shell when the container starts, allowing you to run build commands, edit files, or use version control directly inside the container.

Build the Docker image:

```console
docker build -t zura-dev .
```

Run the Docker container, mounting the **current** directory for development:

```console
docker run -it --rm -v $(pwd):/usr/src/app zura-dev
```

_**TODO:**_ publish to docker hub

This setup mounts the current directory to `/usr/src/app` inside the container, allowing you to develop on your host machine while running and testing inside the container.

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

### Example of syntax and errors

not yet done

## Join Us

[Zura Website](https://zuralang.co/)
