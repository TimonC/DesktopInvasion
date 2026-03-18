#!/bin/bash
set -e

echo "=== Building Release ==="
mkdir -p build_release
cd build_release
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja

echo "=== Creating AppDir ==="
# Create AppDir in /app/build
mkdir -p /app/build/AppDir/usr/bin
mkdir -p /app/build/AppDir/usr/share/applications
mkdir -p /app/build/AppDir/usr/share/icons/hicolor/512x512/apps

# Copy executable
cp DesktopInvasion /app/build/AppDir/usr/bin/

# Copy assets and qml
cp -r ../assets /app/build/AppDir/usr/bin/
cp -r ../qml /app/build/AppDir/usr/bin/

echo "Assets copied:"
ls -la /app/build/AppDir/usr/bin/assets/
echo ""
echo "QML copied:"
ls -la /app/build/AppDir/usr/bin/qml/

# Create desktop entry
cat > /app/build/AppDir/usr/share/applications/DesktopInvasion.desktop <<EOF
[Desktop Entry]
Type=Application
Name=Desktop Invasion
Comment=You can't escape the invasion, no matter where you Alt-Tab!
Exec=DesktopInvasion
Icon=desktop-invasion
Categories=Game;
Terminal=false
EOF

# Copy icon
if [ -f "../assets/icon/icon.png" ]; then
    cp ../assets/icon/icon.png /app/build/AppDir/usr/share/icons/hicolor/512x512/apps/desktop-invasion.png
fi

cd /app/build

echo "=== Setting up SQLite plugin ==="
# Create plugin directory
mkdir -p AppDir/usr/plugins/sqldrivers/

# Copy SQLite plugin ONLY
cp /opt/Qt/6.8.0/gcc_64/plugins/sqldrivers/libqsqlite.so AppDir/usr/plugins/sqldrivers/

# Remove any problematic plugins that might cause issues
rm -f /opt/Qt/6.8.0/gcc_64/plugins/sqldrivers/libqsqlmimer.so 2>/dev/null || true

echo "=== Running linuxdeploy ==="
export QMAKE=/opt/Qt/6.8.0/gcc_64/bin/qmake
export QML_SOURCES_PATHS=/app/qml

/opt/linuxdeploy/AppRun \
    --appdir AppDir \
    --executable AppDir/usr/bin/DesktopInvasion \
    --desktop-file AppDir/usr/share/applications/DesktopInvasion.desktop \
    --icon-file AppDir/usr/share/icons/hicolor/512x512/apps/desktop-invasion.png \
    --plugin qt \
    --output appimage

echo "=== AppImage created ==="
ls -lh *.AppImage

echo ""
echo "=== Copying outputs to volume ==="
# Ensure output directory exists
mkdir -p /app/output

# Copy AppImage to output volume
cp *.AppImage /app/output/

# Also copy the AppDir in case you want to inspect it
echo "Copying AppDir for inspection..."
cp -r AppDir /app/output/

# Create a build info file
cat > /app/output/BUILD_INFO.txt <<EOF
Build Date: $(date)
Build Type: Release
AppImage: $(ls *.AppImage)
Database Location (Runtime):
  - Development: ./pokemon.db (in build directory)
  - AppImage: ~/.local/share/DesktopInvasion/DesktopInvasion/pokemon.db

To run:
  chmod +x /app/output/*.AppImage
  ./app/output/*.AppImage

Database will be automatically created at:
  ~/.local/share/DesktopInvasion/DesktopInvasion/pokemon.db

To view save data location:
  ls -la ~/.local/share/DesktopInvasion/DesktopInvasion/
EOF

echo "=== Build complete ==="
echo "Output files available in ./app/output/"
mv /app/output/Desktop_Invasion-x86_64.AppImage /app/output/DesktopInvasion.AppImage
ls -lah /app/output/
echo ""
echo ""
echo "Database will be created at: ~/.local/share/DesktopInvasion/DesktopInvasion/pokemon.db"
