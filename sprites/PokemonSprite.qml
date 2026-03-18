import QtQuick 2.15

Item {
    id: root
    property string spriteSheet: "qrc:/assets/HGSS/PokGen1_transparent_reordered.png"
    property int row: 0
    property int animation: 0
    property real scaleFactor: 4
    property int spriteOffsetX: 0
    property int spriteOffsetY: 0
    property int itemWidth: 32
    property int itemHeight: 32
    property int frameWidth: 32
    property int frameHeight: 32
    property int frameCount: 2
    property int frameRate: 4
    property alias mouseArea: mouseArea
    property bool jumping: false
    property bool openingButtons: false

    width: itemWidth
    height: itemHeight
    clip: true

    MouseArea {
        id: mouseArea
        anchors.fill: parent
    }

    Loader {
        id: openingButtonsLoader
        source: "qrc:/sprites/OpeningButtons.qml"
        asynchronous: true
        active: true
        z: 9999

        onItemChanged: {
            item.visible = Qt.binding(() => root.openingButtons);
            item.enabled = Qt.binding(() => root.openingButtons);
        }
    }

    SequentialAnimation {
        id: jumpAnim
        running: jumping
        loops: 1
        onStopped: jumping = false

        PropertyAnimation {
            target: sprite
            property: "y"
            to: spriteOffsetY - 5 * scaleFactor
            duration: 150
            easing.type: Easing.OutQuad
        }
        PropertyAnimation {
            target: sprite
            property: "y"
            to: spriteOffsetY
            duration: 150
            easing.type: Easing.InQuad
        }
    }

    AnimatedSprite {
        id: sprite
        x: spriteOffsetX
        y: spriteOffsetY
        z: 9997
        scale: scaleFactor

        source: spriteSheet
        frameWidth: root.frameWidth
        frameHeight: root.frameHeight
        frameCount: root.frameCount
        frameRate: root.frameRate
        interpolate: false
        smooth: false
        antialiasing: false

        frameX: {
            switch (animation) {
                case 0: return 0;
                case 1: return frameWidth * frameCount;
                case 2: return frameWidth * frameCount * 2;
                case 3: return frameWidth * frameCount * 3;
            }
            return 0;
        }
        frameY: row * frameHeight
    }
}

