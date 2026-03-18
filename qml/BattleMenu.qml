import QtQuick 2.15
import QtQuick.Controls 2.15
Rectangle {
    id: root
    color: "transparent"
    property int frameSize: 32
    property int buttonWidth: frameSize * 2
    property int buttonHeight: frameSize * 0.75
    property int buttonFontSize: 0
    property int textBarFontSize: 0
    property int gridSpacing: frameSize * 0.1
    property int menuHeight: 0
    property int menuWidth: 0
    property int pokeSpriteId: 3
    property double spriteScale: 1
    property int pokeSpriteWidth: 16
    property int pokeSpriteHeight: 23
    property string spriteSheet: "qrc:/assets/HGSS/Pokeballs_transparent_reordered.png"
    signal attackChosen(int attackId)
    signal runChosen()
    signal catchChosen(int pokeId)
    signal switchChosen(int partyIdx)
    property alias stack: stack
    property var partyMembers: [
        {spriteId: -1, iconId: -1, name: ""},
        {spriteId: -1, iconId: -1, name: ""},
        {spriteId: -1, iconId: -1, name: ""},
        {spriteId: -1, iconId: -1, name: ""},
        {spriteId: -1, iconId: -1, name: ""},
        {spriteId: -1, iconId: -1, name: ""}
    ]
    function _setPartyMember(partyIdx, iconId, iconId, pokemonName) {
        var temp = partyMembers
        temp[partyIdx] = {spriteId: spriteId, iconId: iconId, name: pokemonName}
        partyMembers = temp
    }
    function showTextBar() {
        stack.replace(textBarComponent)
    }
    function updateText(text) {
        if (stack.currentItem && stack.currentItem.hasOwnProperty("text")) {
            stack.currentItem.text = text
        }
    }
    function getText(){
        if (stack.currentItem && stack.currentItem.hasOwnProperty("text")) {
            return stack.currentItem.text
        }
    }
    function resetToRoot() {
        stack.replace(rootSelection)
    }
    StackView {
        id: stack
        initialItem: textBarComponent
        anchors.fill: parent
        z: 1
        pushEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: 200
            }
        }
        pushExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: 200
            }
        }
        popEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: 200
            }
        }
        popExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: 200
            }
        }
        replaceEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: 10
            }
        }
        replaceExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to: 0
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
            property string text: ""
            height: root.menuHeight
            width: root.menuWidth
            radius: 5
            Text {
                id: textBarText
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.rightMargin: parent.width * 0.2
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
                    onClicked: stack.push(switchSelection)
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
                    onClicked: stack.push(runSelection)
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
        id: switchSelection
        Item {
            id: switchRoot
            property int selectedIndex: 0

            Row {
                anchors.centerIn: parent
                spacing: root.gridSpacing*2

                Grid {
                    columns: 3
                    rows: 2
                    spacing:  0

                    Repeater {
                        model: 6
                        Rectangle {
                            width: root.menuWidth*0.2
                            height: root.menuHeight*0.5
                            visible: root.partyMembers[index].iconId >= 0
                            color: "transparent"
                            radius: 4
                            border.color: switchRoot.selectedIndex === index ? "#4CAF50" : "#e0e0e0"
                            border.width: switchRoot.selectedIndex === index ? 2 : 0.5


                            Rectangle {
                                anchors.fill: parent
                                color: switchRoot.selectedIndex === index ? "#E8F5E9" : "transparent"
                                radius: parent.radius
                                opacity: 0.6
                            }

                            PokemonIcon {
                                anchors.centerIn: parent
                                frameIndex: root.partyMembers[index].iconId
                                scale: switchRoot.selectedIndex === index ? 1.05 : 1
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    switchRoot.selectedIndex = index
                                    root.switchChosen(index)
                                }
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
                width: root.buttonWidth * 1.6
                height: root.buttonHeight * 0.9
                radius: height / 2
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
                        id: ballName
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: pokeImage.right
                        anchors.leftMargin: 4
                        text: "Pokeball"
                        font.pixelSize: root.buttonFontSize
                    }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: ballName.right
                        anchors.leftMargin: 4
                        text: "inf"
                        font.pixelSize: root.buttonFontSize
                        color: "darkgrey"
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
    Component {
        id: runSelection
        Item {
            Grid {
                anchors.centerIn: parent
                columns: 2
                spacing: root.gridSpacing
                RoundButton {
                    palette.button: "blue"
                    text: "Confirm"
                    width: root.buttonWidth
                    height: root.buttonHeight
                    onClicked: root.runChosen()
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
