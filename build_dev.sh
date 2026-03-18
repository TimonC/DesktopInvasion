#!/bin/bash

set -e

: "${XDG_RUNTIME_DIR:=/tmp/xdg}"
mkdir -p "$XDG_RUNTIME_DIR"
chmod 0700 "$XDG_RUNTIME_DIR"

if [ -f "resources.dev.qrc" ]; then
    cp resources.dev.qrc resources.qrc
fi

echo "First script execution - wiping volume"
rm -rf /app/data/* 2>/dev/null || true

if [ "$VALGRIND_MODE" = "1" ]; then
    echo "Starting DesktopInvasion with Valgrind"
    mkdir -p /app/logs
    TIMESTAMP=$(date +%Y%m%d_%H%M%S)
    LOG_FILE="/app/logs/valgrind_${TIMESTAMP}.log"
    echo "Log will be saved to: $LOG_FILE (on host: ./valgrind_logs/)"

    mkdir -p build_valgrind
    cd build_valgrind
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-g -O0" ..
    ninja

    valgrind \
        --leak-check=full \
        --show-leak-kinds=all \
        --track-origins=yes \
        --log-file="$LOG_FILE" \
        ./DesktopInvasion

    echo "Log saved. Check ./valgrind_logs/ on your host machine"
    ls -la /app/logs/

else
    # HMR mode
    mkdir -p build
    cd build
    cmake -G Ninja ..
    ninja

    echo "Starting DesktopInvasion with HMR"
    ./DesktopInvasion &
    APP_PID=$!

    while inotifywait -r -e modify,create,delete ../src ../include ../qml; do
        echo -e "\n\033[1;33m========== HMR triggered at $(date '+%Y-%m-%d %H:%M') ==========\033[0m"

        kill $APP_PID 2>/dev/null || true
        ninja
        ./DesktopInvasion &
        APP_PID=$!
    done

fi
