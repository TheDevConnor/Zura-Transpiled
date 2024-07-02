# Use an official Ubuntu base image
FROM ubuntu:latest

# Set non-interactive installation mode
ARG DEBIAN_FRONTEND=noninteractive

# Set the working directory in the container
WORKDIR /usr/src/app

# Install development tools and dependencies
RUN apt-get update && apt-get install -y \
	cmake \
	make \
	gcc \
	g++ \
	gcc-multilib \
	llvm \
	valgrind \
	git \
    ninja-build \
	&& rm -rf /var/lib/apt/lists/*

# Copy the current directory contents into the container at /usr/src/app
COPY . .

# Make the build script executable
RUN chmod +x ./build.sh

# Build the application in release mode
RUN ./build.sh release

# Optionally, set up a default command or entrypoint for development, such as opening a shell
CMD ["/bin/bash"]