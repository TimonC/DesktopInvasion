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

    signal battleSceneLoaded(var battleSceneItem)

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

    // Battle Scene Loader - ONLY created when loadBattleScene() is called
    Loader {
        id: battleSceneLoader
        anchors.fill: parent
        visible: false
    }

    // Function to load battle scene - call this from C++
    function loadBattleScene(chosenSide) {
        console.log("Loading battle scene...");
        battleSceneLoader.source = "qrc:/sprites/BattleScene.qml";
        battleSceneLoader.visible = true;

        if (battleSceneLoader.item) {
            battleSceneLoader.item.chosenSide = chosenSide;
            battleSceneLoader.item.debugLines = root.debugLines;

            // RESIZE THE ROOT TO MATCH BATTLE SCENE
            root.width = battleSceneLoader.item.width;
            root.height = battleSceneLoader.item.height;

            // REPOSITION THE SPRITE WITHIN THE BATTLE SCENE
            repositionSpriteForBattle(chosenSide);

            console.log("Root resized to:", root.width, "x", root.height);
            battleSceneLoaded(battleSceneLoader.item);
        }
    }

    function repositionSpriteForBattle(chosenSide) {
        // Position the Pokemon sprite appropriately within the battle scene
        // based on which side it's on
        switch(chosenSide) {
            case 0: // North - position at top
                spriteContainer.anchors.centerIn = undefined;
                spriteContainer.anchors.top = root.top;
                spriteContainer.anchors.horizontalCenter = root.horizontalCenter;
                spriteContainer.anchors.topMargin = 20;
                break;
            case 1: // East - position at right
                spriteContainer.anchors.centerIn = undefined;
                spriteContainer.anchors.right = root.right;
                spriteContainer.anchors.verticalCenter = root.verticalCenter;
                spriteContainer.anchors.rightMargin = 20;
                break;
            case 2: // South - position at bottom
                spriteContainer.anchors.centerIn = undefined;
                spriteContainer.anchors.bottom = root.bottom;
                spriteContainer.anchors.horizontalCenter = root.horizontalCenter;
                spriteContainer.anchors.bottomMargin = 20;
                break;
            case 3: // West - position at left
                spriteContainer.anchors.centerIn = undefined;
                spriteContainer.anchors.left = root.left;
                spriteContainer.anchors.verticalCenter = root.verticalCenter;
                spriteContainer.anchors.leftMargin = 20;
                break;
        }
    }

    // Functions to control battle scene
    function set_chosen_side(side) {
        if (battleSceneLoader.item) {
            battleSceneLoader.item.chosenSide = side;
        }
    }

    function swap_visibility() {
        if (battleSceneLoader.item) {
            battleSceneLoader.item.swap_visibility();
        }
    }

    function update_text_bar(text) {
        if (battleSceneLoader.item) {
            battleSceneLoader.item.update_text_bar(text);
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
