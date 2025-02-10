#!/usr/bin/env bash

# Define variables
REPO_URL="https://github.com/TheDevConnor/Zura-Transpiled.git"
INSTALL_DIR="/usr/local/bin"
BUILD_DIR="$HOME/zura_build"
EXECUTABLE="release/zura"
VERSION_FILE="version.txt"
VERSION_FILE_DIR="/usr/local/share/zura"
LOCAL_VERSION_FILE="$VERSION_FILE_DIR/version.txt"

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
    git clone --branch experimental "$REPO_URL" "$BUILD_DIR"
else
    echo "Repository already exists."
fi

# Navigate to the project directory
cd "$BUILD_DIR" || { echo "Error: Failed to enter project directory"; exit 1; }

# Get the latest version from the repo
ONLINE_VERSION=$(curl -s "https://raw.githubusercontent.com/TheDevConnor/Zura-Transpiled/experimental/$VERSION_FILE")

if [ -f "$LOCAL_VERSION_FILE" ]; then
    LOCAL_VERSION=$(cat "$LOCAL_VERSION_FILE")
else
    LOCAL_VERSION="none"
fi

echo "Local version: $LOCAL_VERSION"
echo "Online version: $ONLINE_VERSION"

if [ "$LOCAL_VERSION" != "$ONLINE_VERSION" ]; then
    echo "New version available! Pulling latest changes..."
    git pull origin experimental
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
sudo mv "version.txt" "$INSTALL_DIR"

# Store the new version
echo "Updating version file..."
sudo mkdir -p "$VERSION_FILE_DIR"
echo "$ONLINE_VERSION" | sudo tee "$LOCAL_VERSION_FILE" > /dev/null

echo "Zura installation/update completed successfully (version $ONLINE_VERSION)."
