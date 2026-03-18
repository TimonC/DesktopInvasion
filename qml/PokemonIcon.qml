import QtQuick 2.15

Image {
    id: iconFrame
    source: "qrc:/assets/HGSS/reordered_icons.png"

    sourceClipRect: Qt.rect(0, frameIndex * 30, 40, 30)

    property int frameIndex: 0

    width: 40
    height: 30

    smooth: false
    antialiasing: false
}
