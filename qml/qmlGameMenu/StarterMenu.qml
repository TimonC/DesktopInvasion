import QtQuick 2.15
import "../Style/PokeColor.js" as PokeColor

Rectangle {
    id: root

    color: "#2b2b2b"

    readonly property int pad:           30
    readonly property int dividerW:      1
    readonly property int labelHeight:   30
    readonly property int contentSpacing:8
    readonly property color dividerColor: "#3d3d3d"

    property bool   inNameEditMode:  false

    property color  subheaderColor: "#aaaaaa"
    property color  colorVeryFaint: "#999999"
    property color  colorNameBg:    "#383838"
    property color  colorNameBdr:   "#555555"
    property color  colorNameHov:   "#444444"
    property color  colorNameEdit:  "#404040"
    property color  colorNameEditBdr:"#5294e2"
    property color  colorAccent:    "#5294e2"
    property color  colorSubtext:   "#aaaaaa"

    property int    fontSizeLg:     22
    property int    fontSizeMd:     18
    property int    fontSizeSm:     16
    property string p2pFont:        "Press Start 2P"
    property string dotGothicFont:  "DotGothic16"

    property string playerName:      ""
    property int  trainerId: -1
    property int  pokeId: -1

    signal startGame(string playerName, int trainerId, int pokeId)

    property int  slide:         0

    property var starterList: [
        { id: 1,  name: "BULBASAUR", type: "Grass" },
        { id: 4,  name: "CHARMANDER", type: "Fire" },
        { id: 7,  name: "SQUIRTLE", type: "Water" },
        { id: 152, name: "CHIKORITA", type: "Grass" },
        { id: 155, name: "CYNDAQUIL", type: "Fire" },
        { id: 158, name: "TOTODILE", type: "Water" },
        { id: 252, name: "TREECKO", type: "Grass" },
        { id: 255, name: "TORCHIC", type: "Fire" },
        { id: 258, name: "MUDKIP", type: "Water" },
        { id: 387, name: "TURTWIG", type: "Grass" },
        { id: 390, name: "CHIMCHAR", type: "Fire" },
        { id: 393, name: "PIPLUP", type: "Water" }
    ]

    function getSelectedStarter() {
        for (var i = 0; i < starterList.length; ++i) {
            if (starterList[i].id === root.pokeId)
                return starterList[i];
        }
        return null;
    }

    function toggleNameEditMode() {
        if (inNameEditMode) {
            finishNameEditing()
        } else {
            inNameEditMode = true
            nameField.forceActiveFocus()
        }
    }

    function finishNameEditing() {
        playerName = nameField.text
        inNameEditMode = false
    }

    function cancelNameEditing() {
        nameField.text = playerName
        inNameEditMode = false
    }

    component SpriteTile: Rectangle {
        id: tile
        signal clicked()
        signal hovered()
        signal unhovered()
        property alias spriteSource: sprite.source
        property int spriteWidth: 32
        property int spriteHeight: 32
        property int frameIndex: 0
        property bool selected: false
        property bool hoverEnabled: true
        property double iconScale: 1.0
        property int horizontalOffset: 1
        property int verticalOffset: 0

        width: Math.ceil(spriteWidth * iconScale)
        height: Math.ceil(spriteHeight * iconScale)
        radius: 8
        color: {
            if (mouseArea.containsMouse)
                return Qt.rgba(root.colorAccent.r, root.colorAccent.g, root.colorAccent.b, 0.30)
            if (selected)
                return Qt.rgba(root.colorAccent.r, root.colorAccent.g, root.colorAccent.b, 0.15)
            return "transparent"
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            cursorShape: undefined
            hoverEnabled: tile.hoverEnabled
            onClicked: tile.clicked()
        }

        Rectangle {
            anchors.fill: parent
            color: "transparent"
            radius: 8
            border.width: (mouseArea.containsMouse || selected) ? 2 : 0
            border.color: (mouseArea.containsMouse || selected) ? Qt.rgba(root.colorAccent.r, root.colorAccent.g, root.colorAccent.b, 0.6) : "transparent"
        }

        Image {
            id: sprite
            anchors.centerIn: parent
            anchors.horizontalCenterOffset: horizontalOffset
            anchors.verticalCenterOffset: verticalOffset
            width: Math.ceil(spriteWidth * iconScale)
            height: Math.ceil(spriteHeight * iconScale)
            sourceClipRect: Qt.rect(0, frameIndex * spriteHeight, spriteWidth, spriteHeight)
            smooth: false
            antialiasing: false
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape:  undefined
        onClicked: {
            if (root.inNameEditMode) root.toggleNameEditMode()
        }
    }

    Item {
        id: slide0
        anchors.fill: parent
        anchors.margins: root.pad
        visible: root.slide === 0

        Column {
            width: parent.width
            spacing: 0

            Text {
                width: parent.width
                text: "Welcome to DesktopInvasion!"
                font.family: root.p2pFont
                font.pixelSize: root.fontSizeLg
                color: "#ffffff"
                wrapMode: Text.Wrap
            }

            Item { width: parent.width; height: root.pad }
            Rectangle { width: parent.width; height: root.dividerW; color: root.dividerColor }
            Item { width: parent.width; height: root.pad }

            Text {
                width: parent.width
                text: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi aliquip ex ea commodo consequat."
                font.family: root.dotGothicFont
                font.pixelSize: root.fontSizeMd
                color: "#ffffff"
                wrapMode: Text.Wrap
            }

            Item { width: parent.width; height: root.pad }
            Rectangle { width: parent.width; height: root.dividerW; color: root.dividerColor }
            Item { width: parent.width; height: root.pad }

            Item {
                width: parent.width
                height: 44

                Rectangle {
                    anchors.fill: parent
                    radius: 4
                    color: root.inNameEditMode ? root.colorNameEdit
                           : (nameMouseArea.containsMouse ? root.colorNameHov : root.colorNameBg)
                    border.color: root.inNameEditMode ? root.colorNameEditBdr
                                  : (nameMouseArea.containsMouse ? root.colorAccent : root.colorNameBdr)
                    border.width: root.inNameEditMode ? 2 : 1
                }

                Text {
                    anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: 11 }
                    text: "✎"
                    font.pixelSize: root.fontSizeLg
                    color: root.colorSubtext
                    opacity: nameMouseArea.containsMouse ? 0.9 : 0.4
                    visible: !root.inNameEditMode
                    transform: Scale { xScale: -1 }
                }

                TextInput {
                    id: nameField
                    anchors {
                        left: parent.left
                        right: parent.right
                        leftMargin: 10
                        rightMargin: root.inNameEditMode ? 10 : 36
                        verticalCenter: parent.verticalCenter
                    }
                    text: root.playerName
                    font.family: root.p2pFont
                    font.pixelSize: root.fontSizeMd
                    font.bold: true
                    color: "#ffffff"
                    maximumLength: 24
                    clip: true
                    enabled: root.inNameEditMode
                    readOnly: !root.inNameEditMode
                    cursorVisible: root.inNameEditMode
                    selectByMouse: root.inNameEditMode
                    Keys.onReturnPressed: root.finishNameEditing()
                    Keys.onEscapePressed: root.cancelNameEditing()
                    onFocusChanged: {
                        if (!activeFocus && root.inNameEditMode)
                            root.cancelNameEditing()
                    }
                }

                Text {
                    anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: 10 }
                    text: "Enter your name..."
                    font.family: root.dotGothicFont
                    font.pixelSize: root.fontSizeSm
                    color: root.colorVeryFaint
                    visible: !root.inNameEditMode && nameField.text.length === 0
                }

                MouseArea {
                    id: nameMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: undefined
                    onClicked: {
                        if (!root.inNameEditMode)
                            root.toggleNameEditMode()
                    }
                }
            }

            Item { width: parent.width; height: root.pad }
            Rectangle { width: parent.width; height: root.dividerW; color: root.dividerColor }
            Item { width: parent.width; height: root.pad }

            Row {
                width: parent.width
                layoutDirection: Qt.RightToLeft
                PcButton {
                    width: 4 * 48
                    label: "NEXT →"
                    selectable: root.playerName !== ""
                    onClicked: root.slide = 1
                }
            }
        }
    }

    Item {
        id: slide1
        anchors.fill: parent
        anchors.margins: root.pad
        visible: root.slide === 1

        Column {
            width: parent.width
            spacing: 0

            Row {
                width: parent.width
                PcButton {
                    width: 4 * 48
                    label: "← BACK"
                    selectable: true
                    onClicked: root.slide = 0
                }
            }

            Item { width: parent.width; height: root.pad/1.5 }
            Rectangle { width: parent.width; height: root.dividerW; color: root.dividerColor }
            Item { width: parent.width; height: root.pad/1.5 }

            Text {
                width: parent.width
                text: "Hello " + (nameField.text ?? "") + "!"
                font.family: root.p2pFont
                font.pixelSize: root.fontSizeMd
                color: "#ffffff"
                wrapMode: Text.Wrap
            }

            Item { width: parent.width; height: root.pad/1.5 }

            Text {
                width: parent.width
                text: "Choose your trainer avatar:"
                font.family: root.dotGothicFont
                font.pixelSize: root.fontSizeMd
                color: "#ffffff"
                wrapMode: Text.Wrap
            }

            Item { width: parent.width; height: root.pad/1.5 }
            Rectangle { width: parent.width; height: root.dividerW; color: root.dividerColor }
            Item { width: parent.width; height: root.pad/1.5 }

            Grid {
                width: 64 * 16
                height: 64 * 5
                rows: 5
                columns: 16
                rowSpacing: 0
                columnSpacing: 0

                Repeater {
                    id: trainerRepeater
                    model: 80

                    SpriteTile {
                        spriteSource: "qrc:/assets/HGSS/reordered_trainers.png"
                        spriteWidth: 32
                        spriteHeight: 32
                        frameIndex: index
                        selected: root.trainerId === index
                        hoverEnabled: true
                        iconScale: 2
                        onClicked: { root.trainerId = index; }
                    }
                }
            }

            Item { width: parent.width; height: root.pad }
            Rectangle { width: parent.width; height: root.dividerW; color: root.dividerColor }
            Item { width: parent.width; height: root.pad }

            Row {
                width: parent.width
                layoutDirection: Qt.RightToLeft
                PcButton {
                    width: 4 * 48
                    label: "NEXT →"
                    selectable: root.trainerId !== -1
                    onClicked: root.slide = 2
                }
            }
        }
    }

    Item {
        id: slide2
        anchors.fill: parent
        anchors.margins: root.pad
        visible: root.slide === 2

        property int hoveredStarterIndex: -1
        property var selectedStarter: root.getSelectedStarter()

        Column {
            width: parent.width
            spacing: 0

            Row {
                width: parent.width
                PcButton {
                    width: 4 * 48
                    label: "← BACK"
                    selectable: true
                    onClicked: root.slide = 1
                }
            }

            Item { width: parent.width; height: root.pad }
            Rectangle { width: parent.width; height: root.dividerW; color: root.dividerColor }
            Item { width: parent.width; height: root.pad }

            Row {
                width: parent.width
                spacing: root.pad

                Grid {
                    id: starterGrid
                    columns: 3
                    rows: 4
                    spacing: 16
                    width: parent.width * 0.6

                    Repeater {
                        id: starterRepeater
                        model: root.starterList

                        SpriteTile {
                            spriteSource: "qrc:/assets/HGSS/reordered_icons.png"
                            spriteWidth: 40
                            spriteHeight: 30
                            frameIndex: modelData.id - 1
                            selected: root.pokeId === modelData.id
                            hoverEnabled: true
                            iconScale: 3
                            horizontalOffset: 1
                            verticalOffset: -10
                            onClicked: { root.pokeId = modelData.id; }
                        }
                    }
                }

                Item {
                    width: parent.width * 0.4
                    height: starterGrid.height
                    clip: true

                    Column {
                        anchors.centerIn: parent
                        spacing: root.pad
                        width: parent.width

                        Text {
                            width: parent.width
                            text: root.playerName !== "" ? root.playerName : "TRAINER"
                            font.family: root.p2pFont
                            font.pixelSize: root.fontSizeLg
                            color: "#ffffff"
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                        }

                        Image {
                            anchors.horizontalCenter: parent.horizontalCenter
                            visible: root.trainerId !== -1
                            width: 32 * 6
                            height: 32 * 6
                            source: "qrc:/assets/HGSS/reordered_trainers.png"
                            sourceClipRect: Qt.rect(0, root.trainerId * 32, 32, 32)
                            smooth: false
                            antialiasing: false
                        }
                        Text {
                            width: parent.width
                            text: "YOUR STARTER"
                            font.family: root.p2pFont
                            font.pixelSize: root.fontSizeMd
                            color: "#aaaaaa"
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                        }

                        Text {
                            width: parent.width
                            text: {
                                if (slide2.hoveredStarterIndex !== -1)
                                    return root.starterList[slide2.hoveredStarterIndex].name
                                if (root.pokeId !== -1) {
                                    for (var i = 0; i < root.starterList.length; ++i) {
                                        if (root.starterList[i].id === root.pokeId)
                                            return root.starterList[i].name
                                    }
                                }
                                return ""
                            }
                            font.family: root.p2pFont
                            font.pixelSize: root.fontSizeLg
                            color: {
                                if (slide2.hoveredStarterIndex !== -1)
                                    return PokeColor.typeColor(root.starterList[slide2.hoveredStarterIndex].type)
                                if (root.pokeId !== -1) {
                                    for (var i = 0; i < root.starterList.length; ++i) {
                                        if (root.starterList[i].id === root.pokeId)
                                            return PokeColor.typeColor(root.starterList[i].type)
                                    }
                                }
                                return root.colorAccent
                            }
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }

            Item { width: parent.width; height: root.pad }
            Rectangle { width: parent.width; height: root.dividerW; color: root.dividerColor }
            Item { width: parent.width; height: root.pad }

            Row {
                width: parent.width
                layoutDirection: Qt.RightToLeft
                PcButton {
                    width: 2 * 4 * 48
                    label: "START NEW GAME"
                    selectable: root.pokeId !== -1
                    onClicked: root.startGame(root.playerName, root.trainerId, root.pokeId)
                    btnColor: "green"
                }
            }
        }
    }
}
