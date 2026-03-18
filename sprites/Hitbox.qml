import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: 30*2
    height: 30*2
    visible: true
    z: 3000
property var mouseArea: mouseArea
property var battleButton: battleButton
    property bool openingButtons: false
    property bool clickable: true

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: clickable
        hoverEnabled: true
        onClicked: {
            openingButtons = !openingButtons
        }
    }

    RoundButton {
        id: battleButton
        text: "BATTLE"
        anchors.top: parent.top
        visible: openingButtons
        enabled: openingButtons
        z: 10000
        onClicked: console.log("Battle clicked!")
    }

    Rectangle {
        // optional visual debugging
        anchors.fill: parent
        color: "transparent"
        border.color: "red"
        border.width: 1
        // visible: false  // set to true to debug hitbox area
    }
}

