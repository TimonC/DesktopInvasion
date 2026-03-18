import QtQuick 2.15

SequentialAnimation {
    id: root

    property Item target: parent
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

    PropertyAnimation {
        target: root.target
        property: "x"
        from: target.x  // Start from current position
        to: target.x + (root.direction==1 ? -6*root.scaleFactor :
                       root.direction==3 ? 6*root.scaleFactor :
                       0)
        duration: 50
        easing.type: Easing.InQuad
    }

    PropertyAnimation {
        target: root.target
        property: "y"
        from: target.y  // Start from current position
        to: target.y + (root.direction==0 ? -6*root.scaleFactor :
                       root.direction==2 ? 6*root.scaleFactor :
                       0)
        duration: 50
        easing.type: Easing.InQuad
    }

    PropertyAnimation {
        target: root.target
        property: "x"
        to: target.x  // Return to original x position
        duration: 100
        easing.type: Easing.OutQuad
    }

    PropertyAnimation {
        target: root.target
        property: "y"
        to: target.y  // Return to original y position
        duration: 100
        easing.type: Easing.OutQuad
    }
}
