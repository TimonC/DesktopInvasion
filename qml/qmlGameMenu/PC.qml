import QtQuick 2.15
import "../Style/PokeColor.js" as PokeColor

Item {
    id: root

    property string fontFamily:      "sans-serif"
    property color  partyBackground: "green"
    property color  pcBackground:    "grey"
    property color  buttonColor:     "#5294e2"
    property color  panelBorderColor: "#80FFFFFF"
    property color  panelShadowColor: "#00000055"
    property int    slotWidth:       120
    property int    slotHeight:      90
    property int    freePartySlot:   -1
    property int    panelPadding:    12

    property color  highlightDefault:      "#4a90e2"
    property color  highlightHover:        "#6aaef5"
    property color  highlightSwappable:    "#7ab0f5"
    property color  highlightSwappableHover: "#8fc5ff"
    property color  highlightDisplayed:    "#64b5f6"
    property color  swapButtonHighlight:   "#ff9933"

    property real   highlightOpacity:      0.3
    property real   highlightHoverOpacity: 0.5
    property real   backgroundOpacity:     0.3

    property color  highlightColor:        Qt.rgba(0.29, 0.56, 0.89, highlightOpacity)
    property color  hoverHighlightColor:   Qt.rgba(0.42, 0.68, 0.96, highlightHoverOpacity)
    property color  swapColor:             Qt.rgba(0.48, 0.69, 0.96, highlightOpacity)
    property color  swapHoverColor:        Qt.rgba(0.56, 0.77, 1.0, highlightHoverOpacity)
    property color  displayedColor:        Qt.rgba(0.39, 0.71, 0.96, highlightHoverOpacity)

    readonly property int partyRows:    2
    readonly property int partyColumns: 3
    readonly property int pcRows:       4
    readonly property int pcColumns:    4
    readonly property int maxBoxes:     99

    property int fontSizeLg: 0
    property int fontSizeMd: 0
    property int fontSizeSm: 0

    readonly property int layoutMargin:  30
    readonly property int layoutSpacing: 20
    readonly property int buttonWidth:   48
    readonly property int buttonHeight:  64
    readonly property int labelHeight:   fontSizeLg > 0 ? fontSizeLg + 16 : 28

    readonly property int   panelRadius:      8
    readonly property int   panelBorderWidth: 2
    readonly property int   buttonRadius:     10

    property int currentBoxIndex: 0
    property var partyMap: ({})
    property var boxes:    ({})

    Component.onCompleted: { loadBox(0, []); showBox(0) }

    signal preloadBoxRequested(int boxIndex)
    signal swapRequested(var posx, var posy)

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

    readonly property int pcRowWidth:    root.buttonWidth + root.layoutSpacing
                                       + root.pcColumns * root.slotWidth
                                       + root.layoutSpacing + root.buttonWidth
    readonly property int partyRowWidth: root.partyColumns * root.slotWidth
                                       + root.layoutSpacing
                                       + Math.round(root.buttonWidth * 1.4)

    Column {
        anchors.centerIn: parent
        width:            parent.width
        spacing:          root.layoutSpacing

        Item {
            width:  parent.width
            height: root.partyRows * root.slotHeight

            Item {
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                width:  root.partyRowWidth
                height: root.partyRows * root.slotHeight

                Item {
                    anchors.left:           parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    width:  root.partyColumns * root.slotWidth
                    height: root.partyRows    * root.slotHeight

                    Rectangle {
                        anchors.fill:       parent
                        anchors.margins:    -1
                        anchors.topMargin:  3
                        anchors.leftMargin: 3
                        radius:             root.panelRadius + 1
                        color:              root.panelShadowColor
                        z:                  -1
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius:       root.panelRadius
                        color:        "transparent"
                        border.color: root.panelBorderColor
                        border.width: root.panelBorderWidth
                        z: 1
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius:       root.panelRadius

                        gradient: Gradient {
                            GradientStop {
                                position: 0.0
                                color: {
                                    var c = Qt.color(root.partyBackground)
                                    return Qt.rgba(c.r, c.g, c.b, root.backgroundOpacity * 1.5)
                                }
                            }
                            GradientStop {
                                position: 0.2
                                color: {
                                    var c = Qt.color(root.partyBackground)
                                    return Qt.rgba(c.r * 0.95, c.g * 0.95, c.b * 0.95, root.backgroundOpacity * 1.3)
                                }
                            }
                            GradientStop {
                                position: 0.4
                                color: {
                                    var c = Qt.color(root.partyBackground)
                                    return Qt.rgba(c.r * 0.9, c.g * 0.9, c.b * 0.9, root.backgroundOpacity * 1.1)
                                }
                            }
                            GradientStop {
                                position: 0.6
                                color: {
                                    var c = Qt.color(root.partyBackground)
                                    return Qt.rgba(c.r * 0.85, c.g * 0.85, c.b * 0.85, root.backgroundOpacity)
                                }
                            }
                            GradientStop {
                                position: 0.8
                                color: {
                                    var c = Qt.color(root.partyBackground)
                                    return Qt.rgba(c.r * 0.8, c.g * 0.8, c.b * 0.8, root.backgroundOpacity * 1.1)
                                }
                            }
                            GradientStop {
                                position: 1.0
                                color: {
                                    var c = Qt.color(root.partyBackground)
                                    return Qt.rgba(c.r * 0.75, c.g * 0.75, c.b * 0.75, root.backgroundOpacity * 1.3)
                                }
                            }
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

                PcButton {
                    id: swapButton
                    anchors.right:          parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    label:    "SWAP"
                    btnColor: root.buttonColor
                    highlightColor: root.inSwapMode ? root.swapButtonHighlight : root.buttonColor
                    onClicked: root.toggleSwapMode()
                    fontFamily: root.fontFamily
                }
            }
        }

        Item {
            width:  parent.width
            height: root.labelHeight + root.pcRows * root.slotHeight + (root.panelPadding * 2)

            Row {
                anchors.centerIn: parent
                spacing:          root.layoutSpacing

                PcButton {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: Math.round(root.labelHeight / 2)
                    width:     root.buttonWidth
                    height:    root.buttonHeight
                    label:     "◀"
                    btnColor:  root.buttonColor
                    onClicked: root._slideLeft()
                }

                Item {
                    width:  root.pcColumns * root.slotWidth + root.panelPadding/2
                    height: root.labelHeight + root.pcRows * root.slotHeight + (root.panelPadding * 2)

                    Rectangle {
                        anchors.fill:       parent
                        anchors.margins:    -1
                        anchors.topMargin:  4
                        anchors.leftMargin: 4
                        radius:             root.panelRadius + 1
                        color:              root.panelShadowColor
                        z:                  -1
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius:       root.panelRadius
                        color:        "transparent"
                        border.color: root.panelBorderColor
                        border.width: root.panelBorderWidth
                        z:            1
                    }

                    Item {
                        id: labelStrip
                        anchors.top:   parent.top
                        anchors.left:  parent.left
                        anchors.right: parent.right
                        height:        root.labelHeight + root.panelPadding
                        z:             2

                        Rectangle {
                            anchors.fill: parent
                            radius: root.panelRadius
                            color: {
                                var c = Qt.color(root.pcBackground)
                                return Qt.rgba(c.r, c.g, c.b, root.backgroundOpacity * 1.5)
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            text:           "Box " + (root.currentBoxIndex + 1)
                            color:          "white"
                            font.family:    root.fontFamily
                            font.pixelSize: root.fontSizeLg
                            font.bold:      true
                        }

                        Rectangle {
                            anchors.bottom: parent.bottom
                            anchors.left:   parent.left
                            anchors.right:  parent.right
                            height: 1
                            color: root.panelBorderColor
                        }
                    }

                    Rectangle {
                        anchors.top:    labelStrip.bottom
                        anchors.left:   parent.left
                        anchors.right:  parent.right
                        anchors.bottom: parent.bottom
                        radius: 0

                        gradient: Gradient {
                            GradientStop {
                                position: 0.0
                                color: {
                                    var c = Qt.color(root.pcBackground)
                                    return Qt.rgba(c.r, c.g, c.b, root.backgroundOpacity * 1.5)
                                }
                            }
                            GradientStop {
                                position: 0.2
                                color: {
                                    var c = Qt.color(root.pcBackground)
                                    return Qt.rgba(c.r * 0.95, c.g * 0.95, c.b * 0.95, root.backgroundOpacity * 1.3)
                                }
                            }
                            GradientStop {
                                position: 0.4
                                color: {
                                    var c = Qt.color(root.pcBackground)
                                    return Qt.rgba(c.r * 0.9, c.g * 0.9, c.b * 0.9, root.backgroundOpacity * 1.1)
                                }
                            }
                            GradientStop {
                                position: 0.6
                                color: {
                                    var c = Qt.color(root.pcBackground)
                                    return Qt.rgba(c.r * 0.85, c.g * 0.85, c.b * 0.85, root.backgroundOpacity)
                                }
                            }
                            GradientStop {
                                position: 0.8
                                color: {
                                    var c = Qt.color(root.pcBackground)
                                    return Qt.rgba(c.r * 0.8, c.g * 0.8, c.b * 0.8, root.backgroundOpacity * 1.1)
                                }
                            }
                            GradientStop {
                                position: 1.0
                                color: {
                                    var c = Qt.color(root.pcBackground)
                                    return Qt.rgba(c.r * 0.75, c.g * 0.75, c.b * 0.75, root.backgroundOpacity * 1.3)
                                }
                            }
                        }

                        Item {
                            anchors.fill: parent
                            anchors.margins: root.panelPadding

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
                }

                PcButton {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: Math.round(root.labelHeight / 2)
                    width:     root.buttonWidth
                    height:    root.buttonHeight
                    label:     "▶"
                    btnColor:  root.buttonColor
                    onClicked: root._slideRight()
                }
            }
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
            if (displayed && root.displayedPokemonBox === pcPos[0])
                return root.displayedColor
            if (swappable)
                return hoverArea.containsMouse ? root.swapHoverColor : root.swapColor
            if (hoverArea.containsMouse && iconVisible)
                return root.hoverHighlightColor
            return "transparent"
        }

        color: "transparent"

        Rectangle {
            anchors.fill:    parent
            anchors.margins: 2
            radius:          6
            color:           "#00000020"
            border.color:    "#00000015"
            border.width:    1
        }

        Rectangle {
            anchors.fill:    parent
            anchors.margins: 1
            radius:          root.panelRadius
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
