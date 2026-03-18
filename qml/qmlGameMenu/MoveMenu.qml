import QtQuick 2.15
import QtQuick.Controls 2.15
import "../Style/PokeColor.js" as PokeColor
// MoveMenu
// Fills the exact same bounding box that the three default sections occupy.
// Receives pokeData and emits returnClicked to go back to default.
// Emits nameChanged(newName) when the user edits the pokémon's nickname.
Item {
    id: moveMenu

    property var    pokeData:    null
    property string spriteSheet: "qrc:/assets/HGSS/reordered_sprites.png"
    property int    frameWidth:  32
    property int    frameHeight: 32
    property real   scaleFactor: 6
    property int    rowId:       0

    property string mainFont:   "Press Start 2P"
    property string bodyFont:   "DotGothic16"
    property int    fontSizeLg: 22
    property int    fontSizeMd: 18
    property int    fontSizeSm: 16

    // Add property for name edit mode
    property bool inNameEditMode: false

    // ── Colors (mirror PokeView) ───────────────────────────────────────────────
    property color colorBg:           "#2b2b2b"
    property color colorText:         "#ffffff"
    property color colorSubtext:      "#aaaaaa"
    property color colorFaint:        "#cccccc"
    property color colorVeryFaint:    "#999999"
    property color colorDivider:      "#3d3d3d"
    property color colorMoveCard:     "#383838"
    property color colorEligibleCard: "#303040"
    property color colorTypePillText: "#ffffff"
    property color buttonColor:       "#3c3c3c"
    property color buttonHoverColor:  "#4c4c4c"
    property color buttonActiveColor: "#5294e2"

    // New colors for name input
    property color nameInputBg:       "#3a3a3a"
    property color nameInputHoverBg:  "#4a4a4a"
    property color nameInputEditBg:   "#2a3a5a"
    property color nameInputBorder:   "#5a5a5a"
    property color nameInputEditBorder: "#7aa5ff"

    // ── Layout ────────────────────────────────────────────────────────────────
    property int margin:      14
    property int cardGap:     6
    property int cardPad:     8
    property int typePillW:   84
    property int typePillH:   fontSizeMd + 6
    property int sectionGap:  10

    signal returnClicked()
    signal nameChanged(string newName)

    // Function to toggle name edit mode
    function toggleNameEditMode() {
        if (inNameEditMode) {
            // Exit edit mode
            inNameEditMode = false;
            nameField.focus = false;
            // Emit name changed if different
            if (nameField.text !== (pokeData ? pokeData.name : "")) {
                moveMenu.nameChanged(nameField.text);
            }
        } else {
            // Enter edit mode
            inNameEditMode = true;
            nameField.forceActiveFocus();
        }
    }

    // Keep name field in sync when pokeData changes (new pokemon opened)
    onPokeDataChanged: {
        if (pokeData) nameField.text = pokeData.name
        // Exit edit mode when pokemon changes
        inNameEditMode = false;
    }

    // ── Reusable: type pill ────────────────────────────────────────────────────
    component TypePill: Rectangle {
        property string typeName: ""
        width:  moveMenu.typePillW
        height: moveMenu.typePillH
        radius: 3
        gradient: Gradient {
            GradientStop { position: 0.0; color: PokeColor.lighter(PokeColor.typeColor(typeName)) }
            GradientStop { position: 1.0; color: PokeColor.darker(PokeColor.typeColor(typeName)) }
        }
        Text {
            anchors.centerIn: parent
            text: PokeColor.typeColor(typeName) === "transparent" ? "" : typeName
            font.family: moveMenu.bodyFont
            font.pixelSize: moveMenu.fontSizeSm
            color: moveMenu.colorTypePillText
        }
    }

    // ── Reusable: compact move row (no flavor text) ────────────────────────────
    component CompactMoveCard: Rectangle {
        id: cmc
        property string moveName: ""
        property string moveType: ""
        property int    movePow:  0
        property int    moveAcc:  0

        width:  parent ? parent.width : 200
        height: moveMenu.typePillH + moveMenu.cardPad * 2
        color:  moveMenu.colorMoveCard
        radius: 5

        Row {
            anchors {
                verticalCenter: parent.verticalCenter
                left:  parent.left; right: parent.right
                leftMargin: moveMenu.cardPad; rightMargin: moveMenu.cardPad
            }
            spacing: 8

            TypePill { typeName: cmc.moveType }

            Text {
                width: parent.width - moveMenu.typePillW - powerLabel.implicitWidth - 20
                anchors.verticalCenter: parent.verticalCenter
                text: cmc.moveName
                font.family: moveMenu.mainFont; font.pixelSize: moveMenu.fontSizeSm
                color: moveMenu.colorText; elide: Text.ElideRight
            }

            Text {
                id: powerLabel
                anchors.verticalCenter: parent.verticalCenter
                text: (cmc.movePow > 0 ? cmc.movePow : "—") + "  " +
                      (cmc.moveAcc > 0 ? cmc.moveAcc + "%" : "—")
                font.family: moveMenu.bodyFont; font.pixelSize: moveMenu.fontSizeSm
                color: moveMenu.colorSubtext
            }
        }
    }

    // ── Reusable: eligible move row ───────────────────────────────────────────
    component EligibleMoveRow: Rectangle {
        id: emr
        property string moveName: ""
        property string moveType: ""
        property int    movePow:  0
        property int    moveAcc:  0

        width:  parent ? parent.width : 200
        height: moveMenu.typePillH + moveMenu.cardPad * 2
        color:  moveMenu.colorEligibleCard
        radius: 4

        Row {
            anchors {
                verticalCenter: parent.verticalCenter
                left:  parent.left; right: parent.right
                leftMargin: moveMenu.cardPad; rightMargin: moveMenu.cardPad
            }
            spacing: 8

            TypePill { typeName: emr.moveType }

            Text {
                width: parent.width - moveMenu.typePillW - eligPowerLabel.implicitWidth - 24
                anchors.verticalCenter: parent.verticalCenter
                text: emr.moveName
                font.family: moveMenu.mainFont; font.pixelSize: moveMenu.fontSizeSm
                color: moveMenu.colorText; elide: Text.ElideRight
            }

            Text {
                id: eligPowerLabel
                anchors.verticalCenter: parent.verticalCenter
                text: (emr.movePow > 0 ? emr.movePow : "—") + "  " +
                      (emr.moveAcc > 0 ? emr.moveAcc + "%" : "—")
                font.family: moveMenu.bodyFont; font.pixelSize: moveMenu.fontSizeSm
                color: moveMenu.colorSubtext
            }
        }
    }

    // ══════════════════════════════════════════════════════════════════════════
    //  LAYOUT  –  left panel | divider | right panel
    // ══════════════════════════════════════════════════════════════════════════
    Row {
        anchors.fill:    parent
        anchors.margins: moveMenu.margin
        spacing: 0

        // ── LEFT PANEL ────────────────────────────────────────────────────────
        // Top 60 %: return button + name field + current moves
        // Bottom 40 %: sprite centered
        Item {
            id: leftPanel
            width:  Math.floor(parent.width * 0.38)
            height: parent.height

            // ── TOP HALF ──────────────────────────────────────────────────────
            Column {
                id: leftTop
                anchors {
                    top:   parent.top
                    left:  parent.left
                    right: parent.right
                }
                height: Math.floor(parent.height * 0.6)
                spacing: moveMenu.sectionGap * 1.5  // Increased spacing
                clip: true

                // Return button
                Rectangle {
                    width:  parent.width
                    height: moveMenu.typePillH + 10
                    color:  returnMa.containsMouse ? moveMenu.buttonHoverColor : moveMenu.buttonColor
                    radius: 5
                    Text {
                        anchors.centerIn: parent
                        text: "← RETURN"
                        font.family: moveMenu.mainFont; font.pixelSize: moveMenu.fontSizeSm
                        color: moveMenu.colorText
                    }
                    MouseArea {
                        id: returnMa
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: undefined
                        onClicked:    moveMenu.returnClicked()
                        // Prevent click from propagating to top-level MouseArea
                        propagateComposedEvents: false
                    }
                }

                // ── REDESIGNED NAME INPUT AREA ─────────────────────────────────
                Column {
                    width: parent.width
                    spacing: 4

                    // "NICKNAME" label
                    Text {
                        text: "NICKNAME"
                        font.family: moveMenu.mainFont
                        font.pixelSize: moveMenu.fontSizeSm
                        color: moveMenu.colorSubtext
                        leftPadding: 4
                    }

                    // Clickable name input area
                    Item {
                        width: parent.width
                        height: 48  // Much larger height

                        // Background with hover effect
                        Rectangle {
                            id: nameBg
                            anchors.fill: parent
                            radius: 8
                            color: moveMenu.inNameEditMode ? moveMenu.nameInputEditBg :
                                   (nameMouseArea.containsMouse ? moveMenu.nameInputHoverBg : moveMenu.nameInputBg)
                            border.color: moveMenu.inNameEditMode ? moveMenu.nameInputEditBorder :
                                          (nameMouseArea.containsMouse ? moveMenu.buttonActiveColor : moveMenu.nameInputBorder)
                            border.width: moveMenu.inNameEditMode ? 2 : 1

                            // Subtle inner glow when in edit mode
                            Rectangle {
                                anchors.fill: parent
                                radius: parent.radius
                                color: "transparent"
                                border.color: Qt.rgba(255, 255, 255, 0.1)
                                border.width: 1
                                visible: moveMenu.inNameEditMode
                            }
                        }

                        // Edit icon (pencil) - only visible when not in edit mode
                        Text {
                            anchors {
                                right: parent.right
                                verticalCenter: parent.verticalCenter
                                rightMargin: 12
                            }
                            text: "✎"
                            font.family: "Segoe UI", "Arial" // Fallback for pencil icon
                            font.pixelSize: moveMenu.fontSizeLg
                            color: moveMenu.colorSubtext
                            opacity: nameMouseArea.containsMouse ? 1.0 : 0.6
                            visible: !moveMenu.inNameEditMode
                        }

                        // TextInput for the name
                        TextInput {
                            id: nameField
                            anchors {
                                left: parent.left
                                right: parent.right
                                leftMargin: 12
                                rightMargin: moveMenu.inNameEditMode ? 12 : 40  // More space for pencil when not editing
                                verticalCenter: parent.verticalCenter
                            }
                            text: pokeData ? pokeData.name : ""
                            font.family: moveMenu.mainFont
                            font.pixelSize: moveMenu.fontSizeLg  // Larger font
                            font.bold: true
                            color: moveMenu.colorText
                            maximumLength: 12
                            clip: true

                            // Only enable when in edit mode
                            enabled: moveMenu.inNameEditMode
                            readOnly: !moveMenu.inNameEditMode

                            // Visual cursor when in edit mode
                            cursorVisible: moveMenu.inNameEditMode
                            selectByMouse: moveMenu.inNameEditMode

                            // Handle keyboard events
                            Keys.onReturnPressed: {
                                moveMenu.toggleNameEditMode();
                            }
                            Keys.onEscapePressed: {
                                // Revert to original name
                                nameField.text = pokeData ? pokeData.name : "";
                                moveMenu.toggleNameEditMode();
                            }

                            // Handle focus changes
                            onFocusChanged: {
                                if (!activeFocus && moveMenu.inNameEditMode) {
                                    moveMenu.toggleNameEditMode();
                                }
                            }
                        }

                        // Hint text when empty and not in edit mode
                        Text {
                            anchors {
                                left: parent.left
                                verticalCenter: parent.verticalCenter
                                leftMargin: 12
                            }
                            text: "Click to nickname"
                            font.family: moveMenu.bodyFont
                            font.pixelSize: moveMenu.fontSizeMd
                            color: moveMenu.colorVeryFaint
                            visible: !moveMenu.inNameEditMode && nameField.text.length === 0
                        }

                        // Main click area
                        MouseArea {
                            id: nameMouseArea
                            anchors.fill: parent
                            cursorShape: undefined
                            hoverEnabled: true

                            onClicked: {
                                // mouse.accepted = true;
                                if (!moveMenu.inNameEditMode) {
                                    moveMenu.toggleNameEditMode();
                                }
                            }
                        }
                    }
                }

                // Removed type pills section

                // Divider
                Rectangle { width: parent.width; height: 1; color: moveMenu.colorDivider }

                // "MOVES" label
                Text {
                    text: "MOVES"
                    font.family: moveMenu.mainFont; font.pixelSize: moveMenu.fontSizeSm
                    color: moveMenu.colorSubtext
                }

                // Current 4 moves
                Column {
                    width:   parent.width
                    spacing: moveMenu.cardGap
                    Repeater {
                        model: pokeData ? pokeData.moves : []
                        CompactMoveCard {
                            width:    leftPanel.width
                            moveName: modelData.name
                            moveType: modelData.type
                            movePow:  modelData.power    !== undefined ? modelData.power    : -1
                            moveAcc:  modelData.accuracy !== undefined ? modelData.accuracy : -1
                        }
                    }
                }
            }

            // ── BOTTOM HALF: sprite ────────────────────────────────────────────
            Item {
                anchors {
                    top:    leftTop.bottom
                    bottom: parent.bottom
                    left:   parent.left
                    right:  parent.right
                }

                AnimatedSprite {
                    id: menuSprite
                    anchors.centerIn: parent

                    width:        moveMenu.frameWidth  * moveMenu.scaleFactor
                    height:       moveMenu.frameHeight * moveMenu.scaleFactor
                    running:      true
                    source:       moveMenu.spriteSheet
                    frameWidth:   moveMenu.frameWidth
                    frameHeight:  moveMenu.frameHeight
                    frameCount:   2
                    frameRate:    4
                    interpolate:  false
                    smooth:       false
                    antialiasing: false
                    frameX: moveMenu.frameWidth * 4
                    frameY: moveMenu.rowId * moveMenu.frameHeight

                    property bool isJumping:        false
                    property int  jumpUpDuration:   200
                    property int  jumpDownDuration: 150
                    property int  jumpHeight:       32

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: undefined
                        onClicked: {
                            mouse.accepted = true;
                            if (!menuSprite.isJumping) {
                                menuSprite.isJumping = true
                                spriteJump.start()
                            }
                        }
                    }

                    SequentialAnimation {
                        id: spriteJump
                        PropertyAnimation {
                            target: menuSprite; property: "y"
                            to: menuSprite.y - menuSprite.jumpHeight
                            duration: menuSprite.jumpUpDuration; easing.type: Easing.OutQuad
                        }
                        PropertyAnimation {
                            target: menuSprite; property: "y"
                            to: menuSprite.y
                            duration: menuSprite.jumpDownDuration; easing.type: Easing.InQuad
                        }
                        PropertyAction { target: menuSprite; property: "isJumping"; value: false }
                    }
                }
            }
        }

        // gap + divider
        Item { width: moveMenu.margin; height: parent.height }
        Rectangle { width: 1; height: parent.height; color: moveMenu.colorDivider }
        Item { width: moveMenu.margin; height: parent.height }

        // ── RIGHT PANEL – eligible moves grouped by type ───────────────────────
        Item {
            width:  parent.width - leftPanel.width - moveMenu.margin * 2 - 1
            height: parent.height

            Column {
                width: parent.width
                anchors.top: parent.top
                spacing: 4

                Text {
                    text: "ELIGIBLE MOVES"
                    font.family:    moveMenu.mainFont
                    font.pixelSize: moveMenu.fontSizeSm
                    color: moveMenu.colorSubtext
                    bottomPadding: 4
                }

                Rectangle {
                    width:  parent.width
                    height: moveMenu.height - moveMenu.margin * 2 - 30
                    color:  "transparent"
                    clip:   true

                    property var groupedMoves: {
                        if (!pokeData || !pokeData.eligibleMoves) return []
                        var groups = {}
                        var order  = []
                        for (var i = 0; i < pokeData.eligibleMoves.length; i++) {
                            var m = pokeData.eligibleMoves[i]
                            var t = m.type || "Normal"
                            if (!groups[t]) { groups[t] = []; order.push(t) }
                            groups[t].push(m)
                        }
                        order.sort()
                        var result = []
                        for (var j = 0; j < order.length; j++) {
                            result.push({ typeLabel: order[j], moves: groups[order[j]] })
                        }
                        return result
                    }

                    Flickable {
                        anchors.fill: parent
                        contentWidth:  parent.width
                        contentHeight: eligibleCol.implicitHeight
                        clip: true
                        flickableDirection: Flickable.VerticalFlick
                        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                        Column {
                            id: eligibleCol
                            width:   parent.width
                            spacing: 10

                            Repeater {
                                model: parent.parent.parent.groupedMoves

                                Column {
                                    width:   eligibleCol.width
                                    spacing: moveMenu.cardGap

                                    Row {
                                        spacing: 8
                                        TypePill { typeName: modelData.typeLabel }
                                        Rectangle {
                                            width:  eligibleCol.width - moveMenu.typePillW - 8
                                            height: 1
                                            color:  moveMenu.colorDivider
                                            anchors.verticalCenter: parent.verticalCenter
                                        }
                                    }

                                    Column {
                                        width:   eligibleCol.width
                                        spacing: moveMenu.cardGap
                                        Repeater {
                                            model: modelData.moves
                                            EligibleMoveRow {
                                                width:    eligibleCol.width
                                                moveName: modelData.name
                                                moveType: modelData.type
                                                movePow:  modelData.power    !== undefined ? modelData.power    : -1
                                                moveAcc:  modelData.accuracy !== undefined ? modelData.accuracy : -1
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Top-level MouseArea to handle clicks outside edit area
    MouseArea {
        anchors.fill: parent
        cursorShape: undefined

        // Only enabled when in name edit mode
        enabled: moveMenu.inNameEditMode

        onClicked: {
            // Click anywhere outside will exit edit mode
            moveMenu.toggleNameEditMode();
        }

        // Prevent clicks on interactive elements from being handled here
        propagateComposedEvents: true
    }
}
