import QtQuick 2.15

Rectangle {
    anchors.fill: parent
    color: "transparent"

   Text {
        anchors.top: parent.top
        anchors.right: parent.center
        anchors.topMargin: 10


        text: "OVERLAY IS WORKING"
        color: "pink"
        font.pixelSize: 40
    }
}

