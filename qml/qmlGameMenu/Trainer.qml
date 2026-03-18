import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: "transparent"

    property color  textColor:  "white"
    property int    fontSize:   16
    property int    row:        35
    property double iconScale:  3
    property string fontFamily: "Sans Serif"

    property int frameSize: 32

    Layout.fillWidth:  true
    Layout.fillHeight: true

    RowLayout {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left:           parent.left
        anchors.leftMargin:     root.frameSize
        spacing:                root.frameSize

        Image {
            id: trainerFrame
            scale:  root.iconScale
            source: "qrc:/assets/HGSS/reordered_trainers.png"

            property int frameIndex: root.row
            readonly property int spriteWidth:  root.frameSize
            readonly property int spriteHeight: root.frameSize

            width:  Math.ceil(spriteWidth  * root.iconScale)
            height: Math.ceil(spriteHeight * root.iconScale)

            sourceClipRect: Qt.rect(0, frameIndex * spriteHeight, spriteWidth, spriteHeight)
            smooth:       false
            antialiasing: false
        }

        Text {
            text:           "Player"
            font.pixelSize: root.fontSize
            font.family:    root.fontFamily
            color:          root.textColor
        }
    }
}
