import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: (direction === 0 || direction === 2) ? frameSize * 5 : frameSize * 8
    height: (direction === 0 || direction === 2) ? frameSize * 8 : frameSize * 5
    signal runClicked()
    property bool attackInProgress: false

    property int frameSize: 32
    property int buttonWidth: frameSize * 2
    property int buttonHeight: frameSize * 0.75
    property int buttonFontSize: frameSize * 0.4
    property int gridSpacing: frameSize * 0.1
    property int pokeMargin: frameSize*0.25
    property bool debugLines: false
    property int textBoxHeight: 50
    property int direction: 0

    property alias opponent: opponent
    property alias player: player
    property alias textBar: textBar
    property alias buttonGrid: buttonGrid


    SequentialAnimation {
        id: playerAttackAnim
        running: false
        loops: 1

        property int attackDistance: 20

        onStopped: {
            opponentHitAnim.start();
            root.update_text_bar("It's super effective!");
        }

        PropertyAnimation {
            target: player
            property: "x"
            to: player.originalX + (player.direction==1 ? -playerAttackAnim.attackDistance : player.direction==3 ? playerAttackAnim.attackDistance : 0)
            duration: 50
            easing.type: Easing.InQuad
        }
        PropertyAnimation {
            target: player
            property: "y"
            to: player.originalY + (player.direction==0 ? -playerAttackAnim.attackDistance : player.direction==2 ? playerAttackAnim.attackDistance : 0)
            duration: 50
            easing.type: Easing.InQuad
        }
        PropertyAnimation {
            target: player
            property: "x"
            to: player.originalX
            duration: 100
            easing.type: Easing.OutQuad
        }
        PropertyAnimation {
            target: player
            property: "y"
            to: player.originalY
            duration: 100
            easing.type: Easing.OutQuad
        }
    }

    SequentialAnimation {
        id: opponentHitAnim
        running: false
        loops: 1

        property int attackDistance: 10

        onStopped: {
            root.attackInProgress = false;
            buttonGrid.visible = true;
            textBarText.visible = false;
            textBar.color = "transparent";
            root.update_text_bar("What will you do?");
        }

        PropertyAnimation {
            target: opponent
            property: "x"
            to: opponent.originalX + (opponent.direction==1 ? opponentHitAnim.attackDistance : opponent.direction==3 ? -opponentHitAnim.attackDistance : 0)
            duration: 50
            easing.type: Easing.InQuad
        }
        PropertyAnimation {
            target: opponent
            property: "y"
            to: opponent.originalY + (opponent.direction==0 ? opponentHitAnim.attackDistance : opponent.direction==2 ? -opponentHitAnim.attackDistance : 0)
            duration: 50
            easing.type: Easing.InQuad
        }
        PropertyAnimation {
            target: opponent
            property: "x"
            to: opponent.originalX
            duration: 100
            easing.type: Easing.OutQuad
        }
        PropertyAnimation {
            target: opponent
            property: "y"
            to: opponent.originalY
            duration: 100
            easing.type: Easing.OutQuad
        }
    }

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

    function positionSprite(sprite) {
        var margin = root.pokeMargin;
        switch(sprite.direction) {
            case 0:
                sprite.x = margin + sprite.containerOffsetX;
                sprite.y = root.height - (textBoxHeight + margin + sprite.containerOffsetY + sprite.height);
                break;
            case 1:
                sprite.x = root.width - (margin + sprite.containerOffsetX + sprite.width);
                sprite.y = root.height - (textBoxHeight + margin + sprite.containerOffsetY + sprite.height);
                break;
            case 2:
                sprite.x = margin + sprite.containerOffsetX;
                sprite.y = margin + sprite.containerOffsetY;
                break;
            case 3:
                sprite.x = margin + sprite.containerOffsetX;
                sprite.y = root.height - (textBoxHeight + margin + sprite.containerOffsetY + sprite.height);
                break;
        }
        sprite.originalX = sprite.x;
        sprite.originalY = sprite.y;
    }

    function update_text_bar(newText) {
        textBar.text = newText;
    }

    function handleAttack() {
        if (root.attackInProgress) return;
        root.attackInProgress = true;
        buttonGrid.visible = false;
        textBar.color = "darkgrey";
        textBarText.visible = true;
        root.update_text_bar("Player used Tackle!");
        playerAttackAnim.start();
    }

    Rectangle {
        id: textBar
        color: "transparent"
        property string text: ""
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: root.textBoxHeight

        Text {
            id: textBarText
            anchors.fill: parent
            anchors.margins: 6
            text: textBar.text
            font.pixelSize: 14
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
                onClicked: root.handleAttack()
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
}
