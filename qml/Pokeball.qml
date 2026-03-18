import QtQuick 2.15

Item {
    id: root
    // Top-level configuration
    property int rowId: 0
    property real scaleFactor: 2
    property int frameWidth: 16
    property int frameHeight: 23
    property int throwDuration: 600     // Duration of frames 0-7 to halfway
    property int pauseDuration: 250      // Pause after first animation
    property int catchDuration: 500     // Duration to show frame 0 before drop
    property int dropDuration: 400       // Duration for final drop
    property int bounceUpDuration: 150   // Duration for bounce up
    property int bounceDownDuration: 150 // Duration for bounce down
    property int bounceHeight: 10        // How high to bounce (in pixels)

    // Ellipse properties (changed from circle)
    property real circleShrinkScale: 0.6    // Ellipse starts at 0.6x size
    property int circleAnimationDuration: 750
    property int circleX: 0               // X position for ellipse center
    property int circleY: 0               // Y position for ellipse center
    property int circleBaseWidth: 0       // Base ellipse width
    property int circleBaseHeight: 0      // Base ellipse height

    // Signal emitted when throw animation completes
    signal throwAnimationDone()
    signal pokemonInsideBall()
    signal ballOpened()

    // Extra delay, is set for player intro
    property int delayReveal: 1
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
        circleExpand() // Ellipse expands on release
    }

    // Single image with manual frame control
    Image {
        id: pokeballSprite
        scale: root.scaleFactor
        visible: false
        z: 100
        source: "qrc:/assets/HGSS/reordered_pokeballs.png"
        sourceClipRect: Qt.rect(0, root.frameHeight * root.rowId, root.frameWidth, root.frameHeight)
        smooth: false
        antialiasing: false
    }

    // Red outer ellipse layer
    Rectangle {
        id: outerCircle
        color: "#FF6B6B" // Red color
        radius: Math.min(width, height) / 2  // Creates ellipse effect
        opacity: 0
        visible: false
        z: 99 // Below the pokeball sprite

        // Position at configured ellipse center
        x: root.circleX - width / 2
        y: root.circleY - height / 2
    }

    // Orange inner ellipse layer
    Rectangle {
        id: innerCircle
        color: "#FFA726" // Orange color
        radius: Math.min(width, height) / 2  // Creates ellipse effect
        opacity: 0
        visible: false
        z: 98 // Below outer circle

        // Position at configured ellipse center
        x: root.circleX - width / 2
        y: root.circleY - height / 2

        // Inner ellipse is 70% the size of outer ellipse
        property real innerScale: 0.7
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

    // Outer ellipse animation
    ParallelAnimation {
        id: outerCircleAnimation
        running: false
        loops: 1

        // Ellipse width animation
        NumberAnimation {
            id: outerSizeAnim
            target: outerCircle
            property: "width"
            duration: root.circleAnimationDuration
            easing.type: Easing.InOutQuad
        }

        // Ellipse height animation
        NumberAnimation {
            id: outerHeightAnim
            target: outerCircle
            property: "height"
            duration: root.circleAnimationDuration
            easing.type: Easing.InOutQuad
        }

        // Position animation to keep centered
        NumberAnimation {
            id: outerXAnim
            target: outerCircle
            property: "x"
            duration: root.circleAnimationDuration
            easing.type: Easing.InOutQuad
        }

        NumberAnimation {
            id: outerYAnim
            target: outerCircle
            property: "y"
            duration: root.circleAnimationDuration
            easing.type: Easing.InOutQuad
        }

        // Opacity animation
        NumberAnimation {
            id: outerOpacityAnim
            target: outerCircle
            property: "opacity"
            duration: root.circleAnimationDuration
            easing.type: Easing.InOutQuad
        }

        onStarted: {
            outerCircle.visible = true
        }

        onStopped: {
            outerCircle.visible = false
        }
    }

    // Inner ellipse animation
    ParallelAnimation {
        id: innerCircleAnimation
        running: false
        loops: 1

        // Ellipse width animation
        NumberAnimation {
            id: innerSizeAnim
            target: innerCircle
            property: "width"
            duration: root.circleAnimationDuration
            easing.type: Easing.InOutQuad
        }

        // Ellipse height animation
        NumberAnimation {
            id: innerHeightAnim
            target: innerCircle
            property: "height"
            duration: root.circleAnimationDuration
            easing.type: Easing.InOutQuad
        }

        // Position animation to keep centered
        NumberAnimation {
            id: innerXAnim
            target: innerCircle
            property: "x"
            duration: root.circleAnimationDuration
            easing.type: Easing.InOutQuad
        }

        NumberAnimation {
            id: innerYAnim
            target: innerCircle
            property: "y"
            duration: root.circleAnimationDuration
            easing.type: Easing.InOutQuad
        }

        // Opacity animation
        NumberAnimation {
            id: innerOpacityAnim
            target: innerCircle
            property: "opacity"
            duration: root.circleAnimationDuration
            easing.type: Easing.InOutQuad
        }

        onStarted: {
            innerCircle.visible = true
        }

        onStopped: {
            innerCircle.visible = false
        }
    }

    // Ellipse expands outward (for pokemon release)
    function circleExpand() {
        // Setup outer ellipse animation: start at startScale, expand to full scale
        var outerStartWidth = root.circleBaseWidth * root.circleShrinkScale
        var outerEndWidth = root.circleBaseWidth
        var outerStartHeight = root.circleBaseHeight * root.circleShrinkScale
        var outerEndHeight = root.circleBaseHeight

        outerSizeAnim.from = outerStartWidth
        outerSizeAnim.to = outerEndWidth

        outerHeightAnim.from = outerStartHeight
        outerHeightAnim.to = outerEndHeight

        outerXAnim.from = root.circleX - outerStartWidth / 2
        outerXAnim.to = root.circleX - outerEndWidth / 2

        outerYAnim.from = root.circleY - outerStartHeight / 2
        outerYAnim.to = root.circleY - outerEndHeight / 2

        outerOpacityAnim.from = 1
        outerOpacityAnim.to = 0.7

        // Setup inner ellipse animation (70% of outer size)
        var innerStartWidth = outerStartWidth * innerCircle.innerScale
        var innerEndWidth = outerEndWidth * innerCircle.innerScale
        var innerStartHeight = outerStartHeight * innerCircle.innerScale
        var innerEndHeight = outerEndHeight * innerCircle.innerScale

        innerSizeAnim.from = innerStartWidth
        innerSizeAnim.to = innerEndWidth

        innerHeightAnim.from = innerStartHeight
        innerHeightAnim.to = innerEndHeight

        innerXAnim.from = root.circleX - innerStartWidth / 2
        innerXAnim.to = root.circleX - innerEndWidth / 2

        innerYAnim.from = root.circleY - innerStartHeight / 2
        innerYAnim.to = root.circleY - innerEndHeight / 2

        innerOpacityAnim.from = 1
        innerOpacityAnim.to = 0.8 // Inner ellipse slightly more opaque

        // Start both animations
        outerCircleAnimation.start()
        innerCircleAnimation.start()
    }

    // Ellipse shrinks inward (for pokemon capture)
    function circleShrink() {
        // Setup outer ellipse animation: start at full scale, shrink to startScale
        var outerStartWidth = root.circleBaseWidth
        var outerEndWidth = root.circleBaseWidth * root.circleShrinkScale
        var outerStartHeight = root.circleBaseHeight
        var outerEndHeight = root.circleBaseHeight * root.circleShrinkScale

        outerSizeAnim.from = outerStartWidth
        outerSizeAnim.to = outerEndWidth

        outerHeightAnim.from = outerStartHeight
        outerHeightAnim.to = outerEndHeight

        outerXAnim.from = root.circleX - outerStartWidth / 2
        outerXAnim.to = root.circleX - outerEndWidth / 2

        outerYAnim.from = root.circleY - outerStartHeight / 2
        outerYAnim.to = root.circleY - outerEndHeight / 2

        outerOpacityAnim.from = 0.7
        outerOpacityAnim.to = 1

        // Setup inner ellipse animation (70% of outer size)
        var innerStartWidth = outerStartWidth * innerCircle.innerScale
        var innerEndWidth = outerEndWidth * innerCircle.innerScale
        var innerStartHeight = outerStartHeight * innerCircle.innerScale
        var innerEndHeight = outerEndHeight * innerCircle.innerScale

        innerSizeAnim.from = innerStartWidth
        innerSizeAnim.to = innerEndWidth

        innerHeightAnim.from = innerStartHeight
        innerHeightAnim.to = innerEndHeight

        innerXAnim.from = root.circleX - innerStartWidth / 2
        innerXAnim.to = root.circleX - innerEndWidth / 2

        innerYAnim.from = root.circleY - innerStartHeight / 2
        innerYAnim.to = root.circleY - innerEndHeight / 2

        innerOpacityAnim.from = 0.8 // Inner ellipse slightly more opaque
        innerOpacityAnim.to = 1

        // Start both animations
        outerCircleAnimation.start()
        innerCircleAnimation.start()
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

        // Frame 9 - trigger ellipse shrink for capture
        PropertyAction {
            target: pokeballSprite
            property: "sourceClipRect.x"
            value: root.frameWidth * 9
        }
        ScriptAction{
            script: root.pokemonInsideBall()
        }
        PauseAnimation {
            duration: root.catchDuration*root.delayReveal
        }
        ScriptAction{
            script: root.ballOpened()
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

    function reset(pokeballId) {
        // Stop all running animations
        throwPokeball.stop()
        shakeAnimation.stop()
        jumpAnimation.stop()
        outerCircleAnimation.stop()
        innerCircleAnimation.stop()
        frameTimer.stop()

        // Reset all visual properties to initial state
        pokeballSprite.visible = false
        pokeballSprite.x = 0
        pokeballSprite.y = 0
        pokeballSprite.sourceClipRect = Qt.rect(0, root.frameHeight * pokeballId, root.frameWidth, root.frameHeight)

        outerCircle.visible = false
        outerCircle.opacity = 0
        outerCircle.width = 0
        outerCircle.height = 0

        innerCircle.visible = false
        innerCircle.opacity = 0
        innerCircle.width = 0
        innerCircle.height = 0

        // Reset any other state if needed
        x0 = 0
        x1 = 0
        y0 = 0
        y1 = 0
    }
}
