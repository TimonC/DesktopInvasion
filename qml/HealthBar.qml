import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    property int scaleFactor: 3
    property real totalHealth: 100
    property real animationSpeed: 1000

    ProgressBar {
        id: progressBar
        value: 10
        from: 0
        to: 10
        width: 24*scaleFactor
        height: 4*scaleFactor
    }

    function incrementHealth(healthDelta) {
        var targetValue = (progressBar.value * totalHealth + healthDelta) / totalHealth
        healthAnimation.from = progressBar.value
        healthAnimation.to = Math.min(Math.max(targetValue, 0), 1)
        healthAnimation.duration = Math.abs(healthDelta) * (animationSpeed / totalHealth)
        healthAnimation.start()
    }

    NumberAnimation {
        id: healthAnimation
        target: progressBar
        property: "value"
        easing.type: Easing.InOutQuad
    }
}
