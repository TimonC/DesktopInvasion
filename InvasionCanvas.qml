import QtQuick 2.15

Rectangle {
    anchors.fill: parent
    color: "transparent"

    Text {
        text: "OVERLAY IS WORKING"
        anchors.centerIn: parent
        color: "pink"
        font.pixelSize: 40

                // Stick to the top-right corner
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 10    // optional padding from top
        anchors.rightMargin: 10  // optional padding from right
    }
}

