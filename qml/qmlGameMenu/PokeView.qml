import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../Style/PokeColor.js" as PokeColor


Item {
    id: pokeView
    anchors.fill: parent

    property var pokeData: null

    property string spriteSheet:  "qrc:/assets/HGSS/reordered_sprites.png"
    property int    frameWidth:   32
    property int    frameHeight:  32
    property real   scaleFactor:  6
    property int    rowId:        0


    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        // TOP: sprite + identity
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: parent.height * 0.30
            spacing: 8

            AnimatedSprite {
                id: sprite
                Layout.preferredWidth:  frameWidth  * scaleFactor
                Layout.preferredHeight: frameHeight * scaleFactor
                Layout.alignment: Qt.AlignVCenter
                running:     true
                source:      pokeView.spriteSheet
                frameWidth:  pokeView.frameWidth
                frameHeight: pokeView.frameHeight
                frameCount:  2
                frameRate:   4
                interpolate: false
                smooth:      false
                antialiasing: false
                frameX:      pokeView.frameWidth * 4
                frameY:      pokeView.rowId * pokeView.frameHeight
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                spacing: 4

                Text {
                    text: pokeData ? pokeData.name : ""
                    font.family: root.p2pFont
                    font.pixelSize: 14
                    color: root.textColor
                }
                Text {
                    text: pokeData ? pokeData.pokeName : ""
                    font.family: root.dotGothicFont
                    font.pixelSize: 12
                    color: "#aaaaaa"
                }
                Text {
                    text: pokeData ? "Lv. " + pokeData.level : ""
                    font.family: root.dotGothicFont
                    font.pixelSize: 12
                    color: root.textColor
                }
                RowLayout {
                    spacing: 4
                    Repeater {
                        model: pokeData ? [pokeData.type1, pokeData.type2].filter(t => t && t !== "None") : []
                        Rectangle {
                            width:  typeLbl.implicitWidth + 10
                            height: 18
                            radius: 4
                            color:  PokeColor.typeColor(modelData) ?? "#888"
                            Text {
                                id: typeLbl
                                anchors.centerIn: parent
                                text: modelData
                                font.family: root.dotGothicFont
                                font.pixelSize: 10
                                color: "#ffffff"
                            }
                        }
                    }
                }
            }
        }

        // STATS
        Grid {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: 12
            rowSpacing: 2

            property var statNames: ["HP","Atk","Def","SpA","SpD","Spe"]

            Repeater {
                model: pokeData ? pokeData.stats : []
                RowLayout {
                    spacing: 4
                    Text {
                        text: parent.parent.statNames[index] + ":"
                        font.family: root.dotGothicFont
                        font.pixelSize: 11
                        color: "#aaaaaa"
                        width: 30
                    }
                    Text {
                        text: modelData
                        font.family: root.dotGothicFont
                        font.pixelSize: 11
                        color: root.textColor
                    }
                }
            }
        }

        // FLAVOR TEXT
        Text {
            Layout.fillWidth: true
            text: pokeData ? pokeData.flavorText : ""
            font.family: root.dotGothicFont
            font.pixelSize: 10
            color: "#cccccc"
            wrapMode: Text.WordWrap
        }

        // MOVES
        Repeater {
            model: pokeData ? pokeData.moves : []
            Rectangle {
                Layout.fillWidth: true
                height: moveCol.implicitHeight + 8
                color: "#3a3a3a"
                radius: 4

                ColumnLayout {
                    id: moveCol
                    anchors { left: parent.left; right: parent.right; top: parent.top; margins: 4 }
                    spacing: 2

                    RowLayout {
                        spacing: 6
                        Text {
                            text: modelData.name
                            font.family: root.dotGothicFont
                            font.pixelSize: 11
                            color: root.textColor
                        }
                        Rectangle {
                            width:  movTypeLbl.implicitWidth + 8
                            height: 14
                            radius: 3
                            color:  PokeColor.typeColor(modelData.type) ?? "#888"
                            Text {
                                id: movTypeLbl
                                anchors.centerIn: parent
                                text: modelData.type
                                font.family: root.dotGothicFont
                                font.pixelSize: 9
                                color: "#ffffff"
                            }
                        }
                        Text {
                            text: "Pow:" + modelData.power + "  Acc:" + modelData.accuracy
                            font.family: root.dotGothicFont
                            font.pixelSize: 10
                            color: "#aaaaaa"
                        }
                    }
                    Text {
                        Layout.fillWidth: true
                        text: modelData.flavor
                        font.family: root.dotGothicFont
                        font.pixelSize: 9
                        color: "#999999"
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
