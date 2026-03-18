import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle{
    id: root
    property color textColor
    property int fontSize: 0
    property int row: 35
    property double iconScale: 8
    property string fontFamily: "Sans Serif"
    property bool outline: true

    Layout.fillWidth: true
    Layout.fillHeight: true

    RowLayout{
        anchors.fill: parent
        spacing: 0
        Image {
            id: trainerFrame
            source: "qrc:/assets/HGSS/reordered_trainers.png"

            property int frameIndex: root.row
            readonly property int spriteWidth: 32
            readonly property int spriteHeight: 32
            width: Math.ceil(spriteWidth*root.iconScale)
            height: Math.ceil(spriteHeight*root.iconScale)

            sourceClipRect: Qt.rect(0, frameIndex * spriteHeight, spriteWidth, spriteHeight)

            smooth: false
            antialiasing: false
        }
        Text{
            text: "Player"
            font.pixelSize: root.fontSize
            font.family: root.fontFamily
            color: root.textColor
        }
    }
}
