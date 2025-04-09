#!/usr/bin/env bash

# Define variables
REPO_URL="https://github.com/TheDevConnor/Zura-Transpiled.git"
INSTALL_DIR="/bin"
BUILD_DIR="$HOME/zura_build"
EXECUTABLE="release/zura"
LOCAL_VERSION=$(zura --version | sed -E 's/.*(v[0-9]+\.[0-9]+\.[0-9]+).*/\1/')
# Check for dependencies
DEPENDENCIES=("cmake" "ninja" "gcc" "make" "curl")

for dep in "${DEPENDENCIES[@]}"; do
    if ! command -v $dep &> /dev/null; then
        echo "Error: $dep is not installed. Please install it and run the script again."
        exit 1
    fi
done

# Clone the repo if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    echo "Cloning repository..."
    git clone --branch main "$REPO_URL" "$BUILD_DIR"
else
    echo "Repository already exists."
fi

# Navigate to the project directory
cd "$BUILD_DIR" || { echo "Error: Failed to enter project directory"; exit 1; }

# Get the latest version from the repo
ONLINE_VERSION=$(curl -s "https://api.github.com/repos/TheDevConnor/Zura-Transpiled/releases/latest" \ | jq -r '.tag_name' | sed -E 's/^((v[0-9]+\.[0-9]+\.[0-9]+)).*/\1/')

if [ -z "$LOCAL_VERSION" ]; then
    LOCAL_VERSION="none"
fi

echo "Local version: $LOCAL_VERSION"
echo "Online version: $ONLINE_VERSION"

if [ "$LOCAL_VERSION" != "$ONLINE_VERSION" ]; then
    echo "New version available! Pulling latest changes..."
    git pull origin main
else
    echo "Zura is up to date. Proceeding with build..."
fi

# Build the project
echo "Building Zura..."
./build.sh release

# Verify the build was successful
if [ ! -f "$EXECUTABLE" ]; then
    echo "Error: Build failed. Executable not found at $EXECUTABLE."
    exit 1
fi

# Move executable
echo "Installing Zura..."
sudo mv "$EXECUTABLE" "$INSTALL_DIR"

echo "Zura installation/update completed successfully (version $ONLINE_VERSION)."
