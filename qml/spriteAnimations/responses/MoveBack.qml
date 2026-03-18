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
        from: target.x
        to: target.x + (root.direction==1 ? 2.5*root.scaleFactor :
                       root.direction==3 ? -2.5*root.scaleFactor :
                       0)
        duration: 100
    }

    PropertyAnimation {
        target: root.target
        property: "y"
        from: target.y
        to: target.y + (root.direction==0 ? 2.5*root.scaleFactor :
                       root.direction==2 ? -2.5*root.scaleFactor :
                       0)
        duration: 100
    }

    PropertyAnimation {
        target: root.target
        property: "x"
        to: target.x
        duration: 100
        easing.type: Easing.InQuad
    }

    PropertyAnimation {
        target: root.target
        property: "y"
        to: target.y
        duration: 100
        easing.type: Easing.InQuad
    }
}
