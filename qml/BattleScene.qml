import QtQuick 2.15

Item {
    id: root

    property real scale: 1
    property real frameSize: 32*scale
    property real animationSpeed: 1

    width: (direction === 0 || direction === 2) ? Math.round(frameSize * 6.5) : Math.round(frameSize * 8)
    height: (direction === 0 || direction === 2) ? Math.round(frameSize * 8) : Math.round(frameSize * 6.5)

    property string textBarFontFamily: "DotGothic16"
    property string menuFontFamily: "Press Start 2P"
    property string statusBarFontFamily: "Press Start 2P"

    property int menuWidth: Math.round(frameSize * 6.5)
    property int menuHeight: Math.round(frameSize * 2)
    property int statusBarWidth: Math.round(frameSize*2.5)
    property int statusBarHeight: Math.round(frameSize*1.2)
    property int buttonWidth: Math.round(frameSize * 2.25)
    property int buttonHeight: Math.round(frameSize * 0.75)
    property int gridSpacing: Math.round(frameSize * 0.08)
    property int switchGridSpacing: Math.round(frameSize * 0.05)
    property int borderWidth: Math.max(1, Math.round(frameSize * 0.05))

    property int pokeNameFontSize: Math.round(9*scale)
    property int buttonFontSize: Math.round(9*scale)
    property int moveFontSize: Math.round(8*scale)
    property int textBarFontSize: Math.round(15*scale)

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

    property int faintDuration: 1000 / animationSpeed
    property int outOfPokemonTextDuration: 1000 / animationSpeed
    property int opponentWonDuration: 100 / animationSpeed
    property int opponentCaughtDuration: 100 / animationSpeed
    property int successCatchTextDuration: 2000 / animationSpeed
    property int successCatchJumpDuration: 2000 / animationSpeed
    property int revealOpponentDuration: 1000 / animationSpeed
    property int shakeDuration: 2000 / animationSpeed
    property int catchShakeInterval: 1500 / animationSpeed
    property int ballTransitionDuration: 750 / animationSpeed
    property int runEndDuration: 1000 / animationSpeed
    property int lvlUpDuration: 1500 / animationSpeed

    property bool catchAttemptActive: false
    property var ballOpenedConnection: null
    property var pokemonInsideBallConnection: null

    signal _battleEnded(string endState, bool removeWild)
    signal signalToStartActionRound(int actionIndex, string actionState)
    signal switchedPokemon(int pokedexId, int partyIndex)
    signal requestExperienceSpread()
    signal requestBallCountUpdate(int delta, int row)

    function setBalls(n_greatBall, n_ultraBall, n_masterBall){
        battleMenu.nrOfBalls = [1000, n_greatBall, n_ultraBall, n_masterBall]
    }

    function setInitialTotalHealth(opponentTotalHealth, playerTotalHealth){
        opponent.statusBar.totalHealth  = opponentTotalHealth
        player.statusBar.totalHealth    = playerTotalHealth
    }

    function updatePlayerStatusAilment(label){
        player.statusBar.changeStatusCondition(label, label=="")
    }

    function positionSpriteAndStatusBar(sprite) {
        var hPad = root.menuWidth/15
        switch(sprite.direction) {
            case 0:
                sprite.x = (root.statusBarWidth + root.width - sprite.width)/2 + hPad + (sprite.verticalWidth/2 - sprite.hCenterUp)
                sprite.y = root.height - root.menuHeight - sprite.height - root.statusBarHeight/4
                sprite.statusBar.x = hPad
                sprite.statusBar.y = (root.height - root.menuHeight)/2
                break
            case 1:
                sprite.statusBar.x = root.width - root.frameSize/2 - root.statusBarWidth
                sprite.statusBar.y = 0
                sprite.x = sprite.statusBar.x + root.statusBarWidth/2 - sprite.width/2
                sprite.y = root.height - battleMenu.height - sprite.height + Math.max(0, sprite.height - sprite.horizontalHeight)/2 - root.frameSize/4
                break
            case 2:
                sprite.x = (root.statusBarWidth + root.width - sprite.width)/2 + hPad + (sprite.verticalWidth/2 - sprite.hCenterDown)
                sprite.y = 0
                sprite.statusBar.x = hPad
                sprite.statusBar.y = 0
                break
            case 3:
                sprite.statusBar.x = root.frameSize/2
                sprite.statusBar.y = 0
                sprite.x = sprite.statusBar.x + root.statusBarWidth/2 - sprite.width/2
                sprite.y = root.height - battleMenu.height - sprite.height + Math.max(0, sprite.height - sprite.horizontalHeight)/2 - root.frameSize/4
                break
        }
        sprite.x = Math.round(sprite.x)
        sprite.y = Math.round(sprite.y)
        sprite.statusBar.x = Math.round(sprite.statusBar.x)
        sprite.statusBar.y = Math.round(sprite.statusBar.y)
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
            pokeBallPlayer.throwAt(coords[0], coords[1], coords[2], coords[3], coords[4])
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
        animationSpeed: root.animationSpeed
        statusBarRadius: 4*root.scale
    }

    PokemonSprite {
        id: opponent
        objectName: "opponent"
        name: opponentName
        direction: root.direction
        debugLines: root.debugLines
        debugColor: "red"
        property alias statusBar: root.statusBarOpponent
        animationSpeed: root.animationSpeed
    }

    StatusBar {
        id: statusBarPlayer
        pokeNameFontSize: root.pokeNameFontSize
        fontFamily: root.statusBarFontFamily
        width: root.statusBarWidth
        height: root.statusBarHeight
        animationSpeed: root.animationSpeed
        statusBarRadius: 4*root.scale
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
        animationSpeed: root.animationSpeed
    }

    Pokeball {
        id: pokeBallOpponent
        speed: root.animationSpeed
        scaleFactor: root.scale
        circleX: opponent.x + opponent.width/2
        circleY: opponent.y + opponent.height/2
        circleBaseWidth: direction%2!=0 ? Math.max(opponent.width, opponent.verticalWidth) : opponent.width
        circleBaseHeight: direction%2!=0 ? Math.min(opponent.height, opponent.horizontalHeight) : opponent.height

        onThrowAnimationDone: {
            root.currentActionIndex = 0
            root.actionSequence = tempActionSequence
            sequenceTimer.interval = root.catchShakeInterval/2
            sequenceTimer.start()
        }
        onPokemonInsideBall:{
            pokeBallOpponent.circleShrink()
            opponent.visible =  false
        }
    }

    Pokeball {
        id: pokeBallPlayer
        speed: root.animationSpeed
        scaleFactor: root.scale
    }

    function resetPlayerBall() {
        pokeBallPlayer.reset(root.currentPlayerBallIndex)
        pokeBallPlayer.visible = true

        pokeBallPlayer.circleX = player.x + player.width/2
        pokeBallPlayer.circleY = player.y + player.height/2

        pokeBallPlayer.circleBaseWidth = direction%2!=0 ? Math.max(player.verticalWidth, player.width) : player.width
        pokeBallPlayer.circleBaseHeight = direction%2!=0 ? Math.min(player.height, player.horizontalHeight) : player.height
        if (pokeBallPlayer.circleY - pokeBallPlayer.circleBaseHeight/2 < 0) {
            pokeBallPlayer.circleBaseHeight = pokeBallPlayer.circleY * 2
        }

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
                root.signalToStartActionRound(battleMenu.selectedIndex, "Switch")
            }
        }

        pokeBallPlayer.onPokemonInsideBall.connect(root.pokemonInsideBallConnection)
        pokeBallPlayer.onBallOpened.connect(root.ballOpenedConnection)
    }

    function resetOpponentBall() {
        pokeBallOpponent.reset(root.currentOpponentBallIndex)
        pokeBallOpponent.visible = true

        pokeBallOpponent.circleX = opponent.x + opponent.width/2
        pokeBallOpponent.circleY = opponent.y + opponent.height/2

        pokeBallOpponent.circleBaseWidth = direction%2!=0 ? Math.max(opponent.width, opponent.verticalWidth) : opponent.width
        pokeBallOpponent.circleBaseHeight = direction%2!=0 ? Math.min(opponent.height, opponent.horizontalHeight) : opponent.height

        if (pokeBallOpponent.circleY - pokeBallOpponent.circleBaseHeight/2 < 0) {
            pokeBallOpponent.circleBaseHeight = pokeBallOpponent.circleY * 2
        }
    }

    BattleMenu {
        id: battleMenu
        iconScale: root.scale
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        height: root.menuHeight
        width: root.menuWidth
        buttonWidth: root.buttonWidth
        buttonHeight: root.buttonHeight
        gridSpacing: root.gridSpacing
        switchGridSpacing: root.switchGridSpacing
        borderWidth: root.borderWidth
        menuHeight: root.menuHeight
        menuWidth: root.menuWidth
        buttonFontSize: root.buttonFontSize
        moveFontSize: root.moveFontSize
        textBarFontSize: root.textBarFontSize
        textBarFontFamily: root.textBarFontFamily
        menuFontFamily: root.menuFontFamily
        mainButtonRadius: 0.5
        moveButtonRadius: Math.round(frameSize * 0.125)
        switchButtonRadius: Math.round(frameSize * 0.125)
        ballButtonRadius: Math.round(frameSize * 0.125)
        runButtonRadius: Math.round(frameSize * 0.75)
        textBarRadius: Math.round(frameSize * 0.15625)
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
            pokeBallPlayer.throwAt(coords[0], coords[1], coords[2], coords[3], coords[4])
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
                sequenceTimer.interval = step.delay / root.animationSpeed
                sequenceTimer.start()
                break

            case "attack":
                var attacker = (step.role === "player") ? player : opponent
                attacker.actionForward.running = true
                sequenceTimer.interval = step.delay / root.animationSpeed
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

            case "take-damage":
                var isPlayer = step.role === "player"
                var target = isPlayer ? player : opponent
                target.takeDamage.running = true;
                sequenceTimer.interval = step.delay / root.animationSpeed
                sequenceTimer.start()
                break

            case "change-health":
                var isPlayer = step.role === "player"
                var target = isPlayer ? player : opponent
                let currentHealthRatio = target.statusBar.changeHealth(step.amount)
                if(isPlayer){
                    battleMenu.party.healthRatios[battleMenu.selectedIndex] = currentHealthRatio
                }

                if(currentHealthRatio<=0){
                    root.actionSequence = [
                        {type: "faint", message: target.name + " fainted!", role: step.role, delay: root.faintDuration },
                    ]
                    root.currentActionIndex = 0
                }
                sequenceTimer.interval = step.delay / root.animationSpeed
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
                    sequenceTimer.interval = step.delay / root.animationSpeed
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
                        {type: "text", message: "Player is out of usable pokemon!", delay: root.outOfPokemonTextDuration},
                        {type: "opponent-won", delay: root.opponentWonDuration}
                    ]
                    sequenceTimer.start()
                }
                break

            case "attempt-catch":
                root.currentOpponentBallIndex = step.ballId
                resetOpponentBall()

                if(step.ballId>0){
                    root.requestBallCountUpdate(-1, step.ballId)
                    battleMenu.nrOfBalls[step.ballId] -=1
                }

                var newActions
                if(step.shakes>=4){
                    step.shakes = 3
                    newActions = [
                        {type: "opponent-caught", delay: root.opponentCaughtDuration},
                        {type: "text", message: "Gotcha! " + opponent.name + " was caught!", delay: root.successCatchTextDuration},
                        {type: "jump", delay: root.successCatchJumpDuration}
                    ]
                }
                else{
                    newActions = [
                        {type: "reveal-opponent", message: "Aargh! Almost had it!", delay: root.revealOpponentDuration},
                        {type: "fail-catch", delay: root.ballTransitionDuration}
                    ]
                }

                for (var i = 0; i < step.shakes; i++) {
                    newActions.push({type: "shake", delay: root.shakeDuration})
                }

                root.tempActionSequence = newActions.reverse().concat(root.actionSequence.slice(2))
                root.actionSequence = []

                var coords = calculateBallCoords(opponent)
                pokeBallOpponent.visible = true
                pokeBallOpponent.throwAt(coords[0], coords[1], coords[2], coords[3], coords[4])
                break

            case "shake":
                 pokeBallOpponent.shake()
                 sequenceTimer.interval = step.delay / root.animationSpeed
                 sequenceTimer.start()
                 break

            case "jump":
                 pokeBallOpponent.jump()
                 sequenceTimer.interval = step.delay / root.animationSpeed
                 sequenceTimer.start()
                 break

            case "fail-catch":
                 pokeBallOpponent.release()
                 sequenceTimer.interval = step.delay / root.animationSpeed
                 sequenceTimer.start()
                 break

            case "reveal-opponent":
                opponent.visible = true
                pokeBallOpponent.visible = false
                battleMenu.updateText(step.message)
                sequenceTimer.interval = step.delay / root.animationSpeed
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

    function showExperienceSpreadSequence(spread, lvlups, tmName, ballGet, whichBall) {
        var sequence = []
        if(tmName != "NONE"){
            sequence.push({
                type: "text",
                message: "Got the TM: '" + tmName + "'!",
                delay: root.lvlUpDuration
            })
        }
        if(ballGet>0){
            var ballName = whichBall == 1 ? "Great Ball" : ((whichBall == 2) ? "Ultra Ball" : "Master Ball")
            sequence.push({
                type: "text",
                message: "Got " + ballGet + " " + ballName + "!",
                delay: root.lvlUpDuration
            })
        }
        for (var i = 0; i < 6; i++) {
            if (lvlups[i] >0){
                sequence.push({
                    type: "text",
                    message: battleMenu.party.names[i] + " grew to Lv." + lvlups[i] + "!",
                    delay: root.lvlUpDuration
                })
            }else if (spread[i] > 0){
                sequence.push({
                    type: "text",
                    message: battleMenu.party.names[i] + " gained " + spread[i] + " Exp. Points!",
                    delay: root.lvlUpDuration
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

    function calculateBallCoords(sprite) {
        var pokeballWidth  = pokeBallOpponent.width
        var pokeballHeight = pokeBallOpponent.height

        var x1      = sprite.x + (sprite.width / 2) - (pokeballWidth / 2)
        var x0      = x1 + (sprite.direction == 3 ? -sprite.width : sprite.width)

        var y0      = Math.max(pokeballHeight / 2, sprite.y - pokeballHeight)
        var y1      = sprite.y + sprite.height / 2 - pokeballHeight

        var groundY = sprite.y + Math.min(sprite.height, sprite.horizontalHeight) - pokeballHeight

        return [x0, x1, y0, y1, groundY]
    }

    function oneShotTimer(duration, onFinish){
        Qt.callLater(function() {
            var hideTimer = Qt.createQmlObject('import QtQuick 2.15; Timer {}', root)
            hideTimer.interval = duration / root.animationSpeed
            hideTimer.triggered.connect(function() {
                onFinish()
                hideTimer.destroy()
            })
            hideTimer.start()
        })
    }

    function showTextBar(){battleMenu.showTextBar()}
}
