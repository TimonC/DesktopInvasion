import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    property int scaleFactor: 3
    property real totalHealth: 100
    property alias currentHealthRatio: progressBar.value
    property real animationSpeed: 1000
    width: 32*2
    height: 24
    property alias pokeName: textBarText.text

    Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        width: root.width
        height: parent.height*3/4
        color: "darkgrey"
        border.color: "black"

        Text {
            id: textBarText
            anchors.fill: parent
            anchors.rightMargin: 4
            text: "Pokemon"
            font.pixelSize: parent.height * 0.5
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignRight
            elide: Text.ElideRight
        }
    }

    ProgressBar {
        id: progressBar
        anchors.bottom: parent.bottom
        width: parent.width
        height: parent.height/4
        value: 1
        from: 0
        to: 1
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
