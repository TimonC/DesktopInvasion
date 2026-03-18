import QtQuick 2.15
import QtQuick.Controls 2.15
import "../Style/PokeColor.js" as PokeColor

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

    property bool inNameEditMode:    false

    // The eligible move the user has selected to swap in. -1 = none selected.
    // We store the index into pokeData.eligibleMoves for easy lookup.
    property int selectedEligibleIdx: -1

    // ── Colors ────────────────────────────────────────────────────────────────
    property color colorText:         "#ffffff"
    property color colorSubtext:      "#aaaaaa"
    property color colorVeryFaint:    "#999999"
    property color colorDivider:      "#3d3d3d"
    property color colorMoveCard:     "#383838"
    property color colorMoveCardHov:  "#454545"
    property color colorEligibleCard: "#303040"
    property color colorEligibleHov:  "#3a3a55"
    property color colorEligibleSel:  "#2a3d5a"
    property color colorTypePillText: "#ffffff"
    property color buttonColor:       "#3c3c3c"
    property color buttonHoverColor:  "#4c4c4c"
    property color buttonActiveColor: "#5294e2"
    property color nameInputBg:       "#3a3a3a"
    property color nameInputHoverBg:  "#4a4a4a"
    property color nameInputEditBg:   "#2a3a5a"
    property color nameInputBorder:   "#5a5a5a"
    property color nameInputEditBdr:  "#7aa5ff"

    // ── Layout ────────────────────────────────────────────────────────────────
    property int margin:  14
    property int gap:     6
    property int pad:     8
    property int pillW:   84
    property int pillH:   fontSizeMd + 6
    property int cardH:   pillH + pad * 2
    property int secGap:  10

    signal returnClicked()
    signal nameChanged(string newName)
    signal requestMoveChange(int currentMoveSlot, int newMoveId)

    function toggleNameEditMode() {
        if (inNameEditMode) {
            inNameEditMode = false
            nameField.focus = false
            if (nameField.text !== (pokeData ? pokeData.name : ""))
                moveMenu.nameChanged(nameField.text)
        } else {
            inNameEditMode = true
            nameField.forceActiveFocus()
        }
    }

    onPokeDataChanged: {
        if (pokeData) nameField.text = pokeData.name
        inNameEditMode      = false
        selectedEligibleIdx = -1
    }

    // ── TypePill ──────────────────────────────────────────────────────────────
    component TypePill: Rectangle {
        property string typeName: ""
        width:  moveMenu.pillW
        height: moveMenu.pillH
        radius: 3
        gradient: Gradient {
            GradientStop { position: 0.0; color: PokeColor.lighter(PokeColor.typeColor(typeName)) }
            GradientStop { position: 1.0; color: PokeColor.darker(PokeColor.typeColor(typeName))  }
        }
        Text {
            anchors.centerIn: parent
            text:             PokeColor.typeColor(typeName) === "transparent" ? "" : typeName
            font.family:      moveMenu.bodyFont
            font.pixelSize:   moveMenu.fontSizeSm
            font.bold: true
            color:            moveMenu.colorTypePillText
        }
    }

    // ── CurrentMoveCard ───────────────────────────────────────────────────────
    // Clicking replaces this slot with the selected eligible move (if any).
    component CurrentMoveCard: Rectangle {
        id: cmc
        property string moveName:  ""
        property string moveType:  ""
        property int    movePow:   0
        property int    moveAcc:   0
        property int    slotIndex: 0

        readonly property bool swapReady: moveMenu.selectedEligibleIdx !== -1

        width:  parent ? parent.width : 200
        height: moveMenu.cardH
        radius: 5
        color:  swapReady && cmcMa.containsMouse ? moveMenu.colorMoveCardHov : moveMenu.colorMoveCard
        border.color: swapReady && cmcMa.containsMouse ? moveMenu.buttonActiveColor : "transparent"
        border.width: 1

        Row {
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left; right: parent.right
                leftMargin: moveMenu.pad; rightMargin: moveMenu.pad
            }
            spacing: 8

            TypePill { typeName: cmc.moveType }

            Text {
                width: parent.width - moveMenu.pillW - cmcPower.implicitWidth - 20
                anchors.verticalCenter: parent.verticalCenter
                text:           cmc.moveName
                font.family:    moveMenu.mainFont
                font.pixelSize: moveMenu.fontSizeSm
                color:          moveMenu.colorText
                elide:          Text.ElideRight
            }

            Text {
                id: cmcPower
                anchors.verticalCenter: parent.verticalCenter
                text:           (cmc.movePow > 0 ? cmc.movePow : "—") + "  " +
                                (cmc.moveAcc > 0 ? cmc.moveAcc + "%" : "—")
                font.family:    moveMenu.bodyFont
                font.pixelSize: moveMenu.fontSizeSm
                color:          moveMenu.colorSubtext
            }
        }

        MouseArea {
            id: cmcMa
            anchors.fill:            parent
            hoverEnabled:            true
            cursorShape:             undefined
            propagateComposedEvents: false
            onClicked: {
                if (!cmc.swapReady) return
                var eligMove = moveMenu.pokeData.eligibleMoves[moveMenu.selectedEligibleIdx]
                moveMenu.requestMoveChange(cmc.slotIndex, eligMove.id)
                moveMenu.selectedEligibleIdx = -1
            }
        }
    }

    // ── EligibleMoveRow ───────────────────────────────────────────────────────
    // Clicking selects this move as the one to swap in.
    component EligibleMoveRow: Rectangle {
        id: emr
        property string moveName: ""
        property string moveType: ""
        property int    movePow:  0
        property int    moveAcc:  0
        property int    eligIdx:  -1   // index into pokeData.eligibleMoves

        readonly property bool isSelected: moveMenu.selectedEligibleIdx === eligIdx

        width:  parent ? parent.width : 200
        height: moveMenu.cardH
        radius: 4
        color:  isSelected ? moveMenu.colorEligibleSel
                           : (emrMa.containsMouse ? moveMenu.colorEligibleHov
                                                  : moveMenu.colorEligibleCard)
        border.color: isSelected ? moveMenu.buttonActiveColor : "transparent"
        border.width: isSelected ? 2 : 1

        Row {
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left; right: parent.right
                leftMargin: moveMenu.pad; rightMargin: moveMenu.pad
            }
            spacing: 8

            TypePill { typeName: emr.moveType }

            Text {
                width: parent.width - moveMenu.pillW - emrPower.implicitWidth - 20
                anchors.verticalCenter: parent.verticalCenter
                text:           emr.moveName
                font.family:    moveMenu.mainFont
                font.pixelSize: moveMenu.fontSizeSm
                color:          moveMenu.colorText
                elide:          Text.ElideRight
            }

            Text {
                id: emrPower
                anchors.verticalCenter: parent.verticalCenter
                text:           (emr.movePow > 0 ? emr.movePow : "—") + "  " +
                                (emr.moveAcc > 0 ? emr.moveAcc + "%" : "—")
                font.family:    moveMenu.bodyFont
                font.pixelSize: moveMenu.fontSizeSm
                color:          moveMenu.colorSubtext
            }
        }

        MouseArea {
            id: emrMa
            anchors.fill:            parent
            hoverEnabled:            true
            cursorShape:             undefined
            propagateComposedEvents: false
            onClicked: {
                // Toggle: clicking the already-selected row deselects
                moveMenu.selectedEligibleIdx = (moveMenu.selectedEligibleIdx === emr.eligIdx)
                                               ? -1 : emr.eligIdx
            }
        }
    }

    // ══════════════════════════════════════════════════════════════════════════
    //  LAYOUT
    // ══════════════════════════════════════════════════════════════════════════
    Row {
        anchors.fill:    parent
        anchors.margins: moveMenu.margin
        spacing:         0

        // ── LEFT PANEL ────────────────────────────────────────────────────────
        Item {
            id: leftPanel
            width:  Math.floor(parent.width * 0.38)
            height: parent.height

            Column {
                id: leftTop
                anchors { top: parent.top; left: parent.left; right: parent.right }
                height:  Math.floor(parent.height * 0.6)
                spacing: moveMenu.secGap * 1.5
                clip:    true

                // Return button
                Rectangle {
                    width:  parent.width
                    height: moveMenu.cardH
                    color:  returnMa.containsMouse ? moveMenu.buttonHoverColor : moveMenu.buttonColor
                    radius: 5
                    Text {
                        anchors.centerIn: parent
                        text:           "← RETURN"
                        font.family:    moveMenu.mainFont
                        font.pixelSize: moveMenu.fontSizeSm
                        color:          moveMenu.colorText
                    }
                    MouseArea {
                        id: returnMa
                        anchors.fill:            parent
                        hoverEnabled:            true
                        cursorShape:             undefined
                        propagateComposedEvents: false
                        onClicked:               moveMenu.returnClicked()
                    }
                }

                // Nickname
                Column {
                    width:   parent.width
                    spacing: 4

                    Text {
                        text:           "NICKNAME"
                        font.family:    moveMenu.mainFont
                        font.pixelSize: moveMenu.fontSizeSm
                        color:          moveMenu.colorSubtext
                        leftPadding:    4
                    }

                    Item {
                        width:  parent.width
                        height: 48

                        Rectangle {
                            anchors.fill: parent
                            radius:       8
                            color:        moveMenu.inNameEditMode ? moveMenu.nameInputEditBg
                                          : (nameMouseArea.containsMouse ? moveMenu.nameInputHoverBg
                                                                         : moveMenu.nameInputBg)
                            border.color: moveMenu.inNameEditMode ? moveMenu.nameInputEditBdr
                                          : (nameMouseArea.containsMouse ? moveMenu.buttonActiveColor
                                                                         : moveMenu.nameInputBorder)
                            border.width: moveMenu.inNameEditMode ? 2 : 1
                        }

                        Text {
                            anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: 12 }
                            text:           "✎"
                            font.pixelSize: moveMenu.fontSizeLg
                            color:          moveMenu.colorSubtext
                            opacity:        nameMouseArea.containsMouse ? 1.0 : 0.6
                            visible:        !moveMenu.inNameEditMode
                        }

                        TextInput {
                            id: nameField
                            anchors {
                                left:           parent.left
                                right:          parent.right
                                leftMargin:     12
                                rightMargin:    moveMenu.inNameEditMode ? 12 : 40
                                verticalCenter: parent.verticalCenter
                            }
                            text:           pokeData ? pokeData.name : ""
                            font.family:    moveMenu.mainFont
                            font.pixelSize: moveMenu.fontSizeLg
                            font.bold:      true
                            color:          moveMenu.colorText
                            maximumLength:  12
                            clip:           true
                            enabled:        moveMenu.inNameEditMode
                            readOnly:       !moveMenu.inNameEditMode
                            cursorVisible:  moveMenu.inNameEditMode
                            selectByMouse:  moveMenu.inNameEditMode
                            Keys.onReturnPressed: moveMenu.toggleNameEditMode()
                            Keys.onEscapePressed: {
                                nameField.text = pokeData ? pokeData.name : ""
                                moveMenu.toggleNameEditMode()
                            }
                            onFocusChanged: {
                                if (!activeFocus && moveMenu.inNameEditMode)
                                    moveMenu.toggleNameEditMode()
                            }
                        }

                        Text {
                            anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: 12 }
                            text:           "Click to nickname"
                            font.family:    moveMenu.bodyFont
                            font.pixelSize: moveMenu.fontSizeMd
                            color:          moveMenu.colorVeryFaint
                            visible:        !moveMenu.inNameEditMode && nameField.text.length === 0
                        }

                        MouseArea {
                            id: nameMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape:  undefined
                            onClicked:    { if (!moveMenu.inNameEditMode) moveMenu.toggleNameEditMode() }
                        }
                    }
                }

                Rectangle { width: parent.width; height: 1; color: moveMenu.colorDivider }

                // Moves header
                Item {
                    width:  parent.width
                    height: moveMenu.pillH

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text:           "MOVES"
                        font.family:    moveMenu.mainFont
                        font.pixelSize: moveMenu.fontSizeSm
                        color:          moveMenu.colorSubtext
                    }

                    Text {
                        anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                        text:           moveMenu.selectedEligibleIdx !== -1
                                        ? "pick a slot ←" : ""
                        font.family:    moveMenu.bodyFont
                        font.pixelSize: moveMenu.fontSizeSm
                        color:          moveMenu.buttonActiveColor
                    }
                }

                // Current move slots — always 4, show empty placeholder if no move
                Column {
                    width:   parent.width
                    spacing: moveMenu.gap

                    Repeater {
                        model: 4
                        CurrentMoveCard {
                            width:     leftPanel.width
                            slotIndex: index
                            moveName:  (pokeData && pokeData.moves && index < pokeData.moves.length)
                                       ? pokeData.moves[index].name : "—"
                            moveType:  (pokeData && pokeData.moves && index < pokeData.moves.length)
                                       ? pokeData.moves[index].type : ""
                            movePow:   (pokeData && pokeData.moves && index < pokeData.moves.length
                                        && pokeData.moves[index].power !== undefined)
                                       ? pokeData.moves[index].power : -1
                            moveAcc:   (pokeData && pokeData.moves && index < pokeData.moves.length
                                        && pokeData.moves[index].accuracy !== undefined)
                                       ? pokeData.moves[index].accuracy : -1
                        }
                    }
                }
            }

            // Sprite
            Item {
                anchors {
                    top:    leftTop.bottom
                    bottom: parent.bottom
                    left:   parent.left
                    right:  parent.right
                }
                AnimatedSprite {
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
                    frameX:       moveMenu.frameWidth * 4
                    frameY:       moveMenu.rowId * moveMenu.frameHeight
                }
            }
        }

        // Divider
        Item      { width: moveMenu.margin; height: parent.height }
        Rectangle { width: 1; height: parent.height; color: moveMenu.colorDivider }
        Item      { width: moveMenu.margin; height: parent.height }

        // ── RIGHT PANEL ────────────────────────────────────────────────────────
        Item {
            id: rightPanel
            width:  parent.width - leftPanel.width - moveMenu.margin * 2 - 1
            height: parent.height

            Column {
                width:       parent.width
                anchors.top: parent.top
                spacing:     4

                Text {
                    text:           "ELIGIBLE MOVES"
                    font.family:    moveMenu.mainFont
                    font.pixelSize: moveMenu.fontSizeSm
                    color:          moveMenu.colorSubtext
                    bottomPadding:  4
                }

                Flickable {
                    width:         parent.width
                    height:        rightPanel.height - moveMenu.fontSizeSm - 12
                    contentHeight: eligibleCol.implicitHeight
                    clip:          true
                    flickableDirection: Flickable.VerticalFlick
                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                    Column {
                        id: eligibleCol
                        width:   parent.width
                        spacing: moveMenu.gap

                        Repeater {
                            model: pokeData ? pokeData.eligibleMoves : []
                            EligibleMoveRow {
                                width:    eligibleCol.width
                                eligIdx:  index
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

