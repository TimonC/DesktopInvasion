import QtQuick 2.15

Rectangle {
    id: root
    color: backgroundColor

    // ── Hardcoded layout numbers ───────────────────────────────────────────────
    // PC block is fixed: 432 wide, 390 tall. Everything else is built around it.
    // Left col:  pad(16) + pc(432) + pad(16)  = 464
    // Right col: pad(16) + view(480) + pad(16) = 512
    // Total W:   464 + divider(1) + 512        = 977
    // Trainer:   pad(16) + trainer(120) + gap(12) + pc(390) + pad(16) = 554
    // Total H:   554

    readonly property int outerPad:    16
    readonly property int sectionGap:  12
    readonly property int panelGap:    1

    readonly property int trainerH:    300
    readonly property int pcH:         500
    readonly property int pcW:         640
    readonly property int rightPanelW: 640

    width:  outerPad + pcW + outerPad + panelGap + outerPad + rightPanelW + outerPad
    height: outerPad + trainerH + sectionGap + pcH + outerPad

    // ── Theme ─────────────────────────────────────────────────────────────────
    property color  backgroundColor:     "#2b2b2b"
    property color  buttonColor:         "#3c3c3c"
    property color  buttonSelectedColor: "#5294e2"
    property color  textColor:           "#ffffff"
    property int    headerFontSize:      24
    property int    bodyFontSize:        18
    property string p2pFont:             "Press Start 2P"
    property string dotGothicFont:       "DotGothic16"

    // ── App state ─────────────────────────────────────────────────────────────
    property string currentMenu: "Party"
    readonly property int pcRows:    4
    readonly property int pcColumns: 4
    property var partyPokes: ({})
    property var boxPokes:   ({})

    MouseArea {
        anchors.fill: parent
        cursorShape:  undefined
        onClicked:    { if (pc.inSwapMode) pc.toggleSwapMode() }
    }

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
            pokeView.scaleFactor = poke.isBig ? 4  : 6
        }
    }

    // ── Left column ───────────────────────────────────────────────────────────

    // Trainer strip
    Item {
        x:      root.outerPad
        y:      root.outerPad
        width:  root.pcW
        height: root.trainerH

        Trainer {
            anchors.fill: parent
            textColor:    root.textColor
            fontSize:     root.bodyFontSize
            fontFamily:   root.p2pFont
        }

        Rectangle {
            anchors.left:   parent.left
            anchors.right:  parent.right
            anchors.bottom: parent.bottom
            height: 1
            color:  "#3a3a3a"
        }
    }

    // PC block
    PC {
        id: pc
        x:      root.outerPad
        y:      root.outerPad + root.trainerH + root.sectionGap
        width:  root.pcW
        height: root.pcH
        fontFamily: root.dotGothicFont
    }

    // ── Vertical divider ──────────────────────────────────────────────────────
    Rectangle {
        x:      root.outerPad + root.pcW + root.outerPad
        y:      root.outerPad
        width:  root.panelGap
        height: root.height - root.outerPad * 2
        color:  "#3a3a3a"
    }

    // ── Right panel: PokeView ─────────────────────────────────────────────────
    Item {
        x:      root.outerPad + root.pcW + root.outerPad + root.panelGap + root.outerPad
        y:      root.outerPad
        width:  root.rightPanelW
        height: root.height - root.outerPad * 2

        PokeView {
            id: pokeView
            anchors.fill: parent
        }
    }
}
