import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../Style/PokeColor.js" as PokeColor

Item {
    id: pokeView
    anchors.fill: parent

    property var    pokeData:    null
    property string spriteSheet: "qrc:/assets/HGSS/reordered_sprites.png"
    property int    frameWidth:  32
    property int    frameHeight: 32
    property real   scaleFactor: 6
    property int    rowId:       0

    // ── Layout constants ──────────────────────────────────────────────────────
    property string mainFont:   root.p2pFont
    property string bodyFont:   root.dotGothicFont
    property int    fontSizeLg: 22
    property int    fontSizeMd: 18
    property int    fontSizeSm: 14
    property int    margin:     14
    property int    spacing:    6
    property int    maxSprite:  64 * 4

    property int typeWidth: 64

    property color  dividerColor: "#3d3d3d"

    // Section height ratios
    property real ratioTop:    0.32
    property real ratioFlavor: 0.16
    property real ratioMoves:  0.52

    readonly property real innerH: height - margin * 2
    readonly property real halfW:  (width - margin * 2) / 2

    // ── TOP ROW ───────────────────────────────────────────────────────────────
    Item {
        id: topRow
        anchors { top: parent.top; left: parent.left; right: parent.right; margins: margin }
        height: innerH * ratioTop

        // Sprite half
        Item {
            anchors { top: parent.top; bottom: parent.bottom; left: parent.left }
            width: halfW

            AnimatedSprite {
                anchors.centerIn: parent
                width:       pokeView.frameWidth  * pokeView.scaleFactor
                height:      pokeView.frameHeight * pokeView.scaleFactor
                running:     true
                source:      pokeView.spriteSheet
                frameWidth:  pokeView.frameWidth
                frameHeight: pokeView.frameHeight
                frameCount:  2; frameRate: 4
                interpolate: false; smooth: false; antialiasing: false
                frameX: pokeView.frameWidth * 4
                frameY: pokeView.rowId * pokeView.frameHeight
            }
        }

        // Info half
        Item {
            anchors { top: parent.top; bottom: parent.bottom; right: parent.right }
            width: halfW

            Column {
                anchors.centerIn: parent
                spacing:          pokeView.spacing
                width: halfW

                Text {
                    text: pokeData ? pokeData.name : ""
                    font.family: mainFont; font.pixelSize: fontSizeLg
                    color: root.textColor
                }
                Text {
                    text: pokeData ? pokeData.pokeName : ""
                    font.family: bodyFont; font.pixelSize: fontSizeMd
                    color: "#aaaaaa"
                }
                Row {
                    spacing: 8
                    Text {
                        text: pokeData ? "Lv. " + pokeData.level : ""
                        font.family: bodyFont; font.pixelSize: fontSizeMd
                        color: root.textColor
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text {
                        text: pokeData ? pokeData.nature : ""
                        font.family: bodyFont; font.pixelSize: fontSizeSm
                        color: "#aaaaaa"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
                Row {
                    spacing: 6
                    Repeater {
                        model: pokeData ? [pokeData.type1, pokeData.type2].filter(t => t && t !== "None") : []
                        Rectangle {
                            width: typeWidth
                            height: fontSizeMd + 6
                            radius: 4
                            color: PokeColor.typeColor(modelData)
                            Text {
                                id: lbl; anchors.centerIn: parent
                                text: PokeColor.typeColor(modelData)=="transparent" ? "" : modelData
                                font.family: bodyFont; font.pixelSize: fontSizeSm
                                color: "#ffffff"
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
                                color: "#aaaaaa"; width: 36
                            }
                            Text {
                                text: modelData
                                font.family: bodyFont; font.pixelSize: fontSizeSm
                                color: root.textColor
                            }
                        }
                    }
                }
            }
        }
    }

    // Section divider – subtle, only between sections
    Rectangle {
        id: divider1
        anchors { top: topRow.bottom; left: parent.left; right: parent.right; leftMargin: margin; rightMargin: margin }
        height: 1; color: pokeView.dividerColor
    }

    // ── FLAVOR TEXT ───────────────────────────────────────────────────────────
    Item {
        id: flavorRow
        anchors { top: divider1.bottom; left: parent.left; right: parent.right; leftMargin: margin; rightMargin: margin }
        height: innerH * ratioFlavor

        Text {
            anchors.centerIn: parent
            width: parent.width
            text: pokeData ? pokeData.flavorText : ""
            font.family: bodyFont; font.pixelSize: fontSizeMd
            color: "#cccccc"; wrapMode: Text.WordWrap; elide: Text.ElideRight
            horizontalAlignment: Text.AlignJustify
        }
    }

    Rectangle {
        id: divider2
        anchors { top: flavorRow.bottom; left: parent.left; right: parent.right; leftMargin: margin; rightMargin: margin }
        height: 1; color: pokeView.dividerColor
    }

    // ── MOVES ─────────────────────────────────────────────────────────────────
    Item {
        anchors {
            top: divider2.bottom; left: parent.left; right: parent.right; bottom: parent.bottom
            leftMargin: margin; rightMargin: margin; bottomMargin: margin
        }

        Column {
            anchors.centerIn: parent
            width:   parent.width
            spacing: 4

            Repeater {
                model: pokeData ? pokeData.moves : []
                Rectangle {
                    width:  parent.width
                    height: moveInner.implicitHeight + 8
                    color:  "#383838"; radius: 5

                    Column {
                        id: moveInner
                        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 6 }
                        spacing: 2

                        Row {
                            spacing: 8
                            Rectangle {
                                width: typeWidth
                                height: fontSizeMd + 4
                                radius: 3
                                color: PokeColor.typeColor(modelData.type) ?? "#888"
                                anchors.verticalCenter: parent.verticalCenter
                                Text {
                                    id: pill; anchors.centerIn: parent
                                    text: modelData.type
                                    font.family: bodyFont; font.pixelSize: fontSizeSm
                                    color: "#ffffff"
                                }
                            }Text {
                                width: typeWidth*1.8
                                text: modelData.name
                                font.family: bodyFont; font.pixelSize: fontSizeMd
                                color: root.textColor
                            }
                            Text {
                                text: "Pow: " + modelData.power + "   Acc: " + modelData.accuracy
                                font.family: bodyFont; font.pixelSize: fontSizeSm*0.9
                                color: "#aaaaaa"; anchors.verticalCenter: parent.verticalCenter
                            }

                        }
                        Text {
                            height: fontSizeSm*3
                            width: parent.width
                            text:  modelData.flavor
                            font.family: bodyFont;
                            font.pixelSize: fontSizeSm
                            color: "#888888";
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }
        }
    }
}
