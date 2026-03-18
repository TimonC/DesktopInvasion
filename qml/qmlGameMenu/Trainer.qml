import QtQuick 2.15

// Trainer strip.
// Menu allocates a fixed Item (pcW × trainerH) and sets anchors.fill on this
// component.  All content is centered within whatever space is given.
Rectangle {
    id: root
    color: "transparent"

    // ── Props passed in by Menu ────────────────────────────────────────────────
    property color  textColor:  "white"
    property int    fontSize:   16
    property int    row:        35
    property double iconScale:  3
    property string fontFamily: "Sans Serif"

    property int frameSize: 32

    // ── Content – centered inside the allocated area ───────────────────────────
    Row {
        anchors.centerIn: parent
        spacing:          root.frameSize

        Image {
            id: trainerFrame
            anchors.verticalCenter: parent.verticalCenter

            property int frameIndex: root.row
            readonly property int spriteWidth:  root.frameSize
            readonly property int spriteHeight: root.frameSize

            width:  Math.ceil(spriteWidth  * root.iconScale)
            height: Math.ceil(spriteHeight * root.iconScale)

            source:         "qrc:/assets/HGSS/reordered_trainers.png"
            sourceClipRect: Qt.rect(0, frameIndex * spriteHeight, spriteWidth, spriteHeight)
            smooth:         false
            antialiasing:   false
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text:           "Player"
            font.pixelSize: root.fontSize
            font.family:    root.fontFamily
            color:          root.textColor
        }
    }
}
