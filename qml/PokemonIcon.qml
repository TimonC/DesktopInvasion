import QtQuick 2.15

Image {
    id: iconFrame
    source: "qrc:/assets/HGSS/reordered_icons.png"

    property int frameIndex: 0
    property double iconScale: 1.0

    readonly property int spriteWidth: 40
    readonly property int spriteHeight: 30

    width: Math.ceil(spriteWidth * iconScale)
    height: Math.ceil(spriteHeight * iconScale)

    sourceClipRect: Qt.rect(0, frameIndex * spriteHeight, spriteWidth, spriteHeight)

    smooth: false
    antialiasing: false
}
