import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    anchors.fill: parent
    color: "transparent"

    // Title text
    // Text {
    //     anchors.top: parent.top
    //     anchors.horizontalCenter: parent.horizontalCenter
    //     text: "OVERLAY IS WORKING"
    //     color: "hotpink"
    //     font.pixelSize: 10
    // }

    // Container for dynamically added sprites
    Item {
        id: spriteContainer
        objectName: "spriteContainer"
        anchors.fill: parent
    }
}

