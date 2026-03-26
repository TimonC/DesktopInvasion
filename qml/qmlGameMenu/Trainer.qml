import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: "transparent"

    property real spriteSizeDefault:         2.0
    property real battleSpeedDefault:        1.0
    property int  encounterLevelHighDefault: 5
    property int  encounterLevelLowDefault:  5
    property bool expShareDefault:           false

    readonly property real spriteSizeMin:         1.0
    readonly property real spriteSizeMax:         4.0
    readonly property real spriteSizeStep:        0.5

    readonly property real battleSpeedMin:        1.0
    readonly property real battleSpeedMax:        4.0
    readonly property real battleSpeedStep:       0.5

    readonly property int  encounterHighMin:      0
    readonly property int  encounterHighMax:      25
    readonly property int  encounterHighStep:     5

    readonly property int  encounterLowMin:       0
    readonly property int  encounterLowMax:       25
    readonly property int  encounterLowStep:      5

    property color  textColor:  "white"
    property color  labelColor: "#aaaaaa"
    property int    fontSizeLg: 22
    property int    fontSizeMd: 18
    property int    fontSizeSm: 16
    property int    row:        34
    property int    frameSize: 32
    property string p2pFont:       "Press Start 2P"
    property string dotGothicFont: "DotGothic16"

    property real iconScale: spriteSizeSlider.value

    property real spriteSize:         spriteSizeDefault
    property real battleSpeed:        battleSpeedDefault
    property int  encounterLevelHigh: encounterLevelHighDefault
    property int  encounterLevelLow:  encounterLevelLowDefault
    property bool expShare:           expShareDefault

    component DiscreteSlider : Item {
        id: discreteSlider
        required property string label
        required property real   from
        required property real   to
        required property real   stepSize
        required property real   initialValue

        property bool isNegative: false
        property alias value: mouseArea.value

        width: parent.width
        height: 30

        Row {
            width: parent.width
            height: parent.height
            spacing: 8

            Text {
                text: discreteSlider.label
                font.family: root.dotGothicFont
                font.pixelSize: root.fontSizeSm - 2
                color: root.textColor
                width: 110
                height: parent.height
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                font.bold: true
                opacity: 0.9
            }

            Item {
                id: sliderTrack
                width: parent.width - 110 - valueLabel.width - parent.spacing - 8
                height: parent.height

                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width
                    height: 4
                    radius: 2
                    color: "#333333"
                    border.color: "#666666"
                    border.width: 1
                }

                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    x: 0
                    width: mouseArea.visualPosition * parent.width
                    height: 4
                    radius: 2
                    color: "#7aa9e6"
                    border.color: "#aaccff"
                    border.width: 1
                }

                Repeater {
                    model: (discreteSlider.to - discreteSlider.from) / discreteSlider.stepSize + 1
                    Rectangle {
                        x: (index * discreteSlider.stepSize / (discreteSlider.to - discreteSlider.from)) * sliderTrack.width - width/2
                        y: parent.height/2 - 4
                        width: 2
                        height: 8
                        color: index === 0 || index === model-1 ? "#ffffff" : "#aaaaaa"
                    }
                }

                Rectangle {
                    x: mouseArea.visualPosition * (parent.width - 16)
                    y: parent.height/2 - 8
                    width: 16
                    height: 16
                    radius: 8
                    color: mouseArea.pressed ? "#3a73c0" : "#7aa9e6"
                    border.color: "#ffffff"
                    border.width: 2

                    Rectangle {
                        anchors.centerIn: parent
                        width: 6
                        height: 6
                        radius: 3
                        color: "#ffffff"
                        opacity: 0.8
                    }
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    cursorShape: undefined

                    property real value: discreteSlider.initialValue
                    property real visualPosition: (value - discreteSlider.from) / (discreteSlider.to - discreteSlider.from)

                    function updateValueFromMouse(mouse) {
                        var pos = Math.max(0, Math.min(1, mouse.x / width))
                        var newValue = discreteSlider.from + pos * (discreteSlider.to - discreteSlider.from)

                        if (discreteSlider.stepSize > 0) {
                            var steps = Math.round((newValue - discreteSlider.from) / discreteSlider.stepSize)
                            newValue = discreteSlider.from + steps * discreteSlider.stepSize
                        }

                        newValue = Math.max(discreteSlider.from, Math.min(discreteSlider.to, newValue))
                        value = newValue
                    }

                    onPressed: (mouse) => updateValueFromMouse(mouse)
                    onPositionChanged: (mouse) => {
                        if (pressed) {
                            updateValueFromMouse(mouse)
                        }
                    }

                    onValueChanged: {
                        if (discreteSlider.label === "Sprite size")         root.spriteSize = value
                        else if (discreteSlider.label === "Battle speed")   root.battleSpeed = value
                        else if (discreteSlider.label === "Encounter lvl +") root.encounterLevelHigh = value
                        else if (discreteSlider.label === "Encounter lvl -") root.encounterLevelLow = value
                    }
                }
            }

            Text {
                id: valueLabel
                text: {
                    var val = mouseArea.value
                    if(isNegative){
                        "-" + Number(val % 1 === 0 ? val : val.toFixed(1)).toString()
                    }else{
                        Number(val % 1 === 0 ? val : val.toFixed(1)).toString()
                    }
                }
                font.family: root.p2pFont
                font.pixelSize: root.fontSizeSm - 4
                color: root.labelColor
                width: 35
                height: parent.height
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                style: Text.Raised
                styleColor: "#000000"
            }
        }
    }

    component CheckBoxItem : Item {
        id: checkBoxItem
        required property string label
        required property bool initialValue

        property alias checked: mouseArea.checked

        width: parent.width
        height: 30

        Row {
            width: parent.width
            height: parent.height
            spacing: 8

            Text {
                text: checkBoxItem.label
                font.family: root.dotGothicFont
                font.pixelSize: root.fontSizeSm - 2
                color: root.textColor
                width: 110
                height: parent.height
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                font.bold: true
                opacity: 0.9
            }

            Item {
                width: parent.width - 110 - valueLabel.width - parent.spacing - 8
                height: parent.height

                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    width: 24
                    height: 24
                    radius: 4
                    color: "transparent"
                    border.color: "#3a5f8a"
                    border.width: 2

                    Rectangle {
                        anchors.centerIn: parent
                        width: 18
                        height: 18
                        radius: 1
                        color: mouseArea.checked ? "#7aa9e6" : "transparent"
                    }
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 32
                    text: mouseArea.checked ? "ON" : "OFF"
                    font.family: root.p2pFont
                    font.pixelSize: root.fontSizeSm - 4
                    color: root.textColor
                    style: Text.Raised
                    styleColor: "#000000"
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: undefined

                    property bool checked: checkBoxItem.initialValue

                    onClicked: checked = !checked

                    onCheckedChanged: {
                        if (checkBoxItem.label === "Exp. share") root.expShare = checked
                    }
                }
            }

            Text {
                id: valueLabel
                text: ""
                font.family: root.p2pFont
                font.pixelSize: root.fontSizeSm - 4
                color: root.textColor
                width: 35
                height: parent.height
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                style: Text.Raised
                styleColor: "#000000"
            }
        }
    }

    Row {
        anchors.fill: parent
        anchors.leftMargin: 4
        anchors.rightMargin: 8
        spacing: 16

        Item {
            id: spriteContainer
            width: Math.ceil(root.frameSize * root.iconScale) + 8
            height: parent.height

            Image {
                id: trainerSprite
                anchors.centerIn: parent

                property int spriteWidth: root.frameSize
                property int spriteHeight: root.frameSize

                width: Math.ceil(spriteWidth * root.iconScale)
                height: Math.ceil(spriteHeight * root.iconScale)

                source: "qrc:/assets/HGSS/reordered_trainers.png"
                sourceClipRect: Qt.rect(0, root.row * spriteHeight, spriteWidth, spriteHeight)
                smooth: false
                antialiasing: false
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: undefined
            }
        }

        Column {
            id: slidersColumn
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - spriteContainer.width - parent.spacing - parent.anchors.leftMargin - parent.anchors.rightMargin
            spacing: 2

            DiscreteSlider {
                id: spriteSizeSlider
                label: "Sprite size"
                from: root.spriteSizeMin
                to: root.spriteSizeMax
                stepSize: root.spriteSizeStep
                initialValue: root.spriteSizeDefault
                width: parent.width
            }

            DiscreteSlider {
                id: battleSpeedSlider
                label: "Battle speed"
                from: root.battleSpeedMin
                to: root.battleSpeedMax
                stepSize: root.battleSpeedStep
                initialValue: root.battleSpeedDefault
                width: parent.width
            }

            DiscreteSlider {
                id: encounterHighSlider
                label: "Encounter lvl +"
                from: root.encounterHighMin
                to: root.encounterHighMax
                stepSize: root.encounterHighStep
                initialValue: root.encounterLevelHighDefault
                width: parent.width
            }

            DiscreteSlider {
                id: encounterLowSlider
                label: "Encounter lvl -"
                from: root.encounterLowMin
                to: root.encounterLowMax
                stepSize: root.encounterLowStep
                initialValue: root.encounterLevelLowDefault
                width: parent.width
                isNegative: true
            }

            CheckBoxItem {
                id: expShareCheckbox
                label: "Exp. share"
                initialValue: root.expShareDefault
                width: parent.width
            }
        }
    }
}
