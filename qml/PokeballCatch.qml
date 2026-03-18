import QtQuick 2.15
Item {
    id: root
    // Top-level configuration
    property real scaleFactor: 2
    property int frameWidth: 16
    property int frameHeight: 23
    property int throwDuration: 1000     // Duration of frames 0-7 to halfway
    property int pauseDuration: 200      // Pause after first animation
    property int openDuration: 300     // Duration to show frame 8
    property int dropDuration: 400       // Duration for final drop
    property int bounceUpDuration: 150   // Duration for bounce up
    property int bounceDownDuration: 150 // Duration for bounce down
    property int bounceHeight: 10        // How high to bounce (in pixels)
    width: frameWidth
    height: frameHeight
    z: 9999

    property int x0: 0
    property int x1: 0
    property int y0: 0
    property int y1: 0

    // Method to start the animation
    function throwAt(startX, endX, topY, bottomY) {
        x0 = startX
        x1 = endX
        y0 = topY
        y1 = bottomY
        throwPokeball.start()
    }

    // Single image with manual frame control
    Image {
        id: pokeballSprite
        scale: root.scaleFactor
        visible: false
        z: 100
        source: "qrc:/assets/HGSS/Pokeballs_transparent_reordered.png"
        sourceClipRect: Qt.rect(0, root.frameHeight * 3, root.frameWidth, root.frameHeight)
        smooth: false
        antialiasing: false
    }

    // Timer to animate frames 0-7
    Timer {
        id: frameTimer
        interval: root.throwDuration / 8
        repeat: true
        property int frameIndex: 0
        onTriggered: {
            pokeballSprite.sourceClipRect.x = frameIndex * root.frameWidth
            frameIndex++
            if (frameIndex >= 8) {
                stop()
            }
        }
    }

    // Pokéball throw animation
    SequentialAnimation {
        id: throwPokeball
        running: false
        loops: 1

        // === PHASE 1: Arc animation (frames 0-7) ===
        PropertyAction {
            target: pokeballSprite
            property: "visible"
            value: true
        }
        PropertyAction {
            target: pokeballSprite
            property: "sourceClipRect.x"
            value: 0
        }
        PropertyAction {
            target: pokeballSprite
            property: "x"
            value: root.x0
        }
        PropertyAction {
            target: pokeballSprite
            property: "y"
            value: root.y0 + 32
        }
        ScriptAction {
            script: {
                frameTimer.frameIndex = 0
                frameTimer.start()
            }
        }
        // First half: go up to y0
        ParallelAnimation {
            PropertyAnimation {
                target: pokeballSprite
                property: "x"
                to: root.x0 + (root.x1 - root.x0) / 2
                duration: root.throwDuration / 2
                easing.type: Easing.Linear
            }
            PropertyAnimation {
                target: pokeballSprite
                property: "y"
                to: root.y0
                duration: root.throwDuration / 2
                easing.type: Easing.OutQuad
            }
        }
        // Second half: go back down to y0+32
        ParallelAnimation {
            PropertyAnimation {
                target: pokeballSprite
                property: "x"
                to: root.x1
                duration: root.throwDuration / 2
                easing.type: Easing.Linear
            }
            PropertyAnimation {
                target: pokeballSprite
                property: "y"
                to: root.y0 + 32
                duration: root.throwDuration / 2
                easing.type: Easing.InQuad
            }
        }
        ScriptAction {
            script: frameTimer.stop()
        }

        // === PHASE 2: Brief pause ===
        PauseAnimation {
            duration: root.pauseDuration
        }

        // === PHASE 3: Manually show frames 8-9-0 ===
        // Frame 8
        PropertyAction {
            target: pokeballSprite
            property: "sourceClipRect.x"
            value: root.frameWidth * 8
        }
        PauseAnimation {
            duration: root.openDuration
        }

        // Frame 9
        PropertyAction {
            target: pokeballSprite
            property: "sourceClipRect.x"
            value: root.frameWidth * 9
        }
        PauseAnimation {
            duration: root.openDuration
        }

        // Frame 0
        PropertyAction {
            target: pokeballSprite
            property: "sourceClipRect.x"
            value: 0
        }
        PauseAnimation {
            duration: root.openDuration
        }

        // === PHASE 4: Drop to final position ===
        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: root.y1
            duration: root.dropDuration
            easing.type: Easing.InQuad
        }

        // === PHASE 5: Little bounce at the end ===
        // Bounce up
        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: root.y1 - root.bounceHeight
            duration: root.bounceUpDuration
            easing.type: Easing.OutQuad
        }
        // Bounce down
        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: root.y1
            duration: root.bounceDownDuration
            easing.type: Easing.InQuad
        }
    }
}
