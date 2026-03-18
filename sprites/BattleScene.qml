import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: (chosenSide === 0 || chosenSide === 2) ? frameSize * 5 : frameSize * 8
    height: (chosenSide === 0 || chosenSide === 2) ? frameSize * 8 : frameSize * 5
    layer.enabled: true

    // Scaling properties
    property int frameSize: 32
    property int buttonWidth: frameSize * 2
    property int buttonHeight: frameSize * 0.7
    property int buttonFontSize: frameSize * 0.4
    property int gridSpacing: frameSize * 0.1
    property int chosenSide: 1  // 0=North, 1=East, 2=South, 3=West

    property bool debugLines: false

    // Pokemon sprite configurations
    property var wildPokemon: null  // Set from outside: {spriteSheet, row, direction, etc}
    property var playerPokemon: null  // Set from outside: {spriteSheet, row, direction, etc}

    // Aliases for external access
    property alias textBar: textBar
    property alias buttonGrid: buttonGrid
    property alias attackButton: attackButton
    property alias switchButton: switchButton
    property alias catchButton: catchButton
    property alias runButton: runButton
    property alias mouseArea: mouseArea

    MouseArea {
        id: mouseArea
        anchors.fill: parent
    }

    // Wild Pokemon (opponent) - positioned opposite to player
    Loader {
        id: wildPokemonLoader
        active: wildPokemon !== null
        sourceComponent: pokemonSpriteComponent

        property var config: wildPokemon
        property int side: getOppositeSide(chosenSide)

        Component.onCompleted: positionSprite(this, side)

        Connections {
            target: root
            function onChosenSideChanged() {
                wildPokemonLoader.side = getOppositeSide(chosenSide);
                positionSprite(wildPokemonLoader, wildPokemonLoader.side);
            }
        }
    }

    // Player Pokemon - positioned according to chosenSide
    Loader {
        id: playerPokemonLoader
        active: playerPokemon !== null
        sourceComponent: pokemonSpriteComponent

        property var config: playerPokemon
        property int side: chosenSide

        Component.onCompleted: positionSprite(this, side)

        Connections {
            target: root
            function onChosenSideChanged() {
                playerPokemonLoader.side = chosenSide;
                positionSprite(playerPokemonLoader, chosenSide);
            }
        }
    }

    // Reusable Pokemon sprite component
    Component {
        id: pokemonSpriteComponent

        PokemonSprite {
            spriteSheet: parent.config ? parent.config.spriteSheet || "" : ""
            row: parent.config ? parent.config.row || 0 : 0
            direction: parent.config ? parent.config.direction || 0 : 0
            scaleFactor: parent.config ? parent.config.scaleFactor || 4 : 4
            frameWidth: parent.config ? parent.config.frameWidth || 32 : 32
            frameHeight: parent.config ? parent.config.frameHeight || 32 : 32
            frameCount: parent.config ? parent.config.frameCount || 2 : 2
            frameRate: parent.config ? parent.config.frameRate || 4 : 4
        }
    }

    // Helper function to get opposite side
    function getOppositeSide(side) {
        switch(side) {
            case 0: return 2;  // North -> South
            case 1: return 3;  // East -> West
            case 2: return 0;  // South -> North
            case 3: return 1;  // West -> East
        }
        return 2;
    }

    // Position sprite based on side
    function positionSprite(loader, side) {
        // Reset all anchors first
        loader.anchors.top = undefined;
        loader.anchors.bottom = undefined;
        loader.anchors.left = undefined;
        loader.anchors.right = undefined;
        loader.anchors.horizontalCenter = undefined;
        loader.anchors.verticalCenter = undefined;

        var margin = 20;

        switch(side) {
            case 0: // North - position at top
                loader.anchors.top = root.top;
                loader.anchors.horizontalCenter = root.horizontalCenter;
                loader.anchors.topMargin = margin;
                break;
            case 1: // East - position at right
                loader.anchors.right = root.right;
                loader.anchors.verticalCenter = root.verticalCenter;
                loader.anchors.rightMargin = margin;
                break;
            case 2: // South - position at bottom
                loader.anchors.bottom = root.bottom;
                loader.anchors.horizontalCenter = root.horizontalCenter;
                loader.anchors.bottomMargin = margin + textBar.height;
                break;
            case 3: // West - position at left
                loader.anchors.left = root.left;
                loader.anchors.verticalCenter = root.verticalCenter;
                loader.anchors.leftMargin = margin;
                break;
        }
    }

    // Public functions
    function set_chosen_side(side) {
        chosenSide = side;
    }

    function update_text_bar(newText) {
        textBar.text = newText;
    }

    function swap_visibility() {
        var showButtons = !buttonGrid.visible;
        buttonGrid.visible = showButtons;
        textBarText.visible = !showButtons;

        if (showButtons) {
            textBar.color = "transparent";
        } else {
            textBar.color = "lightgrey";
        }
    }

    // Trigger animations on loaded sprites
    function playWildPokemonTackle() {
        if (wildPokemonLoader.item) {
            wildPokemonLoader.item.tackle = true;
        }
    }

    function playPlayerPokemonTackle() {
        if (playerPokemonLoader.item) {
            playerPokemonLoader.item.tackle = true;
        }
    }

    function playWildPokemonAttack() {
        if (wildPokemonLoader.item) {
            wildPokemonLoader.item.attacked = true;
        }
    }

    function playPlayerPokemonAttack() {
        if (playerPokemonLoader.item) {
            playerPokemonLoader.item.attacked = true;
        }
    }

    // Text bar at the bottom
    Rectangle {
        id: textBar
        objectName: "textBar"
        z: 8000
        property string text: ""

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: frameSize * 1.6

        color: "transparent"
        border.color: "transparent"
        border.width: 1
        radius: 4

        Text {
            id: textBarText
            anchors.fill: parent
            anchors.margins: 6
            text: textBar.text
            font.pixelSize: 14
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            visible: false
            z: 8000
        }

        // Button grid
        Grid {
            id: buttonGrid
            columns: 2
            spacing: gridSpacing
            visible: true

            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            z: 8000

            RoundButton {
                id: attackButton
                text: "Attack"
                font.pixelSize: buttonFontSize
                palette.button: "red"
                width: buttonWidth
                height: buttonHeight
                radius: buttonHeight / 2
                onClicked: console.log("Attack clicked")
            }

            RoundButton {
                id: switchButton
                text: "Switch"
                palette.button: "green"
                font.pixelSize: buttonFontSize
                width: buttonWidth
                height: buttonHeight
                radius: buttonHeight / 2
                onClicked: console.log("Switch clicked")
            }

            RoundButton {
                id: catchButton
                text: "Catch"
                palette.button: "yellow"
                font.pixelSize: buttonFontSize
                width: buttonWidth
                height: buttonHeight
                radius: buttonHeight / 2
                onClicked: console.log("Catch clicked")
            }

            RoundButton {
                id: runButton
                text: "Run"
                palette.button: "blue"
                font.pixelSize: buttonFontSize
                width: buttonWidth
                height: buttonHeight
                radius: buttonHeight / 2
                onClicked: console.log("Run clicked")
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: "green"
        border.width: 1
        visible: debugLines
    }
}
