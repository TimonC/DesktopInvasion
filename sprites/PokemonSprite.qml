import QtQuick 2.15

Item {
    id: root
    property string spriteSheet: "qrc:/assets/HGSS/PokGen1_transparent_reordered.png"
    property int row: 0
    property int animation: 0
    property real scaleFactor: 4
    property int spriteOffsetX: 0
    property int spriteOffsetY: 0
    property int itemWidth: 32
    property int itemHeight: 32
    property int frameWidth: 32
    property int frameHeight: 32
    property int frameCount: 2
    property int frameRate: 4

    width: itemWidth
    height: itemHeight
    clip: true

    AnimatedSprite {
        x: spriteOffsetX
        y: spriteOffsetY
        scale: scaleFactor

        source: spriteSheet
        frameWidth: root.frameWidth
        frameHeight: root.frameHeight
        frameCount: root.frameCount
        frameRate: root.frameRate
        interpolate: false
        smooth: false
        antialiasing: false

        frameX: {
            switch(animation) {
                case 0: return 0;
                case 1: return frameWidth * frameCount;
                case 2: return frameWidth * frameCount * 2;
                case 3: return frameWidth * frameCount * 3;
            }
            return 0;
        }
        frameY: row * frameHeight
    }
}
