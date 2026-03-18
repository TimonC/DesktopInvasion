import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Effects  2.15
Item {
    id: root
    property int scaleFactor: 3
    property real totalHealth: 100
    property alias currentHealthRatio: progressBar.value
    property real animationSpeed: 1000
    property int pokeNameFontSize: 0
    property alias pokeName: textBarText.text

    Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        width: root.width
        height: parent.height * 3/4
        color: "transparent"

        Text {
            id: textBarText
            anchors.fill: parent
            anchors.rightMargin: 4

            color: "white"
            style: Text.Outline
            styleColor: "black"
            renderType: Text.NativeRendering //magic way to make outline nicer
            smooth: true
            antialiasing: true

            text: "Pokemon"
            font.bold: true
            font.pixelSize: root.pokeNameFontSize
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignRight

        }
    }


   ProgressBar {
        id: progressBar
        width: parent.width
        height: parent.height/4
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        value: 1
        from: 0
        to: 1
        background: Rectangle {
            anchors.fill: progressBar
            color: "darkgrey"
            radius: 4
            border.width: 1
            border.color: "black"
        }
        contentItem: Rectangle {
            anchors.left: progressBar.left
            anchors.bottom: progressBar.bottom
            height: progressBar.height
            width: progressBar.width * progressBar.value
            color: progressBar.value >=0.5 ? "green" :  (progressBar.value >=0.25 ? "orange" : "red")
            radius: 4
        }
    }


    function incrementHealth(healthDelta) {
        var targetValue = (progressBar.value * totalHealth + healthDelta) / totalHealth
        healthAnimation.from = progressBar.value
        var newHealth = Math.min(Math.max(targetValue, 0), 1)

        healthAnimation.to = newHealth
        healthAnimation.duration = Math.abs(healthDelta) * (animationSpeed / totalHealth)
        healthAnimation.start()
        return newHealth
    }

    NumberAnimation {
        id: healthAnimation
        target: progressBar
        property: "value"
        // easing.type: Easing.InOutQuad
    }
}
