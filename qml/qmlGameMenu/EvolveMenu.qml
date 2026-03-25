import QtQuick 2.15
import "../Style/PokeColor.js" as PokeColor

Item {
    id: root

    property var    pokeData: null
    property int    selectedEvolutionId: -1

    property string mainFont:           "Press Start 2P"
    property string bodyFont:           "DotGothic16"
    property int    fontSizeLg:         22
    property int    fontSizeMd:         18
    property int    fontSizeSm:         16

    property color colorText:           "#ffffff"
    property color colorSubtext:        "#aaaaaa"
    property color colorAccent:         "#5294e2"
    property color colorMoveCard:       "#383838"
    property color colorDivider:        "#3d3d3d"

    property string spriteSheetNormal:  "qrc:/assets/HGSS/reordered_sprites.png"
    property string spriteSheetBig:     "qrc:/assets/HGSS/reordered_sprites_big.png"
    property int    frameW:             32
    property int    frameH:             32
    property int    spriteScale:        6

    property int    typePillW:          88
    property int    typePillH:          fontSizeMd + 8
    property int    typePillSpacing:    4

    readonly property int pad:          30
    readonly property int cardSpacing:  20
    readonly property int cardWidth:    260
    readonly property int cardHeight:   320

    signal evolutionSelected(int boxId, int slot, var evolveData)
    signal returnClicked()

    function evolutionList() {
        return pokeData ? pokeData.evolvesList : []
    }

    Column {
        anchors.fill: parent
        anchors.margins: pad
        spacing: pad

        Row {
            width: parent.width
            spacing: pad
            PcButton {
                width: 48 * 4
                label: "← BACK"
                onClicked: root.returnClicked()
            }
            Text {
                text: "EVOLVE"
                font.family: root.mainFont
                font.pixelSize: root.fontSizeLg
                color: root.colorText
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Rectangle {
            width: parent.width
            height: 1
            color: root.colorDivider
        }

        Grid {
            width: parent.width
            spacing: cardSpacing
            columns: Math.max(1, Math.floor(parent.width / (cardWidth + cardSpacing)))

            Repeater {
                model: root.evolutionList()

                Item {
                    width: cardWidth
                    height: cardHeight

                    Rectangle {
                        anchors.fill: parent
                        color: root.colorMoveCard
                        radius: 12
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius: 12
                        color: "transparent"
                        border.width: (mouseArea.containsMouse || root.selectedEvolutionId === (modelData.pokedex_id || -1)) ? 2 : 0
                        border.color: Qt.rgba(root.colorAccent.r, root.colorAccent.g, root.colorAccent.b,
                                              (mouseArea.containsMouse ? 0.8 : 0.6))
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius: 12
                        color: mouseArea.containsMouse
                               ? Qt.rgba(root.colorAccent.r, root.colorAccent.g, root.colorAccent.b, 0.2)
                               : "transparent"
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: undefined
                        onClicked: {
                            root.selectedEvolutionId = modelData.pokedex_id
                            root.evolutionSelected(pokeData.box, pokeData.slot, modelData)
                            root.returnClicked()
                        }
                    }

                    Item {
                        anchors.fill: parent
                        anchors.margins: 12

                        Column {
                            anchors.centerIn: parent
                            width: parent.width
                            spacing: 8

                            Text {
                                width: parent.width
                                text: modelData.name || ""
                                font.family: root.mainFont
                                font.pixelSize: root.fontSizeMd
                                color: root.colorText
                                horizontalAlignment: Text.AlignHCenter
                                elide: Text.ElideRight
                            }

                            Item {
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: (modelData.isBig ? 64 : root.frameW) * root.spriteScale
                                height: (modelData.isBig ? 64 : root.frameH) * root.spriteScale

                                AnimatedSprite {
                                    id: evoSprite
                                    width: parent.width
                                    height: parent.height
                                    running: true
                                    source: modelData.isBig ? root.spriteSheetBig : root.spriteSheetNormal
                                    frameWidth: modelData.isBig ? 64 : root.frameW
                                    frameHeight: modelData.isBig ? 64 : root.frameH
                                    frameCount: 2
                                    frameRate: 4
                                    interpolate: false
                                    smooth: false
                                    antialiasing: false
                                    frameX: (modelData.isBig ? 64 : root.frameW) * 4
                                    frameY: (modelData.rowId || 0) * (modelData.isBig ? 64 : root.frameH)
                                }
                            }

                            Row {
                                anchors.horizontalCenter: parent.horizontalCenter
                                spacing: root.typePillSpacing

                                Repeater {
                                    model: [modelData.type1, modelData.type2].filter(function(t) {
                                        return t && t !== "None" && t !== "" && t !== "Null" && t !== "null"
                                    })

                                    Rectangle {
                                        width: root.typePillW
                                        height: root.typePillH
                                        radius: 4
                                        gradient: Gradient {
                                            GradientStop { position: 0.0; color: PokeColor.lighter(PokeColor.typeColor(modelData)) }
                                            GradientStop { position: 1.0; color: PokeColor.darker(PokeColor.typeColor(modelData)) }
                                        }
                                        Text {
                                            anchors.centerIn: parent
                                            text: modelData
                                            font.family: root.bodyFont
                                            font.bold: true
                                            font.pixelSize: root.fontSizeMd
                                            color: "#ffffff"
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        Text {
            width: parent.width
            visible: evolutionList().length === 0
            text: "No evolutions available"
            font.family: root.bodyFont
            font.pixelSize: root.fontSizeMd
            color: root.colorSubtext
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }
}
