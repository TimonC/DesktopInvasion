import QtQuick 2.15
import "../Style/PokeColor.js" as PokeColor

// PokeView.
// Menu gives this a fixed width × height Item and sets anchors.centerIn.
// All internal layout works from this component's own width/height.
Item {
    id: pokeView

    property var    pokeData:    null
    property string spriteSheet: "qrc:/assets/HGSS/reordered_sprites.png"
    property int    frameWidth:  32
    property int    frameHeight: 32
    property real   scaleFactor: 6
    property int    rowId:       0

    // ── Fonts ──────────────────────────────────────────────────────────────────
    property string mainFont:   "Press Start 2P"
    property string bodyFont:   "DotGothic16"
    property int    fontSizeLg: 22
    property int    fontSizeMd: 18
    property int    fontSizeSm: 16

    // ── Colors ─────────────────────────────────────────────────────────────────
    property color colorText:          "#ffffff"
    property color colorSubtext:       "#aaaaaa"
    property color colorFaint:         "#cccccc"
    property color colorVeryFaint:     "#999999"
    property color colorDivider:       "#3d3d3d"
    property color colorMoveCard:      "#383838"
    property color colorTypePillText:  "#ffffff"

    // ── Layout constants ───────────────────────────────────────────────────────
    property int  margin:        14
    property int  sectionGap:    8
    property int  rowSpacing:    6
    property int  moveCardPad:   10
    property int  moveCardGap:   8
    property int  typePillW:     96
    property int  typePillH:     fontSizeMd + 8
    property int  moveNameW:     typePillW * 3
    property int  movePillGap:   typePillW / 4

    // Section height ratios (must sum to 1.0)
    property real ratioTop:    0.32
    property real ratioFlavor: 0.16
    property real ratioMoves:  0.52

    // Derived inner height (excludes top + bottom margin)
    readonly property real innerH: height - margin * 2

    signal editButtonClicked(var pokeData)

    // ── Main content container ─────────────────────────────────────────────────
    // Centered inside whatever space Menu allocates.
    Item {
        anchors.centerIn: parent
        width:  parent.width  - pokeView.margin * 2
        height: parent.height - pokeView.margin * 2

        // ── SECTION 1 – Sprite + Info row ─────────────────────────────────────
        Row {
            id: topRow
            anchors {
                top:   parent.top
                left:  parent.left
                right: parent.right
            }
            height: innerH * ratioTop
            spacing: 0

            // Sprite column
            Item {
                width:  parent.width / 2
                height: parent.height

                Item {
                    anchors.centerIn: parent
                    width:  childrenRect.width
                    height: childrenRect.height

                    AnimatedSprite {
                        id: sprite
                        width:        pokeView.frameWidth  * pokeView.scaleFactor
                        height:       pokeView.frameHeight * pokeView.scaleFactor
                        running:      true
                        source:       pokeView.spriteSheet
                        frameWidth:   pokeView.frameWidth
                        frameHeight:  pokeView.frameHeight
                        frameCount:   2
                        frameRate:    4
                        interpolate:  false
                        smooth:       false
                        antialiasing: false
                        frameX: pokeView.frameWidth * 4
                        frameY: pokeView.rowId * pokeView.frameHeight

                        property bool isJumping:        false
                        property int  jumpUpDuration:   200
                        property int  jumpDownDuration: 150
                        property int  jumpHeight:       32

                        MouseArea {
                            anchors.fill: parent
                            cursorShape:  undefined
                            onClicked: {
                                if (!sprite.isJumping) {
                                    sprite.isJumping = true
                                    jumpAnim.start()
                                }
                            }
                        }

                        SequentialAnimation {
                            id: jumpAnim
                            PropertyAnimation {
                                target: sprite; property: "y"
                                to: sprite.y - sprite.jumpHeight
                                duration: sprite.jumpUpDuration; easing.type: Easing.OutQuad
                            }
                            PropertyAnimation {
                                target: sprite; property: "y"
                                to: sprite.y
                                duration: sprite.jumpDownDuration; easing.type: Easing.InQuad
                            }
                            PropertyAction { target: sprite; property: "isJumping"; value: false }
                        }
                    }
                }
            }

            // Info column
            Item {
                width:  parent.width / 2
                height: parent.height

                Column {
                    anchors.centerIn: parent
                    width:   parent.width * 0.9
                    spacing: pokeView.sectionGap

                   PcButton {
                       id: pokeEditButton
                       anchors.right:          parent.right
                       label:    "EDIT"
                       onClicked: pokeView.editButtonClicked(pokeView.pokeData)
                   }

                   Text {
                        text: pokeData ? pokeData.name : ""
                        font.family: mainFont
                        font.pixelSize: fontSizeLg
                        color: colorText
                        width: parent.width
                        elide: Text.ElideRight
                    }
                    Text {
                        text: pokeData ? pokeData.pokeName : ""
                        font.family: bodyFont
                        font.pixelSize: fontSizeMd
                        color: colorSubtext
                        width: parent.width
                        elide: Text.ElideRight
                    }
                    Row {
                        spacing: rowSpacing
                        Text {
                            text: pokeData ? "Lv. " + pokeData.level : ""
                            font.family: bodyFont; font.pixelSize: fontSizeMd
                            color: colorText; anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: pokeData ? pokeData.nature : ""
                            font.family: bodyFont; font.pixelSize: fontSizeSm
                            color: colorSubtext; anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                    Row {
                        spacing: rowSpacing
                        Repeater {
                            model: pokeData ? [pokeData.type1, pokeData.type2].filter(t => t && t !== "None") : []
                            Rectangle {
                                width: typePillW; height: typePillH; radius: 4
                                gradient: Gradient {
                                    GradientStop { position: 0.0; color: PokeColor.lighter(PokeColor.typeColor(modelData)) }
                                    GradientStop { position: 1.0; color: PokeColor.darker(PokeColor.typeColor(modelData)) }
                                }
                                Text {
                                    anchors.centerIn: parent
                                    text: PokeColor.typeColor(modelData) === "transparent" ? "" : modelData
                                    font.family: bodyFont; font.pixelSize: fontSizeMd
                                    color: colorTypePillText
                                }
                            }
                        }
                    }

                    Item { width: 1; height: 2 }

                    Grid {
                        id: statsGrid
                        columns: 3; columnSpacing: 16; rowSpacing: 2
                        property var statNames: ["HP","Atk","Def","SpA","SpD","Spe"]
                        Repeater {
                            model: pokeData ? pokeData.stats : []
                            Row {
                                spacing: 4
                                Text {
                                    text: statsGrid.statNames[index] + ":"
                                    font.family: bodyFont; font.pixelSize: fontSizeSm
                                    color: colorSubtext; width: 32
                                }
                                Text {
                                    text: modelData
                                    font.family: bodyFont; font.pixelSize: fontSizeSm
                                    color: colorText
                                }
                            }
                        }
                    }
                }
            }
        }

        // ── Divider 1 – between Section 1 and Section 2 ───────────────────────
        Rectangle {
            id: divider1
            anchors {
                top:   topRow.bottom
                left:  parent.left
                right: parent.right
            }
            height: 1
            color:  colorDivider
        }

        // ── SECTION 2 – Flavor text ────────────────────────────────────────────
        Item {
            id: flavorRow
            anchors {
                top:   divider1.bottom
                left:  parent.left
                right: parent.right
            }
            height: innerH * ratioFlavor

            Text {
                anchors.centerIn: parent
                width: parent.width
                text: pokeData ? pokeData.flavorText : ""
                font.family: bodyFont; font.pixelSize: fontSizeMd
                color: colorFaint
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignJustify
            }
        }

        // ── Divider 2 – between Section 2 and Section 3 ───────────────────────
        Rectangle {
            id: divider2
            anchors {
                top:   flavorRow.bottom
                left:  parent.left
                right: parent.right
            }
            height: 1
            color:  colorDivider
        }

        // ── SECTION 3 – Moves ──────────────────────────────────────────────────
        Item {
            anchors {
                top:    divider2.bottom
                left:   parent.left
                right:  parent.right
                bottom: parent.bottom
            }

            Column {
                anchors.centerIn: parent
                width:   parent.width
                spacing: moveCardGap

                Repeater {
                    model: pokeData ? pokeData.moves : []
                    Rectangle {
                        width:  parent.width
                        height: moveInner.implicitHeight + moveCardPad * 2
                        color:  colorMoveCard
                        radius: 5

                        Column {
                            id: moveInner
                            anchors { left: parent.left; right: parent.right; top: parent.top; margins: moveCardPad }
                            spacing: 4

                            Item {
                                width:  parent.width
                                height: Math.max(typeContainer.height, nameContainer.height, powerContainer.height)

                                Row {
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: movePillGap

                                    Item {
                                        id: typeContainer
                                        width: typePillW; height: fontSizeMd + 8
                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: typePillW; height: typePillH; radius: 3
                                            gradient: Gradient {
                                                GradientStop { position: 0.0; color: PokeColor.lighter(PokeColor.typeColor(modelData.type)) }
                                                GradientStop { position: 1.0; color: PokeColor.darker(PokeColor.typeColor(modelData.type)) }
                                            }
                                            Text {
                                                anchors.centerIn: parent
                                                text: modelData.type
                                                font.family: bodyFont; font.pixelSize: fontSizeMd
                                                color: colorTypePillText
                                            }
                                        }
                                    }

                                    Item {
                                        id: nameContainer
                                        width: moveNameW; height: typeContainer.height
                                        Text {
                                            anchors.centerIn: parent
                                            width: parent.width
                                            text: modelData.name
                                            font.family: mainFont; font.pixelSize: fontSizeMd
                                            color: colorText
                                            horizontalAlignment: Text.AlignLeft
                                            verticalAlignment:   Text.AlignVCenter
                                            elide: Text.ElideRight
                                        }
                                    }

                                    Item {
                                        id: powerContainer
                                        width: powerText.implicitWidth; height: typeContainer.height
                                        Text {
                                            id: powerText
                                            anchors.centerIn: parent
                                            text: "Pow: " + modelData.power + "   Acc: " + modelData.accuracy
                                            font.family: bodyFont; font.pixelSize: fontSizeSm
                                            color: colorSubtext; verticalAlignment: Text.AlignVCenter
                                        }
                                    }
                                }
                            }

                            Text {
                                height: fontSizeMd * 2.6
                                width:  parent.width
                                text:   modelData.flavor
                                font.family: bodyFont; font.pixelSize: fontSizeMd
                                color: colorVeryFaint
                                wrapMode: Text.WordWrap
                            }
                        }
                    }
                }
            }
        }
    }
}
