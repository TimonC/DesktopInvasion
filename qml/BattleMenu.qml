import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root

    color: "transparent"

    property int frameSize: 32
    property int buttonWidth: frameSize * 2
    property int buttonHeight: frameSize * 0.75
    property int buttonFontSize: frameSize * 0.4
    property int gridSpacing: frameSize * 0.1
    property int menuHeight: 50
    property int menuWidth: frameSize * 5

    property int pokeSpriteId: 3
    property double spriteScale: 1.5
    property int pokeSpriteWidth: 16
    property int pokeSpriteHeight: 23
    property string spriteSheet: "qrc:/assets/HGSS/Pokeballs_transparent_reordered.png"

    signal attackChosen(int attackId)
    signal runChosen()
    signal catchChosen(int pokeId)

    property alias stack: stack

    function showTextBar() {
        stack.replace(textBarComponent);
    }

    function updateText(text) {
        stack.currentItem.text = text
    }

    function resetToRoot() {
        stack.replace(rootSelection)
    }

    StackView {
        id: stack
        initialItem: rootSelection
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
        id: rootSelection
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
                    onClicked: stack.push(attackSelection)
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
                    onClicked: stack.push(catchSelection)
                }
                RoundButton {
                    text: "Run"
                    palette.button: "blue"
                    font.pixelSize: root.buttonFontSize
                    width: root.buttonWidth
                    height: root.buttonHeight
                    onClicked: stack.push(runSelection);
                }
            }
        }
    }

    Component {
        id: attackSelection
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

Component {
    id: catchSelection
    Rectangle {
        anchors.centerIn: parent
        width: root.buttonWidth * 1.5
        height: root.buttonHeight * 1.2
        color: "white"
        border.color: "black"
        border.width: 2
        radius: 3
        Image {
            id: pokeImage
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 8
            source: root.spriteSheet
            width: root.pokeSpriteWidth * root.spriteScale
            height: root.pokeSpriteHeight * root.spriteScale
            sourceClipRect: Qt.rect(0, root.pokeSpriteHeight * root.pokeSpriteId, root.pokeSpriteWidth, root.pokeSpriteHeight)
            smooth: false
            antialiasing: false
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: pokeImage.right
            anchors.leftMargin: 4
            text: "Pokeball"
            font.pixelSize: root.buttonFontSize
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.catchChosen(3)
        }
    }
}

    Component{
        id: runSelection
        Item {
            anchors.fill: parent
            Grid {
                anchors.centerIn: parent
                columns: 2
                spacing: root.gridSpacing
                RoundButton {
                    text: "Run"
                    palette.button: "blue"
                    font.pixelSize: root.buttonFontSize
                    width: root.buttonWidth
                    height: root.buttonHeight
                    onClicked: root.runChosen();
                }
                RoundButton {
                    text: "Back"
                    palette.button: "blue"
                    font.pixelSize: root.buttonFontSize
                    width: root.buttonWidth
                    height: root.buttonHeight
                    onClicked: stack.pop();
                }
            }
        }
    }
}
