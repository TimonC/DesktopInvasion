import QtQuick 2.15

AnimatedSprite {
    id: root

    property int sheetRow: 0
    property string spriteSheet: "qrc:/assets/HGSS/PokGen1_transparent_reordered.png"
    property real moveSpeed: 20 + Math.random() * 30

    property string currentDirection: "down"
    property bool isMoving: false

    source: spriteSheet
    frameWidth: 32
    frameHeight: 32
    frameCount: 2
    frameRate: 4
    paused: false
    interpolate: false
    smooth: false
    antialiasing: false
    scale: 4
    frameX: {
        switch(currentDirection) {
            case "up": return 0;
            case "left": return 64;
            case "down": return 128;
            case "right": return 192;
        }
        return 128;
    }
    frameY: sheetRow * frameHeight

    Timer {
        id: behaviorTimer
        interval: 1500 + Math.random() * 2000
        running: true
        repeat: true
        onTriggered: root.makeRandomDecision()
    }

    Timer {
        id: moveTimer
        interval: 16
        running: root.isMoving
        repeat: true
        onTriggered: root.updateMovement()
    }

    function makeRandomDecision() {
        var decision = Math.floor(Math.random() * 8);
        switch(decision) {
            case 0: setBehavior("up", false); break;
            case 1: setBehavior("up", true); break;
            case 2: setBehavior("left", false); break;
            case 3: setBehavior("left", true); break;
            case 4: setBehavior("down", false); break;
            case 5: setBehavior("down", true); break;
            case 6: setBehavior("right", false); break;
            case 7: setBehavior("right", true); break;
        }
        constrainToParent();
    }

    function setBehavior(direction, moving) {
        currentDirection = direction;
        isMoving = moving;
        restart();
    }

    function updateMovement() {
        var moveDelta = moveSpeed * 0.016;
        switch(currentDirection) {
            case "up": y -= moveDelta; break;
            case "left": x -= moveDelta; break;
            case "down": y += moveDelta; break;
            case "right": x += moveDelta; break;
        }
        if (x < -100 || x > parent.width + 100 || y < -100 || y > parent.height + 100) {
          updateMovement();
        }
    }

    function constrainToParent() {
        if (!parent) return;
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x + width > parent.width) x = parent.width - width;
        if (y + height > parent.height) y = parent.height - height;
    }

    Component.onCompleted: makeRandomDecision();
}
