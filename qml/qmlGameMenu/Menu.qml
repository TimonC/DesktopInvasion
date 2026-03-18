import QtQuick 2.15

Rectangle {
    id: root
    color: backgroundColor

    // ── Spacing / geometry ─────────────────────────────────────────────────────
    readonly property int pad:      12
    readonly property int dividerW: 1
    readonly property color dividerColor: "#3a3a3a"

    // ── Section dimensions (single source of truth) ────────────────────────────
    readonly property int trainerH:    200
    readonly property int pcH:         640
    readonly property int pcW:         640
    readonly property int rightPanelW: 640

    // ── Icon scales ────────────────────────────────────────────────────────────
    property int iconScale:           8
    property int iconScaleForBig:     6
    property int iconScaleForTrainer: 5

    // ── Overall window size ────────────────────────────────────────────────────
    // Horizontal: pad | pcW | pad  dividerW  pad | rightPanelW | pad
    width:  pad + pcW + pad + dividerW + pad + rightPanelW + pad

    // Vertical: pad | trainerH | pad  dividerW  pad | pcH | pad
    height: pad + trainerH + pad + dividerW + pad + pcH + pad

    // ── Theme ──────────────────────────────────────────────────────────────────
    property color  backgroundColor:     "#2b2b2b"
    property color  buttonColor:         "#3c3c3c"
    property color  buttonSelectedColor: "#5294e2"
    property color  textColor:           "#ffffff"
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

    // ── Background click – cancel swap ─────────────────────────────────────────
    MouseArea {
        anchors.fill: parent
        cursorShape:  undefined
        onClicked: { if (pc.inSwapMode) pc.toggleSwapMode() }
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

        function onShowBoxRequested(boxIndex) { pc.showBox(boxIndex) }
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
    //
    //  Every gap (edge margin, divider breathing room) equals `pad`.
    //  Layout tree maps exactly to the window size formula above:
    //
    //    pad | leftCol(pcW) | pad  [vDivider]  pad | rightCol(rightPanelW) | pad
    //    pad | section1(trainerH) | pad  [hDivider]  pad | section2(pcH) | pad
    // ══════════════════════════════════════════════════════════════════════════
    Row {
        anchors.fill:    parent
        anchors.margins: root.pad
        spacing:         0

        // ── LEFT COLUMN ───────────────────────────────────────────────────────
        Column {
            width:   root.pcW
            height:  parent.height
            spacing: 0

            // ── SECTION 1 – Trainer ───────────────────────────────────────────
            Item {
                width:  parent.width
                height: root.trainerH

                Trainer {
                    anchors.fill: parent
                    textColor:    root.textColor
                    fontSize:     root.fontSizeMd
                    fontFamily:   root.p2pFont
                    iconScale:    root.iconScaleForTrainer
                }
            }

            // pad gap above divider
            Item { width: parent.width; height: root.pad }

            // ── Horizontal divider ────────────────────────────────────────────
            Rectangle {
                width:  parent.width
                height: root.dividerW
                color:  root.dividerColor
            }

            // pad gap below divider
            Item { width: parent.width; height: root.pad }

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
                    fontFamily: root.dotGothicFont
                }
            }
        }

        // pad gap left of vertical divider
        Item { width: root.pad; height: parent.height }

        // ── Vertical divider ──────────────────────────────────────────────────
        Rectangle {
            width:  root.dividerW
            height: parent.height
            color:  root.dividerColor
        }

        // pad gap right of vertical divider
        Item { width: root.pad; height: parent.height }

        // ── RIGHT COLUMN  (Section 3) ──────────────────────────────────────────
        Item {
            width:  root.rightPanelW
            height: parent.height

            // ── SECTION 3 – PokeView ───────────────────────────────────────────
            PokeView {
                id:               pokeView
                anchors.centerIn: parent
                width:            parent.width
                height:           parent.height
                fontSizeLg:       root.fontSizeLg
                fontSizeMd:       root.fontSizeMd
                fontSizeSm:       root.fontSizeSm
                mainFont:         root.p2pFont
                bodyFont:         root.dotGothicFont
            }
        }
    }
}

