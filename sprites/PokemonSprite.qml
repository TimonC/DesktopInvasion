import QtQuick 2.15
Item {
    id: root

    property string spriteSheet: "qrc:/assets/HGSS/PokGen1_transparent_reordered.png"
    property int rowIndex: 0
    property int frameWidth: 32
    property int frameHeight: 32
    property int frameRate: 2
    property int frameCount: 2

Sprite{
    name: up
    source: root.spriteSheet
    frameX: 0 * root.frameCount
    frameY: root.rowIndex * root.frameHeight
    frameWidth: root.frameWidth
    frameHeight: root.frameHeight
    frameCount: root.frameCount
    frameRate: root.frameRate
}

Sprite{
    name: left
    source: root.spriteSheet
    frameX: 1 * root.frameCount * root.frameWidth
    frameY: root.rowIndex * root.frameHeight
    frameWidth: root.frameWidth
    frameHeight: root.frameHeight
    frameCount: root.frameCount
    frameRate: root.frameRate
}

Sprite{
    name: down
    source: root.spriteSheet
    frameX: 2 * root.frameCount * root.frameWidth
    frameY: root.rowIndex * root.frameHeight
    frameWidth: root.frameWidth
    frameHeight: root.frameHeight
    frameCount: root.frameCount
    frameRate: root.frameRate
}

Sprite{
    name: right
    source: root.spriteSheet
    frameX: 3 * root.frameCount * root.frameWidth
    frameY: root.rowIndex * root.frameHeight
    frameWidth: root.frameWidth
    frameHeight: root.frameHeight
    frameCount: root.frameCount
    frameRate: root.frameRate
}



}

