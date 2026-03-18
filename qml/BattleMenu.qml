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
    property int selectedIndex: 0 //for switch menu
    property bool forceSwitchMode: false

    signal attackChosen(int attackId)
    signal runChosen()
    signal catchChosen(int pokeId)
    signal switchChosen(int oldPartyIdx, int newPartyIdx)

    property alias stack: stack

    property var party: {
        "spriteIds"    : [-1, -1, -1, -1, -1, -1],
        "iconIds"      : [-1, -1, -1, -1, -1, -1],
        "ballIds"      : [-1, -1, -1, -1, -1, -1],
        "gens"         : [-1, -1, -1, -1, -1, -1],
        "names"        : ["", "", "", "", "", ""],
        "healthRatios" : [-1, -1, -1, -1, -1, -1],
        "moves"        : [
            [[], [], [], []],
            [[], [], [], []],
            [[], [], [], []],
            [[], [], [], []],
            [[], [], [], []],
            [[], [], [], []]
        ]
    }

    function _setPartyMember(partyIdx, spriteId, iconId, ballId, gen, pokemonName, moves) {
        var temp = party
        temp.spriteIds[partyIdx] = spriteId
        temp.iconIds[partyIdx] = iconId
        temp.ballIds[partyIdx] = ballId
        temp.gens[partyIdx] = gen
        temp.names[partyIdx] = pokemonName
        temp.moves[partyIdx] = moves
        temp.healthRatios[partyIdx] = 1
        party = temp
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

    function forceSwitch() {
        forceSwitchMode = true
        stack.replace(switchSelection)
    }

    function resetToRoot() {
        forceSwitchMode = false
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
                        Move {
                            buttonWidth: root.buttonWidth
                            buttonHeight: root.buttonHeight
                            name: party.moves[0][index].name || ""
                            type: party.moves[0][index].type || "Null"
                            onClicked: root.attackChosen(index)
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
                            visible: root.party.iconIds[index] >= 0
                            color: "transparent"
                            radius: 4
                            border.color: root.selectedIndex === index ? "darkgrey" : "#e0e0e0"
                            border.width: root.selectedIndex === index ? 0.5 : 2
                            Rectangle {
                                anchors.fill: parent
                                radius: parent.radius
                                opacity: root.selectedIndex === index ? 0.4 : 0.6
                                color:   root.selectedIndex === index ? "grey" :
                                         (party.healthRatios[index] >= 0.5 ? "#4CAF50" :
                                         (party.healthRatios[index] >= 0.25 ? "#FF9800" :
                                         (party.healthRatios[index] > 0 ? "#FF0000" : "#8B0000")))
                            }
                            PokemonIcon {
                                anchors.centerIn: parent
                                frameIndex: root.party.iconIds[index]
                                scale: root.selectedIndex === index ? 1.1 : 1
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if(root.selectedIndex !== index && party.healthRatios[index] > 0){
                                        root.switchChosen(root.selectedIndex, index)
                                        root.selectedIndex = index
                                    }
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
            radius: 3

            // Visual feedback
            color: root.forceSwitchMode ? "#b0bec5" : "lightblue"
            opacity: root.forceSwitchMode ? 0.5 : 1.0

            Text {
                anchors.centerIn: parent
                text: "←"
                color: "white"
                font.pixelSize: root.buttonFontSize
            }

            MouseArea {
                anchors.fill: parent
                enabled: !root.forceSwitchMode
                onClicked: stack.pop()
            }
        }
    }
}
