import QtQuick 2.15
import QtQuick.Controls 2.15
import "Style/PokeColor.js" as PokeColor

Item {
    id: root
    property int scaleFactor: 1
    property int animationSpeed: 1
    property int totalHealth: 0
    property alias currentHealthRatio: progressBar.value
    property int healthChangeDuration: 500/animationSpeed

    property alias pokeName: nameLabel.text
    property alias levelText: levelLabel.text

    property int pokeNameFontSize: 0
    property int subTextFontSize: pokeNameFontSize*0.9
    property string fontFamily: ""
    property int statusPadding: pokeNameFontSize/2
    property int statusBarRadius: 0


    component PopoutText: Text {
        color: "white"
        font.bold: true
        font.family: root.fontFamily
        style: Text.Outline
        styleColor: "black"
        renderType: Text.NativeRendering
        smooth: true
        antialiasing: true
    }

    PopoutText {
        id: nameLabel
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: root.height * 2/5
        text: "Pokemon"
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignRight
        font.pixelSize: root.pokeNameFontSize
    }

    Item {
        id: statusLevelRow
        anchors.top: nameLabel.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: root.height * 2/5

        PopoutText {
            id: levelLabel
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: "Lv000"
            font.pixelSize: root.subTextFontSize
        }

        Rectangle {
            id: statusContainer
            anchors.right: levelLabel.left
            anchors.rightMargin: root.statusPadding/2
            anchors.verticalCenter: parent.verticalCenter
            width: statusLabel.implicitWidth + root.statusPadding
            height: statusLabel.implicitHeight + root.statusPadding
            radius: root.statusBarRadius/2
            color: "transparent"
            border.width: 1
            border.color: "transparent"

            gradient: Gradient {
                id: statusGradient
                GradientStop { id: gradientStop1; position: 0 }
                GradientStop { id: gradientStop2; position: 1 }
            }

            PopoutText {
                id: statusLabel
                anchors.fill: parent
                text: "NIL"
                visible: false
                font.pixelSize: root.subTextFontSize
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    ProgressBar {
        id: progressBar
        anchors.top: statusLevelRow.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        value: 1
        from: 0
        to: 1
        background: Rectangle {
            anchors.fill: progressBar
            color: "darkgrey"
            radius: root.statusBarRadius
            border.width: 1
            border.color: "black"
        }
        contentItem: Rectangle {
            anchors.left: progressBar.left
            anchors.bottom: progressBar.bottom
            height: progressBar.height
            width: progressBar.width * progressBar.value
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: PokeColor.lighter(PokeColor.healthColor(progressBar.value))
                }
                GradientStop {
                    position: 1
                    color: PokeColor.darker(PokeColor.healthColor(progressBar.value))
                }
            }
            radius: root.statusBarRadius
        }
    }

    function changeStatusCondition(label, remove) {
        if (remove) {
            statusLabel.text = ""
            statusLabel.visible = false
            statusContainer.color = "transparent"
            statusContainer.border.color = "transparent"
        } else {
            statusLabel.text = label
            statusLabel.visible = true
            statusContainer.visible = true
            var baseColor = PokeColor.statusConditionColor(label)
            statusContainer.color = baseColor
            gradientStop1.color = PokeColor.lighter(baseColor)
            gradientStop2.color = PokeColor.darker(baseColor)
        }
    }

    function changeHealth(healthDelta) {
        var targetValue = (progressBar.value * totalHealth + healthDelta) / totalHealth
        healthAnimation.from = progressBar.value
        var newHealth = Math.min(Math.max(targetValue, 0), 1)

        healthAnimation.to = newHealth
        healthAnimation.duration = healthChangeDuration
        healthAnimation.start()
        return newHealth
    }

    function setLevelText(level){
        root.levelText = "Lv"+level
    }

    NumberAnimation {
        id: healthAnimation
        target: progressBar
        property: "value"
    }
}
