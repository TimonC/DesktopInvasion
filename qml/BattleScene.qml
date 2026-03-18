import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: (direction === 0 || direction === 2) ? frameSize * 6 : frameSize * 8
    height: (direction === 0 || direction === 2) ? frameSize * 8 : frameSize * 6

    // Properties
    property int frameSize: 32
    property int buttonWidth: frameSize * 2
    property int buttonHeight: frameSize * 0.75
    property int buttonFontSize: frameSize * 0.4
    property int gridSpacing: frameSize * 0.1
    property int pokeMargin: frameSize * 0.25
    property bool debugLines: false
    property int menuHeight: 50
    property int menuWidth: frameSize * 5
    property int direction: 0

    property alias opponent: opponent
    property alias player: player
    property alias statusBarOpponent: statusBarOpponent
    property alias statusBarPlayer: statusBarPlayer
    property alias opponentName: statusBarOpponent.pokeName
    property alias playerName: statusBarPlayer.pokeName

    // Attack chain state
    property bool attackInProgress: false
    property var attackSequence: []
    property int currentAttackIndex: 0

    // Catch attempt state
    property int catchShakeCount: 0
    property int catchShakeInterval: 1500
    property int catchReleaseDelay: 500

    signal runChosen()
    signal opponentWon()
    signal playerWon()

    // Debug rectangle
    Rectangle {
        id: containerDebugLines
        anchors.fill: parent
        color: "transparent"
        border.color: debugLines ? "green" : "transparent"
        border.width: 1
    }

    // Sprites
    StatusBar {
        id: statusBarOpponent
    }

    PokemonSprite {
        id: opponent
        objectName: "opponent"
        name: opponentName
        direction: root.direction
        debugLines: root.debugLines
        debugColor: "red"
        property alias statusBar: root.statusBarOpponent
    }

    StatusBar {
        id: statusBarPlayer
    }

    PokemonSprite {
        id: player
        objectName: "player"
        name: playerName
        direction: (root.direction + 2) % 4
        debugLines: root.debugLines
        debugColor: "blue"
        property alias statusBar: root.statusBarPlayer
    }

    // Pokéball animation component
    PokeballCatch {
        id: pokeBallSend
        scaleFactor: 2

        onThrowAnimationDone: {
            root.catchShakeCount = 0
            catchAttemptTimer.interval = root.catchShakeInterval/2 //half duration on the first shake
            catchAttemptTimer.start()
        }
        onPokemonInsideBall:{
            opponent.visible = false
        }
    }

    // Catch attempt timer
    Timer {
        id: catchAttemptTimer
        interval: root.catchShakeInterval
        repeat: false
        onTriggered: root.processCatchAttempt()
    }


    function processCatchAttempt() {
        var failure = Math.random() < 0.1

        if (failure) {
            // Release the pokemon
            pokeBallSend.release()
            opponent.visible = true
            // Hide after delay
            Qt.callLater(function() {
                var hideTimer = Qt.createQmlObject('import QtQuick 2.15; Timer {}', root)
                hideTimer.interval = root.catchReleaseDelay
                hideTimer.triggered.connect(function() {
                    pokeBallSend.visible = false
                    hideTimer.destroy()
                })
                hideTimer.start()
            })
        } else {
            root.catchShakeCount++

            if (root.catchShakeCount >= 3) {
                // Third attempt - jump instead of shake
                pokeBallSend.jump()
                // Pokemon caught!
                console.log("Pokemon caught!")
            } else {
                // Shake and try again
                pokeBallSend.shake()
                catchAttemptTimer.start()
            }
        }
        catchAttemptTimer.interval = root.catchShakeInterval
    }

    // UI
    BattleMenu {
        id: battleMenu
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        height: root.menuHeight
        width: root.menuWidth
        frameSize: root.frameSize
        buttonWidth: root.buttonWidth
        buttonHeight: root.buttonHeight
        buttonFontSize: root.buttonFontSize
        gridSpacing: root.gridSpacing
        menuHeight: root.menuHeight
        menuWidth: root.menuWidth

        onAttackChosen: function(attackId) {
            if (attackId === 0) {
                var playerFirst = false;// Math.random() < 0.5;
                startAttackChain(playerFirst);
            } else {
                console.error("Invalid attack id:", attackId)
            }
        }

        onCatchChosen: function(pokeSpriteId) {
            if (pokeSpriteId === 3) {
                // Calculate center X of opponent sprite (this becomes x1)
                var x1 = opponent.x + (opponent.width / 2) - (32/4);
                var x0 = x1 + 2*(root.direction==1 ? -32 : 32);
                // Y positions
                var y0 = opponent.y - pokeBallSend.frameHeight;
                var y1 = opponent.y + opponent.height - pokeBallSend.frameHeight;

                // Start the animation
                pokeBallSend.visible = true;
                pokeBallSend.throwAt(x0, x1, y0, y1);
            } else {
                console.error("Invalid pokeSprite id:", pokeSpriteId)
            }
        }

        onRunChosen: root.runChosen()
    }

    //Relative positioning of elements
    function positionSpriteAndStatusBar(sprite) {
        var margin = root.pokeMargin;

        // Clear all anchors first
        sprite.statusBar.anchors.left = undefined
        sprite.statusBar.anchors.right = undefined
        sprite.statusBar.anchors.top = undefined
        sprite.statusBar.anchors.bottom = undefined

        switch(sprite.direction) {
            case 0:
                sprite.x = margin + sprite.containerOffsetX;
                sprite.y = root.height - (menuHeight + margin + sprite.containerOffsetY + sprite.height);
                sprite.statusBar.x = 3*32 - margin
                sprite.statusBar.y = sprite.y
                break;
            case 1:
                sprite.x = root.width - (margin + sprite.containerOffsetX + sprite.width);
                sprite.y = root.height - (menuHeight + margin + sprite.height);
                sprite.statusBar.x = root.width - 32*2.5 - margin
                sprite.statusBar.y = (root.height - menuHeight - 3*32)/2
                break;
            case 2:
                sprite.x = margin + sprite.containerOffsetX;
                sprite.y = margin + sprite.containerOffsetY;
                sprite.statusBar.x = 3*32 - margin
                sprite.statusBar.y = sprite.y
                break;
            case 3:
                sprite.x = margin + sprite.containerOffsetX;
                sprite.y = root.height - (menuHeight + margin + sprite.height);
                sprite.statusBar.x = 32/2 + margin
                sprite.statusBar.y = (root.height - menuHeight - 3*32)/2
                break;
        }

        sprite.startingX = sprite.x;
        sprite.startingY = sprite.y;
        sprite.statusBar.visible = true;
    }

    Component.onCompleted: {
        positionSpriteAndStatusBar(player);
        positionSpriteAndStatusBar(opponent);
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

    // Build and start the attack sequence
    function startAttackChain(playerFirst) {
        if (root.attackInProgress) return;

        battleMenu.showTextBar();
        root.attackInProgress = true;
        root.currentAttackIndex = 0;

        // Build attack sequence
        var firstAttacker = playerFirst ? player : opponent;
        var firstDefender = playerFirst ? opponent : player;
        var firstAttackerName = playerFirst ? playerName : opponentName;
        var secondAttacker = playerFirst ? opponent : player;
        var secondDefender = playerFirst ? player : opponent;
        var secondAttackerName = playerFirst ? opponentName : playerName;

        root.attackSequence = [
            // First turn
            { type: "text", message: firstAttackerName + " used Tackle!", delay: 300 },
            { type: "attack", attacker: firstAttacker, delay: 500 },
            { type: "damage", defender: firstDefender, delay: 200 },
            { type: "change-health", defender: firstDefender, delay: 1000 },
            { type: "text", message: "It's super effective!", delay: 1200 },
            // Second turn
            { type: "text", message: secondAttackerName + " used Tackle!", delay: 300 },
            { type: "attack", attacker: secondAttacker, delay: 500 },
            { type: "damage", defender: secondDefender, delay: 200 },
            { type: "change-health", defender: secondDefender, delay: 1000 },
            { type: "text", message: "It's super effective!", delay: 1200 },
            // End
            { type: "end" }
        ];

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
                battleMenu.updateText(step.message);
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
            case "change-health":
                let currentHealthRatio = step.defender.statusBar.incrementHealth(-75);
                sequenceTimer.interval = step.delay;
                if(currentHealthRatio==0){
                    root.attackSequence = [
                        {type: "lose-battle", message: step.defender.name + " fainted!", defender: step.defender, delay: 2000 },
                        {type: "battle-over", defender: step.defender, delay: 100 }
                    ]
                    root.currentAttackIndex = 0;
                }
                sequenceTimer.start();
                break;
            case "lose-battle":
                battleMenu.updateText(step.message);
                step.defender.visible = false;
                sequenceTimer.interval = step.delay;
                sequenceTimer.start();
                break;
            case "battle-over":
                if(step.defender==root.opponent){
                    root.playerWon()
                }else{
                    root.opponentWon()
                }
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
        battleMenu.resetToRoot();
    }
}
