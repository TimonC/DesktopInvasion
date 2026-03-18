import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root

    // Pass-through sprite properties
    property alias spriteSheet: pokemonSprite.spriteSheet
    property alias row: pokemonSprite.row
    property alias direction: pokemonSprite.direction
    property alias scaleFactor: pokemonSprite.scaleFactor
    property alias frameWidth: pokemonSprite.frameWidth
    property alias frameHeight: pokemonSprite.frameHeight
    property alias frameCount: pokemonSprite.frameCount
    property alias frameRate: pokemonSprite.frameRate

    // Animation triggers
    property alias tackle: pokemonSprite.tackle
    property alias attacked: pokemonSprite.attacked

    // Container properties
    property int itemWidth: 0
    property int itemHeight: 0
    property int offsetX: 0
    property int offsetY: 0
    property bool clickable: true
    property bool debugLines: false

    width: itemWidth > 0 ? itemWidth : pokemonSprite.frameWidth * pokemonSprite.scaleFactor
    height: itemHeight > 0 ? itemHeight : pokemonSprite.frameHeight * pokemonSprite.scaleFactor
    clip: true
    layer.enabled: true
    z: 1


    // Sprite container with positioning
    Item {
        id: spriteContainer
        anchors.centerIn: parent
        anchors.horizontalCenterOffset: offsetX
        anchors.verticalCenterOffset: offsetY
        width: pokemonSprite.width
        height: pokemonSprite.height

        PokemonSprite {
            id: pokemonSprite
            anchors.centerIn: parent
        }
    }


    // Debug rectangle
    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: "yellow"
        border.width: 1
        visible: debugLines
    }
}
