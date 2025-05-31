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
sudo apt-get install make gcc valgrind
```

#### openSUSE / SUSE Linux
```sh
sudo zypper install make gcc valgrind
```

#### Red Hat / Fedora
```sh
sudo dnf install gcc make valgrind
```

#### Arch Linux
```sh
sudo pacman -S make gcc valgrind 
```

## Building
To build the Zura compiler:

### Debug build
```sh
make BUILD=debug
```

This will compile the program with debugging symbols.

### Release build
```sh
make BUILD=release
```

This will optimize the binary for performance.

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
