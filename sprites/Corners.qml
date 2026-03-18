import QtQuick 2.15

Rectangle {
    id: root
    anchors.fill: parent
    color: "transparent"
    border.color: "orange"
    border.width: 1
    visible: debugLines
    property bool debugLines: false
}
