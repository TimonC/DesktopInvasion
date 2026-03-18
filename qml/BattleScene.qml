import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root

    property  int scale: 1
    property int frameSize: 32*scale

    width: (direction === 0 || direction === 2) ? frameSize * 6.5 : frameSize * 8
    height: (direction === 0 || direction === 2) ? frameSize * 8 : frameSize * 6.5

    property string textBarFontFamily:  "DotGothic16"
    property string menuFontFamily: "Press Start 2P"
    property string statusBarFontFamily: "Press Start 2P"

    property int menuWidth: frameSize * 6.5
    property int menuHeight: frameSize * 2
    property int statusBarWidth: frameSize*2.5
    property int statusBarHeight: frameSize*1.2
    property int buttonWidth: frameSize * 2.25
    property int buttonHeight: frameSize * 0.75
    property int gridSpacing: frameSize * 0.1

    property int pokeNameFontSize:9*scale
    property int buttonFontSize:9*scale
    property int moveFontSize:8*scale
    property int textBarFontSize: 15*scale

    property bool debugLines: false

    property int direction: 0
    property bool safePokemonSwitch: true
    property alias opponent: opponent
    property alias player: player
    property alias statusBarOpponent: statusBarOpponent
    property alias statusBarPlayer: statusBarPlayer
    property alias opponentName: statusBarOpponent.pokeName
    property alias playerName: statusBarPlayer.pokeName
    property alias opponentLevelText: statusBarOpponent.levelText
    property alias playerLevelText: statusBarPlayer.levelText
    property bool actionInProgress: false
    property var actionSequence: []
    property var tempActionSequence: []
    property int currentActionIndex: 0
    property int currentOpponentBallIndex: 0
    property int currentPlayerBallIndex: 0
    property int catchShakeInterval: 1500
    property int ballTransitionDuration: 750
    property int runEndDuration: 1000
    property bool catchAttemptActive: false
    property var ballOpenedConnection: null
    property var pokemonInsideBallConnection: null
    signal _battleEnded(string endState, bool removeWild);
    signal signalToStartActionRound(int actionIndex, string actionState)
    signal switchedPokemon(int pokedexId, int partyIndex)
    signal requestExperienceSpread();

    function setInitialTotalHealth(opponentTotalHealth, playerTotalHealth){
        opponent.statusBar.totalHealth  = opponentTotalHealth
        player.statusBar.totalHealth    = playerTotalHealth
    }

    function updatePlayerStatusAilment(label){
        player.statusBar.changeStatusCondition(label, label=="")
    }
    function positionSpriteAndStatusBar(sprite) {
        switch(sprite.direction) {
            case 0:
                sprite.x = (root.width - root.statusBarWidth - sprite.width)/2
                sprite.y = root.height - root.menuHeight - sprite.height - root.frameSize/4
                sprite.statusBar.x = root.width - root.statusBarWidth
                sprite.statusBar.y = root.height - root.menuHeight - root.statusBarHeight-root.frameSize*2
                break
            case 1:
                sprite.x = root.width - root.frameSize/2 - root.statusBarWidth/2 - sprite.width/2
                sprite.y = root.height - sprite.height - root.menuHeight - root.frameSize/4
                sprite.statusBar.x = root.width - root.frameSize/2 - root.statusBarWidth
                sprite.statusBar.y = 0
                break
            case 2:
                sprite.x = (root.width - root.statusBarWidth - sprite.width)/2
                sprite.y = 0
                sprite.statusBar.x = root.width - root.statusBarWidth
                sprite.statusBar.y = sprite.y
                break
            case 3:
                sprite.x = root.frameSize/2 + root.statusBarWidth/2 - sprite.width/2
                sprite.y = root.height - sprite.height - root.menuHeight - root.frameSize/4
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

        Qt.callLater(function() {
            root.currentPlayerBallIndex = battleMenu.party.ballIds[0]
            root.resetPlayerBall()
            battleMenu.showTextBar()
            battleMenu.updateText(playerName + ", I choose you!")
            var coords = calculateBallCoords(player)
            player.visible = false

            pokeBallPlayer.throwAt(coords[0], coords[1], coords[2], coords[3])
        })
    }

    Rectangle {
        id: containerDebugLines
        anchors.fill: parent
        color: "transparent"
        border.color: debugLines ? "yellow" : "transparent"
        border.width: 1
    }
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

    Pokeball {
        id: pokeBallOpponent
        scaleFactor: root.scale
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
        scaleFactor: root.scale
    }

        function resetPlayerBall() {
            pokeBallPlayer.reset(root.currentPlayerBallIndex)
            pokeBallPlayer.visible = true

            pokeBallPlayer.circleX = player.x + player.width/2
            pokeBallPlayer.circleY = player.y + player.height/2

            pokeBallPlayer.circleBaseWidth = player.width
            pokeBallPlayer.circleBaseHeight = player.height
            if (pokeBallPlayer.circleY - pokeBallPlayer.circleBaseHeight/2 < 0) {
                pokeBallPlayer.circleBaseHeight = pokeBallPlayer.circleY * 2
            }

            pokeBallPlayer.delayReveal = 2

            if (root.pokemonInsideBallConnection) {
                pokeBallPlayer.onPokemonInsideBall.disconnect(root.pokemonInsideBallConnection)
            }
            if (root.ballOpenedConnection) {
                pokeBallPlayer.onBallOpened.disconnect(root.ballOpenedConnection)
            }

            root.pokemonInsideBallConnection = function() {
                pokeBallPlayer.circleExpand()
            }
            root.ballOpenedConnection = function() {
                pokeBallPlayer.visible = false
                player.visible = true
                statusBarPlayer.visible = true
                if(root.safePokemonSwitch){
                    battleMenu.resetToRoot()
                }else{
                    root.signalToStartActionRound(battleMenu.selectedIndex, "Switch");
                }
            }

            pokeBallPlayer.onPokemonInsideBall.connect(root.pokemonInsideBallConnection)
            pokeBallPlayer.onBallOpened.connect(root.ballOpenedConnection)
        }

    BattleMenu {
        id: battleMenu
        iconScale: root.scale
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
        opponentName: root.opponentName
        onActionRound: function(actionIndex, actionType) {
            root.signalToStartActionRound(actionIndex, actionType)
        }
        onRunChosen: function(removeWild){
            battleMenu.showTextBar()
            battleMenu.updateText("Got away safely!")
            root.oneShotTimer(root.runEndDuration, function(){
                root._battleEnded("PlayerRun", removeWild)
            })
        }
        onSwitchChosen: function(newPartyId){
            let newPlayerName = battleMenu.party.names[newPartyId]
            let newPlayerPokedexId = battleMenu.party.pokedexIds[newPartyId]
            player.visible = false

            root.switchedPokemon(newPlayerPokedexId, newPartyId)
            positionSpriteAndStatusBar(player)

            battleMenu.showTextBar()
            battleMenu.updateText("Go!" + " " + newPlayerName + "!")

            statusBarPlayer.pokeName = newPlayerName
            statusBarPlayer.currentHealthRatio = battleMenu.party.healthRatios[newPartyId]
            statusBarPlayer.totalHealth = battleMenu.party.healthTotals[newPartyId]
            statusBarPlayer.setLevelText(battleMenu.party.lvls[newPartyId])

            root.currentPlayerBallIndex = battleMenu.party.ballIds[newPartyId]
            root.safePokemonSwitch = battleMenu.forceSwitchMode
            root.resetPlayerBall()
            var coords = calculateBallCoords(player)
            pokeBallPlayer.throwAt(coords[0], coords[1], coords[2], coords[3])
        }
    }

    function setPartyMember(partyId, pokedexId, spriteId, ballId, pokemonName, lvl, totalHealth, moves) {
        battleMenu._setPartyMember(partyId, pokedexId, spriteId, ballId, pokemonName, lvl, totalHealth, moves);
    }

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

    function executeNextStep() {
        if (root.currentActionIndex >= root.actionSequence.length) {
            endActionChain()
            return
        }
        var step = root.actionSequence[root.currentActionIndex]
        var logStr="→ [" + (root.currentActionIndex + 1) + "/" + root.actionSequence.length + "] " + step.type.toUpperCase()
        if (step.role) logStr += " (" + step.role + ")"
        if (step.message) logStr += " [\"" + step.message + "\"]"
        if (step.amount) logStr += " [" + step.amount + "]"
        if (step.delay > 0) logStr += " [" + step.delay + "ms]"
        console.log(logStr)

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

            case "status-condition":
                if(step.role==="player"){
                    statusBarPlayer.changeStatusCondition(step.label, step.remove)
                }else{
                    statusBarOpponent.changeStatusCondition(step.label, step.remove)
                }
                sequenceTimer.interval = 0;
                sequenceTimer.start()
                break

            case "change-health":
                var isPlayer = step.role === "player"
                var target = isPlayer ? player : opponent
                if(step.amount<0){
                    target.takeDamage.running = true;
                }

                let currentHealthRatio = target.statusBar.changeHealth(step.amount)
                if(isPlayer){
                    battleMenu.party.healthRatios[battleMenu.selectedIndex] = currentHealthRatio
                }

                if(currentHealthRatio<=0){
                    root.actionSequence = [
                        {type: "faint", message: target.name + " fainted!", role: step.role, delay: 1000 },
                    ]
                    root.currentActionIndex = 0
                }
                sequenceTimer.interval = step.delay
                sequenceTimer.start()
                break

            case "faint":
                battleMenu.updateText(step.message)
                root.currentActionIndex = 0
                if(step.role === "opponent"){
                    opponent.visible = false
                    root.actionInProgress = false
                    root.actionSequence = []
                    root.requestExperienceSpread();
                }else{
                    player.visible = false
                    root.actionSequence = [{type: "force-switch"}]
                    sequenceTimer.interval = step.delay
                    sequenceTimer.start()
                }
                break

            case "force-switch":
                root.currentActionIndex = 0
                if(battleMenu.party.healthRatios.some(ratio => ratio > 0)){
                    root.actionInProgress = false
                    root.actionSequence = []
                    battleMenu.forceSwitch();
                }else{
                    root.actionSequence = [
                        {type: "text", message: "Player is out of usable pokemon!", delay: 1000},
                        {type: "opponent-won", delay: 100}
                    ]
                    sequenceTimer.start()
                }
                break

            case "attempt-catch":
                if(step.shakes>=4){
                    step.shakes = 3;
                    var newActions = [
                        {type: "opponent-caught", delay: 100},
                        {type: "text", message: "Gotcha! " + opponent.name + " was caught!", delay: 2000},
                        {type: "jump", delay: 2000}
                    ]
                }
                else{
                    var newActions = [
                        {type: "reveal-opponent", message: "Aargh! Almost had it!", delay: 1000},
                        {type: "fail-catch", delay: root.ballTransitionDuration}
                    ]
                }

                for (var i = 0; i < step.shakes; i++) {
                    newActions.push({type: "shake", delay: 2000});
                }

                root.tempActionSequence = newActions.reverse().concat(root.actionSequence.slice(2));
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
                 sequenceTimer.interval = step.delay
                 sequenceTimer.start()
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
            case "opponent-caught":
                root._battleEnded("OpponentCaught", true)
                break
            case "opponent-won":
                root._battleEnded("OpponentWon", true)
                break
            case "player-won":
                root._battleEnded("PlayerWon", true)
                break
            case "end":
                endActionChain()
                break
        }
    }
    function showExperienceSpreadSequence(spread, lvlups) {
        var sequence = []
        for (var i = 0; i < 6; i++) {
            if (lvlups[i] >0){
                sequence.push({
                    type: "text",
                    message: battleMenu.party.names[i] + " grew to Lv." + lvlups[i] + "!",
                    delay: 1500
                })
            }else if (spread[i] > 0){
                sequence.push({
                    type: "text",
                    message: battleMenu.party.names[i] + " gained " + spread[i] + " Exp. Points!",
                    delay: 1500
                })
            }
        }
        sequence.push({ type: "player-won", delay: 500})
        executeActionSequence(sequence)
    }

    function endActionChain() {
        console.log("Action chain ended!\n")
        root.actionInProgress = false
        root.actionSequence = []
        root.currentActionIndex = 0
        root.catchAttemptActive = false
        battleMenu.resetToRoot()
    }


    function calculateBallCoords(sprite){
        var pokeballWidth = pokeBallOpponent.width
        var pokeballHeight = pokeBallOpponent.height

        var x1 = sprite.x + (sprite.width / 2) - (pokeballWidth / 2)

        var x0 = x1 + (sprite.direction == 1 ? -sprite.width : sprite.width)

        var y1 = sprite.y + sprite.height - pokeballHeight/2
        var y0 = sprite.y - pokeballHeight

        y0 = Math.max(pokeballHeight/2, y0)
        y1 = Math.max(pokeballHeight/2, y1)

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
