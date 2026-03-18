import QtQuick 2.15

Rectangle {
    id: root
    property string text: ""

    color: "white"
    border.color: "gray"
    border.width: 1
    radius: 4

    width: 32 * 5
    height: 32

    Text {
        anchors.fill: parent
        anchors.margins: 8
        text: root.text
        font.pixelSize: 14
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
