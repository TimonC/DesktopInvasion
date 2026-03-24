import QtQuick 2.15

Item {
    id: evolveMenu

    property var    pokeData:    null
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
    property int gap:    5

    signal returnClicked()

    Column {
        anchors { top: parent.top; left: parent.left; right: parent.right }
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

        Column {
            width:   parent.width
            spacing: evolveMenu.gap

            Repeater {
                model: evolveMenu.pokeData ? evolveMenu.pokeData.evolves : []

                Row {
                    width:   parent.width
                    spacing: 16

                    Text {
                        text:           "Lv. " + modelData.lvl
                        font.family:    evolveMenu.bodyFont
                        font.pixelSize: evolveMenu.fontSizeMd
                        color:          evolveMenu.colorText
                    }

                    Text {
                        text:           "Move ID: " + modelData.moveid
                        font.family:    evolveMenu.bodyFont
                        font.pixelSize: evolveMenu.fontSizeMd
                        color:          evolveMenu.colorSubtext
                    }
                }
            }
        }
    }
}

