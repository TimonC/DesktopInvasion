import QtQuick 2.15

Item {
    id: root
    property color debugColor: "yellow"
    property string spriteSheet: "qrc:/assets/HGSS/reordered_sprites.png"
    property int partyId: 0
    property int row: 0
    property int direction: 0
    property real scaleFactor: 1
    property int frameWidth: 32
    property int frameHeight: 32
    property int frameCount: 2
    property int frameRate: 4

    property int horizontalHeight: 0

    property int attackDistance: 10*scaleFactor

    property int itemWidth: 0
    property int itemHeight: 0
    property bool clickable: true
    property bool debugLines: false
    property string name: "emptyname"
    property alias actionForward: actionForward
    property alias takeDamage: takeDamage

    // Animation speed control (only affects damage and attack animations)
    property int animationSpeed: 1

    // Scaled duration properties
    property int damageFlashDuration: 50 / animationSpeed
    property int attackForwardDuration: 50 / animationSpeed
    property int attackReturnDuration: 100 / animationSpeed

    width: itemWidth > 0 ? itemWidth : frameWidth * scaleFactor
    height: itemHeight > 0 ? itemHeight : frameHeight * scaleFactor
    layer.enabled: true
    z: 1

    property int startingX: 0
    property int startingY: 0

    property Timer startTimer: Timer {
        interval: Math.random() * 125
        running: true
        onTriggered: sprite.running = true
    }

    function updatePokemon(rowId, isBig) {
        if(isBig){
            spriteSheet = "qrc:/assets/HGSS/reordered_sprites_big.png";
        }else{
            spriteSheet = "qrc:/assets/HGSS/reordered_sprites.png";
        }
        row = rowId;

        sprite.running = false;
        Qt.callLater(function() {
            sprite.currentFrame = Math.random() < 0.5 ? 0 : 1;
            sprite.running = true;
        });
    }

    AnimatedSprite {
        id: sprite
        anchors.centerIn: parent
        width: root.frameWidth * scaleFactor
        height: root.frameHeight * scaleFactor
        running: false
        source: root.spriteSheet
        frameWidth: root.frameWidth
        frameHeight: root.frameHeight
        frameCount: root.frameCount
        frameRate: root.frameRate  // NOT scaled by animationSpeed
        currentFrame: Math.random() < 0.5 ? 0 : 1
        interpolate: false
        smooth: false
        antialiasing: false
        frameX: {
            switch (direction) {
                case 0: return 0
                case 1: return frameWidth * frameCount
                case 2: return frameWidth * frameCount * 2
                case 3: return frameWidth * frameCount * 3
            }
            return 0
        }
        frameY: row * frameHeight
    }

    SequentialAnimation {
        id: takeDamage
        running: false
        loops: 1
        SequentialAnimation {
            loops: 3
            PropertyAnimation {
                target: root
                property: "opacity"
                to: 0
                duration: root.damageFlashDuration
            }
            PropertyAnimation {
                target: root
                property: "opacity"
                to: 1
                duration: root.damageFlashDuration
            }
        }
    }

    SequentialAnimation {
        id: actionForward
        loops: 1
        running: false
        property int attackDistance: root.attackDistance
        PropertyAnimation {
            target: root
            property: "x"
            to: root.startingX + (root.direction==1 ? -actionForward.attackDistance : root.direction==3 ? actionForward.attackDistance : 0)
            duration: root.attackForwardDuration
            easing.type: Easing.InQuad
        }
        PropertyAnimation {
            target: root
            property: "y"
            to: root.startingY + (root.direction==0 ? -actionForward.attackDistance : root.direction==2 ? actionForward.attackDistance : 0)
            duration: root.attackForwardDuration
            easing.type: Easing.InQuad
        }
        PropertyAnimation {
            target: root
            property: "x"
            to: root.startingX
            duration: root.attackReturnDuration
            easing.type: Easing.OutQuad
        }
        PropertyAnimation {
            target: root
            property: "y"
            to: root.startingY
            duration: root.attackReturnDuration
            easing.type: Easing.OutQuad
        }
    }

    Rectangle {
        id: containerDebugLines
        anchors.fill: parent
        color: "transparent"
        border.color: debugLines ? debugColor : "transparent"
        border.width: 1
    }
}
