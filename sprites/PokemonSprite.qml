import QtQuick 2.15

Item {
    id: root

    property string spriteSheet: "qrc:/assets/HGSS/PokGen1_transparent_reordered.png"
    property int rowIndex: 0
    property int frameWidth: 32
    property int frameHeight: 32
    property int framesPerDirection: 2
    property int frameRate: 5

    Sprite {
        id: sprite
        source: root.spriteSheet
        frameWidth: root.frameWidth
        frameHeight: root.frameHeight
        frameX: 0
        frameY: root.rowIndex * root.frameHeight
        frameCount: root.framesPerDirection
        frameRate: root.frameRate
    }

    function updateSprite() {
        sprite.source = root.spriteSheet
        sprite.frameY = root.rowIndex * root.frameHeight
        sprite.restart()
    }
}

