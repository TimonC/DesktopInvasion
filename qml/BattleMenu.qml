import QtQuick 2.15
import QtQuick.Controls 2.15
import "Style/PokeColor.js" as PokeColor

Rectangle {
    id: root
    color: "transparent"
    property int buttonTransitionDuration: 100
    property int frameSize: 0
    property int buttonWidth: frameSize * 2
    property int buttonHeight: frameSize * 0.75
    property int menuHeight: 0
    property int menuWidth: 0
    property int buttonFontSize: 0
    property int moveFontSize: 0
    property int textBarFontSize: 0
    property string menuFontFamily: ""
    property string textBarFontFamily: ""
    property int borderWidth: 2
    property int gridSpacing: 3
    property int selectedBorderWidth: 2
    property string opponentName: ""
    property color textBarTextColor: "black"
    property color menuTextColor: "white"
    property color attackTextColor: "white"
    property color attackButtonColor: PokeColor.darker("red")
    property color switchButtonColor: PokeColor.darker("green")
    property color catchButtonColor: PokeColor.darker("orange")
    property color runButtonColor: PokeColor.darker("blue")
    property color borderColor: "#999999"
    property color disabledBorderColor: "#777777"
    property color selectedBorderColor: "#1976D2"
    property color disabledBackgroundColor: "#757575"
    property color placeholderTextColor: "#a0a0a0"
    property real enabledOpacity: 1
    property real disabledOpacity: 0.7
    property color textBarBackgroundColor: "white"
    property color textBarBorderColor: "black"
    property real iconScale: 1.0
    property real normalIconOpacity: 1.0
    property real faintedIconOpacity: 0.7
    property color backButtonColor: "lightblue"
    property color forceSwitchBackButtonColor: "#b0bec5"
    property color catchButtonBackground: "white"
    property color catchButtonPressedBackground: "#f0f0f0"
    property color catchButtonBorderColor: "black"
    property double spriteScale: 1
    property int ballSpriteWidth: 16
    property int ballSpriteHeight: 23
    property string ballSpriteSheet: "qrc:/assets/HGSS/Pokeballs_transparent_reordered.png"
    property var nrOfBalls: [1000, 0, 0, 0]
    property list<string> ballNames: ["Poké Ball", "Great Ball", "Ultra Ball", "Master Ball"]
    property int selectedIndex: 0
    property bool forceSwitchMode: false

    signal actionRound(int actionIndex, string actionType)
    signal attackChosen(int attackId)
    signal runChosen(bool removeWild)
    signal switchChosen(int newPartyIdx)

    property alias stack: stack
    property bool stackReady: false

    property var party: {
        "spriteIds": [-1, -1, -1, -1, -1, -1],
        "iconIds": [-1, -1, -1, -1, -1, -1],
        "ballIds": [-1, -1, -1, -1, -1, -1],
        "gens": [-1, -1, -1, -1, -1, -1],
        "names": ["", "", "", "", "", ""],
        "lvls": [-1, -1, -1, -1, -1, -1],
        "healthRatios": [-1, -1, -1, -1, -1, -1],
        "healthTotals": [-1, -1, -1, -1, -1, -1],
        "moves": [
            [[], [], [], []],
            [[], [], [], []],
            [[], [], [], []],
            [[], [], [], []],
            [[], [], [], []],
            [[], [], [], []]
        ]
    }

    function _setPartyMember(partyIdx, spriteId, iconId, ballId, gen, pokemonName, lvl, totalHealth, moves) {
        var temp = party
        temp.spriteIds[partyIdx] = spriteId
        temp.iconIds[partyIdx] = iconId
        temp.ballIds[partyIdx] = ballId
        temp.gens[partyIdx] = gen
        temp.names[partyIdx] = pokemonName
        temp.lvls[partyIdx] = lvl
        temp.healthTotals[partyIdx] = totalHealth
        temp.moves[partyIdx] = moves
        temp.healthRatios[partyIdx] = 1
        party = temp
    }

    function showTextBar() {
        if (stackReady && stack.currentItem !== textBarView) {
            stack.replace(textBarView)
        }
    }

    function updateText(text) {
        textBarView.text = text
    }

    function getText() {
        return textBarView.text
    }

    function forceSwitch() {
        if (stackReady && stack.currentItem !== switchSelectionView) {
            forceSwitchMode = true
            stack.replace(switchSelectionView)
        }
    }

    function resetToRoot() {
        if (stackReady && stack.currentItem !== rootSelectionView) {
            forceSwitchMode = false
            stack.replace(rootSelectionView)
        }
    }

    component GradientRoundButton: RoundButton {
        id: gradientButton
        required property color buttonColor
        property int wrapMode: Text.NoWrap
        property int elide: Text.ElideRight
        palette.buttonText: root.menuTextColor
        palette.button: gradientButton.buttonColor
        font.pixelSize: root.buttonFontSize
        font.family: root.menuFontFamily
        font.weight: Font.DemiBold
        width: root.buttonWidth
        height: root.buttonHeight
    }

    // Cached views container
    Item {
        id: cachedViews
        anchors.fill: parent
        anchors.topMargin: root.gridSpacing * 3

        // Text Bar View (cached)
        Rectangle {
            id: textBarView
            visible: false
            color: root.textBarBackgroundColor
            border.color: root.textBarBorderColor
            border.width: root.borderWidth
            property string text: ""
            height: root.menuHeight * 0.8
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
                text: textBarView.text
                font.pixelSize: root.textBarFontSize
                font.family: root.textBarFontFamily
                color: root.textBarTextColor
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }
        }

        // Root Selection View (cached)
        Item {
            id: rootSelectionView
            visible: false

            Grid {
                anchors.centerIn: parent
                columns: 2
                spacing: root.gridSpacing

                GradientRoundButton {
                    text: "Fight"
                    buttonColor: root.attackButtonColor
                    onClicked: if (root.stackReady && stack.currentItem !== attackSelectionView) stack.replace(attackSelectionView)
                }

                GradientRoundButton {
                    text: "Switch"
                    buttonColor: root.switchButtonColor
                    onClicked: if (root.stackReady && stack.currentItem !== switchSelectionView) stack.replace(switchSelectionView)
                }

                GradientRoundButton {
                    text: "Catch"
                    buttonColor: root.catchButtonColor
                    onClicked: if (root.stackReady && stack.currentItem !== catchSelectionView) stack.replace(catchSelectionView)
                }

                GradientRoundButton {
                    text: "Run"
                    buttonColor: root.runButtonColor
                    onClicked: if (root.stackReady && stack.currentItem !== runSelectionView) stack.replace(runSelectionView)
                }
            }
        }

        // Attack Selection View (cached)
        Item {
            id: attackSelectionView
            visible: false
            anchors.margins: root.gridSpacing / 2

            Row {
                width: parent.width
                height: parent.height
                layoutDirection: Qt.RightToLeft

                Item {
                    id: attackBackButtonContainer
                    width: root.buttonHeight + root.gridSpacing * 2
                    height: parent.height

                    Rectangle {
                        anchors.centerIn: parent
                        width: root.buttonHeight
                        height: root.menuHeight * 0.7
                        radius: 3
                        color: root.backButtonColor

                        Text {
                            anchors.centerIn: parent
                            text: "←"
                            color: "white"
                            font.pixelSize: root.buttonFontSize
                            font.family: root.menuFontFamily
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: if (root.stackReady && stack.currentItem !== rootSelectionView) stack.replace(rootSelectionView)
                        }
                    }
                }

                Item {
                    width: parent.width - attackBackButtonContainer.width * 2
                    height: parent.height

                    Grid {
                        id: attackGrid
                        anchors.fill: parent
                        anchors.margins: root.gridSpacing
                        columns: 2
                        rows: 2
                        spacing: root.gridSpacing
                        property real cellWidth: (width - spacing) / 2
                        property real cellHeight: (height - spacing) / 2

                        Repeater {
                            model: 4

                            Item {
                                id: moveItem
                                width: attackGrid.cellWidth
                                height: attackGrid.cellHeight

                                property var moveData: (party && party.moves && party.moves[root.selectedIndex]) ?
                                    (party.moves[root.selectedIndex][index] || {name: "---", type: "Null"}) :
                                    {name: "---", type: "Null"}
                                property string moveName: moveData.name || "---"
                                property string moveType: moveData.type || "Null"
                                property bool moveEnabled: moveType !== "Null"
                                property color baseColor: moveEnabled ? PokeColor.typeColor(moveType) : root.disabledBackgroundColor

                                Rectangle {
                                    anchors.fill: parent
                                    radius: 4
                                    color: moveItem.moveEnabled ? PokeColor.lighter(moveItem.baseColor) : root.disabledBorderColor
                                    opacity: moveItem.moveEnabled ? root.enabledOpacity : root.disabledOpacity
                                }

                                Rectangle {
                                    anchors.fill: parent
                                    anchors.margins: root.borderWidth
                                    radius: 2

                                    gradient: Gradient {
                                        GradientStop {
                                            position: 0
                                            color: moveItem.moveEnabled ? PokeColor.lighter(moveItem.baseColor) : root.disabledBackgroundColor
                                        }
                                        GradientStop {
                                            position: 1
                                            color: moveItem.moveEnabled ? PokeColor.darker(moveItem.baseColor) : root.disabledBackgroundColor
                                        }
                                    }
                                    opacity: moveItem.moveEnabled ? root.enabledOpacity : root.disabledOpacity
                                }

                                Text {
                                    anchors.centerIn: parent
                                    width: Math.max(0, parent.width - 8)
                                    text: moveItem.moveName
                                    font.pixelSize: root.moveFontSize
                                    font.weight: Font.DemiBold
                                    font.family: root.menuFontFamily
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    wrapMode: Text.Wrap
                                    maximumLineCount: 2
                                    elide: Text.ElideRight
                                    lineHeight: 1.4
                                    color: moveItem.moveEnabled ? root.attackTextColor : root.placeholderTextColor
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    enabled: moveItem.moveEnabled
                                    onClicked: root.actionRound(index, "Fight")
                                }
                            }
                        }
                    }
                }
            }
        }

        // Switch Selection View (cached)
        Item {
            id: switchSelectionView
            visible: false
            anchors.margins: root.gridSpacing / 2

            Row {
                width: parent.width
                height: parent.height
                layoutDirection: Qt.RightToLeft

                Item {
                    id: switchBackButtonContainer
                    width: root.buttonHeight + root.gridSpacing * 2
                    height: parent.height

                    Rectangle {
                        anchors.centerIn: parent
                        width: root.buttonHeight
                        height: root.menuHeight * 0.7
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
                            onClicked: if (root.stackReady && stack.currentItem !== rootSelectionView) stack.replace(rootSelectionView)
                        }
                    }
                }

                Item {
                    width: parent.width - switchBackButtonContainer.width * 2
                    height: parent.height

                    Grid {
                        anchors.fill: parent
                        anchors.margins: root.gridSpacing
                        columns: 3
                        rows: 2
                        spacing: root.gridSpacing / 2
                        property real cellWidth: (width - spacing * 2) / 3
                        property real cellHeight: (height - spacing) / 2

                        Repeater {
                            model: 6

                            Item {
                                width: parent.cellWidth
                                height: parent.cellHeight

                                property bool isValidPokemon: root.party.iconIds[index] >= 0
                                property bool isAlive: party.healthRatios[index] > 0
                                property bool isSelected: root.selectedIndex === index
                                property bool canSelect: isValidPokemon && isAlive && (!root.forceSwitchMode || !isSelected)

                                Rectangle {
                                    anchors.fill: parent
                                    radius: 4
                                    color: (isValidPokemon && isAlive) ?
                                        (isSelected ? root.selectedBorderColor : root.borderColor) :
                                        root.disabledBorderColor
                                    opacity: isValidPokemon ? root.enabledOpacity : root.disabledOpacity
                                    border.width: isSelected ? root.selectedBorderWidth * 2 : 0
                                    border.color: isSelected ? root.selectedBorderColor : "transparent"
                                }

                                Rectangle {
                                    anchors.fill: parent
                                    anchors.margins: root.borderWidth
                                    radius: 2
                                    opacity: isValidPokemon ? root.enabledOpacity : root.disabledOpacity

                                    gradient: Gradient {
                                        GradientStop {
                                            position: 0
                                            color: isValidPokemon ?
                                                PokeColor.lighter(PokeColor.healthColor(party.healthRatios[index])) :
                                                root.disabledBackgroundColor
                                        }
                                        GradientStop {
                                            position: 1
                                            color: isValidPokemon ?
                                                PokeColor.darker(PokeColor.healthColor(party.healthRatios[index])) :
                                                root.disabledBackgroundColor
                                        }
                                    }
                                }

                                Item {
                                    anchors.centerIn: parent
                                    visible: isValidPokemon

                                    PokemonIcon {
                                        id: iconFrame
                                        anchors.centerIn: parent
                                        frameIndex: root.party.iconIds[index]
                                        scale: root.iconScale
                                        opacity: isAlive ? root.normalIconOpacity : root.faintedIconOpacity
                                    }
                                }

                                Text {
                                    anchors.centerIn: parent
                                    visible: !isValidPokemon
                                    text: "---"
                                    font.pixelSize: root.buttonFontSize
                                    font.family: root.menuFontFamily
                                    color: root.placeholderTextColor
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    enabled: canSelect
                                    onClicked: {
                                        if (!isSelected && isValidPokemon && isAlive) {
                                            root.selectedIndex = index
                                            root.switchChosen(index)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Catch Selection View (cached)
        Item {
            id: catchSelectionView
            visible: false
            anchors.margins: root.gridSpacing / 2

            Row {
                width: parent.width
                height: parent.height
                layoutDirection: Qt.RightToLeft

                Item {
                    id: catchBackButtonContainer
                    width: root.buttonHeight + root.gridSpacing * 2
                    height: parent.height

                    Rectangle {
                        anchors.centerIn: parent
                        width: root.buttonHeight
                        height: root.menuHeight * 0.7
                        radius: 3
                        color: root.backButtonColor

                        Text {
                            anchors.centerIn: parent
                            text: "←"
                            color: "white"
                            font.pixelSize: root.buttonFontSize
                            font.family: root.menuFontFamily
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: if (root.stackReady && stack.currentItem !== rootSelectionView) stack.replace(rootSelectionView)
                        }
                    }
                }

                Item {
                    width: parent.width - catchBackButtonContainer.width * 2
                    height: parent.height

                    Grid {
                        id: catchGrid
                        anchors.fill: parent
                        anchors.margins: root.gridSpacing
                        columns: 2
                        rows: 2
                        spacing: root.gridSpacing
                        property real cellWidth: (width - spacing) / 2
                        property real cellHeight: (height - spacing) / 2

                        Repeater {
                            model: 4

                            Item {
                                width: catchGrid.cellWidth
                                height: catchGrid.cellHeight
                                property bool ballEnabled: root.nrOfBalls[index] > 0

                                Rectangle {
                                    anchors.fill: parent
                                    radius: 4
                                    color: ballEnabled ? root.borderColor : root.disabledBorderColor
                                    opacity: ballEnabled ? root.enabledOpacity : root.disabledOpacity
                                }

                                Rectangle {
                                    anchors.fill: parent
                                    anchors.margins: root.borderWidth
                                    radius: 2
                                    opacity: ballEnabled ? root.enabledOpacity : root.disabledOpacity

                                    gradient: Gradient {
                                        GradientStop {
                                            position: 0
                                            color: ballEnabled ? PokeColor.lighter(PokeColor.typeColor("Normal")) : root.disabledBackgroundColor
                                        }
                                        GradientStop {
                                            position: 1
                                            color: ballEnabled ? PokeColor.darker(PokeColor.typeColor("Normal")) : root.disabledBackgroundColor
                                        }
                                    }
                                }

                                Row {
                                    anchors.centerIn: parent
                                    spacing: root.gridSpacing

                                    Item {
                                        width: root.ballSpriteWidth * root.spriteScale
                                        height: root.ballSpriteHeight * root.spriteScale

                                        Image {
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.verticalCenterOffset: 1
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            source: root.ballSpriteSheet
                                            width: root.ballSpriteWidth * root.spriteScale
                                            height: root.ballSpriteHeight * root.spriteScale
                                            sourceClipRect: Qt.rect(0, root.ballSpriteHeight * index,
                                                                   root.ballSpriteWidth, root.ballSpriteHeight)
                                            smooth: false
                                            antialiasing: false
                                            opacity: ballEnabled ? 1.0 : 0.5
                                        }
                                    }

                                    Text {
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: "×" + (ballEnabled ? (root.nrOfBalls[index] > 999 ? "∞" : root.nrOfBalls[index]) : "0")
                                        font.pixelSize: root.buttonFontSize
                                        font.family: root.menuFontFamily
                                        font.weight: Font.DemiBold
                                        color: root.menuTextColor
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    enabled: ballEnabled
                                    onClicked: root.actionRound(index, "Catch")
                                }
                            }
                        }
                    }
                }
            }
        }

        // Run Selection View (cached)
        Item {
            id: runSelectionView
            visible: false
            anchors.margins: root.gridSpacing / 2

            Row {
                width: parent.width
                height: parent.height
                layoutDirection: Qt.RightToLeft

                Item {
                    id: runBackButtonContainer
                    width: root.buttonHeight + root.gridSpacing * 2
                    height: parent.height

                    Rectangle {
                        anchors.centerIn: parent
                        width: root.buttonHeight
                        height: root.menuHeight * 0.7
                        radius: 3
                        color: root.backButtonColor

                        Text {
                            anchors.centerIn: parent
                            text: "←"
                            color: "white"
                            font.pixelSize: root.buttonFontSize
                            font.family: root.menuFontFamily
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: if (root.stackReady && stack.currentItem !== rootSelectionView) stack.replace(rootSelectionView)
                        }
                    }
                }

                Item {
                    width: parent.width - runBackButtonContainer.width * 2
                    height: parent.height

                    Column {
                        anchors.fill: parent
                        anchors.margins: root.gridSpacing
                        spacing: root.gridSpacing

                        Repeater {
                            model: 2

                            Rectangle {
                                height: (parent.height - parent.spacing) / 2
                                width: parent.width
                                radius: 20
                                border.color: root.borderColor
                                border.width: root.borderWidth

                                gradient: Gradient {
                                    GradientStop { position: 0; color: PokeColor.lighter("blue") }
                                    GradientStop { position: 1; color: PokeColor.darker("blue") }
                                }

                                Text {
                                    text: index === 0 ? "Remove '" + root.opponentName + "'" : "Escape battle"
                                    color: root.menuTextColor
                                    font.pixelSize: root.moveFontSize
                                    font.family: root.menuFontFamily
                                    font.weight: Font.DemiBold
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    anchors.fill: parent
                                    anchors.margins: 8
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: root.runChosen(index === 0)
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // StackView for managing visible view
    StackView {
        id: stack
        anchors.fill: parent
        anchors.topMargin: root.gridSpacing * 3
        z: 1
        initialItem: textBarView

        Component.onCompleted: {
            root.stackReady = true
        }

        pushEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: root.buttonTransitionDuration
            }
        }

        pushExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: root.buttonTransitionDuration
            }
        }

        popEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: root.buttonTransitionDuration
            }
        }

        popExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: root.buttonTransitionDuration
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

    }}
