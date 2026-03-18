import QtQuick 2.15

Rectangle {
    id: root
    color: backgroundColor

    // ── Spacing / geometry ─────────────────────────────────────────────────────
    readonly property int pad:      12
    readonly property int dividerW: 1
    readonly property color dividerColor: "#3a3a3a"

    // ── Section dimensions (single source of truth) ────────────────────────────
    readonly property int trainerH:    160
    readonly property int pcH:         640
    readonly property int pcW:         640
    readonly property int rightPanelW: 640

    // ── Label heights ──────────────────────────────────────────────────────────
    readonly property int labelHeight: 24
    readonly property int contentSpacing: 8

    // ── Icon scales ────────────────────────────────────────────────────────────
    property int iconScale:           8
    property int iconScaleForBig:     6
    property int iconScaleForTrainer: 5

    // ── Overall window size ────────────────────────────────────────────────────
    // Horizontal: pad | pcW | pad  dividerW  pad | rightPanelW | pad
    width:  pad + pcW + pad + dividerW + pad + rightPanelW + pad

    // Vertical: pad | labelHeight | contentSpacing | trainerH | pad  dividerW  pad | labelHeight | contentSpacing | pcH | pad
    height: pad + labelHeight + contentSpacing + trainerH + pad + dividerW + pad + labelHeight + contentSpacing + pcH + pad

    // ── Theme ──────────────────────────────────────────────────────────────────
    property color  backgroundColor:     "#2b2b2b"
    property color  buttonColor:         "#3c3c3c"
    property color  buttonSelectedColor: "#5294e2"
    property color  textColor:           "#ffffff"
    property color  subheaderColor:      "#aaaaaa"  // Matching MoveMenu's colorSubtext
    property int    fontSizeLg: 22
    property int    fontSizeMd: 18
    property int    fontSizeSm: 16
    property string p2pFont:       "Press Start 2P"
    property string dotGothicFont: "DotGothic16"

    property string currentMenu: "Party"
    readonly property int pcRows:    4
    readonly property int pcColumns: 4
    property var partyPokes: ({})
    property var boxPokes:   ({})

    // ── Menu state ─────────────────────────────────────────────────────────────
    // "default"  → normal three-section layout
    // "moveMenu" → full-width MoveMenu overlay
    property string menuState: "default"

    // ── Background click – cancel swap ─────────────────────────────────────────
    MouseArea {
        anchors.fill: parent
        cursorShape:  undefined
        onClicked: {
            if (pc.inSwapMode) pc.toggleSwapMode()
            if (moveMenu.inNameEditMode) moveMenu.toggleNameEditMode()
            moveMenu.selectedEligibleIdx = -1
        }
    }

    Connections {
        target: moveMenu
        function onNameChanged(name) {
            menuBridge.nameChangeRequested(pc.displayedPokemonBox, pc.displayedPokemonSlot, name)
        }
    }
    // ── Bridge connections ─────────────────────────────────────────────────────
    Connections {
        target: menuBridge

        function onPartyDataReady(data) {
            pc.loadParty(data)
            var map = {}
            for (var i = 0; i < data.length; i++) map[data[i].slot] = data[i]
            partyPokes = map
            pc._display([-1, 0])
        }

        function onBoxDataReady(boxIndex, data) {
            pc.loadBox(boxIndex, data)
            var arr = new Array(pcRows * pcColumns)
            for (var i = 0; i < data.length; i++) arr[data[i].slot] = data[i]
            var updated = Object.assign({}, boxPokes)
            updated[boxIndex] = arr
            boxPokes = updated
        }
    }


    Connections {
        target: pc

        function onPreloadBoxRequested(boxIndex) { menuBridge.preloadBoxRequested(boxIndex) }

        function onSwapRequested(posx, posy) {
            menuBridge.swapRequested(posx[0], posx[1], posy[0], posy[1])
            var newParty = Object.assign({}, partyPokes)
            var newBoxes = Object.assign({}, boxPokes)
            var x = posx[0] === -1 ? partyPokes[posx[1]] : boxPokes[posx[0]][posx[1]]
            var y = posy[0] === -1 ? partyPokes[posy[1]] : boxPokes[posy[0]][posy[1]]
            if (posx[0] === -1) { newParty[posx[1]] = y }
            else { var arrX = newBoxes[posx[0]].slice(); arrX[posx[1]] = y; newBoxes[posx[0]] = arrX }
            if (posy[0] === -1) { newParty[posy[1]] = x }
            else { var arrY = newBoxes[posy[0]].slice(); arrY[posy[1]] = x; newBoxes[posy[0]] = arrY }
            partyPokes = newParty
            boxPokes   = newBoxes
        }

        function onDisplay(pcPos) {
            var poke = null
            if (pcPos[0] === -1 && partyPokes[pcPos[1]])
                poke = partyPokes[pcPos[1]]
            else if (pcPos[0] > -1 && boxPokes[pcPos[0]] && boxPokes[pcPos[0]][pcPos[1]])
                poke = boxPokes[pcPos[0]][pcPos[1]]
            else { console.log("ERROR faulty display pos"); return }
            pokeView.pokeData    = poke
            pokeView.rowId       = poke.rowId
            pokeView.spriteSheet = poke.isBig ? "qrc:/assets/HGSS/reordered_sprites_big.png"
                                              : "qrc:/assets/HGSS/reordered_sprites.png"
            pokeView.frameWidth  = poke.isBig ? 64 : 32
            pokeView.frameHeight = poke.isBig ? 64 : 32
            pokeView.scaleFactor = poke.isBig ? root.iconScaleForBig : root.iconScale
        }
    }

    // ══════════════════════════════════════════════════════════════════════════
    //  Main layout  –  two columns separated by a vertical divider
    // ══════════════════════════════════════════════════════════════════════════
    Row {
        anchors.fill:    parent
        anchors.margins: root.pad
        spacing:         0

        // ── LEFT COLUMN ───────────────────────────────────────────────────────
        Column {
            id: leftColumn
            width:   root.pcW
            height:  parent.height
            spacing: 0

            // Visibility: only in default state
            visible: root.menuState === "default"

            // ── LABEL 1 – Trainer ─────────────────────────────────────────────
            Text {
                width: parent.width
                height: root.labelHeight
                text: "TRAINER"
                font.family: root.p2pFont
                font.pixelSize: root.fontSizeSm
                color: root.subheaderColor
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

            Item { width: parent.width; height: root.contentSpacing }

            // ── SECTION 1 – Trainer ───────────────────────────────────────────
            Item {
                width:  parent.width
                height: root.trainerH

                Trainer {
                    anchors.fill: parent
                    textColor:    root.textColor
                    fontSize:     root.fontSizeLg
                    fontFamily:   root.p2pFont
                    iconScale:    root.iconScaleForTrainer
                }
            }

            Item { width: parent.width; height: root.pad }

            Rectangle {
                width:  parent.width
                height: root.dividerW
                color:  root.dividerColor
            }

            Item { width: parent.width; height: root.pad }

            // ── LABEL 2 – PC ──────────────────────────────────────────────────
            Text {
                width: parent.width
                height: root.labelHeight
                text: "PC"
                font.family: root.p2pFont
                font.pixelSize: root.fontSizeSm
                color: root.subheaderColor
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

            Item { width: parent.width; height: root.contentSpacing }

            // ── SECTION 2 – PC ────────────────────────────────────────────────
            Item {
                width:  parent.width
                height: root.pcH

                PC {
                    id:               pc
                    anchors.centerIn: parent
                    width:  pc.pcColumns * pc.slotWidth + pc.buttonWidth * 2 + pc.layoutSpacing * 2
                    height: root.pcH
                    fontSizeLg: root.fontSizeLg
                    fontSizeMd: root.fontSizeMd
                    fontSizeSm: root.fontSizeSm
                    fontFamily: root.p2pFont
                }
            }
        }

        // ── Vertical divider (default state only) ─────────────────────────────
        Item    { width: root.pad;     height: parent.height; visible: root.menuState === "default" }
        Rectangle {
            width:   root.dividerW
            height:  parent.height
            color:   root.dividerColor
            visible: root.menuState === "default"
        }
        Item    { width: root.pad;     height: parent.height; visible: root.menuState === "default" }

        // ── RIGHT COLUMN  (Section 3 – PokeView) ──────────────────────────────
        Item {
            // In default mode this is rightPanelW wide.
            // In moveMenu mode left column + dividers are hidden, so this item
            // naturally stretches to fill the whole Row — we pin width explicitly
            // based on state so nothing reflows unexpectedly.
            width: root.menuState === "default"
                       ? root.rightPanelW
                       : root.pcW + root.pad * 2 + root.dividerW + root.rightPanelW
            height: parent.height

            // ── LABEL 3 – Summary (only visible in default state) ─────────────
            Column {
                width: parent.width
                height: parent.height
                visible: root.menuState === "default"
                spacing: 0

                Row{
                    width: parent.width - 64 //hardcode the buttonsize for offset
                    height: root.labelHeight
                    Text {
                        width: parent.width
                        height: root.labelHeight
                        text: "SUMMARY"
                        font.family: root.p2pFont
                        font.pixelSize: root.fontSizeSm
                        color: root.subheaderColor
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }
                   PcButton {
                       id: pokeEditButton
                       label:    "EDIT"
                       onClicked: editButtonClicked(pokeView.pokeData)
                   }
                }

                Item { width: parent.width; height: root.contentSpacing }

                // ── DEFAULT: PokeView ────────────────────────────────────────
                PokeView {
                    id:               pokeView
                    width:            parent.width
                    height:           parent.height - root.labelHeight - root.contentSpacing
                    fontSizeLg:       root.fontSizeLg
                    fontSizeMd:       root.fontSizeMd
                    fontSizeSm:       root.fontSizeSm
                    mainFont:         root.p2pFont
                    bodyFont:         root.dotGothicFont
                }
            }

            // ── MOVE MENU state ────────────────────────────────────────────────
            MoveMenu {
                id:         moveMenu
                anchors.fill: parent
                fontSizeLg: root.fontSizeLg
                fontSizeMd: root.fontSizeMd
                fontSizeSm: root.fontSizeSm
                mainFont:   root.p2pFont
                bodyFont:   root.dotGothicFont
                visible:    root.menuState === "moveMenu"
                onReturnClicked: root.menuState = "default"
            }
        }
    }
    function editButtonClicked(pokeData) {
        moveMenu.pokeData    = pokeData
        moveMenu.rowId       = pokeData.rowId
        moveMenu.spriteSheet = pokeData.isBig ? "qrc:/assets/HGSS/reordered_sprites_big.png"
                                              : "qrc:/assets/HGSS/reordered_sprites.png"
        moveMenu.frameWidth  = pokeData.isBig ? 64 : 32
        moveMenu.frameHeight = pokeData.isBig ? 64 : 32
        moveMenu.scaleFactor = pokeData.isBig ? root.iconScaleForBig : root.iconScale
        root.menuState       = "moveMenu"

        if (pc.inSwapMode) pc.toggleSwapMode()
        if (moveMenu.inNameEditMode) moveMenu.toggleNameEditMode()
    }

}
