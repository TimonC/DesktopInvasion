import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    width: 800
    height: 600
    color: backgroundColor

    property int margin: 0
    property color backgroundColor: "#2b2b2b"
    property color buttonColor: "#3c3c3c"
    property color buttonSelectedColor: "#5294e2"
    property color textColor: "#ffffff"
    property int headerFontSize: 24
    property int bodyFontSize: 18
    property string fontFamily: "Press Start 2P"
    property string currentMenu: "Party"

    // Debug outline toggle
    property bool showDebugOutlines: true
    property color debugOutlineColor: "#FFD700"

    property double leftSideWidthRatio: 0.5

    RowLayout {
        anchors.fill: parent
        anchors.margins: root.margin
        spacing: 0

        // LEFT COLUMN
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * root.leftSideWidthRatio
            color: root.backgroundColor
            border.color: root.showDebugOutlines ? root.debugOutlineColor : "transparent"
            border.width: root.showDebugOutlines ? 2 : 0

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                Rectangle {
                    id: trainerSection
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * (2/7)
                    color: root.backgroundColor
                    border.color: root.showDebugOutlines ? root.debugOutlineColor : "transparent"
                    border.width: root.showDebugOutlines ? 2 : 0

                    Trainer {
                        anchors.fill: parent
                        textColor: root.textColor
                        fontSize: root.bodyFontSize
                        fontFamily: root.fontFamily
                    }
                }

                Rectangle {
                    id: pcSection
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * (5/7)
                    color: root.backgroundColor
                    border.color: root.showDebugOutlines ? root.debugOutlineColor : "transparent"
                    border.width: root.showDebugOutlines ? 2 : 0

                    PC {
                    }
                }
            }
        }

        // RIGHT COLUMN
        Rectangle {
            id: rightPanel
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: root.backgroundColor
            border.color: root.showDebugOutlines ? root.debugOutlineColor : "transparent"
            border.width: root.showDebugOutlines ? 2 : 0

            PokeView{
            }
            Text {
                anchors.centerIn: parent
                text: "Right Panel (Replaceable)"
                color: root.textColor
                font.pixelSize: 16
            }
        }
    }
}
