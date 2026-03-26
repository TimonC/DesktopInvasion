import QtQuick 2.15
import "../Style/PokeColor.js" as PokeColor

Item {
    id: moveMenu

    property var    pokeData:    null
    property string spriteSheet: "qrc:/assets/HGSS/reordered_sprites.png"
    property int    frameWidth:  32
    property int    frameHeight: 32
    property real   scaleFactor: 6
    property int    rowId:       0

    property string mainFont:   "Press Start 2P"
    property string bodyFont:   "DotGothic16"
    property int    fontSizeLg: 22
    property int    fontSizeMd: 18
    property int    fontSizeSm: 16
    property int maximumNicknameLength: 10


    property int displayedPokemonPos: -1
    property int displayedPokemonBox: -1

    property bool inNameEditMode:      false
    property int  selectedEligibleIdx: -1

    property int jumpDistance:  24

    property color colorText:         "#ffffff"
    property color colorSubtext:      "#aaaaaa"
    property color colorVeryFaint:    "#999999"
    property color colorDivider:      "#3d3d3d"
    property color colorSurface:      "#383838"
    property color colorSurfaceHov:   "#444444"
    property color colorSurfaceSel:   "#404040"
    property color colorEligCard:     "#303030"
    property color colorEligHov:      "#3c3c3c"
    property color colorTypePillText: "#ffffff"
    property color colorAccent:       "#5294e2"
    property color colorError:        "#e25452"
    property color colorNameBg:       "#383838"
    property color colorNameHov:      "#444444"
    property color colorNameEdit:     "#404040"
    property color colorNameBdr:      "#555555"
    property color colorNameEditBdr:  "#5294e2"
    property color colorScrollTrack:  "#2a2a2a"
    property color colorScrollThumb:  "#555555"
    property color colorScrollHov:    "#777777"
    property color colorScrollActive: "#5294e2"

    property int margin:  8
    property int gap:     5
    property int pad:     9
    property int pillW:   82
    property int pillH:   fontSizeMd + 6
    property int cardH:   pillH + pad * 2
    property int secGap:  16

    property int _updateCounter: 0

    signal returnClicked()
    signal nameChanged(string newName)
    signal requestMoveChange(int currentMoveSlot, int newMoveId)

    function toggleNameEditMode() {
        if (inNameEditMode) {
            inNameEditMode = false
            nameField.focus = false
            if (nameField.text !== (pokeData ? pokeData.name : ""))
                moveMenu.nameChanged(nameField.text)
        } else {
            inNameEditMode = true
            selectedEligibleIdx = -1
            nameField.forceActiveFocus()
        }
    }

    onPokeDataChanged: {
        if (pokeData) nameField.text = pokeData.name
        inNameEditMode      = false
        selectedEligibleIdx = -1
    }

    component TypePill: Rectangle {
        property string typeName: ""
        width:  moveMenu.pillW
        height: moveMenu.pillH
        radius: 3
        gradient: Gradient {
            GradientStop { position: 0.0; color: PokeColor.lighter(PokeColor.typeColor(typeName)) }
            GradientStop { position: 1.0; color: PokeColor.darker(PokeColor.typeColor(typeName))  }
        }
        Text {
            anchors.centerIn: parent
            text:             PokeColor.typeColor(typeName) === "transparent" ? "" : typeName
            font.family:      moveMenu.bodyFont
            font.pixelSize:   moveMenu.fontSizeSm
            font.bold:        true
            color:            moveMenu.colorTypePillText
        }
    }

    component CurrentMoveCard: Rectangle {
        id: cmc
        property string moveName:  ""
        property string moveType:  ""
        property int    movePow:   0
        property int    moveAcc:   0
        property int    slotIndex: 0

        readonly property bool swapReady: moveMenu.selectedEligibleIdx !== -1
        readonly property bool isEmpty:   moveName === "—" || moveName === ""

        width:  parent ? parent.width : 200
        height: moveMenu.cardH
        radius: 4
        color:  swapReady && cmcMa.containsMouse ? moveMenu.colorSurfaceHov : moveMenu.colorSurface
        border.color: swapReady && cmcMa.containsMouse ? moveMenu.colorAccent : moveMenu.colorDivider
        border.width: 1

        Row {
            anchors {
                verticalCenter: parent.verticalCenter
                left:  parent.left
                right: parent.right
                leftMargin:  moveMenu.pad
                rightMargin: moveMenu.pad
            }
            spacing: 8

            TypePill { typeName: cmc.isEmpty ? "" : cmc.moveType }

            Text {
                width: parent.width - moveMenu.pillW - cmcPower.implicitWidth - 20
                anchors.verticalCenter: parent.verticalCenter
                text:           cmc.moveName
                font.family:    moveMenu.mainFont
                font.pixelSize: moveMenu.fontSizeSm - 1
                color:          cmc.isEmpty ? moveMenu.colorVeryFaint : moveMenu.colorText
                elide:          Text.ElideRight
            }

            Text {
                id: cmcPower
                anchors.verticalCenter: parent.verticalCenter
                text:           cmc.isEmpty ? ""
                                : ((cmc.movePow > 0 ? cmc.movePow : "—") + "  " +
                                   (cmc.moveAcc > 0 ? cmc.moveAcc + "%" : "—"))
                font.family:    moveMenu.bodyFont
                font.pixelSize: moveMenu.fontSizeSm
                color:          moveMenu.colorSubtext
            }
        }

        MouseArea {
            id: cmcMa
            anchors.fill:            parent
            hoverEnabled:            true
            cursorShape:             undefined
            propagateComposedEvents: false
            onClicked: {
                if (!cmc.swapReady) return

                var newMoves = moveMenu.pokeData.moves.slice()
                if(newMoves.length<cmc.slotIndex) return

                var eligMove = moveMenu.pokeData.eligibleMoves[moveMenu.selectedEligibleIdx]
                newMoves[cmc.slotIndex] = {
                    id:       eligMove.id,
                    name:     eligMove.name,
                    type:     eligMove.type,
                    power:    eligMove.power,
                    accuracy: eligMove.accuracy
                }
                moveMenu.pokeData.moves = newMoves
                moveMenu._updateCounter++

                moveMenu.requestMoveChange(cmc.slotIndex, eligMove.id)

                moveName = eligMove.name
                moveType = eligMove.type
                movePow = eligMove.power !== undefined ? eligMove.power : -1
                moveAcc = eligMove.accuracy !== undefined ? eligMove.accuracy : -1

                moveMenu.selectedEligibleIdx = -1
            }
        }
    }

    component EligibleMoveRow: Rectangle {
        id: emr
        property string moveName: ""
        property string moveType: ""
        property int    movePow:  0
        property int    moveAcc:  0
        property int    eligIdx:  -1

        readonly property bool isSelected: moveMenu.selectedEligibleIdx === eligIdx
        readonly property bool isMoveKnown: {
            if (!moveMenu.pokeData) return false
            moveMenu._updateCounter
            var eligMove = moveMenu.pokeData.eligibleMoves[eligIdx]
            for (var i = 0; i < moveMenu.pokeData.moves.length; i++) {
                var currentMove = moveMenu.pokeData.moves[i]
                if (!currentMove) continue
                if (currentMove.id !== undefined && currentMove.id === eligMove.id) return true
                if (currentMove.name === eligMove.name) return true
            }
            return false
        }

        width:  parent ? parent.width : 200
        height: moveMenu.cardH
        radius: 4
        color:  isMoveKnown ? moveMenu.colorSurface
              : isSelected ? moveMenu.colorSurfaceSel
              : (emrMa.containsMouse ? moveMenu.colorEligHov : moveMenu.colorEligCard)
        border.color: isMoveKnown ? moveMenu.colorDivider
                    : isSelected ? moveMenu.colorAccent
                    : moveMenu.colorDivider
        border.width: isSelected ? 2 : 1

        Row {
            anchors {
                verticalCenter: parent.verticalCenter
                left:  parent.left
                right: parent.right
                leftMargin:  moveMenu.pad
                rightMargin: moveMenu.pad
            }
            spacing: 8

            TypePill { typeName: emr.moveType }

            Text {
                width: parent.width - moveMenu.pillW - emrPower.implicitWidth - 20
                anchors.verticalCenter: parent.verticalCenter
                text:           emr.moveName
                font.family:    moveMenu.mainFont
                font.pixelSize: moveMenu.fontSizeSm - 1
                color:          isMoveKnown ? moveMenu.colorVeryFaint : moveMenu.colorText
                elide:          Text.ElideRight
            }

            Text {
                id: emrPower
                anchors.verticalCenter: parent.verticalCenter
                text:           (emr.movePow > 0 ? emr.movePow : "—") + "  " +
                                (emr.moveAcc > 0 ? emr.moveAcc + "%" : "—")
                font.family:    moveMenu.bodyFont
                font.pixelSize: moveMenu.fontSizeSm
                color:          isMoveKnown ? moveMenu.colorVeryFaint : moveMenu.colorSubtext
            }
        }

        MouseArea {
            id: emrMa
            anchors.fill:            parent
            hoverEnabled:            true
            enabled:                 !emr.isMoveKnown
            cursorShape:             undefined
            propagateComposedEvents: false
            onClicked: {
                var newSelectedIdx = (moveMenu.selectedEligibleIdx === emr.eligIdx) ? -1 : emr.eligIdx
                moveMenu.selectedEligibleIdx = newSelectedIdx
                if (moveMenu.inNameEditMode) moveMenu.toggleNameEditMode()
            }
        }
    }

    Row {
        anchors.fill:    parent
        anchors.margins: moveMenu.margin
        spacing:         0

        Item {
            id: leftPanel
            width:  Math.floor((parent.width - moveMenu.margin * 2 - 1) / 2)
            height: parent.height

            Column {
                id: leftTop
                anchors { top: parent.top; left: parent.left; right: parent.right }
                spacing: 0
                clip:    true

                PcButton {
                    width:  48*4
                    label:  "← BACK"
                    onClicked: moveMenu.returnClicked()
                }


                Item { width: parent.width; height: moveMenu.secGap }

                Rectangle { width: parent.width; height: 1; color: moveMenu.colorDivider }

                Item { width: parent.width; height: moveMenu.gap*2 }

                Column {
                    width:   parent.width
                    spacing: 5

                    Text {
                        text:           "NICKNAME"
                        font.family:    moveMenu.mainFont
                        font.pixelSize: moveMenu.fontSizeSm - 4
                        color:          moveMenu.colorSubtext
                        leftPadding:    2
                    }

                    Item {
                        width:  parent.width
                        height: 44

                        Rectangle {
                            anchors.fill: parent
                            radius:       4
                            color:        moveMenu.inNameEditMode ? moveMenu.colorNameEdit
                                          : (nameMouseArea.containsMouse ? moveMenu.colorNameHov
                                                                         : moveMenu.colorNameBg)
                            border.color: moveMenu.inNameEditMode ? moveMenu.colorNameEditBdr
                                          : (nameMouseArea.containsMouse ? moveMenu.colorAccent
                                                                         : moveMenu.colorNameBdr)
                            border.width: moveMenu.inNameEditMode ? 2 : 1
                        }

                        Text {
                            anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: 11 }
                            text:           "✎"
                            font.pixelSize: moveMenu.fontSizeLg
                            color:          moveMenu.colorSubtext
                            opacity:        nameMouseArea.containsMouse ? 0.9 : 0.4
                            visible:        !moveMenu.inNameEditMode
                            transform: Scale { xScale: -1 }
                        }

                        TextInput {
                            id: nameField
                            anchors {
                                left:           parent.left
                                right:          parent.right
                                leftMargin:     10
                                rightMargin:    moveMenu.inNameEditMode ? 10 : 36
                                verticalCenter: parent.verticalCenter
                            }
                            text:           pokeData ? pokeData.name : ""
                            font.family:    moveMenu.mainFont
                            font.pixelSize: moveMenu.fontSizeMd
                            font.bold:      true
                            color:          moveMenu.colorText
                            maximumLength:  moveMenu.maximumNicknameLength
                            clip:           true
                            enabled:        moveMenu.inNameEditMode
                            readOnly:       !moveMenu.inNameEditMode
                            cursorVisible:  moveMenu.inNameEditMode
                            selectByMouse:  moveMenu.inNameEditMode
                            Keys.onReturnPressed: moveMenu.toggleNameEditMode()
                            Keys.onEscapePressed: {
                                nameField.text = pokeData ? pokeData.name : ""
                                moveMenu.toggleNameEditMode()
                            }
                            onFocusChanged: {
                                if (!activeFocus && moveMenu.inNameEditMode)
                                    moveMenu.toggleNameEditMode()
                            }
                        }

                        Text {
                            anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: 10 }
                            text:           "Click to nickname"
                            font.family:    moveMenu.bodyFont
                            font.pixelSize: moveMenu.fontSizeSm
                            color:          moveMenu.colorVeryFaint
                            visible:        !moveMenu.inNameEditMode && nameField.text.length === 0
                        }

                        MouseArea {
                            id: nameMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape:  undefined
                            onClicked:    { if (!moveMenu.inNameEditMode) moveMenu.toggleNameEditMode() }
                        }
                    }
                }

                Item { width: parent.width; height: moveMenu.secGap }

                Rectangle { width: parent.width; height: 1; color: moveMenu.colorDivider }

                Item { width: parent.width; height: moveMenu.gap }

                Item {
                    width:  parent.width
                    height: moveMenu.pillH

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text:           "MOVES"
                        font.family:    moveMenu.mainFont
                        font.pixelSize: moveMenu.fontSizeSm - 4
                        color:          moveMenu.colorSubtext
                    }

                    Text {
                        anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                        text: moveMenu.selectedEligibleIdx !== -1 ? "← pick slot" : ""
                        font.family:    moveMenu.bodyFont
                        font.pixelSize: moveMenu.fontSizeSm
                        color:          moveMenu.colorAccent
                    }
                }

                Item { width: parent.width; height: moveMenu.gap }

                Column {
                    width:   parent.width
                    spacing: moveMenu.gap
                    Repeater {
                        model: 4
                        CurrentMoveCard {
                            width:     leftPanel.width
                            slotIndex: index
                            moveName:  (pokeData && pokeData.moves && index < pokeData.moves.length)
                                       ? pokeData.moves[index].name : "—"
                            moveType:  (pokeData && pokeData.moves && index < pokeData.moves.length)
                                       ? pokeData.moves[index].type : ""
                            movePow:   (pokeData && pokeData.moves && index < pokeData.moves.length
                                        && pokeData.moves[index].power !== undefined)
                                       ? pokeData.moves[index].power : -1
                            moveAcc:   (pokeData && pokeData.moves && index < pokeData.moves.length
                                        && pokeData.moves[index].accuracy !== undefined)
                                       ? pokeData.moves[index].accuracy : -1
                        }
                    }
                }

                Item { width: parent.width; height: moveMenu.secGap }


                Item { width: parent.width; height: moveMenu.gap }
            }

            Rectangle {
                id: spriteAreaSeparator
                anchors { top: leftTop.bottom; left: parent.left; right: parent.right }
                height: 1
                color:  moveMenu.colorDivider
            }

            Item {
                id: spriteArena
                anchors {
                    top:    spriteAreaSeparator.bottom
                    bottom: parent.bottom
                    left:   parent.left
                    right:  parent.right
                }
                layer.enabled: true
                layer.smooth:  false

                WalkingSprite {
                    id: walkingSprite
                    spriteSheet:  moveMenu.spriteSheet
                    frameWidth:   moveMenu.frameWidth
                    frameHeight:  moveMenu.frameHeight
                    scaleFactor:  moveMenu.scaleFactor
                    rowId:        moveMenu.rowId
                    arenaWidth:   spriteArena.width
                    arenaHeight:  spriteArena.height
                    jumpDistance: moveMenu.jumpDistance
                    active:       spriteArena.visible && moveMenu.visible
                }
            }
        }

        Item      { width: moveMenu.margin; height: parent.height }
        Rectangle { width: 1; height: parent.height; color: moveMenu.colorDivider }
        Item      { width: moveMenu.margin; height: parent.height }

        Item {
            id: rightPanel
            width:  parent.width - leftPanel.width - moveMenu.margin * 2 - 1
            height: parent.height

            Column {
                width:       parent.width
                anchors.top: parent.top
                spacing:     5

                Item {
                    width:  parent.width
                    height: moveMenu.pillH

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text:           "ELIGIBLE MOVES"
                        font.family:    moveMenu.mainFont
                        font.pixelSize: moveMenu.fontSizeSm - 4
                        color:          moveMenu.colorSubtext
                    }

                    Text {
                        anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                        text:           pokeData && pokeData.eligibleMoves
                                        ? pokeData.eligibleMoves.length : ""
                        font.family:    moveMenu.bodyFont
                        font.pixelSize: moveMenu.fontSizeSm
                        color:          moveMenu.colorVeryFaint
                    }
                }

                Item {
                    width:  parent.width
                    height: rightPanel.height - moveMenu.pillH - 5

                    Flickable {
                        id: eligFlickable
                        anchors {
                            top:    parent.top
                            bottom: parent.bottom
                            left:   parent.left
                            right:  scrollBar.left
                            rightMargin: 6
                        }
                        contentHeight:      eligibleCol.implicitHeight
                        clip:               true
                        flickableDirection: Flickable.VerticalFlick
                        boundsBehavior:     Flickable.StopAtBounds
                        boundsMovement:     Flickable.StopAtBounds

                        Column {
                            id: eligibleCol
                            width:   parent.width
                            spacing: moveMenu.gap

                            Repeater {
                                model: pokeData ? pokeData.eligibleMoves : []
                                EligibleMoveRow {
                                    width:    eligibleCol.width
                                    eligIdx:  index
                                    moveName: modelData.name
                                    moveType: modelData.type
                                    movePow:  modelData.power    !== undefined ? modelData.power    : -1
                                    moveAcc:  modelData.accuracy !== undefined ? modelData.accuracy : -1
                                }
                            }
                        }
                    }

                    Item {
                        id: scrollBar
                        anchors { top: parent.top; bottom: parent.bottom; right: parent.right }
                        width:   8
                        visible: eligFlickable.contentHeight > eligFlickable.height
                        opacity: scrollThumbMa.containsMouse || scrollThumbMa.pressed ? 1.0 : 0.5

                        Behavior on opacity { NumberAnimation { duration: 120 } }

                        Rectangle {
                            anchors { top: parent.top; bottom: parent.bottom; horizontalCenter: parent.horizontalCenter }
                            width:  2; radius: 1
                            color:  moveMenu.colorScrollTrack
                        }

                        Rectangle {
                            id: scrollThumb
                            anchors.horizontalCenter: parent.horizontalCenter
                            width:  scrollThumbMa.pressed ? 7 : (scrollThumbMa.containsMouse ? 6 : 4)
                            radius: width / 2
                            color:  scrollThumbMa.pressed         ? moveMenu.colorScrollActive
                                    : scrollThumbMa.containsMouse ? moveMenu.colorScrollHov
                                                                  : moveMenu.colorScrollThumb

                            Behavior on width { NumberAnimation { duration: 100 } }
                            Behavior on color { ColorAnimation  { duration: 100 } }

                            height: Math.max(20,
                                        scrollBar.height * (eligFlickable.height
                                        / Math.max(1, eligFlickable.contentHeight)))
                            y: {
                                var ratio = eligFlickable.contentY
                                            / Math.max(1, eligFlickable.contentHeight - eligFlickable.height)
                                return ratio * (scrollBar.height - scrollThumb.height)
                            }

                            MouseArea {
                                id: scrollThumbMa
                                anchors.fill:    parent
                                hoverEnabled:    true
                                cursorShape:     undefined
                                property real dragStartY:        0
                                property real dragStartContentY: 0
                                onPressed: (mouse) => {
                                    dragStartY        = mouse.y
                                    dragStartContentY = eligFlickable.contentY
                                }
                                onPositionChanged: (mouse) => {
                                    if (!pressed) return
                                    var delta        = mouse.y - dragStartY
                                    var trackUsable  = scrollBar.height - scrollThumb.height
                                    var contentRange = eligFlickable.contentHeight - eligFlickable.height
                                    eligFlickable.contentY = Math.max(0, Math.min(contentRange,
                                        dragStartContentY + delta * (contentRange / Math.max(1, trackUsable))))
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    component WalkingSprite: Item {
        id: ws

        property string spriteSheet:  "qrc:/assets/HGSS/reordered_sprites.png"
        property int    frameWidth:   32
        property int    frameHeight:  32
        property real   scaleFactor:  6
        property int    rowId:        0
        property real   arenaWidth:   200
        property real   arenaHeight:  200
        property int    jumpDistance: 12
        property bool   active:       true

        property int  _direction: 2
        property bool _moving:    false
        property int  _speed:     1
        property int  _frameX:    frameWidth * 2 * 2

        readonly property real _spriteW: frameWidth  * scaleFactor
        readonly property real _spriteH: frameHeight * scaleFactor

        property real _minX: 0
        property real _minY: 0
        property real _maxX: 0
        property real _maxY: 0

        width:  arenaWidth
        height: arenaHeight

        onActiveChanged: {
            if (!active) {
                decisionTimer.stop()
                moveTimer.stop()
                spriteImg.running = false
            } else {
                spriteImg.running = true
                Qt.callLater(function() {
                    _maxX = Math.max(0, arenaWidth  - _spriteW)
                    _maxY = Math.max(0, arenaHeight - _spriteH)
                    randomizePosition()
                    if (!decisionTimer.running) decisionTimer.start()
                })
            }
        }

        onArenaWidthChanged: {
            _maxX = Math.max(0, arenaWidth - _spriteW)
            ensureInBounds()
        }
        onArenaHeightChanged: {
            _maxY = Math.max(0, arenaHeight - _spriteH)
            ensureInBounds()
        }

        Component.onCompleted: {
            _speed = Math.random() < 0.5 ? 1 : 2
            _maxX  = Math.max(0, arenaWidth  - _spriteW)
            _maxY  = Math.max(0, arenaHeight - _spriteH)
            Qt.callLater(function() {
                randomizePosition()
                if (ws.active) decisionTimer.start()
            })
        }

        function setDirection(d) {
            _direction = d
            switch (d) {
                case 0: _frameX = 0;                   break
                case 1: _frameX = frameWidth * 2;      break
                case 2: _frameX = frameWidth * 2 * 2;  break
                case 3: _frameX = frameWidth * 2 * 3;  break
            }
        }

        function ensureInBounds() {
            spriteImg.x = Math.max(_minX, Math.min(_maxX, spriteImg.x))
            spriteImg.y = Math.max(_minY, Math.min(_maxY, spriteImg.y))
        }

        function randomizePosition() {
            spriteImg.x = _minX + Math.random() * Math.max(0, _maxX - _minX)
            spriteImg.y = _minY + Math.random() * Math.max(0, _maxY - _minY)
        }

        Timer {
            id: decisionTimer
            interval: 2000 + Math.floor(Math.random() * 2000)
            running:  false
            repeat:   true
            onTriggered: {
                if (!ws.active || spriteImg.isJumping) return
                var decision = Math.floor(Math.random() * 8)
                ws.setDirection(Math.floor(decision / 2))
                ws._moving = (decision % 2) === 1
                decisionTimer.interval = 2000 + Math.floor(Math.random() * 2000)
                if (ws._moving) {
                    if (!moveTimer.running) moveTimer.start()
                } else {
                    moveTimer.stop()
                }
            }
        }

        Timer {
            id: moveTimer
            interval: 50
            running:  false
            repeat:   true
            onTriggered: {
                if (!ws.active || spriteImg.isJumping || !ws._moving) return
                var dx = 0; var dy = 0
                switch (ws._direction) {
                    case 0: dy = -ws._speed; break
                    case 1: dx = -ws._speed; break
                    case 2: dy =  ws._speed; break
                    case 3: dx =  ws._speed; break
                }
                var nx = spriteImg.x + dx
                var ny = spriteImg.y + dy
                if (nx < ws._minX || nx > ws._maxX) {
                    ws.setDirection(ws._direction === 1 ? 3 : 1)
                    return
                }
                if (ny < ws._minY || ny > ws._maxY) {
                    ws.setDirection(ws._direction === 0 ? 2 : 0)
                    return
                }
                spriteImg.x = nx
                spriteImg.y = ny
            }
        }

        AnimatedSprite {
            id: spriteImg
            width:        ws._spriteW
            height:       ws._spriteH
            running:      ws.active
            source:       ws.spriteSheet
            frameWidth:   ws.frameWidth
            frameHeight:  ws.frameHeight
            frameCount:   2
            frameRate:    4
            interpolate:  false
            smooth:       false
            antialiasing: false
            frameX:       ws._frameX
            frameY:       ws.rowId * ws.frameHeight

            property bool isJumping:        false
            property int  jumpUpDuration:   200
            property int  jumpDownDuration: 150
            property int  jumpHeight:       ws.jumpDistance

            SequentialAnimation {
                id: jumpAnim
                property real _baseY: 0
                ScriptAction {
                    script: { jumpAnim._baseY = spriteImg.y; moveTimer.stop() }
                }
                PropertyAnimation {
                    target: spriteImg; property: "y"
                    to: jumpAnim._baseY - spriteImg.jumpHeight
                    duration: spriteImg.jumpUpDuration; easing.type: Easing.OutQuad
                }
                PropertyAnimation {
                    target: spriteImg; property: "y"
                    to: jumpAnim._baseY
                    duration: spriteImg.jumpDownDuration; easing.type: Easing.InQuad
                }
                ScriptAction {
                    script: {
                        spriteImg.isJumping = false
                        if (ws._moving && ws.active) moveTimer.start()
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape:  undefined
                onClicked: {
                    if (!spriteImg.isJumping) {
                        spriteImg.isJumping = true
                        console.log("Yippeee!")
                        jumpAnim.start()
                    }
                }
            }
        }
    }
}
