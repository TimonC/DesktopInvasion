import QtQuick 2.15

Item {
    id: root
    // Top-level configuration
    property real scaleFactor: 2
    property int frameWidth: 16
    property int frameHeight: 23
    property int throwDuration: 600     // Duration of frames 0-7 to halfway
    property int pauseDuration: 200      // Pause after first animation
    property int catchDuration: 300     // Duration to show frame 0 before drop
    property int dropDuration: 400       // Duration for final drop
    property int bounceUpDuration: 150   // Duration for bounce up
    property int bounceDownDuration: 150 // Duration for bounce down
    property int bounceHeight: 10        // How high to bounce (in pixels)

    // Circle properties
    property real circleShrinkScale: 0.7    // Circle starts at 0.8x size
    property int circleAnimationDuration: 1000
    property int circleX: 0               // X position for circle center
    property int circleY: 0               // Y position for circle center
    property int circleBaseRadius: 0       // Base circle radius

    // Signal emitted when throw animation completes
    signal throwAnimationDone()
    signal pokemonInsideBall()

    width: frameWidth
    height: frameHeight
    z: 100

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

    // Shake the ball at its current position
    function shake() {
        shakeAnimation.start()
    }

    // Make the ball jump
    function jump() {
        jumpAnimation.start()
    }

    function release() {
        pokeballSprite.sourceClipRect.x = root.frameWidth * 9
        circleExpand() // Circle expands on release
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

    // White circle overlay for animations
    Rectangle {
        id: circleOverlay
        color: "white"
        radius: width / 2
        opacity: 0
        visible: false
        z: 99 // Below the pokeball sprite

        // Position at configured circle center
        x: root.circleX - width / 2
        y: root.circleY - height / 2
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

    // Shake animation
    SequentialAnimation {
        id: shakeAnimation
        running: false
        loops: 1

        PropertyAnimation { target: pokeballSprite; property: "x"; to: pokeballSprite.x + 3; duration: 50 }
        PropertyAnimation { target: pokeballSprite; property: "x"; to: pokeballSprite.x - 3; duration: 50 }
        PropertyAnimation { target: pokeballSprite; property: "x"; to: pokeballSprite.x + 2; duration: 50 }
        PropertyAnimation { target: pokeballSprite; property: "x"; to: pokeballSprite.x - 2; duration: 50 }
        PropertyAnimation { target: pokeballSprite; property: "x"; to: pokeballSprite.x + 1; duration: 50 }
        PropertyAnimation { target: pokeballSprite; property: "x"; to: pokeballSprite.x - 1; duration: 50 }
        PropertyAnimation { target: pokeballSprite; property: "x"; to: pokeballSprite.x; duration: 50 }
    }

    // Jump animation
    SequentialAnimation {
        id: jumpAnimation
        running: false
        loops: 1

        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: pokeballSprite.y - 20
            duration: 200
            easing.type: Easing.OutQuad
        }
        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: pokeballSprite.y
            duration: 200
            easing.type: Easing.InQuad
        }
    }

    // Single animation that can run forward or reverse for circle
    ParallelAnimation {
        id: circleAnimation
        running: false
        loops: 1

        // Circle size animation
        NumberAnimation {
            id: sizeAnim
            target: circleOverlay
            property: "width"
            duration: root.circleAnimationDuration
            easing.type: Easing.InOutQuad
        }

        NumberAnimation {
            id: heightAnim
            target: circleOverlay
            property: "height"
            duration: root.circleAnimationDuration
            easing.type: Easing.InOutQuad
        }

        // Position animation to keep centered
        NumberAnimation {
            id: xAnim
            target: circleOverlay
            property: "x"
            duration: root.circleAnimationDuration
            easing.type: Easing.InOutQuad
        }

        NumberAnimation {
            id: yAnim
            target: circleOverlay
            property: "y"
            duration: root.circleAnimationDuration
            easing.type: Easing.InOutQuad
        }

        // Opacity animation
        NumberAnimation {
            id: opacityAnim
            target: circleOverlay
            property: "opacity"
            duration: root.circleAnimationDuration
            easing.type: Easing.InOutQuad
        }

        onStarted: {
            circleOverlay.visible = true;
        }

        onStopped: {
            circleOverlay.visible = false;
        }
    }


    // Circle expands outward (for pokemon release)
    function circleExpand() {
        // Setup animation: start at startScale, expand to endScale
        var start =2 * root.circleBaseRadius * root.circleShrinkScale
        var end = 2 * root.circleBaseRadius;

        sizeAnim.from = start;
        sizeAnim.to = end;

        heightAnim.from = start;
        heightAnim.to = end;

        xAnim.from = root.circleX - start / 2;
        xAnim.to = root.circleX - end / 2;

        yAnim.from = root.circleY - start / 2;
        yAnim.to = root.circleY - end / 2;

        opacityAnim.from = 1;
        opacityAnim.to = 0.8;

        circleAnimation.start();
    }

    // Circle shrinks inward (for pokemon capture)
    function circleShrink() {
        // Setup animation: start at endScale, shrink to startScale
        var start = 2 * root.circleBaseRadius;
        var end = 2 * root.circleBaseRadius * root.circleShrinkScale;

        sizeAnim.from = start;
        sizeAnim.to = end;

        heightAnim.from = start;
        heightAnim.to = end;

        xAnim.from = root.circleX - start / 2;
        xAnim.to = root.circleX - end / 2;

        yAnim.from = root.circleY - start / 2;
        yAnim.to = root.circleY - end / 2;

        opacityAnim.from = 0.8;
        opacityAnim.to = 1;

        circleAnimation.start();
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
            duration: root.catchDuration
        }

        // Frame 9 - trigger circle shrink for capture
        PropertyAction {
            target: pokeballSprite
            property: "sourceClipRect.x"
            value: root.frameWidth * 9
        }
        ScriptAction{
            script: {
                circleShrink() // Circle shrinks on capture
                root.pokemonInsideBall()
            }
        }
        PauseAnimation {
            duration: root.catchDuration
        }

        // Frame 0
        PropertyAction {
            target: pokeballSprite
            property: "sourceClipRect.x"
            value: 0
        }
        PauseAnimation {
            duration: root.catchDuration
        }

        // === PHASE 4: Drop to final position ===
        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: root.y1
            duration: root.dropDuration
            easing.type: Easing.InQuad
        }

        // === PHASE 5: Three bounces at the end ===
        // First bounce (full height)
        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: root.y1 - root.bounceHeight
            duration: root.bounceUpDuration
            easing.type: Easing.OutQuad
        }
        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: root.y1
            duration: root.bounceDownDuration
            easing.type: Easing.InQuad
        }

        // Second bounce (2/3 height)
        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: root.y1 - (root.bounceHeight * 2 / 3)
            duration: root.bounceUpDuration
            easing.type: Easing.OutQuad
        }
        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: root.y1
            duration: root.bounceDownDuration
            easing.type: Easing.InQuad
        }

        // Third bounce (1/3 height)
        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: root.y1 - (root.bounceHeight / 3)
            duration: root.bounceUpDuration
            easing.type: Easing.OutQuad
        }
        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: root.y1
            duration: root.bounceDownDuration
            easing.type: Easing.InQuad
        }

        // Emit signal when done
        ScriptAction {
            script: root.throwAnimationDone()
        }
    }
}
