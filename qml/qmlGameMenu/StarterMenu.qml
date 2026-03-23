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
    property bool   inNameEditMode:  false
    property int  trainerId: -1
    property int pokeId: -1

    signal startGame()
    signal nameChanged(string newName)

    property int  slide:         0

    function toggleNameEditMode() {
        if (inNameEditMode) {
            finishNameEditing()
        } else {
            inNameEditMode = true
            nameField.forceActiveFocus()
        }
    }

    function finishNameEditing() {
        if (nameField.text !== playerName)
            nameChanged(nameField.text)
        playerName = nameField.text
        inNameEditMode = false
    }

    function cancelNameEditing() {
        nameField.text = playerName
        inNameEditMode = false
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

            Item { width: parent.width; height: root.pad }
            Rectangle { width: parent.width; height: root.dividerW; color: root.dividerColor }
            Item { width: parent.width; height: root.pad }

            Text {
                width: parent.width
                text: "Hello " + (nameField.text ?? "") + "!"
                font.family: root.p2pFont
                font.pixelSize: root.fontSizeMd
                color: "#ffffff"
                wrapMode: Text.Wrap
            }

            Item { width: parent.width; height: root.pad/2 }

            Text {
                width: parent.width
                text: "Choose your trainer avatar:"
                font.family: root.dotGothicFont
                font.pixelSize: root.fontSizeMd
                color: "#ffffff"
                wrapMode: Text.Wrap
            }

            Item { width: parent.width; height: root.pad }
            Rectangle { width: parent.width; height: root.dividerW; color: root.dividerColor }
            Item { width: parent.width; height: root.pad }

            Grid {
                width: 32*16*2
                height: 32*5*2
                rows: 5
                columns: 16
                rowSpacing: 0
                columnSpacing: 0

                Repeater {
                    id: partyRepeater
                    model: 79

                    Rectangle {
                        property bool isHovered: hoverArea.containsMouse
                        property bool isSelected: root.trainerId==index

                        width: image.width
                        height: image.height
                        radius: 8
                        color: isHovered ? (Qt.rgba(root.colorAccent.r, root.colorAccent.g, root.colorAccent.b, 0.30))
                                    : (isSelected ? Qt.rgba(root.colorAccent.r, root.colorAccent.g, root.colorAccent.b, 0.15)
                                    : "transparent")

                        MouseArea {
                            id: hoverArea
                            anchors.fill: parent
                            cursorShape: undefined
                            hoverEnabled: true
                            onClicked: {
                                root.trainerId = index;
                            }
                        }

                        Rectangle {
                            anchors.fill: parent
                            color: "transparent"
                            radius: 8
                            border.width: (isHovered || isSelected) ? 2 : 0
                            border.color: (isHovered || isSelected) ? Qt.rgba(root.colorAccent.r, root.colorAccent.g, root.colorAccent.b, 0.6) : "transparent"
                        }

                        Image {
                            id: image
                            anchors.centerIn: parent
                            anchors.horizontalCenterOffset: 1
                            property int spriteWidth: 32
                            property int spriteHeight: 32
                            property double iconScale: 2

                            width: Math.ceil(spriteWidth * iconScale)
                            height: Math.ceil(spriteHeight * iconScale)

                            source: "qrc:/assets/HGSS/reordered_trainers.png"
                            sourceClipRect: Qt.rect(0, index * spriteHeight, spriteWidth, spriteHeight)
                            smooth: false
                            antialiasing: false
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
                layoutDirection: Qt.RightToLeft
                PcButton {
                    width: 4 * 48
                    label: "START"
                    selectable: true
                    onClicked: root.startGame()
                }
            }
        }
    }
}
