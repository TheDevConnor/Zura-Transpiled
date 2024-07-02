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
RUN chmod +x ./entrypoint.sh

# Build the application in release mode
# Copy the entrypoint script and make it executable
COPY entrypoint.sh /usr/local/bin/entrypoint.sh
#RUN chmod +x /usr/local/bin/entrypoint.sh
# Set the entrypoint script to be executed
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]