#!/bin/bash
set -e
if [ -f "CMakeLists_dev.txt" ]; then
    cp CMakeLists_dev.txt CMakeLists.txt
fi
mkdir -p build_linux
cd build_linux
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja
mkdir -p /app/build/AppDir/usr/bin
mkdir -p /app/build/AppDir/usr/share/applications
cp bin/DesktopInvasion /app/build/AppDir/usr/bin/
cp -r ../assets /app/build/AppDir/usr/bin/
cp -r ../qml /app/build/AppDir/usr/bin/
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

cd /app/build
mkdir -p AppDir/usr/plugins/sqldrivers/
cp /opt/Qt/6.8.0/gcc_64/plugins/sqldrivers/libqsqlite.so AppDir/usr/plugins/sqldrivers/
rm -f /opt/Qt/6.8.0/gcc_64/plugins/sqldrivers/libqsqlmimer.so 2>/dev/null #remove problematic drivers

export QMAKE=/opt/Qt/6.8.0/gcc_64/bin/qmake
export QML_SOURCES_PATHS=/app/qml
/opt/linuxdeploy/AppRun \
    --appdir AppDir \
    --executable AppDir/usr/bin/DesktopInvasion \
    --desktop-file AppDir/usr/share/applications/DesktopInvasion.desktop \
    --icon-file /app/src/assets/icon/icon.png \
    --plugin qt \
    --output appimage

mkdir -p /app/output
cp *.AppImage /app/output/
mv /app/output/Desktop_Invasion-x86_64.AppImage /app/output/DesktopInvasion.AppImage 2>/dev/null || true
