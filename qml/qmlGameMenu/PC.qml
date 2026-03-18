import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../Style/PokeColor.js" as PokeColor

Item {
    id: root

    // ── Visual config ──────────────────────────────────────────────────────────
    property string fontFamily:      "sans-serif"
    property color  partyBackground: "white"
    property color  pcBackground:    "green"
    property color  buttonColor:     "#5294e2"
    property int    slotWidth:       120
    property int    slotHeight:      90
    property int    freePartySlot:   -1

    readonly property int partyRows:    2
    readonly property int partyColumns: 3
    readonly property int pcRows:       4
    readonly property int pcColumns:    4
    readonly property int maxBoxes:     99

    property int fontSizeLg: 0
    property int fontSizeMd: 0
    property int fontSizeSm: 0

    // ── Spacing (single source of truth) ──────────────────────────────────────
    readonly property int layoutMargin:  30
    readonly property int layoutSpacing: 20

    // ── Button size ───────────────────────────────────────────────────────────
    readonly property int buttonWidth:  48
    readonly property int buttonHeight: 64

    // ── Label strip height ────────────────────────────────────────────────────
    readonly property int labelHeight: fontSizeLg > 0 ? fontSizeLg + 16 : 28

    // ── Style tokens ──────────────────────────────────────────────────────────
    readonly property int   panelRadius:      14
    readonly property int   panelBorderWidth: 2
    readonly property color panelBorderColor: "#ffffff30"
    readonly property color panelShadowColor: "#00000040"
    readonly property int   buttonRadius:     10

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

    // ── Computed widths for centering ─────────────────────────────────────────
    // PC row total width  = btn + gap + grid + gap + btn
    readonly property int pcRowWidth:    root.buttonWidth + root.layoutSpacing
                                       + root.pcColumns * root.slotWidth
                                       + root.layoutSpacing + root.buttonWidth
    // Party row total width = grid + gap + swapBtn
    readonly property int partyRowWidth: root.partyColumns * root.slotWidth
                                       + root.layoutSpacing
                                       + Math.round(root.buttonWidth * 1.4)

    // ── Root column: party row / pc row ───────────────────────────────────────
    Column {
        anchors.verticalCenter:   parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        spacing:                  root.layoutSpacing

        // ── Party row ─────────────────────────────────────────────────────────
        Item {
            width:  root.pcRowWidth
            height: root.partyRows * root.slotHeight

            Item {
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                width:  root.partyRowWidth
                height: root.partyRows * root.slotHeight

                // Party grid panel
                Item {
                    anchors.left:           parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    width:  root.partyColumns * root.slotWidth
                    height: root.partyRows    * root.slotHeight

                    // Drop shadow
                    Rectangle {
                        anchors.fill:       parent
                        anchors.margins:    -1
                        anchors.topMargin:  3
                        anchors.leftMargin: 3
                        radius:             root.panelRadius + 1
                        color:              root.panelShadowColor
                        z:                  -1
                    }

                    // Border ring
                    Rectangle {
                        anchors.fill: parent
                        radius:       root.panelRadius
                        color:        "transparent"
                        border.color: root.panelBorderColor
                        border.width: root.panelBorderWidth
                        z: 1
                    }

                    // Gradient fill
                    Rectangle {
                        anchors.fill: parent
                        radius:       root.panelRadius
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: PokeColor.lighter(root.partyBackground) }
                            GradientStop { position: 1.0; color: PokeColor.darker(root.partyBackground) }
                        }
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

                // Swap button
                PcButton {
                    id: swapButton
                    anchors.right:          parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    width:    Math.round(root.buttonWidth * 1.4)
                    height:   Math.round(root.buttonHeight * 0.6)
                    label:    "Swap"
                    btnColor: root.inSwapMode ? root.swapColor : root.buttonColor
                    onClicked: root.toggleSwapMode()
                }
            }
        }

        // ── PC row ────────────────────────────────────────────────────────────
        // The panel is labelHeight taller than the raw grid so the label strip
        // sits inside the border/shadow rather than overflowing above it.
        Item {
            width:  root.pcRowWidth
            height: root.labelHeight + root.pcRows * root.slotHeight

            // Left nav — aligned to the grid portion, not the whole panel
            PcButton {
                anchors.left: parent.left
                y:            root.labelHeight + (root.pcRows * root.slotHeight - root.buttonHeight) / 2
                width:        root.buttonWidth
                height:       root.buttonHeight
                label:        "◀"
                btnColor:     root.buttonColor
                onClicked:    root._slideLeft()
            }

            // PC panel (shadow + border + gradient wrap the full height)
            Item {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top:              parent.top
                width:  root.pcColumns * root.slotWidth
                height: parent.height

                // Drop shadow
                Rectangle {
                    anchors.fill:       parent
                    anchors.margins:    -1
                    anchors.topMargin:  4
                    anchors.leftMargin: 4
                    radius:             root.panelRadius + 1
                    color:              root.panelShadowColor
                    z:                  -1
                }
                // Border ring — now correctly wraps label + grid
                Rectangle {
                    anchors.fill: parent
                    radius:       root.panelRadius
                    color:        "transparent"
                    border.color: root.panelBorderColor
                    border.width: root.panelBorderWidth
                    z:            1
                }
                // Gradient fill
                Rectangle {
                    anchors.fill: parent
                    radius:       root.panelRadius
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: PokeColor.lighter(root.pcBackground) }
                        GradientStop { position: 1.0; color: PokeColor.darker(root.pcBackground)  }
                    }
                }

                // ── Label strip ───────────────────────────────────────────────
                Item {
                    id: labelStrip
                    anchors.top:   parent.top
                    anchors.left:  parent.left
                    anchors.right: parent.right
                    height:        root.labelHeight
                    z:             2

                    Text {
                        anchors.centerIn: parent
                        text:           "Box " + (root.currentBoxIndex + 1)
                        color:          "#ffffffcc"
                        font.family:    root.fontFamily
                        font.pixelSize: root.fontSizeLg
                        font.bold:      true
                    }

                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.left:   parent.left
                        anchors.right:  parent.right
                        height: 1
                        color:  "#ffffff25"
                    }
                }

                // ── Grid area — fills everything below the label strip ─────────
                Item {
                    anchors.top:    labelStrip.bottom
                    anchors.left:   parent.left
                    anchors.right:  parent.right
                    anchors.bottom: parent.bottom

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
            }

            // Right nav — aligned to the grid portion
            PcButton {
                anchors.right: parent.right
                y:             root.labelHeight + (root.pcRows * root.slotHeight - root.buttonHeight) / 2
                width:         root.buttonWidth
                height:        root.buttonHeight
                label:         "▶"
                btnColor:      root.buttonColor
                onClicked:     root._slideRight()
            }
        }
    }

    // ── Components ────────────────────────────────────────────────────────────

    component PcButton: Item {
        id: pcButtonRoot
        width:  root.buttonWidth
        height: root.buttonHeight

        property string label:    ""
        property bool   active:   true
        property color  btnColor: root.buttonColor

        signal clicked()

        // ── Derived state ─────────────────────────────────────────────────────
        property bool _hovered: mouseArea.containsMouse && active
        property bool _pressed: mouseArea.pressed        && active

        // Border: lighter at rest, base color on hover (matches reference)
        property color _borderColor: active
            ? (_hovered ? btnColor : PokeColor.lighter(btnColor))
            : "#555555"
        Behavior on _borderColor { ColorAnimation { duration: 120; easing.type: Easing.OutQuad } }

        // ── Scale container (press = squish, release = springy bounce) ────────
        Item {
            id: buttonContent
            anchors.fill: parent
            scale: pcButtonRoot._pressed ? 0.92 : 1.0
            Behavior on scale {
                NumberAnimation {
                    duration:          pcButtonRoot._pressed ? 80 : 200
                    easing.type:       pcButtonRoot._pressed ? Easing.OutQuad : Easing.OutBack
                    easing.overshoot:  1.2
                }
            }

            // Border ring
            Rectangle {
                anchors.fill: parent
                radius:       root.buttonRadius
                color:        pcButtonRoot._borderColor
            }

            // Gradient face
            Rectangle {
                anchors.fill:    parent
                anchors.margins: 2
                radius:          Math.max(0, root.buttonRadius - 2)
                gradient: Gradient {
                    GradientStop {
                        position: 0.0
                        color: pcButtonRoot.active
                            ? (pcButtonRoot._hovered
                               ? pcButtonRoot.btnColor
                               : PokeColor.lighter(pcButtonRoot.btnColor))
                            : "#444444"
                        Behavior on color { ColorAnimation { duration: 120; easing.type: Easing.OutCubic } }
                    }
                    GradientStop {
                        position: 1.0
                        color: pcButtonRoot.active
                            ? (pcButtonRoot._hovered
                               ? PokeColor.darker(PokeColor.darker(pcButtonRoot.btnColor))
                               : PokeColor.darker(pcButtonRoot.btnColor))
                            : "#333333"
                        Behavior on color { ColorAnimation { duration: 120; easing.type: Easing.OutCubic } }
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                text:             pcButtonRoot.label
                color:            pcButtonRoot.active ? "#ffffff" : "#888888"
                font.family:      root.fontFamily
                font.pixelSize:   root.fontSizeMd
                font.bold:        true
                style:            Text.Raised
                styleColor:       "#00000060"
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            enabled:      pcButtonRoot.active
            hoverEnabled: true
            onClicked:    pcButtonRoot.clicked()
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

        property color _baseColor: {
            if (displayed && root.displayedPokemonBox === pcPos[0]) return root.highlightColor
            if (swappable) return hoverArea.containsMouse ? root.highlightColor : root.swapColor
            if (hoverArea.containsMouse && iconVisible)   return root.highlightColor
            return "transparent"
        }

        color: "transparent"

        // Fill / highlight layer
        Rectangle {
            anchors.fill:    parent
            anchors.margins: 1
            radius:          6
            color:           pokemonSlot._baseColor
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

