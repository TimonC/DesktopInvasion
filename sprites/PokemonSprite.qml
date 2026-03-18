import QtQuick 2.15

Item {
    id: root

    // Sprite properties
    property string spriteSheet: "qrc:/assets/HGSS/PokGen1_transparent_reordered.png"
    property int row: 0
    property int direction: 0
    property real scaleFactor: 4
    property int frameWidth: 32
    property int frameHeight: 32
    property int frameCount: 2
    property int frameRate: 4

    // Animation control
    property bool tackle: false
    property bool attacked: false

    width: frameWidth * scaleFactor
    height: frameHeight * scaleFactor

    AnimatedSprite {
        id: sprite
        anchors.centerIn: parent
        scale: scaleFactor

        source: root.spriteSheet
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

    // Tackle animation
    SequentialAnimation {
        id: tackleAnim
        running: tackle
        loops: 1
        onStopped: tackle = false

        PropertyAnimation {
            target: sprite
            property: "x"
            to: (direction==1 ? -6*scaleFactor :
                 direction==3 ? 6*scaleFactor :
                 0)
            duration: 50
            easing.type: Easing.InQuad
        }

        PropertyAnimation {
            target: sprite
            property: "y"
            to: (direction==0 ? -6*scaleFactor :
                 direction==2 ? 6*scaleFactor :
                 0)
            duration: 50
            easing.type: Easing.InQuad
        }

        PropertyAnimation {
            target: sprite
            property: "x"
            to: 0
            duration: 100
            easing.type: Easing.OutQuad
        }

        PropertyAnimation {
            target: sprite
            property: "y"
            to: 0
            duration: 100
            easing.type: Easing.OutQuad
        }
    }

    // Attack animation
    SequentialAnimation {
        id: attackId
        running: attacked
        loops: 1
        onStopped: attacked = false

        PropertyAnimation {
            target: sprite
            property: "x"
            to: (direction==1 ? 2.5*scaleFactor :
                 direction==3 ? -2.5*scaleFactor :
                 0)
            duration: 100
        }

        PropertyAnimation {
            target: sprite
            property: "y"
            to: (direction==0 ? 2.5*scaleFactor :
                 direction==2 ? -2.5*scaleFactor :
                 0)
            duration: 100
        }

        PropertyAnimation {
            target: sprite
            property: "x"
            to: 0
            duration: 100
            easing.type: Easing.InQuad
        }

        PropertyAnimation {
            target: sprite
            property: "y"
            to: 0
            duration: 100
            easing.type: Easing.InQuad
        }
    }
}
