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

    property bool inNameEditMode:     false
    property int  selectedEligibleIdx: -1

    // ── Palette — mirrors PokeView move section ───────────────────────────────
    property color colorText:         "#ffffff"
    property color colorSubtext:      "#aaaaaa"
    property color colorVeryFaint:    "#999999"
    property color colorDivider:      "#3d3d3d"
    property color colorSurface:      "#383838"   // same as colorMoveCard in PokeView
    property color colorSurfaceHov:   "#444444"
    property color colorSurfaceSel:   "#404040"   // selected eligible: slightly lighter grey, no blue
    property color colorEligCard:     "#303030"   // eligible base: just a touch darker
    property color colorEligHov:      "#3c3c3c"
    property color colorTypePillText: "#ffffff"
    property color colorAccent:       "#5294e2"   // only used for borders on hover/select
    property color colorBtn:          "#383838"
    property color colorBtnHov:       "#444444"
    property color colorNameBg:       "#383838"
    property color colorNameHov:      "#444444"
    property color colorNameEdit:     "#404040"
    property color colorNameBdr:      "#555555"
    property color colorNameEditBdr:  "#5294e2"
    property color colorScrollTrack:  "#2a2a2a"
    property color colorScrollThumb:  "#555555"
    property color colorScrollHov:    "#777777"
    property color colorScrollActive: "#5294e2"

    // ── Layout ────────────────────────────────────────────────────────────────
    property int margin:  14
    property int gap:     5
    property int pad:     9
    property int pillW:   82
    property int pillH:   fontSizeMd + 6
    property int cardH:   pillH + pad * 2
    property int secGap:  12

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
            selectedEligibleIdx = -1
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
            font.bold:        true
            color:            moveMenu.colorTypePillText
        }
    }

    // ── CurrentMoveCard ───────────────────────────────────────────────────────
    component CurrentMoveCard: Rectangle {
        id: cmc
        property string moveName:  ""
        property string moveType:  ""
        property int    movePow:   0
        property int    moveAcc:   0
        property int    slotIndex: 0

        readonly property bool swapReady: moveMenu.selectedEligibleIdx !== -1
        readonly property bool isEmpty:   moveName === "—" || moveName === ""

        width:  parent ? parent.width : 200
        height: moveMenu.cardH
        radius: 4
        color:  swapReady && cmcMa.containsMouse ? moveMenu.colorSurfaceHov : moveMenu.colorSurface
        border.color: swapReady && cmcMa.containsMouse ? moveMenu.colorAccent : moveMenu.colorDivider
        border.width: 1

        Row {
            anchors {
                verticalCenter: parent.verticalCenter
                left:  parent.left
                right: parent.right
                leftMargin:  moveMenu.pad
                rightMargin: moveMenu.pad
            }
            spacing: 8

            TypePill { typeName: cmc.isEmpty ? "" : cmc.moveType }

            Text {
                width: parent.width - moveMenu.pillW - cmcPower.implicitWidth - 20
                anchors.verticalCenter: parent.verticalCenter
                text:           cmc.moveName
                font.family:    moveMenu.mainFont
                font.pixelSize: moveMenu.fontSizeSm - 1
                color:          cmc.isEmpty ? moveMenu.colorVeryFaint : moveMenu.colorText
                elide:          Text.ElideRight
            }

            Text {
                id: cmcPower
                anchors.verticalCenter: parent.verticalCenter
                text:           cmc.isEmpty ? ""
                                : ((cmc.movePow > 0 ? cmc.movePow : "—") + "  " +
                                   (cmc.moveAcc > 0 ? cmc.moveAcc + "%" : "—"))
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
    component EligibleMoveRow: Rectangle {
        id: emr
        property string moveName: ""
        property string moveType: ""
        property int    movePow:  0
        property int    moveAcc:  0
        property int    eligIdx:  -1

        readonly property bool isSelected: moveMenu.selectedEligibleIdx === eligIdx

        width:  parent ? parent.width : 200
        height: moveMenu.cardH
        radius: 4
        color:  isSelected ? moveMenu.colorSurfaceSel
                           : (emrMa.containsMouse ? moveMenu.colorEligHov : moveMenu.colorEligCard)
        border.color: isSelected ? moveMenu.colorAccent : moveMenu.colorDivider
        border.width: isSelected ? 2 : 1

        Row {
            anchors {
                verticalCenter: parent.verticalCenter
                left:  parent.left
                right: parent.right
                leftMargin:  moveMenu.pad
                rightMargin: moveMenu.pad
            }
            spacing: 8

            TypePill { typeName: emr.moveType }

            Text {
                width: parent.width - moveMenu.pillW - emrPower.implicitWidth - 20
                anchors.verticalCenter: parent.verticalCenter
                text:           emr.moveName
                font.family:    moveMenu.mainFont
                font.pixelSize: moveMenu.fontSizeSm - 1
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
                moveMenu.selectedEligibleIdx = (moveMenu.selectedEligibleIdx === emr.eligIdx)
                                               ? -1 : emr.eligIdx
                if (moveMenu.inNameEditMode) moveMenu.toggleNameEditMode()
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
            width:  Math.floor((parent.width - moveMenu.margin * 2 - 1) / 2)
            height: parent.height

            Column {
                id: leftTop
                anchors { top: parent.top; left: parent.left; right: parent.right }
                height:  Math.floor(parent.height * 0.62)
                spacing: moveMenu.secGap
                clip:    true

                // Return button
                Rectangle {
                    width:  parent.width
                    height: moveMenu.cardH
                    color:  returnMa.containsMouse ? moveMenu.colorBtnHov : moveMenu.colorBtn
                    radius: 4
                    border.color: moveMenu.colorDivider
                    border.width: 1
                    Text {
                        anchors.centerIn: parent
                        text:           "← RETURN"
                        font.family:    moveMenu.mainFont
                        font.pixelSize: moveMenu.fontSizeSm - 1
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
                    spacing: 5

                    Text {
                        text:           "NICKNAME"
                        font.family:    moveMenu.mainFont
                        font.pixelSize: moveMenu.fontSizeSm - 4
                        color:          moveMenu.colorSubtext
                        leftPadding:    2
                    }

                    Item {
                        width:  parent.width
                        height: 44

                        Rectangle {
                            anchors.fill: parent
                            radius:       4
                            color:        moveMenu.inNameEditMode ? moveMenu.colorNameEdit
                                          : (nameMouseArea.containsMouse ? moveMenu.colorNameHov
                                                                         : moveMenu.colorNameBg)
                            border.color: moveMenu.inNameEditMode ? moveMenu.colorNameEditBdr
                                          : (nameMouseArea.containsMouse ? moveMenu.colorAccent
                                                                         : moveMenu.colorNameBdr)
                            border.width: moveMenu.inNameEditMode ? 2 : 1
                        }

                        Text {
                            anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: 11 }
                            text:           "✎"
                            font.pixelSize: moveMenu.fontSizeMd
                            color:          moveMenu.colorSubtext
                            opacity:        nameMouseArea.containsMouse ? 0.9 : 0.4
                            visible:        !moveMenu.inNameEditMode
                        }

                        TextInput {
                            id: nameField
                            anchors {
                                left:           parent.left
                                right:          parent.right
                                leftMargin:     10
                                rightMargin:    moveMenu.inNameEditMode ? 10 : 36
                                verticalCenter: parent.verticalCenter
                            }
                            text:           pokeData ? pokeData.name : ""
                            font.family:    moveMenu.mainFont
                            font.pixelSize: moveMenu.fontSizeMd
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
                            anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: 10 }
                            text:           "Click to nickname"
                            font.family:    moveMenu.bodyFont
                            font.pixelSize: moveMenu.fontSizeSm
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
                        font.pixelSize: moveMenu.fontSizeSm - 4
                        color:          moveMenu.colorSubtext
                    }

                    Text {
                        anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                        text:           moveMenu.selectedEligibleIdx !== -1 ? "← pick slot" : ""
                        font.family:    moveMenu.bodyFont
                        font.pixelSize: moveMenu.fontSizeSm
                        color:          moveMenu.colorAccent
                    }
                }

                // Current move slots — always 4
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

        // Vertical divider
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
                spacing:     5

                Item {
                    width:  parent.width
                    height: moveMenu.pillH

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text:           "ELIGIBLE MOVES"
                        font.family:    moveMenu.mainFont
                        font.pixelSize: moveMenu.fontSizeSm - 4
                        color:          moveMenu.colorSubtext
                    }

                    Text {
                        anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                        text:           pokeData && pokeData.eligibleMoves
                                        ? pokeData.eligibleMoves.length : ""
                        font.family:    moveMenu.bodyFont
                        font.pixelSize: moveMenu.fontSizeSm
                        color:          moveMenu.colorVeryFaint
                    }
                }

                Item {
                    width:  parent.width
                    height: rightPanel.height - moveMenu.pillH - 5

                    Flickable {
                        id: eligFlickable
                        anchors {
                            top:    parent.top
                            bottom: parent.bottom
                            left:   parent.left
                            right:  scrollBar.left
                            rightMargin: 6
                        }
                        contentHeight:      eligibleCol.implicitHeight
                        clip:               true
                        flickableDirection: Flickable.VerticalFlick
                        // Hard clamp — no bounce
                        boundsBehavior:     Flickable.StopAtBounds
                        boundsMovement:     Flickable.StopAtBounds

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

                    // Scrollbar
                    Item {
                        id: scrollBar
                        anchors {
                            top:    parent.top
                            bottom: parent.bottom
                            right:  parent.right
                        }
                        width:   8
                        visible: eligFlickable.contentHeight > eligFlickable.height
                        opacity: scrollThumbMa.containsMouse || scrollThumbMa.pressed ? 1.0 : 0.5

                        Behavior on opacity { NumberAnimation { duration: 120 } }

                        // Track
                        Rectangle {
                            anchors {
                                top:              parent.top
                                bottom:           parent.bottom
                                horizontalCenter: parent.horizontalCenter
                            }
                            width:  2
                            radius: 1
                            color:  moveMenu.colorScrollTrack
                        }

                        // Thumb
                        Rectangle {
                            id: scrollThumb
                            anchors.horizontalCenter: parent.horizontalCenter
                            width:  scrollThumbMa.pressed ? 7
                                    : (scrollThumbMa.containsMouse ? 6 : 4)
                            radius: width / 2
                            color:  scrollThumbMa.pressed      ? moveMenu.colorScrollActive
                                    : scrollThumbMa.containsMouse ? moveMenu.colorScrollHov
                                                                   : moveMenu.colorScrollThumb

                            Behavior on width { NumberAnimation { duration: 100 } }
                            Behavior on color { ColorAnimation  { duration: 100 } }

                            height: Math.max(20,
                                        scrollBar.height
                                        * (eligFlickable.height
                                           / Math.max(1, eligFlickable.contentHeight)))

                            y: {
                                var ratio = eligFlickable.contentY
                                            / Math.max(1, eligFlickable.contentHeight - eligFlickable.height)
                                return ratio * (scrollBar.height - scrollThumb.height)
                            }

                            MouseArea {
                                id: scrollThumbMa
                                anchors.fill:    parent
                                hoverEnabled:    true
                                cursorShape:     undefined
                                property real dragStartY:        0
                                property real dragStartContentY: 0
                                onPressed: (mouse) => {
                                    dragStartY        = mouse.y
                                    dragStartContentY = eligFlickable.contentY
                                }
                                onPositionChanged: (mouse) => {
                                    if (!pressed) return
                                    var delta        = mouse.y - dragStartY
                                    var trackUsable  = scrollBar.height - scrollThumb.height
                                    var contentRange = eligFlickable.contentHeight - eligFlickable.height
                                    eligFlickable.contentY = Math.max(0, Math.min(contentRange,
                                        dragStartContentY + delta * (contentRange / Math.max(1, trackUsable))))
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

