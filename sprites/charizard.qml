import QtQuick 6.6

SpriteSequence {
    id: charizard
    width: 32
    height: 32
    running: true

    property string sourceSheet: "qrc:/sprites/HGSS/PokGen1_transparent.png"
    property int frameWidth: 32
    property int frameHeight: 32
    property int frameRate: 6

    Sprite {
        name: "up"
        source: charizard.sourceSheet
        frameWidth: charizard.frameWidth
        frameHeight: charizard.frameHeight
        frameCount: 2
        frameRate: charizard.frameRate
        frameX: 0
        frameY: 0
    }

    Sprite {
        name: "left"
        source: charizard.sourceSheet
        frameWidth: charizard.frameWidth
        frameHeight: charizard.frameHeight
        frameCount: 2
        frameRate: charizard.frameRate
        frameX: 1 * charizard.frameWidth
        frameY: 0
    }

    Sprite {
        name: "down"
        source: charizard.sourceSheet
        frameWidth: charizard.frameWidth
        frameHeight: charizard.frameHeight
        frameCount: 2
        frameRate: charizard.frameRate
        frameX: 0
        frameY: 2 * charizard.frameHeight
    }

    Sprite {
        name: "right"
        source: charizard.sourceSheet
        frameWidth: charizard.frameWidth
        frameHeight: charizard.frameHeight
        frameCount: 2
        frameRate: charizard.frameRate
        frameX: 1 * charizard.frameWidth
        frameY: 2 * charizard.frameHeight
    }
}

