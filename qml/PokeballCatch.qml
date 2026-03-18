import QtQuick 2.15

Item {
    id: root

    // Top-level configuration
    property real scaleFactor: 2
    property int frameWidth: 16
    property int frameHeight: 23
    property int throwDuration: 1000     // Duration of the throw motion
    property int landingFrameDuration: 500  // Duration to show frame 9 before returning to frame 0

    width: frameWidth
    height: frameHeight
    z: 9999

    property int spriteCenter: 0
    property int spriteEndY: 0

    // Method to start the animation
    function throwAt(centerX, targetY) {
        spriteCenter = centerX
        spriteEndY = targetY
        throwPokeball.start()
    }

    AnimatedSprite {
        id: pokeballSprite
        scale: root.scaleFactor
        running: false
        visible: false
        z: 100

        source: "qrc:/assets/HGSS/Pokeballs_transparent_reordered.png"
        frameWidth: root.frameWidth
        frameHeight: root.frameHeight
        frameCount: 8
        frameRate: 8000 / root.throwDuration  // 8 frames over throwDuration
        frameX: 0
        frameY: root.frameHeight * 3
        loops: 1

        interpolate: false
        smooth: false
        antialiasing: false
    }

    Image {
        id: staticFrame
        scale: root.scaleFactor
        visible: false
        z: 100

        source: "qrc:/assets/HGSS/Pokeballs_transparent_reordered.png"
        sourceClipRect: Qt.rect(0, root.frameHeight * 3, root.frameWidth, root.frameHeight)

        smooth: false
        antialiasing: false
    }

    // Pokéball throw animation
    SequentialAnimation {
        id: throwPokeball
        running: false
        loops: 1

        PropertyAction {
            target: pokeballSprite
            property: "visible"
            value: true
        }

        PropertyAction {
            target: pokeballSprite
            property: "currentFrame"
            value: 0
        }

        PropertyAction {
            target: pokeballSprite
            property: "x"
            value: root.spriteCenter
        }

        PropertyAction {
            target: pokeballSprite
            property: "y"
            value: root.spriteEndY - 32*2 - root.frameHeight
        }

        PropertyAction {
            target: pokeballSprite
            property: "running"
            value: true
        }

        // Throw motion
        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: root.spriteEndY - root.frameHeight
            duration: root.throwDuration
            easing.type: Easing.InQuad
        }

        // Hide animated sprite
        PropertyAction {
            target: pokeballSprite
            property: "visible"
            value: false
        }

        PropertyAction {
            target: pokeballSprite
            property: "running"
            value: false
        }

        // Position static frame at same location
        PropertyAction {
            target: staticFrame
            property: "x"
            value: root.spriteCenter
        }

        PropertyAction {
            target: staticFrame
            property: "y"
            value: root.spriteEndY - root.frameHeight
        }

        // Show frame 9
        PropertyAction {
            target: staticFrame
            property: "sourceClipRect.x"
            value: root.frameWidth * 9
        }

        PropertyAction {
            target: staticFrame
            property: "visible"
            value: true
        }

        // Wait for the specified duration
        PauseAnimation {
            duration: root.landingFrameDuration
        }

        // Show frame 0
        PropertyAction {
            target: staticFrame
            property: "sourceClipRect.x"
            value: 0
        }

        // Brief pause to show frame 0
        PauseAnimation {
            duration: 100
        }

    }
}
