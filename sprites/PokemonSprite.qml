import QtQuick 2.15

AnimatedSprite {
    id: root

    property int sheetRow: 0
    property string spriteSheet: "qrc:/assets/HGSS/PokGen1_transparent_reordered.png"
    property int animation: 0
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
        switch(animation) {
            case 0: return 0;
            case 1: return 64;
            case 2: return 128;
            case 3: return 192;
        }
        return 0;
    }
    frameY: sheetRow * frameHeight
}
