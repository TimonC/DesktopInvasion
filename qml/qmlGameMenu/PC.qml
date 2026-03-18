import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    // ── Visual config ──────────────────────────────────────────────────────────
    property string fontFamily:      "sans-serif"
    property color  partyBackground: "white"
    property color  pcBackground:    "green"
    property color  buttonColor:     "#5294e2"
    property int    slotWidth:       80*1.5
    property int    slotHeight:      60*1.5
    property int    freePartySlot:   -1

    readonly property int partyRows:    2
    readonly property int partyColumns: 3
    readonly property int pcRows:       4
    readonly property int pcColumns:    4
    readonly property int maxBoxes:     99

    // ── Spacing (single source of truth) ──────────────────────────────────────
    readonly property int layoutMargin:  10
    readonly property int layoutSpacing: 10

    // ── Button size ───────────────────────────────────────────────────────────
    readonly property int buttonWidth:  36
    readonly property int buttonHeight: 56

    // ── State ─────────────────────────────────────────────────────────────────
    property int currentBoxIndex: 0
    property var partyMap: ({})
    property var boxes:    ({})

    Component.onCompleted: { loadBox(0, []); showBox(0) }

    signal preloadBoxRequested(int boxIndex)
    signal swapRequested(var posx, var posy)

    // ── Swap state ─────────────────────────────────────────────────────────────
    property color highlightColor:       Qt.rgba(0, 0.6, 1, 0.5)
    property color swapColor:            "orange"
    property bool  inSwapMode:           false
    property var   swapSource:           null
    property var   displayedPokemonSlot: null
    property int   displayedPokemonBox:  -1

    signal activateSwapMode()
    signal display(var pcPos)

    function _display(pcPos) {
        if (root.displayedPokemonSlot) root.displayedPokemonSlot.displayed = false
        root.displayedPokemonSlot = (pcPos[0] === -1)
            ? partyRepeater.itemAt(pcPos[1])
            : pcRepeater.itemAt(pcPos[1])
        root.displayedPokemonSlot.displayed = true
        root.displayedPokemonBox = pcPos[0]
        display(pcPos)
    }

    function toggleSwapMode() {
        if (inSwapMode) { inSwapMode = false; swapSource = null }
        else            { inSwapMode = true;  activateSwapMode() }
    }

    // ── Root column: party row / pc row ───────────────────────────────────────
    GridLayout {
        anchors.fill:    parent
        anchors.margins: root.layoutMargin
        columns:         1
        rows:            2
        rowSpacing:      root.layoutSpacing
        columnSpacing:   0

        // ── Party row ─────────────────────────────────────────────────────────
        GridLayout {
            Layout.row:             0
            Layout.column:          0
            Layout.alignment:       Qt.AlignHCenter | Qt.AlignTop
            Layout.preferredHeight: root.partyRows * root.slotHeight
            columns:                2
            rows:                   1
            columnSpacing:          root.layoutSpacing
            rowSpacing:             0

            // Party grid
            Item {
                Layout.column:          0
                Layout.row:             0
                Layout.alignment:       Qt.AlignVCenter
                Layout.preferredWidth:  root.partyColumns * root.slotWidth
                Layout.preferredHeight: root.partyRows    * root.slotHeight

                Rectangle {
                    anchors.fill: parent
                    color:        root.partyBackground
                    border.color: "#cccccc"
                    border.width: 1
                }

                Grid {
                    anchors.centerIn: parent
                    rows:             root.partyRows
                    columns:          root.partyColumns
                    rowSpacing:       0
                    columnSpacing:    0

                    Repeater {
                        id: partyRepeater
                        model: root.partyRows * root.partyColumns
                        PokemonSlot {
                            iconVisible: root.partyMap[index] !== undefined
                            frameIndex:  root.partyMap[index] !== undefined ? root.partyMap[index] : 0
                            pcPos:       [-1, index]
                        }
                    }
                }
            }

            PcButton {
                id: swapButton
                Layout.column:          1
                Layout.row:             0
                Layout.alignment:       Qt.AlignVCenter
                Layout.preferredWidth:  root.buttonWidth * 1.4
                Layout.preferredHeight: root.buttonHeight * 0.6
                label:   "Swap"
                color:   root.inSwapMode ? root.swapColor : root.buttonColor
                onClicked: root.toggleSwapMode()
            }
        }

        // ── PC row ────────────────────────────────────────────────────────────
        GridLayout {
            Layout.row:        1
            Layout.column:     0
            Layout.fillWidth:  true
            Layout.fillHeight: true
            Layout.alignment:  Qt.AlignHCenter | Qt.AlignVCenter
            columns:           3
            rows:              1
            columnSpacing:     root.layoutSpacing
            rowSpacing:        0

            PcButton {
                Layout.column:          0
                Layout.row:             0
                Layout.alignment:       Qt.AlignVCenter | Qt.AlignHCenter
                Layout.preferredWidth:  root.buttonWidth
                Layout.preferredHeight: root.buttonHeight
                label: "◀"
                onClicked: root._slideLeft()
            }

            // Box grid
            Item {
                Layout.column:          1
                Layout.row:             0
                Layout.alignment:       Qt.AlignCenter
                Layout.preferredWidth:  root.pcColumns * root.slotWidth
                Layout.preferredHeight: root.pcRows    * root.slotHeight

                Rectangle {
                    anchors.fill: parent
                    color:        root.pcBackground
                    border.color: "#cccccc"
                    border.width: 1
                }

                Grid {
                    anchors.centerIn: parent
                    rows:             root.pcRows
                    columns:          root.pcColumns
                    rowSpacing:       0
                    columnSpacing:    0

                    Repeater {
                        id: pcRepeater
                        model: root.pcRows * root.pcColumns
                        PokemonSlot {
                            property var currentBox: root.boxes[root.currentBoxIndex]
                            iconVisible: currentBox !== undefined && currentBox[index] !== undefined
                            frameIndex:  (currentBox !== undefined && currentBox[index] !== undefined)
                                         ? currentBox[index] : 0
                            pcPos: [root.currentBoxIndex, index]
                        }
                    }
                }
            }

            PcButton {
                Layout.column:          2
                Layout.row:             0
                Layout.alignment:       Qt.AlignVCenter | Qt.AlignHCenter
                Layout.preferredWidth:  root.buttonWidth
                Layout.preferredHeight: root.buttonHeight
                label: "▶"
                onClicked: root._slideRight()
            }
        }
    }

    // ── Components ────────────────────────────────────────────────────────────

    component PcButton: Rectangle {
        width:  root.buttonWidth
        height: root.buttonHeight
        radius: 6
        property string label:  ""
        property bool   active: true
        signal clicked()

        color: active
               ? (press.pressed ? Qt.darker(root.buttonColor, 1.3) : root.buttonColor)
               : "#444"

        Text {
            anchors.centerIn: parent
            text:             parent.label
            color:            parent.active ? "white" : "#888"
            font.family:      root.fontFamily
            font.pixelSize:   18
            font.bold:        true
        }

        MouseArea {
            id: press
            anchors.fill: parent
            enabled:      parent.active
            onClicked:    parent.clicked()
            cursorShape:  undefined
        }
    }


    component PokemonSlot: Rectangle {
        id: pokemonSlot
        width:  root.slotWidth
        height: root.slotHeight

        property int  frameIndex:  0
        property var  pcPos:       [-1, -1]
        property bool displayed:   false
        property bool iconVisible: false

        property bool swappable: {
            if (!root.inSwapMode || root.swapSource === null || root.swapSource === pcPos) return false
            if (root.swapSource[0] === -1) {
                if (pcPos[0] === -1) { if (pcPos[1] >= root.freePartySlot) return false }
                else                 { if (root.freePartySlot <= 1 && !iconVisible) return false }
                return true
            } else {
                if (pcPos[0] === -1 && pcPos[1] > root.freePartySlot) return false
                return true
            }
        }

        color: {
            if (displayed && root.displayedPokemonBox === pcPos[0]) return root.highlightColor
            if (swappable) return hoverArea.containsMouse ? root.highlightColor : root.swapColor
            if (hoverArea.containsMouse && iconVisible)   return root.highlightColor
            return "transparent"
        }

        // swap-mode border overlay
        Rectangle {
            anchors.fill: parent
            color:        "transparent"
            border.color: root.inSwapMode ? "blue" : "transparent"
            border.width: 1
            z: 1
        }

        Image {
            anchors.fill: parent
            visible:      parent.iconVisible
            source:       "qrc:/assets/HGSS/reordered_icons.png"
            readonly property int spriteWidth:  40
            readonly property int spriteHeight: 30
            sourceClipRect: Qt.rect(0, parent.frameIndex * spriteHeight, spriteWidth, spriteHeight)
            smooth:       false
            antialiasing: false
        }

        MouseArea {
            id: hoverArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape:  undefined
            onClicked: {
                if (root.inSwapMode) {
                    if (root.swapSource === null) {
                        root.swapSource = pokemonSlot.pcPos
                        if (pokemonSlot.iconVisible) root._display(pokemonSlot.pcPos)
                        else                         root.toggleSwapMode()
                        return
                    } else {
                        if (pokemonSlot.swappable) {
                            var joinParty = false
                            if (!pokemonSlot.iconVisible) {
                                if (root.swapSource[0] === -1 && pokemonSlot.pcPos[0] !== -1) {
                                    joinParty = root.swapSource[1] < (root.freePartySlot - 1)
                                    root.freePartySlot -= 1
                                }
                                if (root.swapSource[0] !== -1 && pokemonSlot.pcPos[0] === -1) {
                                    root.freePartySlot += 1
                                }
                            }
                            root._executeSwap(root.swapSource, pokemonSlot.pcPos)
                            root.swapRequested(root.swapSource, pokemonSlot.pcPos)
                            root._display(pokemonSlot.pcPos)
                            if (joinParty) {
                                for (var i = root.swapSource[1]; i < root.freePartySlot; i++) {
                                    root._executeSwap([-1, i], [-1, i + 1])
                                    root.swapRequested([-1, i], [-1, i + 1])
                                }
                            }
                        }
                        root.toggleSwapMode()
                        return
                    }
                }
                if (!pokemonSlot.iconVisible) return
                root._display(pokemonSlot.pcPos)
            }
        }
    }
    // ── Swap implementation ───────────────────────────────────────────────────
    function _executeSwap(posx, posy) {
        if (posx[0] === posy[0] && posx[1] === posy[1]) { console.log("Can't swap a slot with itself"); return }

        var iconx = posx[0] === -1 ? root.partyMap[posx[1]] : root.boxes[posx[0]][posx[1]]
        var icony = posy[0] === -1 ? root.partyMap[posy[1]] : root.boxes[posy[0]][posy[1]]

        var newParty = Object.assign({}, root.partyMap)
        var newBoxes = Object.assign({}, root.boxes)

        if (posx[0] === -1) { newParty[posx[1]] = icony }
        else { var arrX = newBoxes[posx[0]].slice(); arrX[posx[1]] = icony; newBoxes[posx[0]] = arrX }

        if (posy[0] === -1) { newParty[posy[1]] = iconx }
        else { var arrY = newBoxes[posy[0]].slice(); arrY[posy[1]] = iconx; newBoxes[posy[0]] = arrY }

        root.partyMap = newParty
        root.boxes    = newBoxes
    }

    function loadParty(dataList) {
        var map = {}
        for (var i = 0; i < dataList.length; i++) map[dataList[i].slot] = dataList[i].iconId
        partyMap = map
        root.freePartySlot = dataList.length
    }

    function loadBox(boxIndex, dataList) {
        var arr = new Array(pcRows * pcColumns)
        for (var i = 0; i < dataList.length; i++) arr[dataList[i].slot] = dataList[i].iconId
        var updated = Object.assign({}, boxes)
        updated[boxIndex] = arr
        boxes = updated
    }

    function showBox(boxIndex) { currentBoxIndex = boxIndex; _requestAdjacentPreloads(boxIndex) }

    function _requestAdjacentPreloads(index) {
        var left  = (index - 1 + maxBoxes) % maxBoxes
        var right = (index + 1) % maxBoxes
        if (!boxes.hasOwnProperty(left))  preloadBoxRequested(left)
        if (!boxes.hasOwnProperty(right)) preloadBoxRequested(right)
    }

    function _slideLeft() {
        var next = (currentBoxIndex - 1 + maxBoxes) % maxBoxes
        if (!boxes.hasOwnProperty(next)) preloadBoxRequested(next)
        currentBoxIndex = next
        _requestAdjacentPreloads(next)
    }

    function _slideRight() {
        var next = (currentBoxIndex + 1) % maxBoxes
        if (!boxes.hasOwnProperty(next)) preloadBoxRequested(next)
        currentBoxIndex = next
        _requestAdjacentPreloads(next)
    }
}
