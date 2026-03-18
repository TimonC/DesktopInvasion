import QtQuick 2.15
import QtQuick.Controls 2.15
import "StyleSheet/PokeType.js" as PokeType

Rectangle {
    id: root
    color: "transparent"

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

    property color textBarTextColor: "black"
    property color menuTextColor: "black"
    property color attackTextColor: "white"

    property color textBarBackgroundColor: "white"
    property color textBarBorderColor: "black"

    property color attackButtonColor: "red"
    property color switchButtonColor: "green"
    property color catchButtonColor: "yellow"
    property color runButtonColor: "blue"

    property color borderColor: "#999999"
    property color disabledBorderColor: "#777777"
    property color disabledBackgroundColor: "#b0b0b0"
    property color placeholderTextColor: "#a0a0a0"
    property real enabledOpacity: 1
    property real disabledOpacity: 0.5

    property color highHealthColor: "#4CAF50"
    property color mediumHealthColor: "#FF9800"
    property color lowHealthColor: "#FF0000"
    property color faintedHealthColor: "#8B0000"

    property real selectedIconScale: 1.1
    property real normalIconOpacity: 1.0
    property real faintedIconOpacity: 0.8

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

    function lighterColor(baseColor) {
        var c = Qt.color(baseColor)
        return Qt.rgba(
            Math.min(1, c.r + 0.15),
            Math.min(1, c.g + 0.15),
            Math.min(1, c.b + 0.15),
            1
        )
    }

    function darkerColor(baseColor) {
        var c = Qt.color(baseColor)
        return Qt.rgba(
            c.r * 0.75,
            c.g * 0.75,
            c.b * 0.75,
            1
        )
    }

    function healthColor(healthRatio) {
        if (healthRatio >= 0.5) return highHealthColor
        if (healthRatio >= 0.25) return mediumHealthColor
        if (healthRatio > 0) return lowHealthColor
        return faintedHealthColor
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
            border.width: root.borderWidth
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
                    palette.buttonText: root.menuTextColor
                    font.pixelSize: root.buttonFontSize
                    font.family: root.menuFontFamily
                    width: root.buttonWidth
                    height: root.buttonHeight
                    onClicked: stack.push(attackSelection)
                }
                RoundButton {
                    text: "Switch"
                    palette.button: root.switchButtonColor
                    palette.buttonText: root.menuTextColor
                    font.pixelSize: root.buttonFontSize
                    font.family: root.menuFontFamily
                    width: root.buttonWidth
                    height: root.buttonHeight
                    onClicked: stack.push(switchSelection)
                }
                RoundButton {
                    text: "Catch"
                    palette.button: root.catchButtonColor
                    palette.buttonText: root.menuTextColor
                    font.pixelSize: root.buttonFontSize
                    font.family: root.menuFontFamily
                    width: root.buttonWidth
                    height: root.buttonHeight
                    onClicked: stack.push(catchSelection)
                }
                RoundButton {
                    text: "Run"
                    palette.button: root.runButtonColor
                    palette.buttonText: root.menuTextColor
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
            spacing: root.gridSpacing

            property real cellWidth: (parent.width - spacing) / 2
            property real cellHeight: (parent.height - spacing) / 2

            Repeater {
                model: 4
                Item {
                    width: cellWidth
                    height: cellHeight

                    property string moveName: party.moves[0][index].name || "---"
                    property string moveType: party.moves[0][index].type || "Null"
                    property bool moveEnabled: moveType !== "Null"
                    property color baseColor: moveEnabled ? PokeType.typeColor(moveType) : root.disabledBackgroundColor

                    Rectangle {
                        anchors.fill: parent
                        radius: 4
                        color: moveEnabled ? lighterColor(baseColor) : root.disabledBorderColor
                        opacity: moveEnabled ? root.enabledOpacity : root.disabledOpacity
                    }

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: root.borderWidth
                        radius: 2
                        gradient: Gradient {
                            GradientStop { position: 0; color: moveEnabled ? lighterColor(baseColor) : root.disabledBackgroundColor }
                            GradientStop { position: 1; color: moveEnabled ? darkerColor(baseColor) : root.disabledBackgroundColor }
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
                        lineHeight: 0.9
                        color: moveEnabled ? root.attackTextColor : root.placeholderTextColor
                    }

                    MouseArea {
                        anchors.fill: parent
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

            property real cellWidth: (parent.width - spacing*2) / 3
            property real cellHeight: (parent.height - spacing) / 2

            Repeater {
                model: 6
                Item {
                    width: cellWidth
                    height: cellHeight

                    Rectangle {
                        anchors.fill: parent
                        radius: 4
                        color: (root.party.iconIds[index] >= 0 && party.healthRatios[index] > 0)
                              ? (root.selectedIndex === index ? root.borderColor : root.disabledBorderColor)
                              : root.disabledBorderColor
                        opacity: root.party.iconIds[index] >= 0 ? root.enabledOpacity : root.disabledOpacity
                    }

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: root.borderWidth
                        radius: 2
                        opacity: root.party.iconIds[index] >= 0 ? root.enabledOpacity : root.disabledOpacity
                        gradient: Gradient {
                            GradientStop { position: 0; color: root.party.iconIds[index] >= 0 ? lighterColor(healthColor(party.healthRatios[index])) : root.disabledBackgroundColor }
                            GradientStop { position: 1; color: root.party.iconIds[index] >= 0 ? darkerColor(healthColor(party.healthRatios[index])) : root.disabledBackgroundColor }
                        }
                    }

                    Item {
                        anchors.centerIn: parent
                        width: 40
                        height: 30
                        visible: root.party.iconIds[index] >= 0

                        PokemonIcon{
                            id: iconFrame
                            anchors.centerIn: parent
                            frameIndex: root.party.iconIds[index]
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
        Item {
            anchors.fill: parent

            Grid {
                id: grid
                columns: 2
                rows: 2
                spacing: root.gridSpacing
                anchors.centerIn: parent

                property real cellWidth: (parent.width - spacing) / 2.5
                property real cellHeight: (parent.height - spacing) / 2

                Repeater {
                    model: 4
                    Item {
                        width: grid.cellWidth
                        height: grid.cellHeight

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
                                GradientStop { position: 0; color: ballEnabled ? lighterColor(PokeType.typeColor("Normal")) : root.disabledBackgroundColor }
                                GradientStop { position: 1; color: ballEnabled ? darkerColor(PokeType.typeColor("Normal")) : root.disabledBackgroundColor }
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
                            onClicked: root.catchChosen(index)
                        }
                    }
                }
            }
        }
    }

    Component {
            id: runContent
            Item {
                anchors.fill: parent
                RoundButton {
                    anchors.centerIn: parent
                    palette.button: root.runButtonColor
                    text: "Confirm run"
                    width: root.buttonWidth*1.6
                    height: root.buttonHeight
                    font.pixelSize: root.buttonFontSize
                    font.family: root.menuFontFamily
                    onClicked: root.runChosen()
                }
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
