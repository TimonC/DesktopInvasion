import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    width:  864
    height: 546
    color: backgroundColor

    property int    margin:              0
    property color  backgroundColor:     "#2b2b2b"
    property color  buttonColor:         "#3c3c3c"
    property color  buttonSelectedColor: "#5294e2"
    property color  textColor:           "#ffffff"
    property int    headerFontSize:      24
    property int    bodyFontSize:        18
    property string p2pFont:             "Press Start 2P"
    property string dotGothicFont:       "DotGothic16"

    property string currentMenu:         "Party"
    property bool   showDebugOutlines:   true
    property color  debugOutlineColor:   "#FFD700"
    property double leftSideWidthRatio:  0.5
    readonly property int pcRows:       4
    readonly property int pcColumns:    4
    property var partyPokes: ({})
    property var boxPokes: ({})
    property alias displayName:  rightText.text

    MouseArea{
        onClicked: {
            console.log("global click")
            if(pc.inSwapMode) pc.toggleSwapMode();
        }
        cursorShape:  undefined
        anchors.fill: parent
    }

    // C++ (Game) -> QML: push party and box data into PC
    Connections {
        target: menuBridge

        function onPartyDataReady(data) {
            console.log("[Menu] party arrived, count:", data.length)
            pc.loadParty(data)
            var map = {}
            for (var i = 0; i < data.length; i++)
                map[data[i].slot] = data[i]
            partyPokes = map
            pc._display([-1,0])
        }

        function onBoxDataReady(boxIndex, data) {
            console.log("[Menu] box", boxIndex, "arrived, count:", data.length)
            pc.loadBox(boxIndex, data)
            var arr = new Array(pcRows * pcColumns)
            for (var i = 0; i < data.length; i++)
                arr[data[i].slot] = data[i]
            var updated = Object.assign({}, boxPokes)
            updated[boxIndex] = arr
            boxPokes = updated
        }

        function onShowBoxRequested(boxIndex) {
            console.log("[Menu] showBox", boxIndex)
            pc.showBox(boxIndex)
        }
    }

    // QML -> C++ (Game): PC wants a box preloaded
    Connections {
        target: pc
        function onPreloadBoxRequested(boxIndex) { menuBridge.preloadBoxRequested(boxIndex) }
        function onSwapRequested(posx, posy) {
            // First, do the C++ swap
            menuBridge.swapRequested(posx[0], posx[1], posy[0], posy[1])

            // Then update QML by creating new objects
            var newParty = Object.assign({}, partyPokes)
            var newBoxes = Object.assign({}, boxPokes)

            // Get the values
            var x = posx[0] == -1 ? partyPokes[posx[1]] : boxPokes[posx[0]][posx[1]]
            var y = posy[0] == -1 ? partyPokes[posy[1]] : boxPokes[posy[0]][posy[1]]

            // Swap in the new objects
            if(posx[0] == -1) {
                newParty[posx[1]] = y
            } else {
                var arrX = newBoxes[posx[0]].slice()
                arrX[posx[1]] = y
                newBoxes[posx[0]] = arrX
            }

            if(posy[0] == -1) {
                newParty[posy[1]] = x
            } else {
                var arrY = newBoxes[posy[0]].slice()
                arrY[posy[1]] = x
                newBoxes[posy[0]] = arrY
            }

            // Assign back - this triggers the redraw!
            partyPokes = newParty
            boxPokes = newBoxes
        }
        function onDisplay(pcPos){
            if(pcPos[0]==-1 && partyPokes[pcPos[1]]){
                displayName = partyPokes[pcPos[1]].name
                rightPanel.rowId = partyPokes[pcPos[1]].rowId

                var isBig = partyPokes[pcPos[1]].isBig
                rightPanel.spriteSheet = isBig ? "qrc:/assets/HGSS/reordered_sprites_big.png" :  "qrc:/assets/HGSS/reordered_sprites.png"
                rightPanel.frameWidth = isBig ? 64 : 32
                rightPanel.frameHeight = isBig ? 64 : 32
                rightPanel.scaleFactor = isBig ? 4 : 6
            }else if(pcPos[0]>-1 && boxPokes[pcPos[0]][pcPos[1]]){
                displayName = boxPokes[pcPos[0]][pcPos[1]].name
                var isBig = boxPokes[pcPos[0]][pcPos[1]].isBig
                rightPanel.spriteSheet = isBig ? "qrc:/assets/HGSS/reordered_sprites_big.png" :  "qrc:/assets/HGSS/reordered_sprites.png"
                rightPanel.frameWidth = isBig ? 64 : 32
                rightPanel.frameHeight = isBig ? 64 : 32
                rightPanel.scaleFactor = isBig ? 4 : 6
                rightPanel.rowId = boxPokes[pcPos[0]][pcPos[1]].rowId
            }else{
                console.log("ERROR faulty display pos")
            }
        }

    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: root.margin
        spacing: 0

        // LEFT COLUMN
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * root.leftSideWidthRatio
            color: root.backgroundColor
            border.color: root.showDebugOutlines ? root.debugOutlineColor : "transparent"
            border.width: root.showDebugOutlines ? 2 : 0

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                Rectangle {
                    id: trainerSection
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * (2/7)
                    color: root.backgroundColor
                    border.color: root.showDebugOutlines ? root.debugOutlineColor : "transparent"
                    border.width: root.showDebugOutlines ? 2 : 0

                    Trainer {
                        anchors.fill: parent
                        textColor:   root.textColor
                        fontSize:    root.bodyFontSize
                        fontFamily:  root.p2pFont
                    }
                }

                Rectangle {
                    id: pcSection
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * (5/7)
                    color: root.backgroundColor
                    border.color: root.showDebugOutlines ? root.debugOutlineColor : "transparent"
                    border.width: root.showDebugOutlines ? 2 : 0

                    PC {
                        id: pc
                        anchors.fill: parent
                        fontFamily: root.dotGothicFont
                    }
                }
            }
        }

        // RIGHT COLUMN
        Rectangle {
            id: rightPanel
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: root.backgroundColor
            border.color: root.showDebugOutlines ? root.debugOutlineColor : "transparent"
            border.width: root.showDebugOutlines ? 2 : 0

            property real scaleFactor: 6
            property int frameWidth: 32
            property int frameHeight: 32
            property int frameCount: 2
            property int frameRate: 4
            property int rowId: 0
            property string spriteSheet: "qrc:/assets/HGSS/reordered_sprites.png"
            PokeView {}

            AnimatedSprite {
                id: sprite
                anchors.centerIn: parent
                width: rightPanel.frameWidth * rightPanel.scaleFactor
                height: rightPanel.frameHeight * rightPanel.scaleFactor
                running: true
                source: rightPanel.spriteSheet
                frameWidth: rightPanel.frameWidth
                frameHeight: rightPanel.frameHeight
                frameCount: rightPanel.frameCount
                frameRate: rightPanel.frameRate
                currentFrame: Math.random() < 0.5 ? 0 : 1
                interpolate: false
                smooth: false
                antialiasing: false
                frameX: rightPanel.frameWidth * 4
                frameY: rightPanel.rowId * rightPanel.frameHeight
            }
            Text {
                id: rightText
                anchors.centerIn: parent
                text: "Right Panel"
                font.family: root.p2pFont
                color: root.textColor
                font.pixelSize: 16
                visible: false
            }
        }
    }
}
