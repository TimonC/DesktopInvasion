import QtQuick 2.15

Item {
    id: root

    property double uiScale:  1.0

    readonly property int baseWidth: pad + pcW + pad + dividerW + pad + rightPanelW + pad
    readonly property int baseHeight: pad + labelHeight + contentSpacing + trainerH + pad + dividerW + pad + labelHeight + contentSpacing + pcH + pad

    width: baseWidth * uiScale
    height: baseHeight * uiScale

    readonly property int pad:      20
    readonly property int dividerW: 1
    readonly property color dividerColor: "#3a3a3a"

    readonly property int trainerH:    160
    readonly property int pcH:         640
    readonly property int pcW:         640
    readonly property int rightPanelW: 640

    readonly property int labelHeight: 24
    readonly property int contentSpacing: 8

    property int iconScale:           8
    property int iconScaleForBig:     6
    property int iconScaleForTrainer: 5

    property color  backgroundColor:     "#2b2b2b"
    property color  textColor:           "#ffffff"
    property color  subheaderColor:      "#aaaaaa"
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

    property string menuState: "default"

    property var    _pendingPokeData:    null
    property int    _pendingRowId:       0
    property string _pendingSheet:       ""
    property int    _pendingFrameWidth:  32
    property int    _pendingFrameHeight: 32
    property real   _pendingScaleFactor: 8
    property int    _pendingEvolveSourceBox: -1
    property int    _pendingEvolveSourceSlot: -1

    property alias spriteSize:         trainer.spriteSize
    property alias battleSpeed:        trainer.battleSpeed
    property alias encounterLevelHigh: trainer.encounterLevelHigh
    property alias encounterLevelLow:  trainer.encounterLevelLow
    property alias expShare:           trainer.expShare

    property alias trainerId: trainer.row
    property string trainerName: "TRAINER"

    function updateDefaults(spriteSize, battleSpeed, lvlRangeDown, lvlRangeUp, expShare){
       trainer.spriteSizeDefault = spriteSize
       trainer.battleSpeedDefault = battleSpeed
       trainer.encounterLevelLowDefault = lvlRangeDown
       trainer.encounterLevelHighDefault = lvlRangeUp
       trainer.expShareDefault = expShare
    }

    function _updateEvolvesMenu(evolveData) {
        _pendingPokeData = evolveData
        root.menuState = "evolveMenu"
        if (evolveMenuLoader.item)
            evolveMenuLoader.item.pokeData = evolveData
    }

    Item {
        id: scaledContent
        width: root.baseWidth
        height: root.baseHeight
        transform: Scale { origin.x: 0; origin.y: 0; xScale: root.uiScale; yScale: root.uiScale }

        Rectangle {
            id: contentRect
            color: backgroundColor
            width: root.baseWidth
            height: root.baseHeight

            MouseArea {
                anchors.fill: parent
                cursorShape:  undefined
                onClicked: {
                    if (pc.inSwapMode) pc.toggleSwapMode()
                    var mm = moveMenuLoader.item
                    if (mm) {
                        if (mm.inNameEditMode) mm.toggleNameEditMode()
                        mm.selectedEligibleIdx = -1
                    }
                }
            }

            Connections {
                target: menuBridge

                function onPartyDataReady(data, displayFirst) {
                    pc.loadParty(data)
                    var map = {}
                    for (var i = 0; i < data.length; i++) map[data[i].slot] = data[i]
                    partyPokes = map
                    if(displayFirst) pc._display([-1, 0])
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

                    var pv = pokeViewLoader.item
                    if (pv) {
                        pv.pokeData    = poke
                        pv.rowId       = poke.rowId
                        pv.spriteSheet = poke.isBig ? "qrc:/assets/HGSS/reordered_sprites_big.png"
                                                    : "qrc:/assets/HGSS/reordered_sprites.png"
                        pv.frameWidth  = poke.isBig ? 64 : 32
                        pv.frameHeight = poke.isBig ? 64 : 32
                        pv.scaleFactor = poke.isBig ? root.iconScaleForBig : root.iconScale
                    }
                }
            }

            Item {
                anchors.fill: parent
                anchors.margins: root.pad

                Row {
                    anchors.fill: parent
                    spacing:         0

                    Column {
                        id: leftColumn
                        width:   root.pcW
                        height:  parent.height
                        spacing: 0
                        visible: root.menuState === "default"

                        Text {
                            width: parent.width; height: root.labelHeight
                            text: root.trainerName
                            font.family: root.p2pFont;
                            font.pixelSize: root.fontSizeSm
                            color: root.subheaderColor
                            horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                        }

                        Item { width: parent.width; height: root.contentSpacing }

                        Item {
                            width: parent.width; height: root.trainerH
                            Trainer {
                                id: trainer
                                anchors.fill: parent
                                anchors.left: parent.left
                                textColor:    root.textColor
                                fontSizeSm:     root.fontSizeSm
                                fontSizeMd:     root.fontSizeMd
                                fontSizeLg:     root.fontSizeLg
                                iconScale:    root.iconScaleForTrainer
                            }
                        }

                        Item { width: parent.width; height: root.pad }
                        Rectangle { width: parent.width; height: root.dividerW; color: root.dividerColor }
                        Item { width: parent.width; height: root.pad }

                        Item { width: parent.width; height: root.contentSpacing }

                        Item {
                            width: parent.width; height: root.pcH
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

                    Item      { width: root.pad;     height: parent.height; visible: root.menuState === "default" }
                    Rectangle { width: root.dividerW; height: parent.height; color: root.dividerColor; visible: root.menuState === "default" }
                    Item      { width: root.pad;     height: parent.height; visible: root.menuState === "default" }

                    Item {
                        width: root.menuState === "default"
                                   ? root.rightPanelW
                                   : root.pcW + root.pad * 2 + root.dividerW + root.rightPanelW
                        height: parent.height

                        Column {
                            width:   parent.width
                            height:  parent.height
                            spacing: 0
                            visible: root.menuState === "default"

                            Row {
                                width:  parent.width - pc.buttonWidth * 4
                                height: root.labelHeight
                                Text {
                                    width: parent.width; height: root.labelHeight
                                    text: "SUMMARY"
                                    font.family: root.p2pFont; font.pixelSize: root.fontSizeSm
                                    color: root.subheaderColor
                                    horizontalAlignment: Text.AlignLeft; verticalAlignment: Text.AlignVCenter
                                }
                                PcButton {
                                    id:    pokeEditButton
                                    label: "EDIT →"
                                    width: pc.buttonWidth * 4
                                    onClicked: editButtonClicked(pokeViewLoader.item ? pokeViewLoader.item.pokeData : null)
                                }
                            }

                            Item { width: parent.width; height: root.contentSpacing }

                            Loader {
                                id:     pokeViewLoader
                                width:  parent.width
                                height: parent.height - root.labelHeight - root.contentSpacing
                                active: root.menuState === "default"
                                source: "PokeView.qml"

                                onLoaded: {
                                    item.fontSizeLg = root.fontSizeLg
                                    item.fontSizeMd = root.fontSizeMd
                                    item.fontSizeSm = root.fontSizeSm
                                    item.mainFont   = root.p2pFont
                                    item.bodyFont   = root.dotGothicFont
                                    item.evolveBtnClicked.connect(root.evolveButtonClicked)
                                }
                            }
                        }

                        Loader {
                            id:           moveMenuLoader
                            anchors.fill: parent
                            active:       root.menuState === "moveMenu"
                            source:       "MoveMenu.qml"

                            onLoaded: {
                                item.fontSizeLg = root.fontSizeLg
                                item.fontSizeMd = root.fontSizeMd
                                item.fontSizeSm = root.fontSizeSm
                                item.mainFont   = root.p2pFont
                                item.bodyFont   = root.dotGothicFont

                                item.returnClicked.connect(root.goToDefaultMenu)

                                item.nameChanged.connect(function(name) {
                                    menuBridge.nameChangeRequested(pc.displayedPokemonBox, pc.displayedPokemonIndex, name)
                                })
                                item.requestMoveChange.connect(function(slot, moveId) {
                                    menuBridge.moveChangeRequested(pc.displayedPokemonBox, pc.displayedPokemonIndex, slot, moveId);
                                })

                                item.spriteSheet = root._pendingSheet
                                item.frameWidth  = root._pendingFrameWidth
                                item.frameHeight = root._pendingFrameHeight
                                item.scaleFactor = root._pendingScaleFactor
                                item.rowId       = root._pendingRowId
                                item.pokeData    = root._pendingPokeData
                            }
                        }

                        Loader {
                            id:           evolveMenuLoader
                            anchors.fill: parent
                            active:       root.menuState === "evolveMenu"
                            source:       "EvolveMenu.qml"

                            onLoaded: {
                                item.fontSizeLg = root.fontSizeLg
                                item.fontSizeMd = root.fontSizeMd
                                item.fontSizeSm = root.fontSizeSm
                                item.mainFont   = root.p2pFont
                                item.bodyFont   = root.dotGothicFont

                                item.returnClicked.connect(root.goToDefaultMenu)
                                item.evolutionSelected.connect(root.handleEvolutionSelected)

                                item.pokeData = root._pendingPokeData
                            }
                        }
                    }
                }
            }
        }
    }

    function handleEvolutionSelected(box, slot, modelData, nickName) {
        menuBridge.evolvePokemon(_pendingEvolveSourceBox, _pendingEvolveSourceSlot, modelData.pokedex_id, nickName)
        goToDefaultMenu()
    }

    function goToDefaultMenu(){
        root.menuState = "default"
        pc._display([pc.displayedPokemonBox, pc.displayedPokemonIndex])
    }

    function editButtonClicked(pokeData) {
        if (!pokeData) return
        if (pc.inSwapMode) pc.toggleSwapMode()

        _pendingPokeData    = pokeData
        _pendingRowId       = pokeData.rowId
        _pendingSheet       = pokeData.isBig ? "qrc:/assets/HGSS/reordered_sprites_big.png"
                                             : "qrc:/assets/HGSS/reordered_sprites.png"
        _pendingFrameWidth  = pokeData.isBig ? 64 : 32
        _pendingFrameHeight = pokeData.isBig ? 64 : 32
        _pendingScaleFactor = pokeData.isBig ? root.iconScaleForBig : root.iconScale

        root.menuState = "moveMenu"
    }

    function evolveButtonClicked(pokeData) {
        if (!pokeData) return
        if (pc.inSwapMode) pc.toggleSwapMode()

        _pendingPokeData = pokeData
        _pendingRowId = pokeData.rowId
        _pendingSheet = pokeData.isBig ? "qrc:/assets/HGSS/reordered_sprites_big.png"
                                       : "qrc:/assets/HGSS/reordered_sprites.png"
        _pendingFrameWidth = pokeData.isBig ? 64 : 32
        _pendingFrameHeight = pokeData.isBig ? 64 : 32
        _pendingScaleFactor = pokeData.isBig ? root.iconScaleForBig : root.iconScale
        _pendingEvolveSourceBox = pc.displayedPokemonBox
        _pendingEvolveSourceSlot = pc.displayedPokemonIndex

        menuBridge.evolvesRequested(pc.displayedPokemonBox, pc.displayedPokemonIndex, pokeData)
    }

    function receivedEvolutions(evolveData){
        if (!evolveData) return
        if (pc.inSwapMode) pc.toggleSwapMode()
        _pendingPokeData = evolveData
        _pendingEvolveSourceBox = pc.displayedPokemonBox
        _pendingEvolveSourceSlot = pc.displayedPokemonIndex
        root.menuState = "evolveMenu"
    }
}
