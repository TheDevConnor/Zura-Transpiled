# Zura 
*Zura a low-level compiled alternative to C and C++*

<p align="center">
  <img src="assets/zura.png" alt="Zura Logo" width="120">
</p>

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
sudo apt-get install make cmake gcc valgrind
```

#### openSUSE / SUSE Linux
```sh
sudo zypper install make cmake gcc valgrind
```

#### Red Hat / Fedora
```sh
sudo dnf install make gcc cmake valgrind
```

#### Arch Linux
```sh
sudo pacman -S make cmake gcc valgrind
```

##### Building

To build the Zura compiler from source, run:
```sh
make BUILD=release -j$(nproc)
```
This will compile the Zura compiler and place the executable in the `release/` directory.
You can also run `make BUILD=debug -j$(nproc)` to build a debug version of the compiler.
Also the `-j$(nproc)` flag will speed up the build process by using all available CPU cores.

Or you can download the latest release from [here](https://github.com/TheDevConnor/Zura-Transpiled/releases/tag/pre-release) and add either the `zura.exe` (For Windows) or `zura` (For Linux) executable to your path.

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
