# Use an official Ubuntu base image
FROM alpine:latest


# Set the working directory in the container
WORKDIR /usr/src/app

# Install development tools and dependencies
RUN apk update && apk add --no-cache \
	bash \
	cmake \
	make \
	gcc \
	g++ \
	valgrind \
	ninja-is-really-ninja \
	&& rm -rf /var/lib/apt/lists/*

# Copy the current directory contents into the container at /usr/src/app
COPY . .

# Make the build script and entry point executable
RUN chmod +x ./build.sh

RUN chmod +x ./entrypoint.sh



# Set the entrypoint script to be executed

ENTRYPOINT ["./entrypoint.sh"]