import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "Style/PokeColor.js" as PokeColor

Rectangle {
    id: root
    color: "transparent"

    // Menu dimensions
    property int menuWidth: 0
    property int menuHeight: 0
    property int buttonWidth: 0
    property int buttonHeight: 0
    property double textBarHeightRatio: 0.9
    property int gridSpacing: 0
    property int borderWidth: 0
    property double contentMarginsRatio: 0.02
    property double backButtonWidthRatio: 0.1
    property double backButtonHeightRatio: 0.8

    // Calculated dimensions
    property real contentWidth: Math.floor(menuWidth * (1 - backButtonWidthRatio * 2 - contentMarginsRatio * 2))
    property real contentHeight: Math.floor(menuHeight * (1 - contentMarginsRatio * 2))
    property real backButtonWidth: Math.floor(menuWidth * backButtonWidthRatio)
    property real backButtonHeight: Math.floor(menuHeight * backButtonHeightRatio)

    // Text properties
    property int buttonFontSize: 0
    property int moveFontSize: 0
    property int textBarFontSize: 0
    property string menuFontFamily: ""
    property string textBarFontFamily: ""
    property color textBarTextColor: "black"
    property color menuTextColor: "white"

    // Colors
    property color fightButtonColor: "#ff3333"
    property color switchButtonColor: "green"
    property color catchButtonColor: "#e67a00"
    property color runButtonColor: "#3366ff"
    property color borderColor: "#999999"
    property color disabledBorderColor: "#777777"
    property color selectedBorderColor: "#1976D2"
    property color disabledBackgroundColor: "#757575"
    property color placeholderTextColor: "#a0a0a0"
    property color textBarBackgroundColor: "white"
    property color textBarBorderColor: "black"
    property color backButtonColor: "lightblue"
    property color forceSwitchBackButtonColor: "#b0bec5"

    // Opacity and effects
    property real enabledOpacity: 1
    property real disabledOpacity: 0.7
    property real normalIconOpacity: 1.0
    property real faintedIconOpacity: 0.7
    property real hoverScale: 1.04
    property int menuTransitionDuration: 0
    property double iconScale: 1.0

    // Game state
    property bool textBarShown: false
    property bool forceSwitchMode: false
    property int selectedIndex: 0
    property string opponentName: ""
    property int ballSpriteWidth: 16
    property int ballSpriteHeight: 23
    property string ballSpriteSheet: "qrc:/assets/HGSS/reordered_pokeballs.png"
    property var nrOfBalls: [1000, 0, 0, 0]
    property list<string> ballNames: ["Poké Ball", "Great Ball", "Ultra Ball", "Master Ball"]

    // Signals
    signal actionRound(int actionIndex, string actionType)
    signal fightChosen(int fightId)
    signal runChosen(bool removeWild)
    signal switchChosen(int newPartyIdx)

    property alias stack: stack

    // Party data
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

    // Functions
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
        root.textBarShown = true
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
        root.textBarShown = false
        stack.replace(switchSelection)
    }

    function resetToRoot() {
        forceSwitchMode = false
        root.textBarShown = false
        stack.replace(rootSelection)
    }

    // Components
    component HoverableMouseArea: MouseArea{
        cursorShape: undefined
        hoverEnabled: true
        z: 1
        property var hoverTarget: parent
        property bool hoverEffectEnabled: true

        // Check if mouse is already over when component appears
        Component.onCompleted: {
            if (containsMouse && hoverEffectEnabled && hoverTarget) {
                if (hoverTarget.hasOwnProperty("scale")) hoverTarget.scale = root.hoverScale
                if (hoverTarget.hasOwnProperty("hovered")) hoverTarget.hovered = true
            }
        }

        onEntered: {
            if (hoverEffectEnabled && hoverTarget) {
                if (hoverTarget.hasOwnProperty("scale")) hoverTarget.scale = root.hoverScale
                if (hoverTarget.hasOwnProperty("hovered")) hoverTarget.hovered = true
            }
        }
        onExited: {
            if (hoverEffectEnabled && hoverTarget) {
                if (hoverTarget.hasOwnProperty("scale")) hoverTarget.scale = 1.0
                if (hoverTarget.hasOwnProperty("hovered")) hoverTarget.hovered = false
                if (hoverTarget.hasOwnProperty("down")) hoverTarget.down = false
            }
        }
        onPressed: {
            if (hoverEffectEnabled && hoverTarget && hoverTarget.hasOwnProperty("down")) {
                hoverTarget.down = true
            }
        }
        onReleased: {
            if (hoverEffectEnabled && hoverTarget && hoverTarget.hasOwnProperty("down")) {
                hoverTarget.down = false
            }
        }
        onCanceled: {
            if (hoverEffectEnabled && hoverTarget && hoverTarget.hasOwnProperty("down")) {
                hoverTarget.down = false
            }
        }
    }
    component GradientRoundButton: Item {
        id: gradientButton
        required property color buttonColor
        required property string text
        signal clicked()

        property alias wrapMode: label.wrapMode
        property alias elide: label.elide
        property bool down: false
        property bool hovered: false
        property alias font: label.font
        property bool enabled: true
        property color borderColor: PokeColor.lighter(buttonColor)
        property real borderWidth: root.borderWidth

        width: Math.floor(root.contentWidth / 2 - root.gridSpacing)
        height: Math.floor(root.contentHeight / 2 - root.gridSpacing)

        scale: 1.0
        Behavior on scale { NumberAnimation { duration: 100 } }

        HoverableMouseArea {
            anchors.fill: parent
            enabled: gradientButton.enabled
            hoverTarget: gradientButton
            onClicked: gradientButton.clicked()
        }
        Rectangle {
            anchors.fill: parent
            radius: height / 2
            color: gradientButton.enabled ? borderColor : root.disabledBorderColor
            opacity: gradientButton.enabled ? root.enabledOpacity : root.disabledOpacity
        }
        Rectangle {
            anchors.fill: parent
            anchors.margins: borderWidth
            radius: Math.max(0, height / 2 - borderWidth)
            gradient: Gradient {
                GradientStop {
                    position: 0.0;
                    color: gradientButton.enabled ?
                           (gradientButton.down ? PokeColor.darker(buttonColor) : PokeColor.lighter(buttonColor))
                           : root.disabledBackgroundColor
                }
                GradientStop {
                    position: 1.0;
                    color: gradientButton.enabled ?
                           (gradientButton.down ? PokeColor.darker(PokeColor.darker(buttonColor)) : PokeColor.darker(buttonColor))
                           : root.disabledBackgroundColor
                }
            }
            Behavior on gradient { ColorAnimation { duration: 100 } }
            Behavior on opacity { NumberAnimation { duration: 100 } }
        }
        Text {
            id: label
            anchors.centerIn: parent
            width: parent.width * 0.9
            text: gradientButton.text
            font.pixelSize: root.buttonFontSize
            font.family: root.menuFontFamily
            font.weight: Font.DemiBold
            color: gradientButton.enabled ? root.menuTextColor : root.placeholderTextColor
            wrapMode: Text.NoWrap
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            opacity: gradientButton.enabled ? 1.0 : root.disabledOpacity
        }
    }

    // StackView
    StackView {
        id: stack
        initialItem: textBarComponent
        width: root.menuWidth
        height: root.menuHeight
        z: 1

        pushEnter: null
        pushExit: null

        popEnter: null
        popExit: null

        replaceEnter: null
        replaceExit: null
    }

    // Components
    Component {
        id: textBarComponent
        Rectangle {
            id: textBar
            width: root.menuWidth
            height: Math.floor(root.menuHeight * root.textBarHeightRatio)
            color: root.textBarBackgroundColor
            border.color: root.textBarBorderColor
            border.width: root.borderWidth
            property string text: ""
            radius: 5
            Text {
                anchors.fill: parent
                anchors.margins: Math.floor(root.menuWidth * 0.02)
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
                    onClicked: stack.push(fightSelection)
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
                anchors.margins: Math.floor(root.contentMarginsRatio * root.menuHeight)
                columns: 2
                rows: 2
                spacing: root.gridSpacing
                property int partyIndex: 0
                Repeater {
                    model: 4
                    Item {
                        id: moveItem
                        width: Math.floor(fightGrid.width / 2 - root.gridSpacing / 2)
                        height: Math.floor(fightGrid.height / 2 - root.gridSpacing / 2)
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
                        property bool hovered: false
                        property bool down: false
                        scale: 1.0
                        Behavior on scale { NumberAnimation { duration: 100 } }
                        Rectangle {
                            anchors.fill: parent
                            radius: 4
                            color: moveEnabled ? (down ? PokeColor.darker(baseColor) : PokeColor.lighter(baseColor)) : root.disabledBorderColor
                            opacity: moveEnabled ? root.enabledOpacity : root.disabledOpacity
                            Behavior on color { ColorAnimation { duration: 100 } }
                        }
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: root.borderWidth
                            radius: 2
                            gradient: Gradient {
                                GradientStop { position: 0; color: moveEnabled ? (down ? PokeColor.darker(baseColor) : PokeColor.lighter(baseColor)) : root.disabledBackgroundColor }
                                GradientStop { position: 1; color: moveEnabled ? (down ? PokeColor.darker(PokeColor.darker(baseColor)) : PokeColor.darker(baseColor)) : root.disabledBackgroundColor }
                            }
                            opacity: moveEnabled ? root.enabledOpacity : root.disabledOpacity
                            Behavior on gradient { ColorAnimation { duration: 100 } }
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
                        HoverableMouseArea {
                            anchors.fill: parent
                            enabled: moveEnabled
                            hoverTarget: moveItem
                            hoverEffectEnabled: moveEnabled
                            onClicked: root.actionRound(index, "Fight")
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
                anchors.margins: Math.floor(root.contentMarginsRatio * root.menuHeight)
                columns: 3
                rows: 2
                spacing: Math.floor(root.gridSpacing / 2)
                Repeater {
                    model: 6
                    Item {
                        id: switchItem
                        width: Math.floor((parent.width - root.gridSpacing) / 3)
                        height: Math.floor((parent.height - root.gridSpacing) / 2)
                        property bool hovered: false
                        property bool down: false
                        property color healthColor: root.party.pokedexIds[index] >= 0 ?
                                                   PokeColor.healthColor(party.healthRatios[index]) :
                                                   root.disabledBackgroundColor
                        property bool isSelected: root.selectedIndex === index
                        property bool isEmpty: root.party.pokedexIds[index] < 0
                        property bool isEnabled: !root.forceSwitchMode ?
                                               (root.selectedIndex !== index &&
                                                root.party.pokedexIds[index] >= 0 &&
                                                party.healthRatios[index] > 0) :
                                               (root.party.pokedexIds[index] >= 0 &&
                                                party.healthRatios[index] > 0)
                        scale: 1.0
                        Behavior on scale { NumberAnimation { duration: 100 } }
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
                                        (down ? PokeColor.darker(healthColor) : PokeColor.lighter(healthColor))
                                        : root.disabledBackgroundColor
                                }
                                GradientStop {
                                    position: 1;
                                    color: root.party.pokedexIds[index] >= 0 ?
                                        (down ? PokeColor.darker(PokeColor.darker(healthColor)) : PokeColor.darker(healthColor))
                                        : root.disabledBackgroundColor
                                }
                            }
                            Behavior on gradient { ColorAnimation { duration: 100 } }
                        }
                        Item {
                            anchors.centerIn: parent
                            visible: root.party.pokedexIds[index] >= 0
                            PokemonIcon {
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
                        HoverableMouseArea {
                            anchors.fill: parent
                            enabled: isEnabled
                            hoverTarget: switchItem
                            hoverEffectEnabled: isEnabled && !isSelected && !isEmpty
                            onClicked: {
                                if(root.selectedIndex !== index &&
                                   root.party.pokedexIds[index] >= 0 &&
                                   party.healthRatios[index] > 0){
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
                anchors.margins: Math.floor(root.contentMarginsRatio * root.menuHeight)
                columns: 2
                rows: 2
                spacing: root.gridSpacing
                Repeater {
                    model: 4
                    Item {
                        id: ballItem
                        width: Math.floor((grid.width - root.gridSpacing) / 2)
                        height: Math.floor((grid.height - root.gridSpacing) / 2)
                        property bool ballEnabled: root.nrOfBalls[index] > 0
                        property bool hovered: false
                        property bool down: false
                        property color typeColor: PokeColor.typeColor("Normal")
                        scale: 1.0
                        Behavior on scale { NumberAnimation { duration: 100 } }
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
                                GradientStop { position: 0; color: ballEnabled ? (down ? PokeColor.darker(typeColor) : PokeColor.lighter(typeColor)) : root.disabledBackgroundColor }
                                GradientStop { position: 1; color: ballEnabled ? (down ? PokeColor.darker(PokeColor.darker(typeColor)) : PokeColor.darker(typeColor)) : root.disabledBackgroundColor }
                            }
                            Behavior on gradient { ColorAnimation { duration: 100 } }
                        }
                        Row {
                            anchors.centerIn: parent
                            spacing: root.gridSpacing
                            Item {
                                width: root.ballSpriteWidth * root.iconScale
                                height: root.ballSpriteHeight * root.iconScale
                                Image {
                                    source: root.ballSpriteSheet
                                    sourceClipRect: Qt.rect(0, root.ballSpriteHeight * index,
                                                           root.ballSpriteWidth, root.ballSpriteHeight)
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.verticalCenterOffset: 1 * root.iconScale
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    width: root.ballSpriteWidth * root.iconScale
                                    height: root.ballSpriteHeight * root.iconScale
                                    opacity: ballEnabled ? 1.0 : 0.5
                                    smooth:false
                                    antialiasing:false
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
                        HoverableMouseArea {
                            anchors.fill: parent
                            enabled: ballEnabled
                            hoverTarget: ballItem
                            onClicked: root.actionRound(index, "Catch")
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
                anchors.margins: Math.floor(root.contentMarginsRatio * root.menuHeight)
                spacing: Math.floor(root.gridSpacing / 2)
                Repeater {
                    model: [
                        { text: "Remove '" + root.opponentName + "'", action: function() { root.runChosen(true) } },
                        { text: "Escape battle", action: function() { root.runChosen(false) } }
                    ]
                    Item {
                        id: runOptionItem
                        height: Math.floor((parent.height - root.gridSpacing / 2) / 2)
                        width: parent.width
                        property bool hovered: false
                        property bool down: false
                        scale: 1.0
                        Behavior on scale { NumberAnimation { duration: 100 } }
                        Rectangle {
                            anchors.fill: parent
                            radius: 20
                            border.color: root.borderColor
                            border.width: root.borderWidth
                            gradient: Gradient {
                                GradientStop { position: 0; color: down ? PokeColor.darker(root.runButtonColor) : PokeColor.lighter(root.runButtonColor) }
                                GradientStop { position: 1; color: down ? PokeColor.darker(PokeColor.darker(root.runButtonColor)) : PokeColor.darker(root.runButtonColor) }
                            }
                            Behavior on gradient { ColorAnimation { duration: 100 } }
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
                        HoverableMouseArea {
                            anchors.fill: parent
                            hoverTarget: runOptionItem
                            onClicked: modelData.action()
                        }
                    }
                }
            }
        }
    }

    Component {
        id: backButton
        GradientRoundButton {
            buttonColor: root.forceSwitchMode ? root.forceSwitchBackButtonColor : root.backButtonColor
            opacity: root.forceSwitchMode ? 0.5 : 1.0
            width: root.backButtonWidth
            height: root.backButtonHeight
            onClicked: if(!root.forceSwitchMode) stack.pop()
            text: "←"
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
                anchors.margins: Math.floor(root.contentMarginsRatio * root.menuHeight)
                spacing: Math.floor(root.contentMarginsRatio * root.menuWidth)
                Item {
                    Layout.preferredWidth: root.backButtonWidth
                    Layout.fillHeight: true
                }
                Loader {
                    id: contentLoader
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
                Item {
                    Layout.preferredWidth: root.backButtonWidth
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
            onLoaded: item.content = fightContent
        }
    }

    Component {
        id: switchSelection
        Loader {
            width: root.menuWidth
            height: root.menuHeight
            sourceComponent: selectionTemplate
            onLoaded: item.content = switchContent
        }
    }

    Component {
        id: catchSelection
        Loader {
            width: root.menuWidth
            height: root.menuHeight
            sourceComponent: selectionTemplate
            onLoaded: item.content = catchContent
        }
    }

    Component {
        id: runSelection
        Loader {
            width: root.menuWidth
            height: root.menuHeight
            sourceComponent: selectionTemplate
            onLoaded: item.content = runContent
        }
    }
}
