import QtQuick 2.15

SequentialAnimation {
    id: root

    // Use default property to allow child animations
    default property var animationChildren

    property Item target: null
    property int direction: 0
    property real scaleFactor: 4

    running: false
    loops: 1

    signal animationFinished()

    onStopped: animationFinished()

    function startAnimation() {
        running = true;
    }

    function stopAnimation() {
        running = false;
    }

    // Initialize child animations with our target
    onTargetChanged: {
        if (animationChildren && target) {
            for (var i = 0; i < animationChildren.length; i++) {
                if (animationChildren[i].hasOwnProperty('target')) {
                    animationChildren[i].target = target;
                }
            }
        }
    }
}
