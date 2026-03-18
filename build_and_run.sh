#!/bin/bash
# Create build dir
mkdir -p build
cd build

# Initial build
cmake ..
make


# Ensure XDG_RUNTIME_DIR exists and has correct permissions
mkdir -p "$XDG_RUNTIME_DIR"
chmod 0700 "$XDG_RUNTIME_DIR"
# Run the app in background
./DesktopInvasion &
APP_PID=$!

# Watch for changes in source folder
while inotifywait -r -e modify,create,delete ../src; do
    echo "Change detected, rebuilding..."
    kill $APP_PID 2>/dev/null
    cmake ..
    make
    ./DesktopInvasion &
    APP_PID=$!
done

