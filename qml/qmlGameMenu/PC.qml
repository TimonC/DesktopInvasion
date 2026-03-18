import QtQuick 2.15
import QtQuick.Layouts 1.15
Item{
    id: root
    anchors.fill: parent
    property color partyBackGround: "white"
    property color pcBackGround: "green"

    property int slotWidth: 80
    property int slotHeight: 60
    property int partyRows: 2
    property int partyColumns: 3
    property int pcRows: 4
    property int pcColumns: 4

    property var partyData: [
        {iconId: 0, slot: 0},
        {iconId: 1, slot: 1},
        {iconId: 2, slot: 2}
    ]

    property var pcData: [
        {iconId: 492, row: 0, col: 0},
        {iconId: 150, row: 3, col: 3}
    ]

    property var partyMap: ({})
    property var pcMap: ({})
    Component.onCompleted: {
        fillParty(partyData)
        fillPC(pcData)
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        Item{
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 1
            Item{
                anchors.centerIn: parent
                width: party.width
                height: party.height
                Rectangle{
                    anchors.fill: parent
                    color: root.partyBackGround
                }
                Grid{
                    id: party
                    width: root.partyColumns * root.slotWidth
                    height: root.partyRows * root.slotHeight
                    rows: root.partyRows
                    columns: root.partyColumns
                    rowSpacing: 0
                    columnSpacing: 0
                    Repeater {
                        model: root.partyRows * root.partyColumns
                        PokemonSlot {
                            iconVisible: partyMap[index] !== undefined
                            frameIndex: partyMap[index] || 0
                        }
                    }
                }
            }
        }

        Item{
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 2
            Item{
                anchors.centerIn: parent
                width: pc.width
                height: pc.height
                Rectangle{
                    anchors.fill: parent
                    color: root.pcBackGround
                }
                Grid{
                    id: pc
                    width: root.pcColumns * root.slotWidth
                    height: root.pcRows * root.slotHeight
                    rows: root.pcRows
                    columns: root.pcColumns
                    rowSpacing: 0
                    columnSpacing: 0
                    Repeater {
                        model: root.pcRows * root.pcColumns
                        PokemonSlot {
                            iconVisible: pcMap[index] !== undefined
                            frameIndex: pcMap[index] || 0
                        }
                    }
                }
            }
        }
    }

    function fillParty(dataList) {
        partyData = dataList
        var map = {}
        for(var i = 0; i < dataList.length; i++) {
            map[dataList[i].slot] = dataList[i].iconId
        }
        partyMap = map
    }

    function fillPC(dataList) {
        pcData = dataList
        var map = {}
        for(var i = 0; i < dataList.length; i++) {
            var key = dataList[i].row * root.pcRows + dataList[i].col
            map[key] = dataList[i].iconId
        }
        pcMap = map
    }
    component PokemonSlot: Rectangle {
        width: root.slotWidth
        height: root.slotHeight
        property bool iconVisible: false
        property int frameIndex: 0
        color: (hoverArea.containsMouse && iconVisible) ? "blue" : "transparent"
        Image {
            anchors.fill: parent
            visible: iconVisible
            source: "qrc:/assets/HGSS/reordered_icons.png"
            readonly property int spriteWidth: 40
            readonly property int spriteHeight: 30
            sourceClipRect: Qt.rect(0, frameIndex * spriteHeight, spriteWidth, spriteHeight)
            smooth: false
            antialiasing: false
        }
        MouseArea {
            id: hoverArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: undefined
        }
    }
}
