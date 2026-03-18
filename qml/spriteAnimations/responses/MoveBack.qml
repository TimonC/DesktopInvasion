import QtQuick 2.15

SequentialAnimation {
    id: root

    property Item target: parent
    property int direction: 0
    property real scaleFactor: 4

    running: false
    loops: 1

    PropertyAnimation {
        target: root.target
        property: "x"
        to: (direction==1 ? 2.5*scaleFactor :
             direction==3 ? -2.5*scaleFactor :
             0)
        duration: 100
    }

    PropertyAnimation {
        target: root.target
        property: "y"
        to: (direction==0 ? 2.5*scaleFactor :
             direction==2 ? -2.5*scaleFactor :
             0)
        duration: 100
    }

    PropertyAnimation {
        target: root.target
        property: "x"
        to: 0
        duration: 100
        easing.type: Easing.InQuad
    }

    PropertyAnimation {
        target: root.target
        property: "y"
        to: 0
        duration: 100
        easing.type: Easing.InQuad
    }
}
