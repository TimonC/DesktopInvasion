import QtQuick 2.15

Item {
    id: root
    property real speed: 1.0

    property int rowId: 0
    property real scaleFactor: 1
    property real scale: 2 * scaleFactor
    property int frameWidth: 16
    property int frameHeight: 23
    property int groundY: 0

    property int throwDuration: Math.round(600 / speed)
    property int pauseDuration: Math.round(250 / speed)
    property int catchDuration: Math.round(500 / speed)
    property int dropDuration: Math.round(400 / speed)
    property int bounceUpDuration: Math.round(150 / speed)
    property int bounceDownDuration: Math.round(150 / speed)
    property int bounceHeight: 10 * scaleFactor

    property real circleShrinkScale: 0.6
    property int circleAnimationDuration: Math.round(750 / speed)
    property int circleX: 0
    property int circleY: 0
    property int circleBaseWidth: 0
    property int circleBaseHeight: 0

    signal throwAnimationDone()
    signal pokemonInsideBall()
    signal ballOpened()

    property real delayReveal: 1.5
    width: frameWidth
    height: frameHeight
    z: 100

    property int x0: 0
    property int x1: 0
    property int y0: 0
    property int y1: 0

    function throwAt(startX, endX, topY, bottomY, groundPosition) {
        x0 = startX
        x1 = endX
        y0 = topY
        y1 = bottomY
        groundY = groundPosition
        throwPokeball.start()
    }

    function shake() { shakeAnimation.start() }
    function jump() { jumpAnimation.start() }
    function release() {
        pokeballSprite.sourceClipRect.x = root.frameWidth * 9
        circleExpand()
    }

    Image {
        id: pokeballSprite
        scale: root.scale
        visible: false
        z: 100
        source: "qrc:/assets/HGSS/reordered_pokeballs.png"
        sourceClipRect: Qt.rect(0, root.frameHeight * root.rowId, root.frameWidth, root.frameHeight)
        smooth: false
        antialiasing: false
    }

    Rectangle {
        id: outerCircle
        color: "#FF6B6B"
        radius: Math.min(width, height) / 2
        opacity: 0
        visible: false
        z: 99
        x: root.circleX - width / 2
        y: root.circleY - height / 2
    }

    Rectangle {
        id: innerCircle
        color: "#FFA726"
        radius: Math.min(width, height) / 2
        opacity: 0
        visible: false
        z: 98
        x: root.circleX - width / 2
        y: root.circleY - height / 2
        property real innerScale: 0.7
    }

    Timer {
        id: frameTimer
        interval: root.throwDuration / 8
        repeat: true
        property int frameIndex: 0
        onTriggered: {
            pokeballSprite.sourceClipRect.x = frameIndex * root.frameWidth
            frameIndex++
            if (frameIndex >= 8) stop()
        }
    }

    SequentialAnimation {
        id: shakeAnimation
        running: false
        loops: 1
        PropertyAnimation { target: pokeballSprite; property: "x"; to: pokeballSprite.x + 3*root.scale; duration: Math.round(50 / speed) }
        PropertyAnimation { target: pokeballSprite; property: "x"; to: pokeballSprite.x - 3*root.scale; duration: Math.round(50 / speed) }
        PropertyAnimation { target: pokeballSprite; property: "x"; to: pokeballSprite.x + 2*root.scale; duration: Math.round(50 / speed) }
        PropertyAnimation { target: pokeballSprite; property: "x"; to: pokeballSprite.x - 2*root.scale; duration: Math.round(50 / speed) }
        PropertyAnimation { target: pokeballSprite; property: "x"; to: pokeballSprite.x + 1*root.scale; duration: Math.round(50 / speed) }
        PropertyAnimation { target: pokeballSprite; property: "x"; to: pokeballSprite.x - 1*root.scale; duration: Math.round(50 / speed) }
        PropertyAnimation { target: pokeballSprite; property: "x"; to: pokeballSprite.x; duration: Math.round(50 / speed) }
    }

    SequentialAnimation {
        id: jumpAnimation
        running: false
        loops: 1
        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: pokeballSprite.y - 12*root.scale
            duration: Math.round(200 / speed)
            easing.type: Easing.OutQuad
        }
        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: pokeballSprite.y
            duration: Math.round(200 / speed)
            easing.type: Easing.InQuad
        }
    }

    ParallelAnimation {
        id: outerCircleAnimation
        running: false
        loops: 1
        NumberAnimation { id: outerSizeAnim; target: outerCircle; property: "width"; duration: root.circleAnimationDuration; easing.type: Easing.InOutQuad }
        NumberAnimation { id: outerHeightAnim; target: outerCircle; property: "height"; duration: root.circleAnimationDuration; easing.type: Easing.InOutQuad }
        NumberAnimation { id: outerXAnim; target: outerCircle; property: "x"; duration: root.circleAnimationDuration; easing.type: Easing.InOutQuad }
        NumberAnimation { id: outerYAnim; target: outerCircle; property: "y"; duration: root.circleAnimationDuration; easing.type: Easing.InOutQuad }
        NumberAnimation { id: outerOpacityAnim; target: outerCircle; property: "opacity"; duration: root.circleAnimationDuration; easing.type: Easing.InOutQuad }
        onStarted: outerCircle.visible = true
        onStopped: outerCircle.visible = false
    }

    ParallelAnimation {
        id: innerCircleAnimation
        running: false
        loops: 1
        NumberAnimation { id: innerSizeAnim; target: innerCircle; property: "width"; duration: root.circleAnimationDuration; easing.type: Easing.InOutQuad }
        NumberAnimation { id: innerHeightAnim; target: innerCircle; property: "height"; duration: root.circleAnimationDuration; easing.type: Easing.InOutQuad }
        NumberAnimation { id: innerXAnim; target: innerCircle; property: "x"; duration: root.circleAnimationDuration; easing.type: Easing.InOutQuad }
        NumberAnimation { id: innerYAnim; target: innerCircle; property: "y"; duration: root.circleAnimationDuration; easing.type: Easing.InOutQuad }
        NumberAnimation { id: innerOpacityAnim; target: innerCircle; property: "opacity"; duration: root.circleAnimationDuration; easing.type: Easing.InOutQuad }
        onStarted: innerCircle.visible = true
        onStopped: innerCircle.visible = false
    }

    function circleExpand() {
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
        innerOpacityAnim.to = 0.8

        outerCircleAnimation.start()
        innerCircleAnimation.start()
    }

    function circleShrink() {
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
        innerOpacityAnim.from = 0.8
        innerOpacityAnim.to = 1

        outerCircleAnimation.start()
        innerCircleAnimation.start()
    }

    SequentialAnimation {
        id: throwPokeball
        running: false
        loops: 1

        PropertyAction { target: pokeballSprite; property: "visible"; value: true }
        PropertyAction { target: pokeballSprite; property: "sourceClipRect.x"; value: 0 }
        PropertyAction { target: pokeballSprite; property: "x"; value: root.x0 }
        PropertyAction { target: pokeballSprite; property: "y"; value: root.y0 + 32 * root.scaleFactor }

        ScriptAction { script: { frameTimer.frameIndex = 0; frameTimer.start() } }

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
                to: root.y0 + 32 * root.scaleFactor
                duration: root.throwDuration / 2
                easing.type: Easing.InQuad
            }
        }

        ScriptAction { script: frameTimer.stop() }

        PauseAnimation { duration: root.pauseDuration }

        PropertyAction { target: pokeballSprite; property: "sourceClipRect.x"; value: root.frameWidth * 8 }
        PauseAnimation { duration: root.catchDuration }

        PropertyAction { target: pokeballSprite; property: "sourceClipRect.x"; value: root.frameWidth * 9 }
        ScriptAction { script: root.pokemonInsideBall() }
        PauseAnimation { duration: root.catchDuration * root.delayReveal }
        ScriptAction { script: root.ballOpened() }

        PropertyAction { target: pokeballSprite; property: "sourceClipRect.x"; value: 0 }
        PauseAnimation { duration: root.catchDuration }

        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: root.groundY
            duration: root.dropDuration
            easing.type: Easing.InQuad
        }

        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: root.groundY - root.bounceHeight
            duration: root.bounceUpDuration
            easing.type: Easing.OutQuad
        }
        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: root.groundY
            duration: root.bounceDownDuration
            easing.type: Easing.InQuad
        }

        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: root.groundY - (root.bounceHeight * 2 / 3)
            duration: root.bounceUpDuration
            easing.type: Easing.OutQuad
        }
        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: root.groundY
            duration: root.bounceDownDuration
            easing.type: Easing.InQuad
        }

        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: root.groundY - (root.bounceHeight / 3)
            duration: root.bounceUpDuration
            easing.type: Easing.OutQuad
        }
        PropertyAnimation {
            target: pokeballSprite
            property: "y"
            to: root.groundY
            duration: root.bounceDownDuration
            easing.type: Easing.InQuad
        }

        ScriptAction { script: root.throwAnimationDone() }
    }

    function reset(pokeballId) {
        throwPokeball.stop()
        shakeAnimation.stop()
        jumpAnimation.stop()
        outerCircleAnimation.stop()
        innerCircleAnimation.stop()
        frameTimer.stop()

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

        x0 = 0
        x1 = 0
        y0 = 0
        y1 = 0
        groundY = 0
    }
}
