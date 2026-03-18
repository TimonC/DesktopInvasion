#!/bin/bash
set -e

: "${XDG_RUNTIME_DIR:=/tmp/xdg}"
mkdir -p "$XDG_RUNTIME_DIR"
chmod 0700 "$XDG_RUNTIME_DIR"

if [ ! -f "/app/data/.initialized" ]; then
    echo "Initializing volume"
    rm -rf /app/data/* 2>/dev/null || true
    touch /app/data/.initialized
fi

mkdir -p build
cd build
cmake -G Ninja ..
ninja

echo "Starting DesktopInvasion with HMR"
./DesktopInvasion &
APP_PID=$!

LAST_BUILD=$(date +%s)

while inotifywait -r -e modify,create,delete ../src ../include ../qml ../assets ../resources.qrc; do
    echo -e "\n\033[1;33m========== HMR triggered ==========\033[0m"

    kill $APP_PID 2>/dev/null || true

    CHANGED_FILES=$(find ../qml ../assets -name "*.qml" -o -name "*.js" -o -name "*.png" -o -name "*.ttf" -newer /tmp/last_build 2>/dev/null | head -5)

    if [ -n "$CHANGED_FILES" ]; then
        echo "QML/assets changed: $(echo "$CHANGED_FILES" | tr '\n' ' ')"
        # Touch resources.qrc to trigger rcc rebuild
        touch ../resources.qrc
    fi

    ninja

    touch /tmp/last_build

    ./DesktopInvasion &
    APP_PID=$!
done
