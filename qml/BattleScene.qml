// BattleScene.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
Item {
    id: root
    width: (direction === 0 || direction === 2) ? frameSize * 6 : frameSize * 8
    height: (direction === 0 || direction === 2) ? frameSize * 8 : frameSize * 6
    // Properties
    property int frameSize: 32
    property int menuWidth: frameSize * 6
    property int menuHeight: frameSize * 2.4
    property int statusBarWidth: frameSize*2.5
    property int statusBarHeight: 25
    property int buttonWidth: frameSize * 2
    property int buttonHeight: frameSize * 0.75
    property int gridSpacing: frameSize * 0.1
    property int pokeNameFontSize: 12
    property int buttonFontSize: 11
    property int moveFontSize: 10
    property int textBarFontSize: 14
    property string textBarFontFamily: "Arial"
    property string menuFontFamily: "Arial"
    property string statusBarFontFamily: "Arial"
    property bool debugLines: false
    property int direction: 0
    property bool safePokemonSwitch: true
    property alias opponent: opponent
    property alias player: player
    property alias statusBarOpponent: statusBarOpponent
    property alias statusBarPlayer: statusBarPlayer
    property alias opponentName: statusBarOpponent.pokeName
    property alias playerName: statusBarPlayer.pokeName
    // Action chain state
    property bool actionInProgress: false
    property var actionSequence: []
    property var tempActionSequence: []
    property int currentActionIndex: 0
    // Catch attempt state
    property int currentOpponentBallIndex: 0
    property int currentPlayerBallIndex: 0
    property int catchShakeInterval: 1500
    property int ballTransitionDuration: 750
    property int runEndDuration: 1000
    property bool catchAttemptActive: false
    signal _battleEnded(string endState);
    signal _startActionRound(int actionIndex, string actionState)

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
                sprite.y = root.height - (menuHeight + sprite.containerOffsetY + sprite.height - gridSpacing*3)//compensate for downward shift of menu by gridSpacin*3
                sprite.statusBar.x = root.width - root.statusBarWidth
                sprite.statusBar.y = sprite.y
                break
            case 1:
                sprite.x = root.width - (sprite.containerOffsetX + sprite.width)
                sprite.y = root.height - (sprite.height + menuHeight - gridSpacing*2)
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
                sprite.y = root.height - (sprite.height + menuHeight - gridSpacing*2)
                sprite.statusBar.x = root.frameSize/2
                sprite.statusBar.y = 0
                break
        }
        sprite.startingX = sprite.x
        sprite.startingY = sprite.y
    }
    Component.onCompleted: {
        positionSpriteAndStatusBar(player)
        positionSpriteAndStatusBar(opponent)
        // Schedule for next event loop
        Qt.callLater(function() {
            root.currentPlayerBallIndex = battleMenu.party.ballIds[0]
            resetPlayerBall()
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
        fontFamily: root.statusBarFontFamily
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
        fontFamily: root.statusBarFontFamily
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
        circleBaseWidth: opponent.width
        circleBaseHeight: opponent.height
        circleX: opponent.x + opponent.width/2
        circleY: opponent.y + opponent.height/2
        onThrowAnimationDone: {
            root.currentActionIndex = 0
            root.actionSequence = tempActionSequence
            sequenceTimer.interval = root.catchShakeInterval/2
            sequenceTimer.start()
        }
        onPokemonInsideBall:{
            pokeBallOpponent.circleShrink()
            opponent.visible=false
        }
    }
    Pokeball {
        id: pokeBallPlayer
    }
    function resetPlayerBall() {
        pokeBallPlayer.reset(root.currentPlayerBallIndex)
        pokeBallPlayer.visible = true
        pokeBallPlayer.circleBaseWidth = player.width
        pokeBallPlayer.circleBaseHeight = player.height
        pokeBallPlayer.circleX = player.x + player.width/2
        pokeBallPlayer.circleY = player.y + player.height/2
        pokeBallPlayer.scaleFactor = 2
        pokeBallPlayer.circleAnimationDuration = 1000
        pokeBallPlayer.delayReveal = 2
        pokeBallPlayer.onPokemonInsideBall.connect(function() {
            pokeBallPlayer.circleExpand()
        })
        pokeBallPlayer.onBallOpened.connect(function() {
            pokeBallPlayer.visible = false
            player.visible = true
            statusBarPlayer.visible = true
            if(root.safePokemonSwitch){
                battleMenu.resetToRoot()
            }else{
                _startActionRound(battleMenu.selectedIndex, "Switch");
            }
        })
    }
    // UI
    BattleMenu {
        id: battleMenu
        frameSize: root.frameSize
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        height: root.menuHeight
        width: root.menuWidth
        buttonWidth: root.buttonWidth
        buttonHeight: root.buttonHeight
        gridSpacing: root.gridSpacing
        menuHeight: root.menuHeight
        menuWidth: root.menuWidth
        buttonFontSize: root.buttonFontSize
        moveFontSize: root.moveFontSize
        textBarFontSize: root.textBarFontSize
        textBarFontFamily: root.textBarFontFamily
        menuFontFamily: root.menuFontFamily

        // Forward the startActionRound signal from BattleMenu to BattleScene
        onStartActionRound: function(actionIndex, actionType) {
            root._startActionRound(actionIndex, actionType)
        }

        // Legacy signal handlers (can be removed or kept for compatibility)
        onRunChosen: function(){
            battleMenu.showTextBar()
            battleMenu.updateText("Got away safely!")
            root.oneShotTimer(root.runEndDuration, function(){
                root._battleEnded("PlayerRun")
            })
        }

        onSwitchChosen: function(oldPartyId, newPartyId){
            battleMenu.party.healthRatios[oldPartyId] = statusBarPlayer.currentHealthRatio
            let newPlayerName = battleMenu.party.names[newPartyId]
            let newPlayerGeneration = battleMenu.party.gens[newPartyId]
            let newPlayerSpriteId = battleMenu.party.spriteIds[newPartyId]
            player.visible = false

            positionSpriteAndStatusBar(player)
            battleMenu.showTextBar()
            battleMenu.updateText("Go!" + " " + newPlayerName + "!")
            statusBarPlayer.pokeName = newPlayerName;
            statusBarPlayer.currentHealthRatio = battleMenu.party.healthRatios[newPartyId];
            statusBarPlayer.totalHealth = 100;
            root.currentPlayerBallIndex = battleMenu.party.ballIds[newPartyId]
            root.safePokemonSwitch = battleMenu.forceSwitchMode
            root.resetPlayerBall()
            var coords = calculateBallCoords(player)
            pokeBallPlayer.throwAt(coords[0], coords[1], coords[2], coords[3])
        }
    }

    function setPartyMember(partyId, spriteId, iconId, ballId, gen, pokemonName, moves) {
        battleMenu._setPartyMember(partyId, spriteId, iconId, ballId, gen, pokemonName, moves);
    }

    // Action sequence
    Timer {
        id: sequenceTimer
        repeat: false
        onTriggered: executeNextStep()
    }

    function executeActionSequence(sequence) {
        if (root.actionInProgress) return
        root.actionInProgress = true
        root.currentActionIndex = 0
        root.actionSequence = sequence
        executeNextStep()
    }

    // Execute the next step in the sequence
    function executeNextStep() {
        if (root.currentActionIndex >= root.actionSequence.length) {
            endActionChain()
            return
        }
        var step = root.actionSequence[root.currentActionIndex]

        console.log(root.currentActionIndex + " - " + step.type)
        root.currentActionIndex++
        switch(step.type) {
            case "text":
                battleMenu.updateText(step.message)
                sequenceTimer.interval = step.delay
                sequenceTimer.start()
                break
            case "attack":
                var attacker = (step.role === "player") ? player : opponent
                attacker.actionForward.running = true
                sequenceTimer.interval = step.delay
                sequenceTimer.start()
                break
            case "damage":
                var defender = (step.role === "player") ? player : opponent
                defender.takeDamage.running = true
                sequenceTimer.interval = step.delay
                sequenceTimer.start()
                break
            case "change-health":
                var target = (step.role === "player") ? player : opponent
                let currentHealthRatio = target.statusBar.incrementHealth(step.amount)
                sequenceTimer.interval = step.delay
                if(currentHealthRatio==0){
                    root.actionSequence = [
                        {type: "lose-battle", message: target.name + " fainted!", role: step.role, delay: 2000 },
                        {type: "battle-over", role: step.role, delay: 100 }
                    ]
                    root.currentActionIndex = 0
                }
                sequenceTimer.start()
                break
            case "lose-battle":
                battleMenu.updateText(step.message)
                var loser = (step.role === "player") ? player : opponent
                loser.visible = false
                sequenceTimer.interval = step.delay
                sequenceTimer.start()
                break

            case "attempt-catch":
                var newActions = step.shakes >= 3
                    ?
                [
                    {type: "succeed-catch", delay: 100},
                    {type: "text", message: "Gotcha! " + opponent.name + " was caught!", delay: 2000},
                    {type: "jump", delay: 2000}
                ]
                    :
                [
                    {type: "reveal-opponent", message: "Aargh! Almost had it!", delay: 1000},
                    {type: "fail-catch", delay: root.ballTransitionDuration}
                ]

                for (var i = 0; i < step.shakes; i++) {
                    newActions.push({type: "shake", delay: 2000});
                }

                root.tempActionSequence = newActions.reverse().concat(root.actionSequence.slice(2)); //slice off the opening text and attempt-catch
                root.actionSequence = []

                var coords = calculateBallCoords(opponent)
                pokeBallOpponent.visible = true
                pokeBallOpponent.throwAt(coords[0], coords[1], coords[2], coords[3])
                break;

            case "shake":
                 pokeBallOpponent.shake()
                 sequenceTimer.interval = step.delay
                 sequenceTimer.start()
                 break
            case "jump":
                 pokeBallOpponent.jump()
                 break
            case "fail-catch":
                 pokeBallOpponent.release()
                 sequenceTimer.interval = step.delay
                 sequenceTimer.start()
                 break
            case "reveal-opponent":
                opponent.visible = true
                pokeBallOpponent.visible = false
                battleMenu.updateText(step.message)
                sequenceTimer.interval = step.delay
                sequenceTimer.start()
                break

            case "succeed-catch":
                root._battleEnded("OpponentCaught")
                break
            case "battle-over":
                root.actionInProgress = false
                root.actionSequence = []
                root.currentActionIndex = 0
                if(step.role === "opponent"){
                    root._battleEnded("PlayerWon")
                }else{
                    if(battleMenu.party.healthRatios.some(ratio => ratio > 0)){
                        battleMenu.forceSwitch();
                    }else{
                        root._battleEnded("OpponentWon")
                    }
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
            hideTimer.interval = duration
            hideTimer.triggered.connect(function() {
                onFinish()
                hideTimer.destroy()
            })
            hideTimer.start()
        })
    }

    function showTextBar(){battleMenu.showTextBar()}
}
