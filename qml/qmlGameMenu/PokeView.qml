import QtQuick 2.15
import "../Style/PokeColor.js" as PokeColor

Item {
    id: pokeView

    property var    pokeData:    null
    property string spriteSheet: "qrc:/assets/HGSS/reordered_sprites.png"
    property int    frameWidth:  32
    property int    frameHeight: 32
    property real   scaleFactor: 6
    property int    rowId:       0
    property int  jumpDistance:  24

    property string mainFont:   "Press Start 2P"
    property string bodyFont:   "DotGothic16"
    property int    fontSizeLg: 22
    property int    fontSizeMd: 18
    property int    fontSizeSm: 16

    property color colorText:          "#ffffff"
    property color colorSubtext:       "#aaaaaa"
    property color colorFaint:         "#cccccc"
    property color colorVeryFaint:     "#999999"
    property color colorDivider:       "#3d3d3d"
    property color colorMoveCard:      "#383838"
    property color colorTypePillText:  "#ffffff"

    property int  margin:        14
    property int  sectionGap:    8
    property int  rowSpacing:    6
    property int  moveCardPad:   10
    property int  moveCardGap:   8
    property int  typePillW:     96
    property int  typePillH:     fontSizeMd + 8
    property int  moveNameW:     typePillW * 3
    property int  movePillGap:   typePillW / 4

    property real ratioTop:    0.32
    property real ratioFlavor: 0.16
    property real ratioMoves:  0.52

    readonly property real innerH: height - margin * 2

    signal evolveBtnClicked(var pokeData)

    function spriteJump(){
        if (!sprite.isJumping) {
            sprite.isJumping = true
            console.log("Yippeee!")
            jumpAnim.start()
        }
    }

    Item {
        anchors.centerIn: parent
        width:  parent.width  - pokeView.margin * 2
        height: parent.height - pokeView.margin * 2

        Row {
            id: topRow
            anchors {
                top:   parent.top
                left:  parent.left
                right: parent.right
            }
            height: innerH * ratioTop
            spacing: pokeView.sectionGap*2

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

                        MouseArea {
                            anchors.fill: parent
                            cursorShape:  undefined
                            onClicked: pokeView.spriteJump()
                        }

                        SequentialAnimation {
                            id: jumpAnim
                            PropertyAnimation {
                                target: sprite; property: "y"
                                to: sprite.y - pokeView.jumpDistance
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

            Item {
                width:  parent.width / 2
                height: parent.height

                Column {
                    anchors.centerIn: parent
                    width:   parent.width
                    spacing: pokeView.sectionGap


                   Text {
                        text: pokeData ? pokeData.name : ""
                        font.family: mainFont
                        font.pixelSize: fontSizeLg
                        color: colorText
                        width: parent.width
                        elide: Text.ElideRight
                    }
                    Row {
                        width: parent.width*0.9
                        height: fontSizeLg
                        spacing: rowSpacing

                        Text {
                            width: parent.width*0.4
                            text: pokeData ? pokeData.pokeName : ""
                            font.family: bodyFont
                            font.pixelSize: fontSizeMd
                            color: colorSubtext
                            elide: Text.ElideRight
                        }
                        PcButton {
                            height: parent.height*1.8
                            width: parent.width*0.6
                            visible: pokeData && pokeData.evolves && pokeData.evolves.length > 0
                            enabled: pokeData && pokeData.evolves && pokeData.evolves.length > 0
                            label: "++EVOLVE++"
                            btnColor: "#e67a00"
                            fontSize: root.fontSizeSm
                            onClicked: pokeView.evolveBtnClicked(pokeData)
                        }
                    }
                    Column{
                        Row {
                            Text {
                                text: "Lv."
                                font.family: bodyFont; font.pixelSize: fontSizeMd
                                color: colorSubtext; anchors.verticalCenter: parent.verticalCenter
                            }
                            Rectangle{
                                width: rowSpacing/2
                                height:1
                                color: "transparent"
                            }
                            Text {
                                text: pokeData ? pokeData.level : ""
                                font.family: bodyFont; font.pixelSize: fontSizeMd
                                color: colorText; anchors.verticalCenter: parent.verticalCenter
                            }
                            Rectangle{
                                width: rowSpacing*2
                                height:1
                                color: "transparent"
                            }
                            Text {
                                text: pokeData ? pokeData.nature : ""
                                font.family: bodyFont; font.pixelSize: fontSizeSm
                                color: colorSubtext; anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                        Row {
                            Text {
                                text: "XP: "
                                font.family: bodyFont; font.pixelSize: fontSizeSm
                                color: colorSubtext;
                            }
                            Text {
                                text: (pokeData && pokeData.currentXP!==null) ?  pokeData.currentXP : ""
                                font.family: bodyFont; font.pixelSize: fontSizeSm
                                color: colorText
                            }
                            Text {
                                text: " / " + ((pokeData && pokeData.requiredXP!==null) ? pokeData.requiredXP : "")
                                font.family: bodyFont; font.pixelSize: fontSizeSm
                                color: colorSubtext
                            }
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
                                    font.family: bodyFont
                                    font.bold: true
                                    font.pixelSize: fontSizeMd
                                    color: colorTypePillText
                                }
                            }
                        }
                    }

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
                height: parent.height - moveCardGap * 5
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
                                                font.family: bodyFont
                                                font.bold:  true
                                                font.pixelSize: fontSizeMd
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
