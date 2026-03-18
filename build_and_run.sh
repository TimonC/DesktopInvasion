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
    echo -e "\n\033[1;33m========== HMR triggered at $(date '+%Y-%m-%d %H:%M:%S') ==========\033[0m"
    kill $APP_PID 2>/dev/null
    cmake ..
    make
    ./DesktopInvasion &
    APP_PID=$!
done

