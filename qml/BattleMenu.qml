import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    color: "transparent"

    // Expose properties from root
    property int frameSize: 32
    property int buttonWidth: frameSize * 2
    property int buttonHeight: frameSize * 0.75
    property int buttonFontSize: frameSize * 0.4
    property int gridSpacing: frameSize * 0.1
    property int menuHeight: 50
    property int menuWidth: frameSize * 5

    signal attackChosen(int attackId)
    signal runClicked()

    property alias stack: stack

    function showTextBar() {
        stack.replace(textBarComponent);
    }

    function updateText(text) {
        stack.currentItem.text = text
    }

    function resetToRoot() {
        stack.replace(rootSelectionComponent)
    }

    StackView {
        id: stack
        initialItem: rootSelectionComponent
        anchors.fill: parent
        z: 1
    pushEnter: Transition {
        PropertyAnimation {
            property: "opacity"
            from: 0
            to:1
            duration: 200
        }
    }
    pushExit: Transition {
        PropertyAnimation {
            property: "opacity"
            from: 1
            to:0
            duration: 200
        }
    }
    popEnter: Transition {
        PropertyAnimation {
            property: "opacity"
            from: 0
            to:1
            duration: 200
        }
    }
    popExit: Transition {
        PropertyAnimation {
            property: "opacity"
            from: 1
            to:0
            duration: 200
        }
    }
    replaceEnter: Transition {
        PropertyAnimation {
            property: "opacity"
            from: 0
            to:1
            duration: 10
        }
    }
    replaceExit: Transition {
        PropertyAnimation {
            property: "opacity"
            from: 1
            to:0
            duration: 10
        }
    }
    }

    Component {
        id: textBarComponent
        Rectangle {
            id: textBar
            color: "darkgrey"
            property string text: "UNINITIALIZED TEXT!!!!"
            height: root.menuHeight
            width: root.menuWidth
            radius: 5
            Text {
                id: textBarText
                anchors.fill: parent
                anchors.margins: 6
                text: textBar.text
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
                spacing: root.gridSpacing
                RoundButton {
                    text: "Attack"
                    palette.button: "red"
                    font.pixelSize: root.buttonFontSize
                    width: root.buttonWidth
                    height: root.buttonHeight
                    onClicked: stack.push(attackSelectionComponent)
                }
                RoundButton {
                    text: "Switch"
                    palette.button: "green"
                    font.pixelSize: root.buttonFontSize
                    width: root.buttonWidth
                    height: root.buttonHeight
                }
                RoundButton {
                    text: "Catch"
                    palette.button: "yellow"
                    font.pixelSize: root.buttonFontSize
                    width: root.buttonWidth
                    height: root.buttonHeight
                }
                RoundButton {
                    text: "Run"
                    palette.button: "blue"
                    font.pixelSize: root.buttonFontSize
                    width: root.buttonWidth
                    height: root.buttonHeight
                    onClicked: root.runClicked()
                }
            }
        }
    }

    Component {
        id: attackSelectionComponent
        Item {
            anchors.fill: parent
            Grid {
                anchors.centerIn: parent
                columns: 2
                spacing: root.gridSpacing

                Repeater {
                    model: 4
                    Rectangle {
                        width: root.buttonWidth
                        height: root.buttonHeight
                        color: "white"
                        border.color: "black"
                        border.width: 2
                        radius: 3

                        Text {
                            anchors.centerIn: parent
                            text: "TACKLE"
                            font.pixelSize: root.buttonFontSize
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: root.attackChosen(0)
                        }
                    }
                }
            }
        }
    }
}
