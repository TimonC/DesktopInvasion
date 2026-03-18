import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    width: 800
    height: 600
    color: backgroundColor

    property color backgroundColor: "#2b2b2b"
    property color buttonColor: "#3c3c3c"
    property color buttonSelectedColor: "#5294e2"
    property color textColor: "#ffffff"

    property int headerFontSize: 18
    property int bodyFontSize: 14
    property string fontFamily: "Sans Serif"

    property int headerHeight: height / 8
    property int contentHeight: height - headerHeight

    property string currentMenu: "Party"

    Rectangle{
        Layout.fillWidth: true
        Layout.fillHeight: true
        color: root.backgroundColor
        RowLayout {
            anchors.fill: parent
            spacing: 0
        }
    }
}
