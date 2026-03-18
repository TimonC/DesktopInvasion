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
    property alias hpBarOpponent: hpBarOpponent
    property alias hpBarPlayer: hpBarPlayer
    property alias textBar: textBar
    property alias buttonGrid: buttonGrid

    // Attack chain state
    property bool attackInProgress: false
    property var attackSequence: []
    property int currentAttackIndex: 0

    signal runClicked()

    // Debug rectangle
    Rectangle {
        id: containerDebugLines
        anchors.fill: parent
        color: "transparent"
        border.color: debugLines ? "green" : "transparent"
        border.width: 1
    }

    // Sprites
    HealthBar{
        id: hpBarOpponent
    }

    PokemonSprite {
        id: opponent
        objectName: "opponent"
        direction: root.direction
        debugLines: root.debugLines
        debugColor: "red"
        property alias healthBar: root.hpBarOpponent
    }

   HealthBar{
       id: hpBarPlayer
    }

    PokemonSprite {
        id: player
        objectName: "player"
        direction: (root.direction + 2) % 4
        debugLines: root.debugLines
        debugColor: "blue"
        property alias healthBar: root.hpBarPlayer
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
        radius: 5
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
            } } }


    function positionSpriteAndHealthbar(sprite) {
        var margin = root.pokeMargin;

        // Clear all anchors first
        sprite.healthBar.anchors.left = undefined
        sprite.healthBar.anchors.right = undefined
        sprite.healthBar.anchors.top = undefined
        sprite.healthBar.anchors.bottom = undefined

        switch(sprite.direction) {
            case 0:
                sprite.x = margin + sprite.containerOffsetX;
                sprite.y = root.height - (textBoxHeight + margin + sprite.containerOffsetY + sprite.height);
                sprite.healthBar.x = 3*32 - margin
                sprite.healthBar.y = sprite.y
                break;
            case 1:
                sprite.x = root.width - (margin + sprite.containerOffsetX + sprite.width);
                sprite.y = root.height - (textBoxHeight + margin + sprite.height);
                sprite.healthBar.x = root.width - 32 - margin
                sprite.healthBar.y = root.height - (textBoxHeight + 3*32 - margin)
                break;
            case 2:
                sprite.x = margin + sprite.containerOffsetX;
                sprite.y = margin + sprite.containerOffsetY;
                sprite.healthBar.x = 3*32 - margin
                sprite.healthBar.y = sprite.y
                break;
            case 3:
                sprite.x = margin + sprite.containerOffsetX;
                sprite.y = root.height - (textBoxHeight + margin + sprite.height);
                sprite.healthBar.x = 32 + margin
                sprite.healthBar.y = root.height - (textBoxHeight + 3*32 - margin)
                break;
        }
        sprite.startingX = sprite.x;
        sprite.startingY = sprite.y;
        sprite.healthBar.visible = true;
    }


    Component.onCompleted: {
        positionSpriteAndHealthbar(player);
        positionSpriteAndHealthbar(opponent);
    }

    function update_text_bar(newText) {
        textBar.text = newText;
    }

    // Main sequence timer
    Timer {
        id: sequenceTimer
        repeat: false
        onTriggered: executeNextStep()
    }

    function scheduleNext(delay, func) {
        sequenceTimer.interval = delay;
        sequenceTimer.callback = func;
        sequenceTimer.start();
    }

    // Attack button handler
    function onAttackButtonClicked() {
        var playerFirst = Math.random() < 0.5;
        startAttackChain(playerFirst);
    }

    // Build and start the attack sequence
    function startAttackChain(playerFirst) {
        if (root.attackInProgress) return;

        root.attackInProgress = true;
        root.currentAttackIndex = 0;

        // Build attack sequence
        var firstAttacker = playerFirst ? player : opponent;
        var firstDefender = playerFirst ? opponent : player;
        var firstAttackerName = playerFirst ? "Player" : "Opponent";

        var secondAttacker = playerFirst ? opponent : player;
        var secondDefender = playerFirst ? player : opponent;
        var secondAttackerName = playerFirst ? "Opponent" : "Player";

        root.attackSequence = [
            // First turn
            { type: "text", message: firstAttackerName + " used Tackle!", delay: 300 },
            { type: "attack", attacker: firstAttacker, delay: 500 },
            { type: "damage", defender: firstDefender, delay: 500 },
            { type: "text", message: "It's super effective!", delay: 1200 },

            // Second turn
            { type: "text", message: secondAttackerName + " used Tackle!", delay: 300 },
            { type: "attack", attacker: secondAttacker, delay: 500 },
            { type: "damage", defender: secondDefender, delay: 500 },
            { type: "text", message: "It's super effective!", delay: 1200 },

            // End
            { type: "end" }
        ];

        buttonGrid.visible = false;
        textBar.color = "darkgrey";
        textBarText.visible = true;

        executeNextStep();
    }

    // Execute the next step in the sequence
    function executeNextStep() {
        if (root.currentAttackIndex >= root.attackSequence.length) {
            endAttackChain();
            return;
        }

        var step = root.attackSequence[root.currentAttackIndex];
        root.currentAttackIndex++;

        switch(step.type) {
            case "text":
                update_text_bar(step.message);
                sequenceTimer.interval = step.delay;
                sequenceTimer.start();
                break;

            case "attack":
                step.attacker.actionForward.running = true;
                sequenceTimer.interval = step.delay;
                sequenceTimer.start();
                break;

            case "damage":
                step.defender.takeDamage.running = true;
                sequenceTimer.interval = step.delay;
                sequenceTimer.start();
                break;

            case "end":
                endAttackChain();
                break;
        }
    }

    // End the attack sequence
    function endAttackChain() {
        root.attackInProgress = false;
        root.attackSequence = [];
        root.currentAttackIndex = 0;
        buttonGrid.visible = true;
        textBarText.visible = false;
        textBar.color = "transparent";
    }
}
