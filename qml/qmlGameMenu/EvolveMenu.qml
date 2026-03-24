import QtQuick 2.15
import "../Style/PokeColor.js" as PokeColor

Item {
    id: evolveMenu

    property var pokeData: null
    property real spriteScale: 5
    property int jumpDistance: 18

    property string mainFont:   "Press Start 2P"
    property string bodyFont:   "DotGothic16"
    property int    fontSizeLg: 22
    property int    fontSizeMd: 18
    property int    fontSizeSm: 16

    property color colorText:    "#ffffff"
    property color colorSubtext: "#aaaaaa"
    property color colorDivider: "#3d3d3d"

    property int margin: 8
    property int secGap: 16
    property int gap:    8

    property string spriteSheetNormal: "qrc:/assets/HGSS/reordered_sprites.png"
    property string spriteSheetBig:    "qrc:/assets/HGSS/reordered_sprites_big.png"
    property int    frameW:            32
    property int    frameH:            32
    property int    typePillW: 88
    property int    typePillH: fontSizeMd + 8

    signal evolutionSelected(int targetPokedexId)
    signal returnClicked()

    Column {
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        anchors.margins: evolveMenu.margin
        spacing: evolveMenu.secGap

        PcButton {
            width:  48 * 4
            label:  "← BACK"
            onClicked: evolveMenu.returnClicked()
        }

        Rectangle { width: parent.width; height: 1; color: evolveMenu.colorDivider }

        Text {
            text:           "EVOLVE"
            font.family:    evolveMenu.mainFont
            font.pixelSize: evolveMenu.fontSizeSm - 4
            color:          evolveMenu.colorSubtext
        }

        Text {
            text:           evolveMenu.pokeData ? evolveMenu.pokeData.name : ""
            font.family:    evolveMenu.mainFont
            font.pixelSize: evolveMenu.fontSizeSm
            color:          evolveMenu.colorText
        }

        Rectangle { width: parent.width; height: 1; color: evolveMenu.colorDivider }

        Item {
            width:  parent.width
            height: childrenRect.height

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 12

                Repeater {
                    model: evolveMenu.pokeData ? evolveMenu.pokeData.evolvesList : []

                    Column {
                        id: evoCol
                        width:   112
                        spacing: 6

                        Text {
                            width:               parent.width
                            text:                modelData.name
                            font.family:         evolveMenu.mainFont
                            font.pixelSize:      evolveMenu.fontSizeSm - 2
                            color:               evolveMenu.colorText
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode:            Text.NoWrap
                            elide:               Text.ElideRight
                        }

                        Item {
                            width:  parent.width
                            height: (modelData.isBig ? 64 : evolveMenu.frameH) * evolveMenu.spriteScale

                            AnimatedSprite {
                                id: evoSprite

                                readonly property bool isBig: modelData.isBig === true

                                anchors.horizontalCenter: parent.horizontalCenter
                                width:        (isBig ? 64 : evolveMenu.frameW) * evolveMenu.spriteScale
                                height:       (isBig ? 64 : evolveMenu.frameH) * evolveMenu.spriteScale
                                running:      true
                                source:       isBig ? evolveMenu.spriteSheetBig : evolveMenu.spriteSheetNormal
                                frameWidth:   isBig ? 64 : evolveMenu.frameW
                                frameHeight:  isBig ? 64 : evolveMenu.frameH
                                frameCount:   2
                                frameRate:    4
                                interpolate:  false
                                smooth:       false
                                antialiasing: false
                                frameX:       (isBig ? 64 : evolveMenu.frameW) * 4
                                frameY:       modelData.rowId * (isBig ? 64 : evolveMenu.frameH)

                                property bool isJumping: false
                                property int  jumpUpDuration:   200
                                property int  jumpDownDuration: 150

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape:  undefined
                                    onClicked: {
                                        if (!evoSprite.isJumping) {
                                            evoSprite.isJumping = true
                                            spriteJumpAnim.start()
                                        }
                                    }
                                }

                                SequentialAnimation {
                                    id: spriteJumpAnim
                                    PropertyAnimation {
                                        target:      evoSprite
                                        property:    "y"
                                        to:          evoSprite.y - evolveMenu.jumpDistance
                                        duration:    evoSprite.jumpUpDuration
                                        easing.type: Easing.OutQuad
                                    }
                                    PropertyAnimation {
                                        target:      evoSprite
                                        property:    "y"
                                        to:          evoSprite.y
                                        duration:    evoSprite.jumpDownDuration
                                        easing.type: Easing.InQuad
                                    }
                                    PropertyAction {
                                        target:   evoSprite
                                        property: "isJumping"
                                        value:    false
                                    }
                                    ScriptAction {
                                        script: {
                                            evolveMenu.evolutionSelected(modelData.pokedex_id)
                                            evolveMenu.returnClicked()
                                        }
                                    }
                                }
                            }
                        }

                        Row {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 4

                            Repeater {
                                model: [modelData.type1, modelData.type2].filter(function(t) {
                                    return t && t !== "None" && t !== ""
                                })

                                Rectangle {
                                    readonly property int pillW: Math.min(
                                        evolveMenu.typePillW,
                                        Math.floor((evoCol.width - 4) / 2)
                                    )
                                    width:  pillW
                                    height: evolveMenu.typePillH
                                    radius: 4
                                    gradient: Gradient {
                                        GradientStop {
                                            position: 0.0
                                            color: PokeColor.lighter(PokeColor.typeColor(modelData))
                                        }
                                        GradientStop {
                                            position: 1.0
                                            color: PokeColor.darker(PokeColor.typeColor(modelData))
                                        }
                                    }
                                    Text {
                                        anchors.centerIn: parent
                                        text:           modelData
                                        font.family:    evolveMenu.bodyFont
                                        font.bold:      true
                                        font.pixelSize: evolveMenu.fontSizeMd - 2
                                        color:          "#ffffff"
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
