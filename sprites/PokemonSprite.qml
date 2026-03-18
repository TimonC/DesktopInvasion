import QtQuick 2.15
import QtQuick.Controls 2.15
Item {
    id: root
    property string spriteSheet: "qrc:/assets/HGSS/PokGen1_transparent_reordered.png"
    property int row: 0
    property int direction: 0
    property real scaleFactor: 4
    property int spriteOffsetX: 0
    property int spriteOffsetY: 0
    property int itemWidth: 0
    property int itemHeight: 0
    property int frameWidth: 32
    property int frameHeight: 32
    property int frameCount: 2
    property int frameRate: 4

    property bool clickable:true
    property bool jumping: false
    property bool tackle: false
    property bool attacked: false

    width: itemWidth
    height: itemHeight
    clip: true
    layer.enabled: true
    z: 1

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
            switch (direction) {
                case 0: return 0;
                case 1: return frameWidth * frameCount;
                case 2: return frameWidth * frameCount * 2;
                case 3: return frameWidth * frameCount * 3;
            }
            return 0;
        }
        frameY: row * frameHeight
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

    SequentialAnimation {
        id: tackleAnim
        running: tackle
        loops: 1
        onStopped: tackle = false

        PropertyAnimation {
            target: sprite
            property: "x"
            to: (direction==1 ? spriteOffsetX-4*scaleFactor :
                 direction==3 ? spriteOffsetX+4*scaleFactor :
                 sprite.x)
            duration: 50
            easing.type: Easing.InQuad
        }

        PropertyAnimation {
            target: sprite
            property: "y"
            to: (direction==0 ? spriteOffsetY-4*scaleFactor :
                 direction==2 ? spriteOffsetY+4*scaleFactor :
                 sprite.y)
            duration: 50
            easing.type: Easing.InQuad
        }

        PropertyAnimation {
            target: sprite
            property: "x"
            to: spriteOffsetX
            duration: 100
            easing.type: Easing.OutQuad
        }

        PropertyAnimation {
            target: sprite
            property: "y"
            to: spriteOffsetY
            duration: 100
            easing.type: Easing.OutQuad
        }
    }

    SequentialAnimation {
        id: attackId
        running: attacked
        loops: 1
        onStopped: attacked = false

        PropertyAnimation {
            target: sprite
            property: "x"
            to: (direction==1 ? spriteOffsetX+1*scaleFactor :
                 direction==3 ? spriteOffsetX-1*scaleFactor :
                 sprite.x)
            duration: 100
            // easing.type: Easing.OutQuad
        }

        PropertyAnimation {
            target: sprite
            property: "y"
            to: (direction==0 ? spriteOffsetY+1*scaleFactor :
                 direction==2 ? spriteOffsetY-1*scaleFactor :
                 sprite.y)
            duration: 100
            // easing.type: Easing.OutQuad
        }

        PropertyAnimation {
            target: sprite
            property: "x"
            to: spriteOffsetX
            duration: 100
            easing.type: Easing.InQuad
        }

        PropertyAnimation {
            target: sprite
            property: "y"
            to: spriteOffsetY
            duration: 100
            easing.type: Easing.InQuad
        }
    }
    Rectangle {
        // optional visual debugging
        anchors.fill: parent
        color: "transparent"
        border.color: "yellow"
        border.width: 1
        // visible: false
    }
}

