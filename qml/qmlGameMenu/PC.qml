import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    anchors.fill: parent

    // --- Visual config ---
    property color partyBackground: "white"
    property color pcBackground:    "green"
    property color arrowColor:      "#5294e2"
    property int   slotWidth:       80
    property int   slotHeight:      60
    readonly property int partyRows:    2
    readonly property int partyColumns: 3
    readonly property int pcRows:       4
    readonly property int pcColumns:    4
    readonly property int maxBoxes:     99

    // --- State ---
    property int currentBoxIndex: 0
    property var partyMap: ({})   // slot -> iconId
    property var boxes:    ({})   // boxIndex -> array[16] of iconId|undefined

    // Replaced by C++ calls to loadParty/loadBox/showBox on menu open
    Component.onCompleted: {
        loadBox(0, [])
        showBox(0)
    }

    // --- Signals ---
    signal preloadBoxRequested(int boxIndex)

    // --- Public API ---

    function loadParty(dataList) {
        var map = {}
        for (var i = 0; i < dataList.length; i++)
            map[dataList[i].slot] = dataList[i].iconId
        partyMap = map
    }

    function loadBox(boxIndex, dataList) {
        var arr = new Array(pcRows * pcColumns)
        for (var i = 0; i < dataList.length; i++)
            arr[dataList[i].slot] = dataList[i].iconId
        var updated = Object.assign({}, boxes)
        updated[boxIndex] = arr
        boxes = updated
    }

    function showBox(boxIndex) {
        currentBoxIndex = boxIndex
        _requestAdjacentPreloads(boxIndex)
    }

    // --- Internal ---

    function _requestAdjacentPreloads(index) {
        var left  = (index - 1 + maxBoxes) % maxBoxes
        var right = (index + 1) % maxBoxes
        if (!boxes.hasOwnProperty(left))  preloadBoxRequested(left)
        if (!boxes.hasOwnProperty(right)) preloadBoxRequested(right)
    }

    function _slideLeft() {
        var next = (currentBoxIndex - 1 + maxBoxes) % maxBoxes
        if (!boxes.hasOwnProperty(next)) preloadBoxRequested(next)
        currentBoxIndex = next
        _requestAdjacentPreloads(next)
    }

    function _slideRight() {
        var next = (currentBoxIndex + 1) % maxBoxes
        if (!boxes.hasOwnProperty(next)) preloadBoxRequested(next)
        currentBoxIndex = next
        _requestAdjacentPreloads(next)
    }

    // --- Layout ---

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Party
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: partyRows * slotHeight

            Item {
                anchors.centerIn: parent
                width:  partyColumns * slotWidth
                height: partyRows    * slotHeight

                Rectangle {
                    anchors.fill: parent
                    color: root.partyBackground
                }

                Grid {
                    rows:         root.partyRows
                    columns:      root.partyColumns
                    rowSpacing:   0
                    columnSpacing: 0

                    Repeater {
                        model: root.partyRows * root.partyColumns
                        PokemonSlot {
                            iconVisible: root.partyMap[index] !== undefined
                            frameIndex:  root.partyMap[index] || 0
                        }
                    }
                }
            }
        }

        // PC + arrows
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ArrowButton {
                anchors.left:           parent.left
                anchors.verticalCenter: parent.verticalCenter
                direction: "left"
                onClicked: root._slideLeft()
            }

            Item {
                anchors.centerIn: parent
                width:  pcColumns * slotWidth
                height: pcRows    * slotHeight

                Rectangle {
                    anchors.fill: parent
                    color: root.pcBackground
                }

                Grid {
                    rows:         root.pcRows
                    columns:      root.pcColumns
                    rowSpacing:   0
                    columnSpacing: 0

                    Repeater {
                        model: root.pcRows * root.pcColumns
                        PokemonSlot {
                            property var currentBox: root.boxes[root.currentBoxIndex]
                            iconVisible: currentBox !== undefined && currentBox[index] !== undefined
                            frameIndex:  (currentBox !== undefined && currentBox[index] !== undefined)
                                         ? currentBox[index] : 0
                        }
                    }
                }
            }

            ArrowButton {
                anchors.right:          parent.right
                anchors.verticalCenter: parent.verticalCenter
                direction: "right"
                onClicked: root._slideRight()
            }
        }
    }

    // --- Components ---

    component PokemonSlot: Rectangle {
        width:  root.slotWidth
        height: root.slotHeight
        property bool iconVisible: false
        property int  frameIndex:  0
        color: (hoverArea.containsMouse && iconVisible) ? "blue" : "transparent"

        Image {
            anchors.fill: parent
            visible: parent.iconVisible
            source:  "qrc:/assets/HGSS/reordered_icons.png"
            readonly property int spriteWidth:  40
            readonly property int spriteHeight: 30
            sourceClipRect: Qt.rect(0, parent.frameIndex * spriteHeight, spriteWidth, spriteHeight)
            smooth:       false
            antialiasing: false
        }

        MouseArea {
            id:           hoverArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: undefined
        }
    }

    component ArrowButton: Rectangle {
        width:  28
        height: 48
        radius: 4
        property string direction: "right"
        property bool   active:    true      // "enabled" is a reserved QML property
        signal clicked()

        color: active
               ? (press.pressed ? Qt.darker(root.arrowColor, 1.3) : root.arrowColor)
               : "#444"

        Text {
            anchors.centerIn: parent
            text:       parent.direction === "left" ? "◀" : "▶"
            color:      parent.active ? "white" : "#888"
            font.pixelSize: 14
        }

        MouseArea {
            id:          press
            anchors.fill: parent
            enabled:     parent.active
            onClicked:   parent.clicked()
        }
    }
}
