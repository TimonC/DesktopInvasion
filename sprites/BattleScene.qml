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
    property int chosenSide: 0  // 0=North, 1=East, 2=South, 3=West

    property bool debugLines: false

    // Top-level Pokemon properties (for easy C++ access)
    property int direction: 0
    property string opponentSpriteSheet: ""
    property int opponentRow: 0
    property string playerSpriteSheet: ""
    property int playerRow: 0
    property real scaleFactor: 4.0  // Shared scale for both Pokemon

    // Aliases for external access
    property alias textBar: textBar
    property alias buttonGrid: buttonGrid
    property alias attackButton: attackButton
    property alias switchButton: switchButton
    property alias catchButton: catchButton
    property alias runButton: runButton

    // Opponent Pokemon (wild) - positioned opposite to player
    PokemonSprite {
        id: opponentSprite
        objectName: "opponentSprite"

        // Bind to top-level properties
        spriteSheet: root.opponentSpriteSheet
        row: root.opponentRow
        direction: root.direction
        scaleFactor: root.scaleFactor

        Component.onCompleted: positionSprite(opponentSprite, getOppositeSide(chosenSide))

        Connections {
            target: root
            function onChosenSideChanged() {
                positionSprite(opponentSprite, getOppositeSide(chosenSide));
            }
        }
    }

    // Player Pokemon - positioned according to chosenSide
    PokemonSprite {
        id: playerSprite
        objectName: "playerSprite"

        // Bind to top-level properties
        spriteSheet: root.playerSpriteSheet
        row: root.playerRow
        direction: (root.direction + 2) % 4
        scaleFactor: root.scaleFactor

        Component.onCompleted: positionSprite(playerSprite, chosenSide)

        Connections {
            target: root
            function onChosenSideChanged() {
                positionSprite(playerSprite, chosenSide);
            }
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
    function positionSprite(sprite, side) {
        // Reset all anchors first
        sprite.anchors.top = undefined;
        sprite.anchors.bottom = undefined;
        sprite.anchors.left = undefined;
        sprite.anchors.right = undefined;
        sprite.anchors.horizontalCenter = undefined;
        sprite.anchors.verticalCenter = undefined;

        var margin = 20;

        switch(side) {
            case 0: // North - position at top
                sprite.anchors.top = root.top;
                sprite.anchors.horizontalCenter = root.horizontalCenter;
                sprite.anchors.topMargin = margin;
                break;
            case 1: // East - position at right
                sprite.anchors.right = root.right;
                sprite.anchors.verticalCenter = root.verticalCenter;
                sprite.anchors.rightMargin = margin;
                break;
            case 2: // South - position at bottom
                sprite.anchors.bottom = root.bottom;
                sprite.anchors.horizontalCenter = root.horizontalCenter;
                sprite.anchors.bottomMargin = margin + textBar.height;
                break;
            case 3: // West - position at left
                sprite.anchors.left = root.left;
                sprite.anchors.verticalCenter = root.verticalCenter;
                sprite.anchors.leftMargin = margin;
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

    // Trigger animations directly on the sprites
    function playOpponentTackle() {
        opponentSprite.tackle = true;
    }

    function playPlayerTackle() {
        playerSprite.tackle = true;
    }

    function playOpponentAttack() {
        opponentSprite.attacked = true;
    }

    function playPlayerAttack() {
        playerSprite.attacked = true;
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
