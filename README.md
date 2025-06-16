<h1>
  <img src="assets/zura.png" alt="Zura Logo" width="40" style="vertical-align: middle; margin-right: 10px;">
    Zura a low-level compiled alternative to C and C++
</h1>

<p align="center">
  <a href="#why">Why?</a> |
  <a href="#language-goals">Goals</a> |
  <a href="#Project-Status">Status</a> |
  <a href="#Getting-Started">Getting Started</a> |
  <a href="#usage">Usage</a> |
  <a href="#join-us">Join Us</a>
</p>

---

## Introduction

**Zura** is a statically typed, compiled systems programming language inspired by **C** and **Go**. It is designed to offer simplicity, safety, and high performance while remaining close to the metal. Zura is still in early development and not yet ready for production use.

[documentation](docs/docs.md) for usage details and language design notes.

---

## Why?

Modern low-level programming often comes with trade-offs between safety, simplicity, and performance. C and C++ are powerful, but notoriously difficult to master safely.
Zura follows the principle of *“Simple yet powerful.”* It is designed to retain the low-level control of C while reducing complexity 

Zura aims to:

- Provide a safer and more modern alternative to C.
- Keep compilation fast and output highly performant.
- Reduce boilerplate while preserving control over memory and hardware.

---

## Language Goals

- **Minimal & Explicit Syntax** – Avoid hidden control flow or magic.
- **Fast Compilation** – Prioritize developer feedback cycles.
- **Zero-Cost Abstractions** – Avoid performance penalties for convenience.
- **Manual Memory Control** – Support fine-grained memory management.
- **Toolchain Simplicity** – No complex build systems required.

---

## Project Status

Zura is in the early experimental phase. There is **no stable compiler or runtime yet**.

Current progress:

- Core language strategy defined
- Project structure, governance, and evolution process in place
- Basic compilation pipeline in progress
- Standard library and tooling ecosystem still under design

This project explores the possibility of building a true C/C++ successor that could gain real-world adoption.

---

## Getting Started

You’ll need the following tools installed:

- [Make](https://www.gnu.org/software/make/)
- [GCC](https://gcc.gnu.org/)
- [Valgrind](https://valgrind.org/) (optional, for memory debugging)

---

### Linux Installation

#### Debain / Ubuntu
```sh
sudo apt-get install cmake gcc valgrind ninja
```

#### openSUSE / SUSE Linux
```sh
sudo zypper install cmake gcc valgrind ninja
```

#### Red Hat / Fedora
```sh
sudo dnf install gcc cmake valgrind ninja
```

#### Arch Linux
```sh
sudo pacman -S cmake gcc valgrind ninja
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

## Usage

```sh
./zura build <filepath> -name <binary>
```

if you wish to save the assembly code to a file, use the `-save` flag:

```sh
./zura build <filepath> -name <binary> -save
```

Then run the binary with:

```sh
./<binary>
```

## Join Us

[Zura Website](https://thedevconnor.github.io/Zura-Website/)
