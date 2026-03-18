import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../Style/PokeColor.js" as PokeColor

Rectangle {
    id: root
    property int fontSizeSm: 0
    property int fontSizeMd: 0
    property int typeWidth: 0
    property string mainFont: ""
    property string bodyFont: ""
    property var modelData: undefined
    property color textColor: undefined
    property string moveName: ""
    property string moveType: ""
    property string movePow: ""
    property string moveAcc: ""
    property string moveFlavorText: ""

    width:  parent.width
    height: moveInner.implicitHeight + 8
    color:  "#383838"
    radius: 5


    Column {
        id: moveInner
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: 6
        }
        spacing: 2

        Row {
            spacing: 8
            Rectangle {
                width: typeWidth
                height: fontSizeMd + 4
                radius: 3
                color: PokeColor.typeColor(moveType) ?? "#888"
                anchors.verticalCenter: parent.verticalCenter
                Text {
                    id: pill; anchors.centerIn: parent
                    text: moveType
                    font.family: bodyFont
                    font.pixelSize: fontSizeSm
                    color: "#ffffff"
                }
            }Text {
                width: typeWidth*1.8
                text: moveName
                font.family: bodyFont
                font.pixelSize: fontSizeMd
                color: textColor
            }
            Text {
                text: "Pow: " + movePow + "   Acc: " + moveAcc
                font.family: bodyFont
                font.pixelSize: fontSizeSm*0.9
                color: "#aaaaaa"
                anchors.verticalCenter: parent.verticalCenter
            }

        }
        Text {
            height: fontSizeSm*3
            width: parent.width
            text:  moveFlavorText
            font.family: bodyFont
            font.pixelSize: fontSizeSm
            color: "#888888"
            wrapMode: Text.WordWrap
        }
    }
}
