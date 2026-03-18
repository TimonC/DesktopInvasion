import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root

    color: "transparent"

    property int frameSize: 32
    property int buttonWidth: frameSize * 2
    property int buttonHeight: frameSize * 0.75
    property int buttonFontSize: frameSize * 0.4
    property int textBarFontSize: frameSize * 0.42
    property int gridSpacing: frameSize * 0.1
    property int menuHeight:50
    property int menuWidth: frameSize * 5 * 0.9

    property int pokeSpriteId: 3
    property double spriteScale: 1
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
        color: "white"
        border.color: "black"
        border.width: 2
        property string text: "UNINITIALIZED TEXT!!!!"
        height: root.menuHeight
        width: root.menuWidth
        radius: 5

        Text {
            id: textBarText
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: parent.width * 0.2 //Wrap at 80%
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.margins: 6
            text: textBar.text
            font.pixelSize: root.textBarFontSize
            verticalAlignment: Text.AlignVCenter

            wrapMode: Text.WordWrap
            maximumLineCount: 2
            elide: Text.ElideRight
        }
    }
}

    Component {
        id: rootSelection
        Item {
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
            Row {
                anchors.centerIn: parent
                spacing: root.gridSpacing

                Grid {
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
                                text: "Tackle"
                                font.pixelSize: root.buttonFontSize
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: root.attackChosen(0)
                            }
                        }
                    }
                }

                Loader {
                    anchors.verticalCenter: parent.verticalCenter
                    sourceComponent: backButton
                }
            }
        }
    }


Component {
    id: catchSelection

    Item {
        RoundButton {
            id: pokeballButton
            anchors.centerIn: parent
            width: root.buttonWidth * 1.3
            height: root.buttonHeight * 0.9
            radius: height/2

            background: Rectangle {
                radius: parent.radius
                color: pokeballButton.pressed ? "#f0f0f0" : "white"
                border.color: "black"
                border.width: 2
            }

            contentItem: Item {
                Image {
                    id: pokeImage
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: 0.58
                    anchors.left: parent.left
                    source: root.spriteSheet
                    width: root.pokeSpriteWidth * root.spriteScale
                    height: root.pokeSpriteHeight * root.spriteScale
                    sourceClipRect: Qt.rect(0, root.pokeSpriteHeight * root.pokeSpriteId,
                                           root.pokeSpriteWidth, root.pokeSpriteHeight)
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
            }

            onClicked: root.catchChosen(3)
        }

        Loader {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            sourceComponent: backButton
        }
    }
}

    Component{
        id: runSelection
        Item {
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
                Loader {
                    sourceComponent: backButton
                }
            }
        }
    }
    Component {
        id: backButton
        Rectangle {
            width: root.buttonHeight
            height: root.buttonHeight
            color: "lightblue"
            radius: 3
            Text {
                anchors.centerIn: parent
                text: "←"
                color: "white"
                font.pixelSize: root.buttonFontSize
            }
            MouseArea {
                anchors.fill: parent
                onClicked: stack.pop()
            }
        }
    }
}


