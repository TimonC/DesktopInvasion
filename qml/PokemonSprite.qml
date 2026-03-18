import QtQuick 2.15

Item {
    id: root
    property color debugColor: "yellow"
    // Sprite properties
    property string spriteSheet: "qrc:/assets/HGSS/reordered_sprites.png"
    property int partyId: 0
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
    property string name: "emptyname"
    property alias actionForward: actionForward
    property alias takeDamage: takeDamage
    width: itemWidth > 0 ? itemWidth : frameWidth * scaleFactor
    height: itemHeight > 0 ? itemHeight : frameHeight * scaleFactor
    layer.enabled: true
    z: 1
    //Store starting pos for animations
    property int startingX: 0
    property int startingY: 0

    // Random delay timer to prevent sync between sprites
    property Timer startTimer: Timer {
        interval: Math.random() * 125
        running: true
        onTriggered: sprite.running = true
    }

    // Method to change sprite source
    function updatePokemon(rowId, isBig) {
        // Update sprite sheet based on generation
        if(isBig){
            spriteSheet = "qrc:/assets/HGSS/reordered_sprites_big.png";
        }else{
            spriteSheet = "qrc:/assets/HGSS/reordered_sprites.png";
        }
        row = rowId;

        // Restart the sprite animation
        sprite.running = false;
        Qt.callLater(function() {
            sprite.currentFrame = Math.random() < 0.5 ? 0 : 1;
            sprite.running = true;
        });
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
                duration: 50
            }
            PropertyAnimation {
                target: root
                property: "opacity"
                to: 1
                duration: 50
            }
        }
    }

    SequentialAnimation {
        id: actionForward
        loops: 1
        running: false
        property int attackDistance: 20
        PropertyAnimation {
            target: root
            property: "x"
            to: root.startingX + (root.direction==1 ? -actionForward.attackDistance : root.direction==3 ? actionForward.attackDistance : 0)
            duration: 50
            easing.type: Easing.InQuad
        }
        PropertyAnimation {
            target: root
            property: "y"
            to: root.startingY + (root.direction==0 ? -actionForward.attackDistance : root.direction==2 ? actionForward.attackDistance : 0)
            duration: 50
            easing.type: Easing.InQuad
        }
        PropertyAnimation {
            target: root
            property: "x"
            to: root.startingX
            duration: 100
            easing.type: Easing.OutQuad
        }
        PropertyAnimation {
            target: root
            property: "y"
            to: root.startingY
            duration: 100
            easing.type: Easing.OutQuad
        }
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
