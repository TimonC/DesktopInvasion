import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "Style/PokeColor.js" as PokeColor

Rectangle {
    id: root
    color: "transparent"

    property int menuTransitionDuration: 100

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

    property string opponentName: ""

    property color textBarTextColor: "black"
    property color menuTextColor: "white"

    property color fightButtonColor: PokeColor.darker("red")
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
    property string ballSpriteSheet: "qrc:/assets/HGSS/reordered_pokeballs.png"
    property var nrOfBalls: [1000, 0, 0, 0]
    property list<string> ballNames: ["Poké Ball", "Great Ball", "Ultra Ball", "Master Ball"]

    property int selectedIndex: 0
    property bool forceSwitchMode: false

    // Layout proportions
    property real backButtonWidthRatio: 0.1  // 10% of width
    property real backButtonHeightRatio: 0.9  // 90% of height
    property real contentMarginsRatio: 0.02  // 2% margin

    // Dynamic calculated properties
    property real contentWidth: menuWidth * (1 - backButtonWidthRatio * 2 - contentMarginsRatio * 4)
    property real contentHeight: menuHeight * (1 - contentMarginsRatio * 2)
    property real backButtonWidth: menuWidth * backButtonWidthRatio
    property real backButtonHeight: menuHeight * backButtonHeightRatio

    signal actionRound(int actionIndex, string actionType)
    signal fightChosen(int fightId)
    signal runChosen(bool removeWild)
    signal switchChosen(int newPartyIdx)

    property alias stack: stack

    property var party: {
        "pokedexIds": [-1, -1, -1, -1, -1, -1],
        "spriteIds": [-1, -1, -1, -1, -1, -1],
        "ballIds": [-1, -1, -1, -1, -1, -1],
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

    function _setPartyMember(partyIdx, pokedexId, spriteId, ballId, pokemonName, lvl, totalHealth, moves) {
        var temp = party
        temp.pokedexIds[partyIdx] = pokedexId
        temp.spriteIds[partyIdx] = spriteId
        temp.ballIds[partyIdx] = ballId

        temp.names[partyIdx] = pokemonName
        temp.lvls[partyIdx] = lvl

        temp.healthTotals[partyIdx] = totalHealth
        if(totalHealth>0) temp.healthRatios[partyIdx] = 1;

        temp.moves[partyIdx] = moves
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

    function getText() {
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

        // Dynamic sizing based on content area
        width: root.contentWidth / 2 - root.gridSpacing
        height: root.contentHeight / 2 - root.gridSpacing
    }

    StackView {
        id: stack
        initialItem: textBarComponent
        width: root.menuWidth
        height: root.menuHeight
        z: 1

        pushEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: root.menuTransitionDuration
            }
        }

        pushExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: root.menuTransitionDuration
            }
        }

        popEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: root.menuTransitionDuration
            }
        }

        popExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: root.menuTransitionDuration
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
            width: root.menuWidth
            height: root.menuHeight
            color: root.textBarBackgroundColor
            border.color: root.textBarBorderColor
            border.width: root.borderWidth
            property string text: ""
            radius: 5

            Text {
                id: textBarText
                anchors.fill: parent
                anchors.margins: root.menuWidth * 0.02 // 2% margin
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

    Component {
        id: rootSelection
        Item {
            width: root.menuWidth
            height: root.menuHeight

            Grid {
                anchors.centerIn: parent
                columns: 2
                spacing: root.gridSpacing
                width: root.contentWidth
                height: root.contentHeight

                GradientRoundButton {
                    text: "Fight"
                    buttonColor: root.fightButtonColor
                    onClicked: {
                        stack.push(fightSelection)
                    }
                }

                GradientRoundButton {
                    text: "Switch"
                    buttonColor: root.switchButtonColor
                    onClicked: stack.push(switchSelection)
                }

                GradientRoundButton {
                    text: "Catch"
                    buttonColor: root.catchButtonColor
                    onClicked: stack.push(catchSelection)
                }

                GradientRoundButton {
                    text: "Run"
                    buttonColor: root.runButtonColor
                    onClicked: stack.push(runSelection)
                }
            }
        }
    }

    Component {
        id: fightContent
        Item {
            width: root.contentWidth
            height: root.contentHeight

            Grid {
                id: fightGrid
                anchors.fill: parent
                anchors.margins: root.contentMarginsRatio * root.menuHeight
                columns: 2
                rows: 2
                spacing: root.gridSpacing

                property int partyIndex: 0

                Repeater {
                    model: 4

                    Item {
                        id: moveItem
                        width: fightGrid.width / 2 - root.gridSpacing / 2
                        height: fightGrid.height / 2 - root.gridSpacing / 2

                        // Function that re-evaluates when dependencies change
                        function getMoveData() {
                            if (party && party.moves && party.moves[root.selectedIndex]) {
                                return party.moves[root.selectedIndex][index] || {name: "---", type: "Null"}
                            }
                            return {name: "---", type: "Null"}
                        }

                        property string moveName: getMoveData().name || "---"
                        property string moveType: getMoveData().type || "Null"
                        property bool moveEnabled: moveType !== "Null"
                        property color baseColor: moveEnabled ? PokeColor.typeColor(moveType) : root.disabledBackgroundColor

                        Rectangle {
                            anchors.fill: parent
                            radius: 4
                            color: moveEnabled ? PokeColor.lighter(baseColor) : root.disabledBorderColor
                            opacity: moveEnabled ? root.enabledOpacity : root.disabledOpacity
                        }

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: root.borderWidth
                            radius: 2
                            gradient: Gradient {
                                GradientStop { position: 0; color: moveEnabled ? PokeColor.lighter(baseColor) : root.disabledBackgroundColor }
                                GradientStop { position: 1; color: moveEnabled ? PokeColor.darker(baseColor) : root.disabledBackgroundColor }
                            }
                            opacity: moveEnabled ? root.enabledOpacity : root.disabledOpacity
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
                            lineHeight: 1.4
                            color: moveEnabled ? root.menuTextColor : root.placeholderTextColor
                        }

                        MouseArea {
                            anchors.fill: parent
                            enabled: moveEnabled
                            onClicked: {
                                root.actionRound(index, "Fight")
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: switchContent
        Item {
            width: root.contentWidth
            height: root.contentHeight

            Grid {
                anchors.fill: parent
                anchors.margins: root.contentMarginsRatio * root.menuHeight
                columns: 3
                rows: 2
                spacing: root.gridSpacing / 2

                Repeater {
                    model: 6

                    Item {
                        width: (parent.width - root.gridSpacing) / 3
                        height: (parent.height - root.gridSpacing) / 2

                        Rectangle {
                            anchors.fill: parent
                            radius: 4
                            color: (root.party.pokedexIds[index] >= 0 && party.healthRatios[index] > 0)
                                  ? (root.selectedIndex === index ? root.selectedBorderColor : root.borderColor)
                                  : root.disabledBorderColor
                            opacity: root.party.pokedexIds[index] >= 0 ? root.enabledOpacity : root.disabledOpacity
                            border.width: root.selectedIndex === index ? root.borderWidth * 2 : 0
                            border.color: root.selectedIndex === index ? root.selectedBorderColor : "transparent"
                        }

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: root.borderWidth
                            radius: 2
                            opacity: root.party.pokedexIds[index] >= 0 ? root.enabledOpacity : root.disabledOpacity
                            gradient: Gradient {
                                GradientStop {
                                    position: 0;
                                    color: root.party.pokedexIds[index] >= 0 ?
                                        PokeColor.lighter(PokeColor.healthColor(party.healthRatios[index]))
                                        : root.disabledBackgroundColor
                                }
                                GradientStop {
                                    position: 1;
                                    color: root.party.pokedexIds[index] >= 0 ?
                                        PokeColor.darker(PokeColor.healthColor(party.healthRatios[index]))
                                        : root.disabledBackgroundColor
                                }
                            }
                        }

                        Item {
                            anchors.centerIn: parent
                            visible: root.party.pokedexIds[index] >= 0

                            PokemonIcon {
                                id: iconFrame
                                anchors.centerIn: parent
                                frameIndex: root.party.pokedexIds[index]-1
                                iconScale: root.iconScale
                                opacity: party.healthRatios[index] > 0 ? root.normalIconOpacity : root.faintedIconOpacity
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            visible: root.party.pokedexIds[index] < 0
                            text: "---"
                            font.pixelSize: root.buttonFontSize
                            font.family: root.menuFontFamily
                            color: root.placeholderTextColor
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        MouseArea {
                            anchors.fill: parent
                            enabled: !root.forceSwitchMode ||
                                     (root.selectedIndex !== index &&
                                      root.party.pokedexIds[index] >= 0 &&
                                      party.healthRatios[index] > 0)
                            onClicked: {
                                if(root.selectedIndex !== index &&
                                   root.party.pokedexIds[index] >= 0 &&
                                   party.healthRatios[index] > 0){
                                    var oldIndex = root.selectedIndex
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

    Component {
        id: catchContent
        Item {
            width: root.contentWidth
            height: root.contentHeight

            Grid {
                id: grid
                anchors.fill: parent
                anchors.margins: root.contentMarginsRatio * root.menuHeight
                columns: 2
                rows: 2
                spacing: root.gridSpacing

                Repeater {
                    model: 4

                    Item {
                        width: (grid.width - root.gridSpacing) / 2
                        height: (grid.height - root.gridSpacing) / 2

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
                                GradientStop { position: 0; color: ballEnabled ? PokeColor.lighter(PokeColor.typeColor("Normal")) : root.disabledBackgroundColor }
                                GradientStop { position: 1; color: ballEnabled ? PokeColor.darker(PokeColor.typeColor("Normal")) : root.disabledBackgroundColor }
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
                            onClicked: {
                                root.actionRound(index, "Catch")
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: runContent
        Item {
            width: root.contentWidth
            height: root.contentHeight

            Column {
                anchors.fill: parent
                anchors.margins: root.contentMarginsRatio * root.menuHeight
                spacing: root.gridSpacing / 2

                Repeater {
                    model: [
                        {
                            text: "Remove '" + root.opponentName + "'",
                            action: function() { root.runChosen(true) }
                        },
                        {
                            text: "Escape battle",
                            action: function() { root.runChosen(false) }
                        }
                    ]

                    Rectangle {
                        height: (parent.height - root.gridSpacing / 2) / 2
                        width: parent.width
                        radius: 20
                        border.color: root.borderColor
                        border.width: root.borderWidth

                        gradient: Gradient {
                            GradientStop { position: 0; color: PokeColor.lighter("blue") }
                            GradientStop { position: 1; color: PokeColor.darker("blue") }
                        }

                        Text {
                            text: modelData.text
                            color: root.menuTextColor
                            font.pixelSize: root.moveFontSize
                            font.family: root.menuFontFamily
                            font.weight: Font.DemiBold
                            anchors.centerIn: parent
                            anchors.margins: parent.height * 0.1
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            anchors.fill: parent
                            lineHeight: 1.4
                            wrapMode: Text.Wrap
                            maximumLineCount: 2
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: modelData.action()
                        }
                    }
                }
            }
        }
    }

    Component {
        id: backButton
        Rectangle {
            width: root.backButtonWidth
            height: root.backButtonHeight
            radius: 3
            color: root.forceSwitchMode ? root.forceSwitchBackButtonColor : root.backButtonColor
            opacity: root.forceSwitchMode ? 0.5 : 1.0

            Text {
                anchors.centerIn: parent
                text: "←"
                color: "white"
                font.pixelSize: Math.min(root.buttonFontSize * 1.5, root.backButtonHeight * 0.5)
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

            width: root.menuWidth
            height: root.menuHeight

            RowLayout {
                anchors.fill: parent
                anchors.margins: root.contentMarginsRatio * root.menuHeight
                spacing: root.contentMarginsRatio * root.menuWidth

                // Left spacer (10% width)
                Item {
                    Layout.preferredWidth: root.backButtonWidthRatio * root.menuWidth
                    Layout.fillHeight: true
                }

                // Content area (80% width)
                Loader {
                    id: contentLoader
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                // Back button area (10% width)
                Item {
                    Layout.preferredWidth: root.backButtonWidthRatio * root.menuWidth
                    Layout.fillHeight: true

                    Loader {
                        sourceComponent: backButton
                        anchors.centerIn: parent
                    }
                }
            }
        }
    }

    Component {
        id: fightSelection
        Loader {
            width: root.menuWidth
            height: root.menuHeight
            sourceComponent: selectionTemplate
            onLoaded: {
                item.content = fightContent
            }
        }
    }

    Component {
        id: switchSelection
        Loader {
            width: root.menuWidth
            height: root.menuHeight
            sourceComponent: selectionTemplate
            onLoaded: {
                item.content = switchContent
            }
        }
    }

    Component {
        id: catchSelection
        Loader {
            width: root.menuWidth
            height: root.menuHeight
            sourceComponent: selectionTemplate
            onLoaded: {
                item.content = catchContent
            }
        }
    }

    Component {
        id: runSelection
        Loader {
            width: root.menuWidth
            height: root.menuHeight
            sourceComponent: selectionTemplate
            onLoaded: {
                item.content = runContent
            }
        }
    }
}
