import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: (direction === 0 || direction === 2) ? frameSize * 6 : frameSize * 8
    height: (direction === 0 || direction === 2) ? frameSize * 8 : frameSize * 6

    // Properties
    property int frameSize: 32

    property int menuWidth: frameSize * 6
    property int menuHeight: 50
    property int statusBarWidth: frameSize*2.5
    property int statusBarHeight: 25
    property int buttonWidth: frameSize * 2
    property int buttonHeight: frameSize * 0.75
    property int gridSpacing: frameSize * 0.1

    property int pokeNameFontSize: frameSize * 0.40
    property int buttonFontSize: frameSize * 0.40
    property int textBarFontSize: frameSize * 0.45

    property bool debugLines: false
    property int direction: 0

    // Action timing delays (delays are applied AFTER action)
    property int textDelay: 300
    property int attackDelay: 500
    property int damageDelay: 200
    property int healthChangeDelay: 1000
    property int effectiveTextDelay: 1200


    property alias opponent: opponent
    property alias player: player
    property alias statusBarOpponent: statusBarOpponent
    property alias statusBarPlayer: statusBarPlayer
    property alias opponentName: statusBarOpponent.pokeName
    property alias playerName: statusBarPlayer.pokeName

    // Action chain state
    property bool actionInProgress: false
    property var actionSequence: []
    property int currentActionIndex: 0

    // Catch attempt state
    property int catchShakeCount: 0
    property int catchShakeInterval: 1500
    property int ballTransitionDuration: 750
    property bool catchAttemptActive: false

    signal runChosen()
    signal opponentWon()
    signal playerWon()
    signal pokemonCaught()



    Component.onCompleted: {
        positionSpriteAndStatusBar(player)
        positionSpriteAndStatusBar(opponent)

        // Schedule for next event loop
        Qt.callLater(function() {
            battleMenu.showTextBar()
            battleMenu.updateText(playerName + ", I choose you!")
            var coords = calculateBallCoords(player)
            player.visible = false
            pokeBallPlayer.throwAt(coords[0], coords[1], coords[2], coords[3])
        })
    }
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
        pokeNameFontSize: root.pokeNameFontSize
        width: root.statusBarWidth
        height: root.statusBarHeight
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
        pokeNameFontSize: root.pokeNameFontSize
        width: root.statusBarWidth
        height: root.statusBarHeight
    }

    PokemonSprite {
        id: player
        objectName: "player"
        visible: false
        name: playerName
        direction: (root.direction + 2) % 4
        debugLines: root.debugLines
        debugColor: "blue"
        property alias statusBar: root.statusBarPlayer
    }

    // Pokéball animation component
    Pokeball {
        id: pokeBallOpponent
        scaleFactor: 2

        // Configure circle properties
        circleBaseRadius: Math.max(opponent.width/2, opponent.height/2)
        circleX: opponent.x + opponent.width/2  // Center on opponent
        circleY: opponent.y + opponent.height/2

        onThrowAnimationDone: {
            root.catchShakeCount = 0
            catchAttemptTimer.interval = root.catchShakeInterval/2
            catchAttemptTimer.start()
        }
        onPokemonInsideBall:{
            pokeBallOpponent.circleShrink()
            opponent.visible=false
        }
    }
    Pokeball {
        id: pokeBallPlayer
        scaleFactor: 2

        // Configure circle properties
        circleBaseRadius: Math.max(player.width/2, player.height/2)
        circleAnimationDuration: 1000
        delayReveal: 2 //longer animation for player reveal
        circleX: player.x + player.width/2  // Center on player
        circleY: player.y + player.height/2

        onPokemonInsideBall:{
            pokeBallPlayer.circleExpand()
        }
        onBallOpened:{
            pokeBallPlayer.visible=false
            player.visible=true
            statusBarPlayer.visible = true
            battleMenu.resetToRoot()
        }
    }
    // UI
    BattleMenu {
        id: battleMenu
        frameSize: root.frameSize
        buttonFontSize: root.buttonFontSize
        textBarFontSize: root.textBarFontSize
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        height: root.menuHeight
        width: root.menuWidth
        buttonWidth: root.buttonWidth
        buttonHeight: root.buttonHeight
        gridSpacing: root.gridSpacing
        menuHeight: root.menuHeight
        menuWidth: root.menuWidth
        onAttackChosen: function(attackId) {
            if (attackId === 0) {
                var playerFirst = Math.random() < 0.5;
                startActionChain("attack", playerFirst)
            } else {
                console.error("Invalid attack id:", attackId)
            }
        }

        onCatchChosen: function(pokeSpriteId) {
            if (pokeSpriteId === 3) {
                startActionChain("catch")
            } else {
                console.error("Invalid pokeSprite id:", pokeSpriteId)
            }
        }

        onRunChosen: function(){
            battleMenu.showTextBar()
            battleMenu.updateText("Got away safely!")
            root.oneShotTimer(3000, function(){
                root.runChosen()
            })
        }
    }

    //Relative positioning of elements
    function positionSpriteAndStatusBar(sprite) {
        // Clear all anchors first
        sprite.statusBar.anchors.left = undefined
        sprite.statusBar.anchors.right = undefined
        sprite.statusBar.anchors.top = undefined
        sprite.statusBar.anchors.bottom = undefined

        switch(sprite.direction) {
            case 0:
                sprite.x = (root.width - root.statusBarWidth - sprite.width)/2
                sprite.y = root.height - (menuHeight + sprite.containerOffsetY + sprite.height)
                sprite.statusBar.x = root.width - root.statusBarWidth
                sprite.statusBar.y = sprite.y
                break
            case 1:
                sprite.x = root.width - (sprite.containerOffsetX + sprite.width)
                sprite.y = root.height - (sprite.height + menuHeight*1.2) //arbitrary 0.2 increase to raise the sprites
                sprite.statusBar.x = root.width - root.frameSize/2 - root.statusBarWidth
                sprite.statusBar.y = 0
                break
            case 2:
                sprite.x = (root.width - root.statusBarWidth - sprite.width)/2
                sprite.y = sprite.containerOffsetY
                sprite.statusBar.x = root.width - root.statusBarWidth
                sprite.statusBar.y = sprite.y
                break
            case 3:
                sprite.x = sprite.containerOffsetX
                sprite.y = root.height - (sprite.height + menuHeight*1.2)
                sprite.statusBar.x = root.frameSize/2
                sprite.statusBar.y = 0
                break
        }

        sprite.startingX = sprite.x
        sprite.startingY = sprite.y
        sprite.statusBar.visible = true
    }
    function setPartyMember(partyIdx, iconId, pokemonName) {
       battleMenu._setPartyMember(partyIdx, iconId, pokemonName);
    }


    // Action sequence
    Timer {
        id: sequenceTimer
        repeat: false
        onTriggered: executeNextStep()
    }

    function scheduleNext(delay, func) {
        sequenceTimer.interval = delay
        sequenceTimer.callback = func
        sequenceTimer.start()
    }

    // Build and start the action sequence (attack or catch)
    function startActionChain(actionType, playerFirst) {
        if (root.actionInProgress) return

        battleMenu.showTextBar()
        root.actionInProgress = true
        root.currentActionIndex = 0

        if (actionType === "catch") {
            // Catch sequence
            root.catchAttemptActive = true
            root.catchShakeCount = 0
            var coords = calculateBallCoords(opponent)

            root.actionSequence = [
                { type: "text", message: "Player used one Poké Ball!", delay: 300 },
                { type: "throw-ball", coords: coords, delay: 500 },
                { type: "wait-catch-result" } // Special step that waits for catch timer
            ]
        } else {
            // Attack sequence
            var firstAttacker = playerFirst ? player : opponent
            var firstDefender = playerFirst ? opponent : player
            var firstAttackerName = playerFirst ? playerName : opponentName
            var secondAttacker = playerFirst ? opponent : player
            var secondDefender = playerFirst ? player : opponent
            var secondAttackerName = playerFirst ? opponentName : playerName

            root.actionSequence = [
                // First turn
                { type: "text", message: firstAttackerName + " used Tackle!", delay: textDelay },
                { type: "attack", attacker: firstAttacker, delay: attackDelay },
                { type: "damage", defender: firstDefender, delay: damageDelay },
                { type: "change-health", defender: firstDefender, delay: healthChangeDelay },
                { type: "text", message: "It's super effective!", delay: effectiveTextDelay },
                // Second turn
                { type: "text", message: secondAttackerName + " used Tackle!", delay: textDelay },
                { type: "attack", attacker: secondAttacker, delay: attackDelay },
                { type: "damage", defender: secondDefender, delay: damageDelay },
                { type: "change-health", defender: secondDefender, delay: healthChangeDelay },
                { type: "text", message: "It's super effective!", delay: effectiveTextDelay },
                // End
                { type: "end" }
            ]
        }

        executeNextStep()
    }

    // Execute the next step in the sequence
    function executeNextStep() {
        if (root.currentActionIndex >= root.actionSequence.length) {
            endActionChain()
            return
        }

        var step = root.actionSequence[root.currentActionIndex]
        root.currentActionIndex++

        switch(step.type) {
            case "text":
                battleMenu.updateText(step.message)
                sequenceTimer.interval = step.delay
                sequenceTimer.start()
                break
            case "throw-ball":
                pokeBallOpponent.visible = true
                pokeBallOpponent.throwAt(step.coords[0], step.coords[1], step.coords[2], step.coords[3])
                sequenceTimer.interval = step.delay
                sequenceTimer.start()
                break
            case "wait-catch-result":
                // Don't proceed - wait for catch timer to resolve
                break
            case "attack":
                step.attacker.actionForward.running = true
                sequenceTimer.interval = step.delay
                sequenceTimer.start()
                break
            case "damage":
                step.defender.takeDamage.running = true
                sequenceTimer.interval = step.delay
                sequenceTimer.start()
                break
            case "change-health":
                let currentHealthRatio = step.defender.statusBar.incrementHealth(-75)
                sequenceTimer.interval = step.delay
                if(currentHealthRatio==0){
                    root.actionSequence = [
                        {type: "lose-battle", message: step.defender.name + " fainted!", defender: step.defender, delay: 2000 },
                        {type: "battle-over", defender: step.defender, delay: 100 }
                    ]
                    root.currentActionIndex = 0
                }
                sequenceTimer.start()
                break
            case "lose-battle":
                battleMenu.updateText(step.message)
                step.defender.visible = false
                sequenceTimer.interval = step.delay
                sequenceTimer.start()
                break
            case "battle-over":
                if(step.defender==root.opponent){
                    root.playerWon()
                }else{
                    root.opponentWon()
                }
                break
            case "end":
                endActionChain()
                break
        }
    }

    // End the action sequence
    function endActionChain() {
        root.actionInProgress = false
        root.actionSequence = []
        root.currentActionIndex = 0
        root.catchAttemptActive = false
        battleMenu.resetToRoot()
    }





    // Catch attempt sequence
    Timer {
        id: catchAttemptTimer
        interval: root.catchShakeInterval
        repeat: false
        onTriggered: root.processCatchAttempt()
    }


    function processCatchAttempt() {
        var failureRate = 0.05 + (0.7 * statusBarOpponent.currentHealthRatio); // 75% fail at full HP, 10% at 1 HP
        var failure = Math.random() < failureRate;

        if (failure) {
            // Release the pokemon and opponent attacks
            pokeBallOpponent.release()
            battleMenu.updateText("Aargh! Almost had it!")

            root.oneShotTimer(root.ballTransitionDuration, function() {
                opponent.visible = true
                pokeBallOpponent.visible = false

                root.actionSequence = [
                    { type: "text", message: battleMenu.getText(), delay: 300 },//hack to get a bit more delay after failed catch
                    { type: "text", message: opponentName + " used Tackle!", delay: textDelay },
                    { type: "attack", attacker: opponent, delay: attackDelay },
                    { type: "damage", defender: player, delay: damageDelay },
                    { type: "change-health", defender: player, delay: healthChangeDelay },
                    { type: "text", message: "It's super effective!", delay: effectiveTextDelay },
                    { type: "end" }
                ]
                root.currentActionIndex = 0
                executeNextStep()
            })

        } else {
            root.catchShakeCount++

            if (root.catchShakeCount >= 3) {
                pokeBallOpponent.jump()
                battleMenu.updateText("Gotcha! " + opponentName + " was caught!")

                root.oneShotTimer(2000, function() {
                    root.pokemonCaught()
                })
            } else {
                // Shake and try again
                pokeBallOpponent.shake()
                catchAttemptTimer.start()
            }
        }
        catchAttemptTimer.interval = root.catchShakeInterval
    }


    function calculateBallCoords(sprite){
            var x1 = sprite.x + (sprite.width / 2) - (root.frameSize/4)
            var x0 = x1 + 2*(sprite.direction==1 ? -root.frameSize : root.frameSize)
            // Y positions
            var y0 = Math.max(0, sprite.y - pokeBallOpponent.frameHeight)
            var y1 = sprite.y + sprite.height - pokeBallOpponent.frameHeight
            return [x0, x1, y0, y1]
    }

    function oneShotTimer(duration, onFinish){
            Qt.callLater(function() {
                var hideTimer = Qt.createQmlObject('import QtQuick 2.15; Timer {}', root)
                hideTimer.interval = root.ballTransitionDuration
                hideTimer.triggered.connect(function() {
                    onFinish()
                    hideTimer.destroy()
                })
                hideTimer.start()
            })
    }
}
