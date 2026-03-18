import QtQuick 2.15
import QtQuick.Controls 2.15
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
    property alias battleButton: battleButton

    property bool openingButtons: false
    property bool clickable:true
    property bool jumping: false

    width: itemWidth
    height: itemHeight
    clip: true
    layer.enabled: true
    z: 1

signal mouseDrag(int deltaX, int deltaY)

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: clickable
    }


    RoundButton {
            id: battleButton
            text: "BATTLE"
            enabled: root.openingButtons
            visible: root.openingButtons
            z:8000
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
        z: 1000
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

