import QtQuick 2.15

Image {
    id: iconFrame
    source: "qrc:/assets/HGSS/reordered_icons.png"

    property int frameIndex: 0
    property real iconScale: 1.0

    readonly property int spriteWidth: 40
    readonly property int spriteHeight: 30

    width: spriteWidth * iconScale
    height: spriteHeight * iconScale

    sourceClipRect: Qt.rect(0, frameIndex * spriteHeight, spriteWidth, spriteHeight)

    smooth: false
    antialiasing: false


    function scaledWidth() {
        return spriteWidth * iconScale
    }

    function scaledHeight() {
        return spriteHeight * iconScale
    }
}
