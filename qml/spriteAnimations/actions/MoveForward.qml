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
        to: (direction==1 ? -6*scaleFactor :
             direction==3 ? 6*scaleFactor :
             0)
        duration: 50
        easing.type: Easing.InQuad
    }

    PropertyAnimation {
        target: root.target
        property: "y"
        to: (direction==0 ? -6*scaleFactor :
             direction==2 ? 6*scaleFactor :
             0)
        duration: 50
        easing.type: Easing.InQuad
    }

    PropertyAnimation {
        target: root.target
        property: "x"
        to: 0
        duration: 100
        easing.type: Easing.OutQuad
    }

    PropertyAnimation {
        target: root.target
        property: "y"
        to: 0
        duration: 100
        easing.type: Easing.OutQuad
    }
}
