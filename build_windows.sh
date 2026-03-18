#!/usr/bin/env bash
set -euo pipefail

# Convert icon if possible
if command -v convert &> /dev/null && [ -f "assets/icon/icon.png" ]; then
    echo "Converting icon.png to icon.ico..."
    convert assets/icon/icon.png -define icon:auto-resize=256,128,64,48,32,16 assets/icon/icon.ico
fi

# Debug: locate Qt6 CMake files
echo "=== Debug: Checking Qt6 CMake files ==="
ls -la /opt/Qt/6.8.0/win64_mingw/lib/cmake/Qt6 || echo "Directory not found!"
find /opt/Qt -name "Qt6Config.cmake" 2>/dev/null || echo "Qt6Config.cmake not found"
echo "=== End debug ==="

# Clean and create build directory
rm -rf build_windows
mkdir -p build_windows
cd build_windows

# Configure with CMake – using CMAKE_PREFIX_PATH instead of explicit Qt6_DIR
cmake -G "Unix Makefiles" \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=/opt/qt6-mingw-toolchain.cmake \
      -DCMAKE_PREFIX_PATH=/opt/Qt/6.8.0/win64_mingw \
      -DQT_HOST_PATH=/opt/Qt/6.8.0/linux_gcc_64 \
      ..

# Build (using all available cores)
make -j$(nproc)

# Prepare package directory
PACKAGE_DIR="../output/DesktopInvasion-Windows"
rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR"
cp DesktopInvasion.exe "$PACKAGE_DIR/"

# Run windeployqt to gather Qt DLLs
echo "Running windeployqt..."
wine /opt/Qt/6.8.0/win64_mingw/bin/windeployqt.exe --release --qmldir ../qml "$PACKAGE_DIR/DesktopInvasion.exe"

# Copy MinGW runtime DLLs (required for running on Windows without MinGW installed)
echo "Copying MinGW runtime DLLs..."
MINGW_DLL_DIR="/usr/x86_64-w64-mingw32/lib"
if [ -d "$MINGW_DLL_DIR" ]; then
    cp "$MINGW_DLL_DIR"/libstdc++-6.dll "$PACKAGE_DIR/" 2>/dev/null || echo "libstdc++-6.dll not found"
    cp "$MINGW_DLL_DIR"/libgcc_s_seh-1.dll "$PACKAGE_DIR/" 2>/dev/null || echo "libgcc_s_seh-1.dll not found"
    cp "$MINGW_DLL_DIR"/libwinpthread-1.dll "$PACKAGE_DIR/" 2>/dev/null || echo "libwinpthread-1.dll not found"
else
    echo "MinGW DLL directory not found at $MINGW_DLL_DIR; skipping runtime DLL copy."
fi

# Create README
cat > "$PACKAGE_DIR/README.txt" <<EOF
Desktop Invasion - Windows Version
To play: Double-click DesktopInvasion.exe
EOF

# Create zip archive
cd ../output
rm -f DesktopInvasion-Windows.zip
zip -r DesktopInvasion-Windows.zip DesktopInvasion-Windows/

echo "Package created: output/DesktopInvasion-Windows.zip"
