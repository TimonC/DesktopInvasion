import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: menuContainer
    color: "transparent"

    // Expose properties from root
    property int frameSize: 32
    property int buttonWidth: frameSize * 2
    property int buttonHeight: frameSize * 0.75
    property int buttonFontSize: frameSize * 0.4
    property int gridSpacing: frameSize * 0.1
    property int menuHeight: 50
    property int menuWidth: frameSize * 5

    // Expose signals
    signal attackButtonClicked()
    signal runClicked()

    // Expose stack for external control
    property alias stack: stack

    StackView {
        id: stack
        initialItem: rootSelectionComponent
        anchors.fill: parent
        z: 1
    }

    Component {
        id: textBarComponent
        Rectangle {
            id: textBar
            color: "darkgrey"
            property string text: ""
            height: menuContainer.menuHeight
            width: menuContainer.menuWidth
            radius: 5
            Text {
                id: textBarText
                anchors.fill: parent
                anchors.margins: 6
                text: "UNINITIALIZED TEXT!!!"
                font.pixelSize: 13
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    Component {
        id: rootSelectionComponent
        Item {
            anchors.fill: parent
            Grid {
                anchors.centerIn: parent
                columns: 2
                spacing: menuContainer.gridSpacing
                RoundButton {
                    text: "Attack"
                    palette.button: "red"
                    font.pixelSize: menuContainer.buttonFontSize
                    width: menuContainer.buttonWidth
                    height: menuContainer.buttonHeight
                    onClicked: menuContainer.attackButtonClicked()
                }
                RoundButton {
                    text: "Switch"
                    palette.button: "green"
                    font.pixelSize: menuContainer.buttonFontSize
                    width: menuContainer.buttonWidth
                    height: menuContainer.buttonHeight
                }
                RoundButton {
                    text: "Catch"
                    palette.button: "yellow"
                    font.pixelSize: menuContainer.buttonFontSize
                    width: menuContainer.buttonWidth
                    height: menuContainer.buttonHeight
                }
                RoundButton {
                    text: "Run"
                    palette.button: "blue"
                    font.pixelSize: menuContainer.buttonFontSize
                    width: menuContainer.buttonWidth
                    height: menuContainer.buttonHeight
                    onClicked: menuContainer.runClicked()
                }
            }
        }
    }
}
