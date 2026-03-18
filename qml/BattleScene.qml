import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: (direction === 0 || direction === 2) ? frameSize * 5 : frameSize * 8
    height: (direction === 0 || direction === 2) ? frameSize * 8 : frameSize * 5

    // Properties
    property int frameSize: 32
    property int buttonWidth: frameSize * 2
    property int buttonHeight: frameSize * 0.75
    property int buttonFontSize: frameSize * 0.4
    property int gridSpacing: frameSize * 0.1
    property int pokeMargin: frameSize * 0.25
    property bool debugLines: false
    property int textBoxHeight: 50
    property int textBoxWidth: frameSize * 5
    property int direction: 0

    property alias opponent: opponent
    property alias player: player
    property alias textBar: textBar
    property alias buttonGrid: buttonGrid

    // Attack chain state
    property bool attackInProgress: false
    property bool isPlayerTurn: true
    property int turnsCompleted: 0

    signal runClicked()

    // Sprites
    PokemonSprite {
        id: opponent
        objectName: "opponent"
        direction: root.direction
        debugLines: root.debugLines
        debugColor: "red"
    }

    PokemonSprite {
        id: player
        objectName: "player"
        direction: (root.direction + 2) % 4
        debugLines: root.debugLines
        debugColor: "blue"
    }

    // UI
    Rectangle {
        id: textBar
        color: "transparent"
        property string text: ""
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        height: root.textBoxHeight
        width: root.textBoxWidth

        Text {
            id: textBarText
            anchors.fill: parent
            anchors.margins: 6
            text: textBar.text
            font.pixelSize: 13
            verticalAlignment: Text.AlignVCenter
            visible: false
        }

        Grid {
            id: buttonGrid
            columns: 2
            spacing: gridSpacing
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2

            RoundButton {
                text: "Attack"
                palette.button: "red"
                font.pixelSize: buttonFontSize
                width: buttonWidth
                height: buttonHeight
                onClicked: root.onAttackButtonClicked()
            }
            RoundButton {
                text: "Switch"
                palette.button: "green"
                font.pixelSize: buttonFontSize
                width: buttonWidth
                height: buttonHeight
            }
            RoundButton {
                text: "Catch"
                palette.button: "yellow"
                font.pixelSize: buttonFontSize
                width: buttonWidth
                height: buttonHeight
            }
            RoundButton {
                text: "Run"
                palette.button: "blue"
                font.pixelSize: buttonFontSize
                width: buttonWidth
                height: buttonHeight
                onClicked: root.runClicked()
            }
        }
    }

    // Position sprites on the battle field
    function positionSprite(sprite) {
        var margin = root.pokeMargin;
        switch(sprite.direction) {
            case 0:
                sprite.x = margin + sprite.containerOffsetX;
                sprite.y = root.height - (textBoxHeight + margin + sprite.containerOffsetY + sprite.height);
                break;
            case 1:
                sprite.x = root.width - (margin + sprite.containerOffsetX + sprite.width);
                sprite.y = root.height - (textBoxHeight + margin + sprite.height);
                break;
            case 2:
                sprite.x = margin + sprite.containerOffsetX;
                sprite.y = margin + sprite.containerOffsetY;
                break;
            case 3:
                sprite.x = margin + sprite.containerOffsetX;
                sprite.y = root.height - (textBoxHeight + margin + sprite.height);
                break;
        }
        sprite.startingX = sprite.x;
        sprite.startingY = sprite.y;
    }

    Component.onCompleted: {
        positionSprite(player);
        positionSprite(opponent);
    }

    function update_text_bar(newText) {
        textBar.text = newText;
    }

    // Delay timer for sequencing
    Timer {
        id: delayTimer
        repeat: false
        property var callback: null
        onTriggered: {
            if (callback) callback();
        }
    }

    function delayedCall(milliseconds, func) {
        delayTimer.interval = milliseconds;
        delayTimer.callback = func;
        delayTimer.start();
    }

    // Attack button handler
    function onAttackButtonClicked() {
        var playerFirst = Math.random() < 0.5;
        startAttackChain(playerFirst);
    }

    // Start the attack sequence
    function startAttackChain(playerFirst) {
        if (root.attackInProgress) return;

        root.attackInProgress = true;
        root.isPlayerTurn = playerFirst;
        root.turnsCompleted = 0;

        buttonGrid.visible = false;
        textBar.color = "darkgrey";
        textBarText.visible = true;

        executeAttackTurn();
    }

    // Execute one turn of attack
    function executeAttackTurn() {
        var attacker = root.isPlayerTurn ? player : opponent;
        var defender = root.isPlayerTurn ? opponent : player;
        var attackerName = root.isPlayerTurn ? "Player" : "Opponent";

        update_text_bar(attackerName + " used Tackle!");

        // Start attack animation after brief delay
        delayedCall(200, function() {
            attacker.actionForward.running = true;
        });
    }

    // Monitor player attack animation
    Connections {
        target: player.actionForward
        enabled: root.attackInProgress && root.isPlayerTurn

        function onRunningChanged() {
            delayedCall(200, function() {
                opponent.takeDamage.running = true;
            });
        }
    }

    // Monitor opponent attack animation
    Connections {
        target: opponent.actionForward
        enabled: root.attackInProgress && !root.isPlayerTurn

        function onRunningChanged() {
            delayedCall(200, function() {
                player.takeDamage.running = true;
            });
        }
    }

    // Monitor player taking damage
    Connections {
        target: player.takeDamage
        enabled: root.attackInProgress && !root.isPlayerTurn

        function onRunningChanged() {
            handleTurnComplete();
        }
    }

    // Monitor opponent taking damage
    Connections {
        target: opponent.takeDamage
        enabled: root.attackInProgress && root.isPlayerTurn

        function onRunningChanged() {
            handleTurnComplete();
        }
    }

    // Handle completion of one turn
    function handleTurnComplete() {
        update_text_bar("It's super effective!");

        delayedCall(1200, function() {
            root.turnsCompleted++;

            if (root.turnsCompleted < 2) {
                // Switch turns and execute next attack
                root.isPlayerTurn = !root.isPlayerTurn;
                executeAttackTurn();
            } else {
                // Both turns complete, end battle
                endAttackChain();
            }
        });
    }

    // End the attack sequence
    function endAttackChain() {
        root.attackInProgress = false;
        root.turnsCompleted = 0;
        buttonGrid.visible = true;
        textBarText.visible = false;
        textBar.color = "transparent";
    }
}
