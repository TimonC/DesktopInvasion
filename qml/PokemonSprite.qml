import QtQuick 2.15
import "spriteAnimations/actions" as Actions
import "spriteAnimations/responses" as Responses

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

    // Random delay timer to prevent sync
    property Timer startTimer: Timer {
        interval: Math.random() * 125
        running: true
        onTriggered: sprite.running = true
    }

    AnimatedSprite {
        id: sprite
        anchors.centerIn: parent
        scale: scaleFactor

        running: false // Don't start immediately - wait for timer
        source: root.spriteSheet
        frameWidth: root.frameWidth
        frameHeight: root.frameHeight
        frameCount: root.frameCount
        frameRate: root.frameRate
        currentFrame: Math.random() < 0.5 ? 0 : 1
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

    Actions.MoveForward {
        id: attackAnim
        target: sprite
        direction: root.direction
        scaleFactor: root.scaleFactor
        running: root.tackle
        onStopped: root.tackle = false
    }

    Responses.MoveBack {
        id: attackedAnim
        target: sprite
        direction: root.direction
        scaleFactor: root.scaleFactor
        running: root.attacked
        onStopped: root.attacked = false
    }
}
