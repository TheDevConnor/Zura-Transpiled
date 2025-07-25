#include "lsp.hpp"

std::string lsp::getBuiltinNote(const std::string &builtin) {
  if (builtin == "@output") {
    return "This function will, as you may have guessed, output the string, character, array, or numerical argument and put into a file descriptor.\n"
            "In order to print to the console (STDOUT), you would use the file descriptor `1` (POSIX).\n"
            "For all other file descriptors like files or sockets, you would need to open it first.\n"
            "After that, however, you can freely write to it using this function.\n"
            "\n"
            "Example:\n"
            "```zura\n"
            "const main := fn () int! {\n"
            "\t@output(1, \"Hello, World!\\n\"); # Print to console\n"
            "};\n"
            "```";
  } else
  if (builtin == "@input") {
    return "This function will read from a file descriptor and place the inputted amount of contents (in bytes) into a piece of memory.\n"
           "This piece of memory can be allocated via `@alloc`, or from an array with a large enough size.\n"
           "The file descriptor can be any valid file descriptor. If you want to read from the console (STDIN), you would use the file descriptor `0` (POSIX).\n"
           "\n"
           "Example 1:\n"
           "```zura\n"
           "const main := fn () int! {\n"
           "\tconst buffer: *char = @cast<*char>(@alloc(100)); # Allocate 100 bytes of memory\n"
           "\t@input(0, buffer, 100); # \"Read '100' bytes of 'stdin' and place into 'buffer'\"\n"
           "\t@output(1, buffer); # Print the contents of the buffer to the console\n"
           "\t@free(buffer, 100); # Free the allocated memory\n"
          "};\n"
          "```\n"
          "\nExample 2:\n"
          "```zura\n"
          "const main := fn () int! {\n"
          "\tconst buffer: [100]char = [0]; # Create an array of 100 characters\n"
          "\t@input(0, @cast<*char>(buffer), 100); # Read '100' bytes of 'stdin' and place into 'buffer'\n"
          "\t@output(1, buffer); # Print the contents of the buffer to the console\n"
          "};\n"
          "```";
  } else
  if (builtin == "@outputln") {
    return "This function is equivalent to `@output`, but it will automatically append a newline character at the end of the output.\n"
           "This is useful for printing messages where you don't have to deal with the different line ending standards. (\\r\\n vs \\n).\n"
           "\n"
           "Example:\n"
           "```zura\n"
           "const main := fn () int! {\n"
           "\t@outputln(1, \"Hello, World!\"); # Print to console with a newline\n"
           "};\n"
           "```";
  } else
  if (builtin == "@alloc") {
    return "This function will allocate a piece of memory of the given size (in bytes) and return the pointer to it.\n"
           "This is useful for dynamically allocating memory at runtime.\n"
           "The @alloc function returns a void pointer that can later be type-casted for use of any type.\n"
           "Additionally, if the allocation fails, the returned pointer will be `nil` (0).\n"
           "\n"
           "Example:\n"
           "```zura\n"
           "const main := fn () int! {\n"
           "\tconst buffer: *char = @alloc(100); # Allocate 100 bytes of memory\n"
           "\t@output(1, @cast<int!>(buffer)); # Print the address of a pointer (in decimal form)\n"
           "\t@free(buffer, 100); # Frees the allocated memory\n"
           "};\n"
           "```";
  } else
  if (builtin == "@free") {
    return "Every time you allocate memory with `@alloc`, you must free it with `@free` to avoid leaking memory, as well as many other issues.\n"
           "This function requires a pointer to the allocated memory as well as how many bytes to free.\n"
           "> [!NOTE]\n"
           "> The number of bytes to free must be the same as how many bytes you allocated. This is due to low-level system calls and other things you probably don't care about.\n"
           "\n"
            "Example:\n"
            "```zura\n"
            "const main := fn () int! {\n"
            "\tconst buffer: *char = @alloc(100); # Allocate 100 bytes of memory\n"
            "\t@output(1, @cast<int!>(buffer)); # Print the address of a pointer (in decimal form)\n"
            "\t@free(buffer, 100); # Frees the allocated memory\n"
            "};\n"
            "```";
  } else
  if (builtin == "@memcpy") {
    return "This function will copy a specified number of bytes from one memory location to another.\n"
           "It is useful for copying or transferring data from one buffer or array to the next.\n"
           "The function takes three arguments: the source pointer, the destination pointer, and the number of bytes to copy.\n"
           "> [!WARNING]\n"
           "> It is very important that the destination has been fully allocated and has enough space to hold the copied data.\n"
           "> If you did not do this, then the program will not work as intended, or, more likely, crash altogether..\n"
           "Additionally, the type of the pointers used does not matter, whether they match or differ. A byte-for-byte copy from the source to the destination will be performed.\n"
           "\n"
           "Example:\n"
            "```zura\n"
            "const main := fn () int! {\n"
            "\tconst source: [14]char = \"Hello, World!\"; # Create an array of 100 characters\n"
            "\tconst destination: *char = @cast<*char>(@alloc(14)); # Allocate 14 bytes of memory\n"
            "\t@memcpy(&source, destination, 14); # Copy the contents of 'source' to 'destination'\n"
            "\t@output(1, destination); # Print the contents of the destination to the console\n"
            "\t@free(destination, 14); # Free the allocated memory\n"
            "};\n"
            "```";
  } else
  if (builtin == "@cast") {
    return "This function will cast an object of one type to another type.\n"
           "It is used for converting between integers of different sizes, floating-point and integers, pointers, etc.\n"
           "This function is similar to a template function, as a type must be specified in between angle brackets (`<` and `>`).\n"
           "> [!NOTE]\n"
           "> All integer-to-float and float-to-integer conversions are done with truncation, not rounding. This means that the float will be floored. (effectively, you can imagine that the decimal point is being chopped off.)"
           "\n"
           "> [!CAUTION]\n"
           "> PLEASE BE ADVISED: Pointer casting is more similar to a bit cast than to a dynamic cast. The underlying types of either the input or the output are not checked or modified.\n"
           "\n"
           "> [!IMPORTANT]\n"
           "> Casting from a structure to another structure is not supported, even if the structures are of the same byte size or have the same fields.\n"
           "\n"
            "Example:\n"
            "```zura\n"
            "const main := fn () int! {\n"
            "\tconst floating: double = 4.2; # Create an integer\n"
            "\tconst notFloating: int! = @cast<int!>(floating); # Return the truncated, 8-byte integer version of the floating point number.\n"
            "\t@output(1, notFloating); # Will print '4'\n"
            "}\n"
            "```";
  } else
  if (builtin == "@sizeof") {
    return "This function will return the size of a given type or expression in bytes.\n"
           "This can be useful when using dynamic memory allocation on a structure or type that you aren't done with or when you don't feel like counting.\n"
           "This function's only input is an expression or a type. It returns an 8-byte integer containing the size in bytes.\n"
            "\n"
            "Example:\n"
            "```zura\n"
            "const main := fn () int! {\n"
            "\tconst size: int! = @sizeof(int!); # Get the size of an integer\n"
            "\t@output(1, size); # Will print '8'\n"
            "}\n"
            "```";
  } else
  if (builtin == "@open") {
    return "This function takes in a path, either relative or absolute, and returns a file descriptor opened from that path.\n"
           "It is useful for opening files used in I/O. However, it is not designed for use in sockets. That is the job of the `@socket` function.\n"
           "On an error or unscucessful opening, the function returns a negative number (USUALLY -1, but can sometimes be different...)\n"
           "It takes in a string or string-based type as an argument, as well as three boolean flags.\n"
           "> [!NOTE]\n"
           "> On a low-level-systems level, the per-user/group flag is not supplied. By default, it will be RW- for the user, group, and superuser.\n"
           "> [!IMPORANT]\n"
           "> PLEASE remember to use the `@close` function to close the file descriptor when you are done with it.\n"
           "The three boolean flags are as follows:\n"
           " - read: Allows for the reading of the file. (Why would you ever have this off, by the way?)\n"
           " - write: Allows for writing to the file.\n"
           " - create: Creates the file at the path if it does not exist. On a system permissions level, this requires the write permission on the user for the directory.\n"
           "\n"
            "Example:\n"
            "```zura\n"
            "const main := fn () int! {\n"
            "\tconst fd: int? = @open(\"/tmp/test.txt\", true, true, false); # Opens a file stored in /tmp/test.txt. The given permissions are read and write, but not create.\n"
            "\t@output(1, fd); # Will print the file descriptor number\n"
            "\t@outputln(1, \"File opened successfully!\"); # Will print 'File opened successfully!'\n"
            "\t@close(fd); # Closes the file descriptor\n"
            "}\n"
            "```";
  } else
  if (builtin == "@close") {
    return "This function will close a file descriptor, freeing up the resources associated with it.\n"
           "It takes in a single argument, which is the file descriptor to close.\n"
           "If the file descriptor is invalid or already closed, the function will do nothing.\n"
           "\n"
           "Example:\n"
           "```zura\n"
           "const main := fn () int! {\n"
           "\tconst fd: int? = @open(\"/tmp/test.txt\", true, true, false); # Opens a file stored in /tmp/test.txt. The given permissions are read and write, but not create.\n"
           "\t@output(1, fd); # Will print the file descriptor number\n"
           "\t@close(fd); # Closes the file descriptor\n"
           "}\n"
           "```";
  } else
  if (builtin == "@socket") {
    return "This builtin function is amongst one of the most complicated ones. @socket will create a listenable socket on the system.\n"
           "It is used along with `@bind`, `@listen`, and `@accept` to create a webserver or a socket server.\n"
           "It takes in three arguments: the domain, the type, and the protocol.\n"
           "The domain can be `AF_INET` for IPv4 or `AF_INET6` for IPv6. While technically there are like 8 more on the Linux level, they are all kinda useless.\n"
           "The type can be `SOCK_STREAM` for pretty much all your needs, or `SOCK_DGRAM` for UDP sockets.\n"
           "Okay, let me break the fourth wall for a moment: I, personally, do not have literally ANY clue on what this does. Please pass 0 for this argument to let the system decide whatever this thing does for you.\n"
           "On success, it returns a positive file descriptor that you should pass into `@bind`.\n"
           "Bind, as the name suggests, will bind the socket to an address and port. It is a little complicated, so go enjoy yourself with the docs.\n"
           "After the socket has been binded, you will use listen to allow the socket to be listening for incoming connections. Look at its documentation next.\n"
           "Finally, you can use `@accept` to wait and accept an incoming connection. It will return yet ANOTHER file descriptor for the new connection. This is the FD you can read from and write to.\n"
           "After you are done with the daughter connection, you can close it with `@close`. You will also use `@close` to close the FD from `@socket`.\n"
            "> [!NOTE]\n"
            "> Somewhat similarly to C, you should use `@recv` and `@send` for reading and writing to the socket. You could also use `@input` for reading, but that is both unclear and not recommended.\n"
            "\n"
            "Example:\n"
            "```zura\n"
            "const main := fn () int! {\n"
            "\tconst socketFD: int? = @socket(AF_INET, SOCK_STREAM, 0); # Create a socket with the given domain, type, and protocol\n"
            "\tif (socketFD < 0) {\n"
            "\t\t@outputln(1, \"Failed to create socket!\"); # Print an error message if the socket creation failed\n"
            "\t\treturn -1; # Return an error code\n"
            "\t}\n"
            "\thave bindInfo: SockAddrIn = {\n # An address structure for a client, SockAddrOut, also exists\n"
            "\t\tfamily: AF_INET, # Yes, you have to specify... again. BLAME POSIX\n"
            "\t\tport: 0x901f # This is the network byte order (big-endian) representation of 8080"
            "\t\taddr: 0 # 0 means listen to basically anybody trying to connect to you. If you're a server, you probably want this.\n"
            "\t};\n"
            "\t@bind(socketFD, &bindInfo, sizeof(bindInfo));"
            "\t@listen(socketFD, 5); # Start listening on the socket. A backlog of 5 means that up to 5 connections can be waiting to be accepted, after which all subsequent connections are refused.\n"
            "\t@outputln(1, \"Socket is listening on port 8080!\"); # Print a message to the console\n"
            "\twhile (true) {\n"
            "\t\tconst clientFD: int? = @accept(socketFD, nil, nil); # Accept an incoming connection. The last two are used to get information about the connection, which we will not be using today..\n"
            "\t\thave buffer: [1024]char = [0]; # Pre-allocate 1024 bytes of memory for whatever this client has to say\n"
            "\t\t@recv(clientFD, buffer, 1024, 0); # Read up to 1024 bytes from the client into the buffer\n"
            "\t\t@output(1, buffer); # Print the contents of the buffer to the console\n"
            "\t\t@close(clientFD); # Close the client connection\n"
            "\t}\n"
            "\t# We will actually never get here because of the infinite loop, but we are adding this for illustration purposes.\n"
            "\t@close(socketFD); # Close the socket\n"
            "}\n"
            "```";
  } else
  if (builtin == "@bind") {
    return "This function will bind a socket to an address and port.\n"
           "Depending on what your socket will do, this builtin function is usually what you'd tweak.\n"
           "There is a `sockaddr` struct pointer that you must pass into bind. This struct is not defined for you, but as bind is only expecting a pointer, you can get creative.\n"
           "The function takes in three arguments: the file descriptor returned by the `@socket` function, a pointer to the SockAddr structure, and the byte size of said structure.\n"
           "If you are using AF_INET (meaning IPv4), and you want to be a server that recieves connections, you might expect a pointer to a struct like this:\n"
           "```zura\n"
           "const SockAddrInIPv4 := struct {\n"
           "\tfamily: short # AF_INET = 16 bytes\n"
           "\tport: short # The port number to listen to. All connections are localhost. NOTE: Network byte order is BIG ENDIAN! Your system likely ISNT! This means you can write a htons function or go online and swap them out. Example: 8080 is 0x901f.\n"
           "\taddr: int # This specifies what type of connections to let in. 0 means to listen to basically everyone.\n"
           "\tzero: [8]char = [0] # This is an 8-byte padding. It should never have anything other than 0, as per the standard. But I don't think it matters.\n"
           "}\n"
           "> [!NOTE]\n"
           "> If you were to look at the man page for `bind` (`man 2 bind`), it specifies a generic `sockaddr` structure. Depending on your usecase, you would NOT use whatever is in the"
           "> generic `sockaddr` structure, but whatever your specific need is. Like, for example, the SockAddrInIPv4."
           "\n"
           "Example:\n"
           "```zura\n"
           "const main := fn () int! {\n"
           "\tconst socketFD: int? = @socket(AF_INET, SOCK_STREAM, 0); # Create a socket with the given domain, type, and protocol\n"
           "\tif (socketFD < 0) {\n"
           "\t\t@outputln(1, \"Failed to create socket!\"); # Print an error message if the socket creation failed\n"
           "\t\treturn -1; # Return an error code\n"
           "\t}\n"
           "\thave bindInfo: SockAddrInIPv4 = {\n # An address structure for a client, SockAddrOut, also exists\n"
           "\t\tfamily: AF_INET, # Yes, you have to specify... again. BLAME POSIX\n"
           "\t\tport: 0x901f # This is the network byte order (big-endian) representation of 8080\n"
           "\t\taddr: 0 # 0 means listen to basically anybody trying to connect to you. If you're a server, you probably want this.\n"
           "\t\tzero: [0] # Necessary padding"
           "\t};\n"
           "\t@bind(socketFD, &bindInfo, sizeof(bindInfo));\n"
           "\t@outputln(1, \"Socket is bound to port 8080!\"); # Print a message to the console\n"
           "\t@close(socketFD); # Close the socket\n"
           "}\n"
           "```";
  } else
  if (builtin == "@listen") {
    return "This function will make a socket open and public and actually able to be accessed by the outside world.\n"
           "It can only be used on sockets that have bene binded using, you guessed it, `@bind`.\n"
           "It takes in two arguments: the file descriptor returned by the `@socket` function and the backlog size,\n"
           "which is the maximum number of connections that can be waiting to be accepted before being automatically refused.\n"
           "> [!NOTE]\n"
           "> The backlog size is how many unaccepted connections can be waiting. You can have as many active (accepted) connections as you want, but the backlog is only for those of which have not yet been accepted.\n"
           "\n"
            "Example:\n"
            "```zura\n"
            "const main := fn () int! {\n"
            "\tconst socketFD: int? = @socket(AF_INET, SOCK_STREAM, 0); # Create a socket with the given domain, type, and protocol\n"
            "\tif (socketFD < 0) {\n"
            "\t\t@outputln(1, \"Failed to create socket!\"); # Print an error message if the socket creation failed\n"
            "\t\treturn -1; # Return an error code\n"
            "\t}\n"
            "\thave bindInfo: SockAddrInIPv4 = {\n # An address structure for a client, SockAddrOut, also exists\n"
            "\t\tfamily: AF_INET, # Yes, you have to specify... again. BLAME POSIX\n"
            "\t\tport: 0x901f # This is the network byte order (big-endian) representation of 8080\n"
            "\t\taddr: 0 # 0 means listen to basically anybody trying to connect to you. If you're a server, you probably want this.\n"
            "\t\tzero: [0] # Necessary padding\n"
            "\t};\n"
            "\t@bind(socketFD, &bindInfo, sizeof(bindInfo));\n"
            "\t@listen(socketFD, 5); # Start listening on the socket. A backlog of 5 means that up to 5 connections can be waiting to be accepted, after which all subsequent connections are refused.\n"
            "\t@outputln(1, \"Socket is listening on port 8080!\"); # Print a message to the console\n"
            "\t@close(socketFD); # Close the socket\n"
            "}\n"
            "```";
  } else
  if (builtin == "@accept") {
    return "This function will accept an incoming connection on a listening socket.\n"
           "The function takes in three arguments, of which only one is required: the file descriptor returned by the `@socket` function, a pointer to the `sockaddr` structure, and the size of the structure. However, you can pass `nil` into both of these and it will work just fine.\n"
           "Your computer can technically have as many active connections as it wants, but you should still ensure that you close the connection when you are done with it.\n"
           "On success, a positive integer representing the file descriptor of the new connection is returned.\n"
           "This file descriptor can be used to read and write to the new connection using @recv and @send.\n"
           "> [!NOTE]\n"
           "> In most scenarios, the @recv and @input functions are equivalent. @recv is more explicitly used for sockets, and it can contain flags.\n"
           "> Additionally, when handling a zero-length message, @recv will consume the data, while @input will do nothing (pretty much just nop).\n"
           "\n"
           "Example:\n"
           "const main := fn () int! {\n"
           "\tconst socketFD: int? = @socket(AF_INET, SOCK_STREAM, 0); # Create a socket with the given domain, type, and protocol\n"
           "\tif (socketFD < 0) {\n"
           "\t\t@outputln(1, \"Failed to create socket!\"); # Print an error message if the socket creation failed\n"
           "\t\treturn -1; # Return an error code\n"
           "\t}\n"
           "\thave bindInfo: SockAddrInIPv4 = {\n # An address structure for a client, SockAddrOut, also exists\n"
           "\t\tfamily: AF_INET, # Yes, you have to specify... again. BLAME POSIX\n"
           "\t\tport: 0x901f # This is the network byte order (big-endian) representation of 8080\n"
           "\t\taddr: 0 # 0 means listen to basically anybody trying to connect to you. If you're a server, you probably want this.\n"
           "\t\tzero: [0] # Necessary padding\n"
           "\t};\n"
           "\t@bind(socketFD, &bindInfo, sizeof(bindInfo));\n"
           "\t@listen(socketFD, 5); # Start listening on the socket. A backlog of 5 means that up to 5 connections can be waiting to be accepted, after which all subsequent connections are refused.\n"
           "\t@outputln(1, \"Socket is listening on port 8080!\"); # Print a message to the console\n"
           "\twhile (true) {\n"
            "\t\tconst clientFD: int? = @accept(socketFD, nil, nil); # Accept an incoming connection. The last two are used to get information about the connection, which we will not be using today..\n"
            "\t\thave buffer: [1024]char = [0]; # Pre-allocate 1024 bytes of memory for whatever this client has to say\n"
            "\t\t@recv(clientFD, buffer, 1024, 0); # Read up to 1024 bytes from the client into the buffer\n"
            "\t\t@output(1, buffer); # Print the contents of the buffer to the console\n"
            "\t\t@close(clientFD); # Close the client connection\n"
            "\t}\n"
           "\t# We will actually never get here because of the infinite loop, but we are adding this for illustration purposes.\n"
           "\t@close(socketFD); # Close the socket\n"
           "}\n"
           "```";
  } else
  if (builtin == "@recv") {
    return "This function will read data from a socket connection and place it into a buffer.\n"
           "It takes in four arguments: the file descriptor of the socket, a pointer to the buffer, the number of bytes to read, and flags.\n"
           "The function returns the number of bytes read, or -1 on error.\n"
           "> [!NOTE]\n"
           "> This function is similar to `@input`, but it is specifically designed for sockets and can handle flags. See @input for details.\n"
           "\n"
           "Example:\n"
           "```zura\n"
           "const main := fn () int! {\n"
           "\tconst socketFD: int? = @socket(AF_INET, SOCK_STREAM, 0); # Create a socket with the given domain, type, and protocol\n"
           "\tif (socketFD < 0) {\n"
           "\t\t@outputln(1, \"Failed to create socket!\"); # Print an error message if the socket creation failed\n"
           "\t\treturn -1; # Return an error code\n"
           "\t}\n"
           "\thave bindInfo: SockAddrInIPv4 = {\n # An address structure for a client, SockAddrOut, also exists\n"
           "\t\tfamily: AF_INET, # Yes, you have to specify... again. BLAME POSIX\n"
           "\t\tport: 0x901f # This is the network byte order (big-endian) representation of 8080\n"
           "\t\taddr: 0 # 0 means listen to basically anybody trying to connect to you. If you're a server, you probably want this.\n"
           "\t\tzero: [0] # Necessary padding\n"
           "\t};\n"
           "\t@bind(socketFD, &bindInfo, sizeof(bindInfo));\n"
           "\t@listen(socketFD, 5); # Start listening on the socket. A backlog of 5 means that up to 5 connections can be waiting to be accepted, after which all subsequent connections are refused.\n"
           "\t@outputln(1, \"Socket is listening on port 8080!\"); # Print a message to the console\n"
           "\twhile (true) {\n"
            "\t\tconst clientFD: int? = @accept(socketFD, nil, nil); # Accept an incoming connection. The last two are used to get information about the connection, which we will not be using today..\n"
            "\t\thave buffer: [1024]char = [0]; # Pre-allocate 1024 bytes of memory for whatever this client has to say\n"
            "\t\t@recv(clientFD, buffer, 1024, 0); # Read up to 1024 bytes from the client into the buffer\n"
            "\t\t@output(1, buffer); # Print the contents of the buffer to the console\n"
            "\t\t@close(clientFD); # Close the client connection\n"
            "\t}\n"
           "\t# We will actually never get here because of the infinite loop, but we are adding this for illustration purposes.\n"
           "\t@close(socketFD); # Close the socket\n"
           "}\n"
           "```";
  } else
  if (builtin == "@send") {
    return "This function will write data to a socket connection from a buffer.\n"
           "It takes in four arguments: the file descriptor of the socket, a pointer to the buffer, the number of bytes to write, and flags.\n"
           "The function returns the number of bytes written, or -1 on error.\n"
           "> [!NOTE]\n"
           "> This function is similar to `@output`, but it is specifically designed for sockets and can handle flags. See @output for details.\n"
           "\n"
           "Example:\n"
           "```zura\n"
           "const main := fn () int! {\n"
           "\tconst socketFD: int? = @socket(AF_INET, SOCK_STREAM, 0); # Create a socket with the given domain, type, and protocol\n"
           "\tif (socketFD < 0) {\n"
           "\t\t@outputln(1, \"Failed to create socket!\"); # Print an error message if the socket creation failed\n"
           "\t\treturn -1; # Return an error code\n"
           "\t}\n"
           "\thave bindInfo: SockAddrInIPv4 = {\n # An address structure for a client, SockAddrOut, also exists\n"
           "\t\tfamily: AF_INET, # Yes, you have to specify... again. BLAME POSIX\n"
           "\t\tport: 0x901f # This is the network byte order (big-endian) representation of 8080\n"
           "\t\taddr: 0 # 0 means listen to basically anybody trying to connect to you. If you're a server, you probably want this.\n"
           "\t\tzero: [0] # Necessary padding\n"
           "\t};\n"
           "\t@bind(socketFD, &bindInfo, sizeof(bindInfo));\n"
           "\t@listen(socketFD, 5); # Start listening on the socket. A backlog of 5 means that up to 5 connections can be waiting to be accepted, after which all subsequent connections are refused.\n"
           "\t@outputln(1, \"Socket is listening on port 8080!\"); # Print a message to the console\n"
           "\twhile (true) {\n"
           "\t\tconst clientFD: int? = @accept(socketFD, nil, nil); # Accept an incoming connection. The last two are used to get information about the connection, which we will not be using today..\n"
            "\t\thave buffer: [1024]char = [0]; # Pre-allocate 1024 bytes of memory for whatever this client has to say\n"
            "\t\t@recv(clientFD, buffer, 1024, 0); # Read up to 1024 bytes from the client into the buffer\n"
            "\t\t@output(1, buffer); # Print the contents of the buffer to the console\n"
            "\t\t@send(clientFD, buffer, 1024, 0); # Send the contents of the buffer back to the client\n"
            "\t\t@close(clientFD); # Close the client connection\n"
           "\t}\n"
           "\t# We will actually never get here because of the infinite loop, but we are adding this for illustration purposes.\n"
           "\t@close(socketFD); # Close the socket\n"
           "}\n"
           "```";
  } else
  if (builtin == "@getArgv") {
    return "This function will return the command line arguments passed into the program.\n"
           "It returns a *[]str, and you can access elements in it like a normal array.\n"
           "Be careful that the first element is the relative path to the program, then the actual arguments follow.\n"
           "Also be advised that the array is NOT null-terminated. You use argc to determine, and that's it.\n"
           "\n"
           "Example:\n"
           "```zura\n"
            "const main := fn () int! {\n"
            "\thave args: *[]str = @getArgv(); # Get the command line arguments\n"
            "\t@outputln(1, args[0]); # Print the first argument (the program name)\n"
            "\treturn argc;"
            "};\n"
           "```";
  } else
  if (builtin == "@getArgc") {
    return "This function will return how many arguments (or elements) are in argv.\n"
           "You need argc in order to determine how many elements there actually are. If you attempt to access an element that is out of bounds, you get a segmentation fault.\n"
           "The function takes in no arguments and returns an `int!` that lists how many arguments there are.\n"
           "Argc is always >= 1, as the first argument is the path to the program.\n"
            "\n"
            "Example:\n"
            "```zura\n"
            "const main := fn () int! {\n"
            "\tconst argc: int! = @getArgc(); # Get the number of command line arguments\n"
            "\t@outputln(1, argc); # Print the number of arguments\n"
            "\thave args: *[]str = @getArgv(); # Get the command line arguments\n"  
            "\t@outputln(1, args[0]); # Print the first argument (the program name)\n"
            "\treturn argc; # Return the number of arguments\n"
            "};\n"
            "```";
  } else
  if (builtin == "@extern") {
    return "This function will declare a function exposed by @link as usable by the program.\n"
           "It is used when importing a (static) library and you want to select certain functions to be used.\n"
           "You would NOT, however, use extern when importing a Zura module, as those are compiled on the fly and are not considered static nor external.\n"
           "The function takes in a string that is the name of the function.\n"
           "This newly exposed function is callable via @call.\n"
           "\n"
           "Example:\n"
           "```zura\n"
           "# By default, standard libraries are actually NOT included in the final executable.\n"
           "# This not only saves space in the final executable, but it also means that most calls are very direct,\n"
           "# do not require external linking, are more efficient, more portable, and, in my own personal SovietPancakes opinion,\n"
           "# more fun and easier to see what is happening.\n"
           "@link(\"stdlibc\");"
           "@extern(\"printf\"); # Declare the printf function as an external function\n"
           "const main := fn () int! {\n"
           "\t@call<printf>(\"Hello, World!\\n\"); # Call the printf function with the string \"Hello, World!\\n\" (PS: for reasons above, the builtin @outputln function is better here LOL)\n"
           "\treturn 0; # Return 0 to indicate success\n"
            "};\n"
            "```";
  } else
  if (builtin == "@link") {
    return "This function will link a static library to the program.\n"
           "It is used to import a static library and make its functions available for use in the program.\n"
           "The function takes in a string that is the name of the library to link.\n"
           "> [!NOTE]\n"
           "> The library must be compiled and available in the system's library path.\n"
           "\n"
           "Example:\n"
           "```zura\n"
           "# By default, standard libraries are actually NOT included in the final executable.\n"
           "# This not only saves space in the final executable, but it also means that most calls are very direct,\n"
           "# do not require external linking, are more efficient, more portable, and, in my own personal SovietPancakes opinion,\n"
           "# more fun and easier to see what is happening.\n"
           "@link(\"stdlibc\"); # Link the standard C library\n"
           "@extern(\"printf\"); # Declare the printf function as an external function\n"
           "const main := fn () int! {\n"
           "\t@call<printf>(\"Hello, world!\"); # Print to console with a newline\n"
           "\treturn 0; # Return 0 to indicate success\n"
            "};\n"
            "```";
  } else
  if (builtin == "@call") {
    return "This function will call an external function that has been declared with @extern.\n"
           "It is used to call functions from static libraries or other external sources.\n"
           "The function takes in a string that is the name of the function to call, as well as any arguments to pass to the function.\n"
           "> [!NOTE]\n"
           "> The function must have been declared with @extern before it can be called with @call.\n"
           "\n"
           "Example:\n"
           "```zura\n"
           "# By default, standard libraries are actually NOT included in the final executable.\n"
           "# This not only saves space in the final executable, but it also means that most calls are very direct,\n"
           "# do not require external linking, are more efficient, more portable, and, in my own personal SovietPancakes opinion,\n"
           "# more fun and easier to see what is happening.\n"
           "@link(\"stdlibc\"); # Link the standard C library\n"
           "@extern(\"printf\"); # Declare the printf function as an external function\n"
           "const main := fn () int! {\n"
           "\t@call<printf>(\"Hello, world!\"); # Call the printf function with the string \"Hello, world!\"\n"
           "\treturn 0; # Return 0 to indicate success\n"
            "};\n"
            "```";
  } else 
  if (builtin == "@import") {
    return "This function will import another Zura file and make its functions usable in your current one.\n"
           "This could be useful for organization, reusing old code, or importing somebody else's code.\n"
           "It takes in a string for the path (can be relative or absolute) to the new Zura file to import.\n"
           "Everything you expect to happen when importing will happen. All public functions will be accessible to all new files that import it, or that import you.\n"
           "\n"
            "Example (test.zu):\n"
            "```zura\n"
            "const favoriteAnimal := fn () int! {\n"
            "\t@outputln(1, \"My favorite animal is a cat!\"); # Print my favorite animal to the console (NOTHING ON DOG PEOPLE BTW)\n"
            "};\n"
            "```\n"
            "Example (main.zu):\n"
            "```zura\n"
            "@import(\"test.zu\"); # Import the test.zu file\n"
            "const main := fn () int! {\n"
            "\tfavoriteAnimal(); # Call the favoriteAnimal function from the imported file\n"
            "\treturn 0; # Return 0 to indicate success\n"
            "};\n"
            "```";
  } else
  if (builtin == "@streq") {
    return "This function will compare two strings and return a boolean for whether or not they match.\n"
           "It takes in two string arguments and returns a boolean value. Obviously.\n"
           "> [!NOTE]\n"
           "> This function will read until it reaches either a difference between the two strings or a null terminator. This means that \"hello\\0 world!\" and \"hello\" would be percieved as equivalent.\n"
           "> Take caution in removing null terminators or write your own implementation, lol\n" 
           "Example:\n"
           "```zura\n"
           "const main := fn () int! {\n"
           "\tconst str1: string = \"Hello\";\n"
           "\tconst str2: string = \"Hello\";\n"
           "\tconst str3: string = \"World\";\n"
           "\t@output(1, @streq(str1, str2)); # Will print 'true'\n"
           "\t@output(1, @streq(str1, str3)); # Will print 'false'\n"
           "}\n"
           "```";
  } else
  {
    return "";
  }
}