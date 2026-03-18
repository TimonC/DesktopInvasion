import QtQuick 2.15

Item {
    id: root
     property color debugColor: "yellow"
    // Sprite properties
    property string spriteSheet: "qrc:/assets/HGSS/PokGen1_transparent_reordered.png"
    property int row: 0
    property int direction: 0
    property real scaleFactor: 4
    property int frameWidth: 32
    property int frameHeight: 32
    property int frameCount: 2
    property int frameRate: 4

    // Container properties
    property int itemWidth: 0
    property int itemHeight: 0
    property int containerOffsetX: 0
    property int containerOffsetY: 0
    property bool clickable: true
    property bool debugLines: false

    // Store original position for animations
    property real originalX: 0
    property real originalY: 0

    width: itemWidth > 0 ? itemWidth : frameWidth * scaleFactor
    height: itemHeight > 0 ? itemHeight : frameHeight * scaleFactor
    layer.enabled: true
    z: 1

    // Random delay timer to prevent sync
    property Timer startTimer: Timer {
        interval: Math.random() * 125
        running: true
        onTriggered: sprite.running = true
    }

    AnimatedSprite {
        id: sprite
        // REMOVE ANCHORS - use manual positioning
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        scale: scaleFactor

        running: false
        source: root.spriteSheet
        frameWidth: root.frameWidth
        frameHeight: root.frameHeight
        frameCount: root.frameCount
        frameRate: root.frameRate
        currentFrame: Math.random() < 0.5 ? 0 : 1
        interpolate: false
        smooth: false
        antialiasing: false

        frameX: {
            switch (direction) {
                case 0: return 0;
                case 1: return frameWidth * frameCount;
                case 2: return frameWidth * frameCount * 2;
                case 3: return frameWidth * frameCount * 3;
            }
            return 0;
        }
        frameY: row * frameHeight
    }

    // Debug rectangle
    Rectangle {
        id: containerDebugLines
        anchors.fill: parent
        color: "transparent"
        border.color: debugLines ? debugColor : "transparent"
        border.width: 1
    }
}
