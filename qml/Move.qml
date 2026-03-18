import QtQuick 2.15
import QtQuick.Controls 2.15
import "StyleSheet/PokeType.js" as PokeType

Rectangle {
    id: root
    width: root.buttonWidth
    height: root.buttonHeight
    color: PokeType.typeColor(type)
    border.color: "black"
    border.width: 2
    radius: 3

    property int buttonWidth: 64
    property int buttonHeight: 48
    property string name: ""
    property string type: "Normal"

    Text {
        id: moveName
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -height * 0.25
        width: parent.width * 0.9
        text: root.name
        font.pixelSize: 12
        wrapMode: Text.WordWrap
        maximumLineCount: 2
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
    }

    Text {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: moveName.height * 0.25
        text: root.type
        font.pixelSize: 10
        color: "white"
        style: Text.Outline
        styleColor: "black"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: parent.clicked()
    }

    signal clicked()
}
