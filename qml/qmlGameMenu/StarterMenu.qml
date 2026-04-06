import QtQuick 2.15
import QtQuick.Controls 2.15
import "../Style/PokeColor.js" as PokeColor

Rectangle {
    id: root
    color: "#2b2b2b"
    property double uiScale: 1
    readonly property int pad: Math.round(30 * uiScale)
    readonly property int dividerW: Math.round(1 * uiScale)
    readonly property int labelHeight: Math.round(30 * uiScale)
    readonly property int contentSpacing: Math.round(8 * uiScale)
    readonly property color dividerColor: "#3d3d3d"
    property string linkText: "desktopinvasion.com"
    property string linkURL: "https://www.desktopinvasion.com/?page=credits"
    property bool inNameEditMode: false
    property bool inNickNameEditMode: false
    property color subheaderColor: "#aaaaaa"
    property color colorVeryFaint: "#999999"
    property color colorNameBg: "#383838"
    property color colorNameBdr: "#555555"
    property color colorNameHov: "#444444"
    property color colorNameEdit: "#404040"
    property color colorNameEditBdr: "#5294e2"
    property color colorAccent: "#5294e2"
    property color colorSubtext: "#aaaaaa"
    property int fontSizeLg: Math.round(22 * uiScale)
    property int fontSizeMd: Math.round(18 * uiScale)
    property int fontSizeSm: Math.round(16 * uiScale)
    property string p2pFont: "Press Start 2P"
    property string dotGothicFont: "DotGothic16"
    property string playerName: ""
    property string nickName: ""
    property int trainerId: -1
    property int pokeId: -1
    property int maxTrainerNameLength: 10
    property int maxNickNameLength: 10
    signal startGame(string playerName, string nickName, int trainerId, int pokeId)
    property int slide: 0
    property var starterList: [
        { id: 1,  name: "BULBASAUR", type: "Grass" },
        { id: 152, name: "CHIKORITA", type: "Grass" },
        { id: 252, name: "TREECKO", type: "Grass" },
        { id: 387, name: "TURTWIG", type: "Grass" },
        { id: 4,  name: "CHARMANDER", type: "Fire" },
        { id: 155, name: "CYNDAQUIL", type: "Fire" },
        { id: 255, name: "TORCHIC", type: "Fire" },
        { id: 390, name: "CHIMCHAR", type: "Fire" },
        { id: 7,  name: "SQUIRTLE", type: "Water" },
        { id: 158, name: "TOTODILE", type: "Water" },
        { id: 258, name: "MUDKIP", type: "Water" },
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
    function toggleNickNameEditMode() {
        if (inNickNameEditMode) {
            finishNickNameEditing()
        } else {
            inNickNameEditMode = true
            nickNameField.forceActiveFocus()
        }
    }
    function finishNickNameEditing() {
        nickName = nickNameField.text
        inNickNameEditMode = false
    }
    function cancelNickNameEditing() {
        nickNameField.text = nickName
        inNickNameEditMode = false
    }

    // Modal overlay that blocks all clicks while editing
    Rectangle {
        anchors.fill: parent
        color: "transparent"
        visible: root.inNameEditMode || root.inNickNameEditMode
        z: 1
        MouseArea {
            anchors.fill: parent
            cursorShape: undefined
            // Consume clicks to prevent them from reaching underlying elements
            onClicked: { /* do nothing, just block */ }
        }
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
        width: Math.ceil(spriteWidth * iconScale * root.uiScale)
        height: Math.ceil(spriteHeight * iconScale * root.uiScale)
        radius: Math.round(8 * root.uiScale)
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
            radius: Math.round(8 * root.uiScale)
            border.width: (mouseArea.containsMouse || selected) ? Math.round(2 * root.uiScale) : 0
            border.color: (mouseArea.containsMouse || selected) ? Qt.rgba(root.colorAccent.r, root.colorAccent.g, root.colorAccent.b, 0.6) : "transparent"
        }
        Image {
            id: sprite
            anchors.centerIn: parent
            anchors.horizontalCenterOffset: horizontalOffset * root.uiScale
            anchors.verticalCenterOffset: verticalOffset * root.uiScale
            width: Math.ceil(spriteWidth * iconScale * root.uiScale)
            height: Math.ceil(spriteHeight * iconScale * root.uiScale)
            sourceClipRect: Qt.rect(0, frameIndex * spriteHeight, spriteWidth, spriteHeight)
            smooth: false
            antialiasing: false
        }
    }
    Item {
        id: slide0
        anchors.fill: parent
        anchors.margins: root.pad
        visible: root.slide === 0

        Flickable {
            anchors.fill: parent
            contentHeight: column0.implicitHeight
            contentWidth: Math.max(column0.implicitWidth, parent.width)
            clip: true
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }

            Column {
                id: column0
                spacing: 0
                Text {
                    width: root.width - 2 * root.pad
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
                    width: root.width*0.8
                    text: "DesktopInvasion is a desktop pet game that spawns sprites on top of your desktop screen. Battle wild sprites with a MOUSE DOUBLE CLICK. In a battle you can catch wild sprites or train those that you have already caught. DesktopInvasion can be controlled from the SYSTEM TRAY; that's that section in the corner of your desktop where you have the icons for your Wifi, Bluetooth, etc. If you RIGHT CLICK the DesktopInvasion icon, you can:\n\n• Open the game menu\n• Toggle whether the sprites are active \n• Toggle \"Pet mode\" to interact with your sprites\n• Manage your saved games\n• Quit DesktopInvasion"
                    font.family: root.dotGothicFont
                    font.pixelSize: root.fontSizeMd
                    color: "#ffffff"
                    wrapMode: Text.Wrap
                    horizontalAlignment: Text.AlignJustify
                }
                Item { width: parent.width; height: root.pad }
                Rectangle { width: parent.width; height: root.dividerW; color: root.dividerColor }
                Item { width: parent.width; height: root.pad }
                Text {
                    width: root.width*0.8
                    text: "DesktopInvasion is a non-commercial passion project that is available for free. I have never and will never seek any fees or donations for this game. This game makes extensive use of material that I do not own, namely the Pokémon sprite assets as well as the Pokémon gameplay. It is my sincere belief that I have used this material in a spirit of fair use. Apart from moderate use of LLMs, all source code is written by me, with gameplay based on widely available sources on gen-4 gameplay. Note that I have simplified the gameplay by, among other things, excluding items, abilities and a large number of moves."
                    font.family: root.dotGothicFont
                    font.pixelSize: root.fontSizeMd
                    color: "#ffffff"
                    wrapMode: Text.Wrap
                }
                Item { width: parent.width; height: root.pad / 2 + Math.round(3 * uiScale) }
                Row {
                    spacing: Math.round(5 * uiScale)
                    width: root.width - 2 * root.pad
                    Text {
                        text: "For a full list of credits, see:"
                        font.family: root.dotGothicFont
                        font.pixelSize: root.fontSizeMd
                        color: "#ffffff"
                        wrapMode: Text.Wrap
                    }
                    Text {
                        text: root.linkText
                        font.family: root.dotGothicFont
                        font.pixelSize: root.fontSizeMd
                        color: "#ffffff"
                        font.underline: true
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: undefined
                            hoverEnabled: false
                            onClicked: Qt.openUrlExternally(root.linkURL)
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
                        width: Math.round(4 * 48 * uiScale)
                        label: "NEXT →"
                        selectable: true
                        onClicked: root.slide = 1
                    }
                }
            }
        }
    }
    Item {
        id: slide1
        anchors.fill: parent
        anchors.margins: root.pad
        visible: root.slide === 1

        Flickable {
            anchors.fill: parent
            contentHeight: column1.implicitHeight
            contentWidth: Math.max(column1.implicitWidth, parent.width)
            clip: true
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }

            Column {
                id: column1
                spacing: 0
                Row {
                    width: root.width - 2 * root.pad
                    PcButton {
                        width: Math.round(4 * 48 * uiScale)
                        label: "← BACK"
                        selectable: true
                        onClicked: root.slide = 0
                    }
                }
                Item { width: parent.width; height: root.pad }
                Rectangle { width: parent.width; height: root.dividerW; color: root.dividerColor }
                Item { width: parent.width; height: root.pad }
                Text {
                    width: root.width - 2 * root.pad
                    text: root.inNameEditMode ? "[PRESS ENTER TO CONFIRM] Choose your trainer name:" : "Choose your trainer name:"
                    font.family: root.dotGothicFont
                    font.pixelSize: root.fontSizeMd
                    color: "#ffffff"
                    wrapMode: Text.Wrap
                }
                Item { width: parent.width; height: root.pad }
                Item {
                    width: parent.width
                    height: Math.round(44 * uiScale)
                    z: 2 // above modal overlay
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
                        anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: Math.round(11 * uiScale) }
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
                            leftMargin: Math.round(10 * uiScale)
                            rightMargin: root.inNameEditMode ? Math.round(10 * uiScale) : Math.round(36 * uiScale)
                            verticalCenter: parent.verticalCenter
                        }
                        text: root.playerName
                        font.family: root.p2pFont
                        font.pixelSize: root.fontSizeMd
                        font.bold: true
                        color: "#ffffff"
                        maximumLength: root.maxTrainerNameLength
                        clip: true
                        enabled: true
                        readOnly: !root.inNameEditMode
                        cursorVisible: root.inNameEditMode
                        selectByMouse: root.inNameEditMode
                        Keys.onReturnPressed: root.finishNameEditing()
                        Keys.onEscapePressed: root.cancelNameEditing()
                    }
                    Text {
                        anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: Math.round(10 * uiScale) }
                        text: "Enter name..."
                        font.family: root.dotGothicFont
                        font.pixelSize: root.fontSizeSm
                        color: root.colorVeryFaint
                        visible: !root.inNameEditMode && nameField.text.length === 0
                    }
                    MouseArea {
                        id: nameMouseArea
                        anchors.fill: parent
                        cursorShape: undefined
                        hoverEnabled: true
                        onClicked: {
                            if (!root.inNameEditMode) {
                                root.toggleNameEditMode()
                            } else {
                                nameField.forceActiveFocus()
                            }
                        }
                    }
                }
                Item { width: parent.width; height: root.pad }
                Rectangle { width: parent.width; height: root.dividerW; color: root.dividerColor }
                Item { width: parent.width; height: root.pad }
                Text {
                    width: root.width - 2 * root.pad
                    text: "Choose your trainer avatar:"
                    font.family: root.dotGothicFont
                    font.pixelSize: root.fontSizeMd
                    color: "#ffffff"
                    wrapMode: Text.Wrap
                }
                Item { width: parent.width; height: root.pad }
                Grid {
                    id: trainerGrid
                    width: Math.round(64 * 16 * uiScale)
                    height: Math.round(64 * 5 * uiScale)
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
                        width: Math.round(4 * 48 * uiScale)
                        label: "NEXT →"
                        selectable: root.playerName !== "" && root.trainerId !== -1
                        onClicked: root.slide = 2
                    }
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

        Flickable {
            anchors.fill: parent
            contentHeight: column2.implicitHeight
            contentWidth: Math.max(column2.implicitWidth, parent.width)
            clip: true
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }

            Column {
                id: column2
                spacing: 0
                Row {
                    width: root.width - 2 * root.pad
                    PcButton {
                        width: Math.round(4 * 48 * uiScale)
                        label: "← BACK"
                        selectable: true
                        onClicked: root.slide = 1
                    }
                }
                Item { width: parent.width; height: root.pad }
                Rectangle { width: parent.width; height: root.dividerW; color: root.dividerColor }
                Item { width: parent.width; height: root.pad }
                Text {
                    width: root.width - 2 * root.pad
                    text: "Choose your starter:"
                    font.family: root.dotGothicFont
                    font.pixelSize: root.fontSizeMd
                    color: "#ffffff"
                    wrapMode: Text.Wrap
                }
                Item { width: parent.width; height: root.pad }
                Row {
                    spacing: root.pad
                    Grid {
                        id: starterGrid
                        columns: 4
                        rows: 3
                        spacing: Math.round(16 * uiScale)
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
                                onHovered: slide2.hoveredStarterIndex = index
                                onUnhovered: slide2.hoveredStarterIndex = -1
                                onClicked: {
                                    root.pokeId = modelData.id;
                                    root.nickName = modelData.name
                                }
                            }
                        }
                    }
                    Item {
                        width: root.width - 2 * root.pad - starterGrid.width - root.pad
                        height: starterGrid.height
                        clip: true
                        Column {
                            anchors.centerIn: parent
                            spacing: root.pad / 2
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
                                width: Math.round(32 * 6 * uiScale)
                                height: Math.round(32 * 6 * uiScale)
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
                Text {
                    width: root.width - 2 * root.pad
                    text: root.inNickNameEditMode ? "[PRESS ENTER TO CONFIRM] Choose a nickname for your starter:" : "(Optional) Choose a nickname for your starter:"
                    font.family: root.dotGothicFont
                    font.pixelSize: root.fontSizeMd
                    color: "#ffffff"
                    wrapMode: Text.Wrap
                }
                Item { width: parent.width; height: root.pad }
                Item {
                    width: parent.width
                    height: Math.round(44 * uiScale)
                    z: 2 // above modal overlay
                    Rectangle {
                        anchors.fill: parent
                        radius: 4
                        color: root.inNickNameEditMode ? root.colorNameEdit
                               : (nickNameMouseArea.containsMouse ? root.colorNameHov : root.colorNameBg)
                        border.color: root.inNickNameEditMode ? root.colorNameEditBdr
                                      : (nickNameMouseArea.containsMouse ? root.colorAccent : root.colorNameBdr)
                        border.width: root.inNickNameEditMode ? 2 : 1
                    }
                    Text {
                        anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: Math.round(11 * uiScale) }
                        text: "✎"
                        font.pixelSize: root.fontSizeLg
                        color: root.colorSubtext
                        opacity: nickNameMouseArea.containsMouse ? 0.9 : 0.4
                        visible: !root.inNickNameEditMode
                        transform: Scale { xScale: -1 }
                    }
                    TextInput {
                        id: nickNameField
                        anchors {
                            left: parent.left
                            right: parent.right
                            leftMargin: Math.round(10 * uiScale)
                            rightMargin: root.inNickNameEditMode ? Math.round(10 * uiScale) : Math.round(36 * uiScale)
                            verticalCenter: parent.verticalCenter
                        }
                        text: root.nickName
                        font.family: root.p2pFont
                        font.pixelSize: root.fontSizeMd
                        font.bold: true
                        color: "#ffffff"
                        maximumLength: root.maxNickNameLength
                        clip: true
                        enabled: true
                        readOnly: !root.inNickNameEditMode
                        cursorVisible: root.inNickNameEditMode
                        selectByMouse: root.inNickNameEditMode
                        Keys.onReturnPressed: root.finishNickNameEditing()
                        Keys.onEscapePressed: root.cancelNickNameEditing()
                    }
                    Text {
                        anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: Math.round(10 * uiScale) }
                        text: "Enter a starter nickname..."
                        font.family: root.dotGothicFont
                        font.pixelSize: root.fontSizeSm
                        color: root.colorVeryFaint
                        visible: !root.inNickNameEditMode && nickNameField.text.length === 0
                    }
                    MouseArea {
                        id: nickNameMouseArea
                        anchors.fill: parent
                        cursorShape: undefined
                        hoverEnabled: true
                        onClicked: {
                            if (!root.inNickNameEditMode) {
                                root.toggleNickNameEditMode()
                            } else {
                                nickNameField.forceActiveFocus()
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
                        width: Math.round(2 * 4 * 48 * uiScale)
                        label: "START NEW GAME"
                        selectable: (root.pokeId !== -1) && (root.nickName != "")
                        onClicked: root.startGame(root.playerName, root.nickName, root.trainerId, root.pokeId)
                        btnColor: "#e67a00"
                    }
                }
            }
        }
    }
}
