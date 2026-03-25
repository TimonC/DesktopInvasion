import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../Style/PokeColor.js" as PokeColor

Item {
    id: root

    property var    pokeData: null
    property int    selectedEvolutionId: -1
    property var    selectedEvolution: null
    property string viewState: "grid"
    property real   confirmationColumnRatio: 0.5
    property string headerText: "EVOLVE"

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

    signal evolutionSelected(int boxId, int slot, var evolveData, string nickName)
    signal returnClicked()

    property var confirmConnection: null
    property var cancelConnection: null

    function evolutionList() {
        return pokeData ? pokeData.evolvesList : []
    }

    function showComparison(baseData, evolveData) {
        baseSprite.frameX = (baseData.isBig ? 64 : root.frameW) * 4
        baseSprite.frameY = (baseData.rowId || 0) * (baseData.isBig ? 64 : root.frameH)
        baseName.text = baseData.pokeName
        baseTypes.model = [baseData.type1, baseData.type2].filter(function(t) {
            return t && t !== "None" && t !== "" && t !== "Null" && t !== "null"
        })

        evoSprite.frameX = (evolveData.isBig ? 64 : root.frameW) * 4
        evoSprite.frameY = (evolveData.rowId || 0) * (evolveData.isBig ? 64 : root.frameH)
        evoName.text = evolveData.name || ""
        evoTypes.model = [evolveData.type1, evolveData.type2].filter(function(t) {
            return t && t !== "None" && t !== "" && t !== "Null" && t !== "null"
        })
    }

    function areYouSure(box, slot, modelData) {
        root.selectedEvolution = modelData
        root.viewState = "confirmation"
        root.headerText = "ARE YOU SURE?"
        subheader.visible = true
        showComparison(root.pokeData, modelData)

        if (root.confirmConnection) confirmBtn.clicked.disconnect(root.confirmConnection)
        if (root.cancelConnection) cancelBtn.clicked.disconnect(root.cancelConnection)

        root.confirmConnection = function() {
            var name = root.pokeData.name == root.pokeData.pokeName? modelData.name : root.pokeData.name
            root.evolutionSelected(box, slot, modelData, name)
            root.returnClicked()
        }
        confirmBtn.clicked.connect(root.confirmConnection)

        root.cancelConnection = function() {
            root.viewState = "grid"
            root.selectedEvolution = null
            root.headerText = "EVOLVE"
            subheader.visible = false
        }
        cancelBtn.clicked.connect(root.cancelConnection)
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
                id: evolveHeader
                text: root.headerText
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

        Text {
            id: subheader
            width: parent.width
            text: "Evolutions can't be undone!"
            font.family: root.bodyFont
            font.pixelSize: root.fontSizeMd
            color: root.colorSubtext
            horizontalAlignment: Text.AlignHCenter
            visible: false
        }

        Rectangle {
            width: parent.width
            height: pad
            color: "transparent"
        }

        Column {
            id: confirmationColumn
            width: parent.width
            spacing: pad * 2
            visible: root.viewState === "confirmation"

            Row {
                id: comparisonRow
                spacing: pad / 2
                anchors.horizontalCenter: parent.horizontalCenter

                Item {
                    width: (root.pokeData && root.pokeData.isBig ? 64 : root.frameW) * root.spriteScale
                    height: (root.pokeData && root.pokeData.isBig ? 64 : root.frameH) * root.spriteScale
                    Column {
                        anchors.centerIn: parent
                        spacing: 12
                        Text {
                            id: baseName
                            width: parent.width
                            text: ""
                            font.family: root.mainFont
                            font.pixelSize: root.fontSizeMd
                            color: root.colorText
                            horizontalAlignment: Text.AlignHCenter
                            elide: Text.ElideRight
                        }
                        Row {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: root.typePillSpacing
                            Repeater {
                                id: baseTypes
                                delegate: Rectangle {
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
                        Item {
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: (root.pokeData && root.pokeData.isBig ? 64 : root.frameW) * root.spriteScale
                            height: (root.pokeData && root.pokeData.isBig ? 64 : root.frameH) * root.spriteScale
                            AnimatedSprite {
                                id: baseSprite
                                width: parent.width
                                height: parent.height
                                running: true
                                source: (root.pokeData && root.pokeData.isBig) ? root.spriteSheetBig : root.spriteSheetNormal
                                frameWidth: (root.pokeData && root.pokeData.isBig) ? 64 : root.frameW
                                frameHeight: (root.pokeData && root.pokeData.isBig) ? 64 : root.frameH
                                frameCount: 2
                                frameRate: 4
                                interpolate: false
                                smooth: false
                                antialiasing: false
                            }
                        }
                    }
                }

                Text {
                    text: "→"
                    font.family: root.mainFont
                    font.pixelSize: root.fontSizeLg
                    color: "white"
                    anchors.verticalCenter: parent.verticalCenter
                }

                Item {
                    width: (root.selectedEvolution && root.selectedEvolution.isBig ? 64 : root.frameW) * root.spriteScale
                    height: (root.selectedEvolution && root.selectedEvolution.isBig ? 64 : root.frameH) * root.spriteScale
                    Column {
                        anchors.centerIn: parent
                        spacing: 12
                        Text {
                            id: evoName
                            width: parent.width
                            text: ""
                            font.family: root.mainFont
                            font.pixelSize: root.fontSizeMd
                            color: root.colorText
                            horizontalAlignment: Text.AlignHCenter
                            elide: Text.ElideRight
                        }
                        Row {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: root.typePillSpacing
                            Repeater {
                                id: evoTypes
                                delegate: Rectangle {
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
                        Item {
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: (root.selectedEvolution && root.selectedEvolution.isBig ? 64 : root.frameW) * root.spriteScale
                            height: (root.selectedEvolution && root.selectedEvolution.isBig ? 64 : root.frameH) * root.spriteScale
                            AnimatedSprite {
                                id: evoSprite
                                width: parent.width
                                height: parent.height
                                running: true
                                source: (root.selectedEvolution && root.selectedEvolution.isBig) ? root.spriteSheetBig : root.spriteSheetNormal
                                frameWidth: (root.selectedEvolution && root.selectedEvolution.isBig) ? 64 : root.frameW
                                frameHeight: (root.selectedEvolution && root.selectedEvolution.isBig) ? 64 : root.frameH
                                frameCount: 2
                                frameRate: 4
                                interpolate: false
                                smooth: false
                                antialiasing: false
                            }
                        }
                    }
                }
            }

            Row {
                id: areYouSureRow
                spacing: pad
                visible: root.viewState === "confirmation"
                anchors.horizontalCenter: parent.horizontalCenter
                PcButton {
                    id: confirmBtn
                    width: 48 * 4
                    label: "CONFIRM"
                    btnColor: "#e67a00"
                }
                PcButton {
                    id: cancelBtn
                    width: 48 * 4
                    label: "CANCEL"
                }
            }
        }

        Grid {
            id: evolveOptionsGrid
            width: parent.width
            spacing: cardSpacing
            columns: Math.max(1, Math.floor(parent.width / (cardWidth + cardSpacing)))
            visible: root.viewState === "grid"

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
                        border.width:  2
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
                            root.areYouSure(pokeData.box, pokeData.slot, modelData)
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
                                    id: evoSpriteCard
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
            visible: evolutionList().length === 0 && root.viewState === "grid"
            text: "No evolutions available"
            font.family: root.bodyFont
            font.pixelSize: root.fontSizeMd
            color: root.colorSubtext
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }
}
