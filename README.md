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
- [llvm](https://llvm.org/docs/GettingStarted.html)

Commands to install the above packages on Windows:
```bash
# Install chocolatey
Set-ExecutionPolicy Bypass -Scope Process -Force; `
  iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))

# Install packages
choco install cmake make mingw llvm
```

Commands to install the above packages on Ubuntu and Debian: 
```bash
sudo apt install cmake make gcc gcc-multilib llvm
```
For Arch Linux:
```bash
sudo pacman -S cmake make gcc gcc-multilib llvm
```

Now if you want to build zura for the debug mode do
```bash
mkdir debug
cd Debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```
and now the exacutable will be availbe for you to use to debug with

Or if you want the release version do this
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release 
make
```
This will create a `zura` executable in the `build` directory.

or you can use the bash script to build it as well

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

## Example of syntax and errors
not yet down

## Command that you can run
```bash 
zura --help
```
This command will show you all of the commands that you can run in the zura compiler.

##  Developement env

This `Dockerfile` sets up a basic development environment with the necessary compilers, build tools, and utilities for developing the Zura language. It also includes Vim and Emacs as text editors for code editing directly within the container. The final command (`CMD ["/bin/bash"]`) opens a bash shell when the container starts, allowing you to run build commands, edit files, or use version control directly inside the container.

To build and run this Docker container, use the following commands:


```console
# Build the Docker image
docker build -t zura-dev .

# Run the Docker container, mounting the current directory for development
docker run -it --rm -v $(pwd):/usr/src/app zura-dev
```


This setup mounts the current directory to `/usr/src/app` inside the container, allowing you to develop on your host machine while running and testing inside the container.