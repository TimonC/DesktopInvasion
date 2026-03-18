#!/bin/bash
set -e

mkdir -p build_release
cd build_release
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja

# Create AppDir
mkdir -p /app/output/AppDir/usr/bin
mkdir -p /app/output/AppDir/usr/share/applications
mkdir -p /app/output/AppDir/usr/share/icons/hicolor/16x16/apps

cp DesktopInvasion /app/output/AppDir/usr/bin/
cp -r ../assets /app/output/AppDir/usr/bin/
cp -r ../qml /app/output/AppDir/usr/bin/

cat > /app/output/AppDir/usr/share/applications/DesktopInvasion.desktop <<EOF
[Desktop Entry]
Type=Application
Name=Desktop Invasion
Comment=You can't escape the invasion, no matter where you Alt-Tab!
Exec=DesktopInvasion
Icon=desktop-invasion
Categories=Game;
Terminal=false
EOF

if [ -f "../assets/HGSS/PokeballIcon.png" ]; then
    cp ../assets/HGSS/PokeballIcon.png /app/output/AppDir/usr/share/icons/hicolor/16x16/apps/desktop-invasion.png
fi

cd /app/output

# Clean previous AppDir libs
rm -rf AppDir/usr/lib/* 2>/dev/null || true

# Explicitly copy the SQLite plugin we need
mkdir -p AppDir/usr/plugins/sqldrivers/
cp /opt/Qt/6.8.0/gcc_64/plugins/sqldrivers/libqsqlite.so AppDir/usr/plugins/sqldrivers/

# Remove any problematic plugins that might cause issues
rm -f /opt/Qt/6.8.0/gcc_64/plugins/sqldrivers/libqsqlmimer.so 2>/dev/null || true

# Run linuxdeploy with explicit Qt installation path
export QMAKE=/opt/Qt/6.8.0/gcc_64/bin/qmake
export QML_SOURCES_PATHS=/app/qml

/opt/linuxdeploy/AppRun \
    --appdir AppDir \
    --executable AppDir/usr/bin/DesktopInvasion \
    --desktop-file AppDir/usr/share/applications/DesktopInvasion.desktop \
    --icon-file AppDir/usr/share/icons/hicolor/16x16/apps/desktop-invasion.png \
    --plugin qt \
    --output appimage

echo "=== AppImage created ==="
ls -la *.AppImage
echo ""
echo "Users can run: chmod +x DesktopInvasion*.AppImage && ./DesktopInvasion*.AppImage"
