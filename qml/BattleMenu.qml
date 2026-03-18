import QtQuick 2.15
import QtQuick.Controls 2.15
import "StyleSheet/PokeType.js" as PokeType

Rectangle {
    id: root
    color: "transparent"

    property int frameSize: 0
    property int buttonWidth: frameSize * 2
    property int buttonHeight: frameSize * 0.75
    property int gridSpacing: 3
    property int menuHeight: 0
    property int menuWidth: 0

    property int buttonFontSize: 0
    property int moveFontSize: 0
    property int textBarFontSize: 0

    // Font properties
    property string menuFontFamily: ""
    property string textBarFontFamily: ""

    // Color properties - consolidated for consistent styling
    property color menuTextColor: "black"
    property color textBarTextColor: "black"

    // Background colors
    property color textBarBackgroundColor: "white"
    property color textBarBorderColor: "black"

    // Button colors
    property color attackButtonColor: "red"
    property color switchButtonColor: "green"
    property color catchButtonColor: "yellow"
    property color runButtonColor: "blue"

    // Disabled state colors (shared between attackContent and switchContent)
    property color disabledBorderColor: "#d0d0d0"
    property color disabledBackgroundColor: "#e0e0e0"
    property color disabledTextColor: "#a0a0a0"
    property color placeholderTextColor: "#a0a0a0"

    // Enabled border colors
    property color enabledBorderColor: "#e0e0e0"
    property color selectedBorderColor: "darkgrey"

    // Health indicator colors (for switchContent)
    property color highHealthColor: "#4CAF50"      // Green for >= 50% health
    property color mediumHealthColor: "#FF9800"    // Orange for >= 25% health
    property color lowHealthColor: "#FF0000"       // Red for > 0% health
    property color faintedHealthColor: "#8B0000"   // Dark red for 0% health

    // Opacity values
    property real enabledOpacity: 0.6
    property real disabledOpacity: 0.3
    property real selectedIconScale: 1.1
    property real normalIconOpacity: 1.0
    property real faintedIconOpacity: 0.5

    // Back button colors
    property color backButtonColor: "lightblue"
    property color forceSwitchBackButtonColor: "#b0bec5"

    // Catch content colors
    property color catchButtonBackground: "white"
    property color catchButtonPressedBackground: "#f0f0f0"
    property color catchButtonBorderColor: "black"
    property color ballCountTextColor: "darkgrey"

    property double spriteScale: 1
    property int ballSpriteWidth: 16
    property int ballSpriteHeight: 23
    property string ballSpriteSheet: "qrc:/assets/HGSS/Pokeballs_transparent_reordered.png"
    property var nrOfBalls: [1000, 0, 0, 0]

    property int selectedIndex: 0
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

    Component {
        id: textBarComponent
        Rectangle {
            id: textBar
            color: root.textBarBackgroundColor
            border.color: root.textBarBorderColor
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
                font.family: root.textBarFontFamily
                color: root.textBarTextColor
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }
        }
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
        id: rootSelection
        Item {
            Grid {
                anchors.centerIn: parent
                columns: 2
                spacing: root.gridSpacing
                RoundButton {
                    text: "Attack"
                    palette.button: root.attackButtonColor
                    font.pixelSize: root.buttonFontSize
                    font.family: root.menuFontFamily
                    width: root.buttonWidth
                    height: root.buttonHeight
                    onClicked: stack.push(attackSelection)
                }
                RoundButton {
                    text: "Switch"
                    palette.button: root.switchButtonColor
                    font.pixelSize: root.buttonFontSize
                    font.family: root.menuFontFamily
                    width: root.buttonWidth
                    height: root.buttonHeight
                    onClicked: stack.push(switchSelection)
                }
                RoundButton {
                    text: "Catch"
                    palette.button: root.catchButtonColor
                    font.pixelSize: root.buttonFontSize
                    font.family: root.menuFontFamily
                    width: root.buttonWidth
                    height: root.buttonHeight
                    onClicked: stack.push(catchSelection)
                }
                RoundButton {
                    text: "Run"
                    palette.button: root.runButtonColor
                    font.pixelSize: root.buttonFontSize
                    font.family: root.menuFontFamily
                    width: root.buttonWidth
                    height: root.buttonHeight
                    onClicked: stack.push(runSelection)
                }
            }
        }
    }

    Component {
        id: attackContent
        Grid {
            columns: 2
            rows: 2
            spacing: root.gridSpacing/2

            property real cellWidth: (parent.width - spacing) / 2
            property real cellHeight: (parent.height - spacing) / 2

            Repeater {
                model: 4
                Rectangle {
                    id: moveRect
                    width: cellWidth
                    height: cellHeight
                    color: "transparent"
                    radius: 4

                    property string moveName: party.moves[0][index].name || "---"
                    property string moveType: party.moves[0][index].type || "Null"
                    property bool moveEnabled: moveType !== "Null"
                    property color typeColor: moveEnabled ? PokeType.typeColor(moveType) : root.disabledBorderColor

                    border.color: moveEnabled ? typeColor : root.disabledBorderColor
                    border.width: 2

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 2
                        radius: parent.radius - 2
                        opacity: moveEnabled ? root.enabledOpacity : root.disabledOpacity
                        color: moveEnabled ? typeColor : root.disabledBackgroundColor
                    }

                    Text {
                        anchors.centerIn: parent
                        width: Math.max(0, parent.width - 8)
                        text: moveName
                        font.pixelSize: root.moveFontSize
                        font.weight: Font.DemiBold
                        font.family: root.menuFontFamily
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.Wrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                        lineHeight: 0.9
                        color: moveEnabled ? "white" : root.disabledTextColor
                    }

                    MouseArea {
                        anchors.fill: parent
                        anchors.margins: 2
                        enabled: moveEnabled
                        onClicked: root.attackChosen(index)
                    }
                }
            }
        }
    }

    Component {
        id: switchContent
        Grid {
            columns: 3
            rows: 2
            spacing: root.gridSpacing

            property real cellWidth: (parent.width - spacing * 2) / 3
            property real cellHeight: (parent.height - spacing) / 2

            Repeater {
                model: 6
                Rectangle {
                    width: cellWidth
                    height: cellHeight
                    color: "transparent"
                    radius: 4
                    border.color: (root.party.iconIds[index] >= 0 && party.healthRatios[index] > 0)
                                  ? (root.selectedIndex === index ? root.selectedBorderColor : root.enabledBorderColor)
                                  : root.disabledBorderColor
                    border.width: root.selectedIndex === index ? 0.5 : 2

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 2
                        radius: parent.radius - 2
                        opacity: root.party.iconIds[index] >= 0 ? root.enabledOpacity : root.disabledOpacity
                        color: root.party.iconIds[index] >= 0
                               ? (party.healthRatios[index] >= 0.5 ? root.highHealthColor :
                                  (party.healthRatios[index] >= 0.25 ? root.mediumHealthColor :
                                  (party.healthRatios[index] > 0 ? root.lowHealthColor : root.faintedHealthColor)))
                               : root.disabledBackgroundColor
                    }

                    Item {
                        anchors.centerIn: parent
                        width: 40
                        height: 30
                        visible: root.party.iconIds[index] >= 0

                        Image {
                            id: iconFrame
                            anchors.centerIn: parent
                            source: "qrc:/assets/HGSS/PokemonIcons_filtered_reordered.png"
                            sourceClipRect: Qt.rect(0, root.party.iconIds[index] * 30, 40, 30)
                            property int frameIndex: root.party.iconIds[index]

                            width: 40
                            height: 30

                            smooth: false
                            antialiasing: false

                            scale: root.selectedIndex === index ? root.selectedIconScale : 1
                            opacity: party.healthRatios[index] > 0 ? root.normalIconOpacity : root.faintedIconOpacity
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        visible: root.party.iconIds[index] < 0
                        text: "---"
                        font.pixelSize: root.buttonFontSize
                        font.family: root.menuFontFamily
                        color: root.placeholderTextColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    MouseArea {
                        anchors.fill: parent
                        anchors.margins: 2
                        enabled: !root.forceSwitchMode ||
                                 (root.selectedIndex !== index &&
                                  root.party.iconIds[index] >= 0 &&
                                  party.healthRatios[index] > 0)
                        onClicked: {
                            if(root.selectedIndex !== index &&
                               root.party.iconIds[index] >= 0 &&
                               party.healthRatios[index] > 0){
                                root.switchChosen(root.selectedIndex, index)
                                root.selectedIndex = index
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: catchContent
        Grid {
            columns: 2
            rows: 2
            spacing: root.gridSpacing

            property real cellWidth: (parent.width - spacing) / 2
            property real cellHeight: (parent.height - spacing) / 2

            Repeater {
                model: 4
                RoundButton {
                    width: cellWidth
                    height: cellHeight
                    radius: height / 2
                    enabled: root.nrOfBalls[index] > 0
                    opacity: root.nrOfBalls[index] > 0 ? 1.0 : 0.5
                    background: Rectangle {
                        radius: parent.radius
                        color: parent.pressed ? root.catchButtonPressedBackground : root.catchButtonBackground
                        border.color: root.catchButtonBorderColor
                        border.width: 2
                    }
                    contentItem: Item {
                        anchors.fill: parent
                        anchors.margins: 4

                        Image {
                            id: ballIcon
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            source: root.ballSpriteSheet
                            width: root.ballSpriteWidth * root.spriteScale
                            height: root.ballSpriteHeight * root.spriteScale
                            sourceClipRect: Qt.rect(0, root.ballSpriteHeight * index,
                                                   root.ballSpriteWidth, root.ballSpriteHeight)
                            smooth: false
                            antialiasing: false
                            opacity: root.nrOfBalls[index] > 0 ? 1.0 : 0.3
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: ballIcon.right
                            anchors.leftMargin: 8
                            anchors.right: countText.left
                            anchors.rightMargin: 4
                            text: ["Pokeball", "Great Ball", "Ultra Ball", "Master Ball"][index]
                            font.pixelSize: root.buttonFontSize
                            font.family: root.menuFontFamily
                            color: root.menuTextColor
                            elide: Text.ElideRight
                        }
                        Text {
                            id: countText
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            anchors.rightMargin: 8
                            text: root.nrOfBalls[index] > 999 ? "∞" : root.nrOfBalls[index]
                            font.pixelSize: root.buttonFontSize
                            font.family: root.menuFontFamily
                            color: root.ballCountTextColor
                        }
                    }
                    onClicked: root.catchChosen(index)
                }
            }
        }
    }

    Component {
        id: runContent
        RoundButton {
            anchors.centerIn: parent
            palette.button: root.runButtonColor
            text: "Confirm"
            width: root.buttonWidth
            height: root.buttonHeight
            font.pixelSize: root.buttonFontSize
            font.family: root.menuFontFamily
            onClicked: root.runChosen()
        }
    }

    Component {
        id: backButton
        Rectangle {
            width: root.buttonHeight
            height: root.buttonHeight
            radius: 3
            color: root.forceSwitchMode ? root.forceSwitchBackButtonColor : root.backButtonColor
            opacity: root.forceSwitchMode ? 0.5 : 1.0
            Text {
                anchors.centerIn: parent
                text: "←"
                color: "white"
                font.pixelSize: root.buttonFontSize
                font.family: root.menuFontFamily
            }
            MouseArea {
                anchors.fill: parent
                enabled: !root.forceSwitchMode
                onClicked: stack.pop()
            }
        }
    }

    Component {
        id: selectionTemplate
        Item {
            property alias content: contentLoader.sourceComponent

            anchors.fill: parent
            anchors.margins: root.gridSpacing

            Rectangle {
                id: container
                anchors.fill: parent
                color: "transparent"

                Row {
                    id: mainRow
                    anchors.fill: parent
                    spacing: root.gridSpacing

                    Item {
                        id: contentContainer
                        height: parent.height
                        width: parent.width - backButtonContainer.width - parent.spacing

                        Loader {
                            id: contentLoader
                            anchors.fill: parent
                            anchors.margins: root.gridSpacing
                        }
                    }

                    Item {
                        id: backButtonContainer
                        width: root.buttonHeight + root.gridSpacing * 2
                        height: parent.height

                        Loader {
                            sourceComponent: backButton
                            anchors.centerIn: parent
                            width: root.buttonHeight
                            height: root.buttonHeight
                        }
                    }
                }
            }
        }
    }

    Component {
        id: attackSelection
        Loader {
            sourceComponent: selectionTemplate
            onLoaded: {
                item.content = attackContent
            }
        }
    }
    Component {
        id: switchSelection
        Loader {
            sourceComponent: selectionTemplate
            onLoaded: {
                item.content = switchContent
            }
        }
    }
    Component {
        id: catchSelection
        Loader {
            sourceComponent: selectionTemplate
            onLoaded: {
                item.content = catchContent
            }
        }
    }
    Component {
        id: runSelection
        Loader {
            sourceComponent: selectionTemplate
            onLoaded: {
                item.content = runContent
            }
        }
    }
}
