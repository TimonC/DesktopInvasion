import QtQuick 2.15

Item {
    id: root
    property string spriteSheet: "qrc:/assets/HGSS/PokGen1_transparent_reordered.png"
    property int sheetRow: 0
    property int animation: 0
    property int frameWidth: 32
    property int frameHeight: 32
    property int frameCount: 2
    property int frameRate: 4
    property real scaleFactor: 4

    scale: scaleFactor

    Rectangle {
        x: 0
        y: 0
        width: frameWidth
        height: frameHeight
        color: "transparent"
        border.color: "yellow"
        border.width: 1
    }

    AnimatedSprite {
        source: spriteSheet
        frameWidth: root.frameWidth
        frameHeight: root.frameHeight
        frameCount: root.frameCount
        frameRate: root.frameRate
        interpolate: false
        smooth: false
        antialiasing: false

        frameX: {
            switch(animation) {  //Explicit switch statement for automatic update on 'animation' prop change
                case 0: return 0;
                case 1: return frameWidth * frameCount;
                case 2: return frameWidth * frameCount * 2;
                case 3: return frameWidth * frameCount * 3;
            }
            return 0;
        }
        frameY: sheetRow * frameHeight
    }
}
