import QtQuick 2.15

SequentialAnimation {
    id: root

    property Item target: parent
    property int direction: 0
    property real scaleFactor: 4

    running: false
    loops: 1

    // Signal - emitted when animation finishes
    signal animationFinished()

    // Connect to built-in signal
    onStopped: animationFinished()

    // Slot - call this to start the animation
    function startAnimation() {
        running = true;
    }

    function stopAnimation() {
        running = false;
    }
}
