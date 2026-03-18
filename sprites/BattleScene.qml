import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: (chosenSide === 0 || chosenSide === 2) ? frameSize * 6 : frameSize * 8.25
    height: (chosenSide === 0 || chosenSide === 2) ? frameSize * 6.5 : frameSize * 4

    // Scaling properties
    property int frameSize: 32
    property int buttonWidth: frameSize*1.25
    property int buttonHeight: frameSize*0.75
    property int buttonFontSize: frameSize*0.25
    property int buttonSpacing: frameSize*0.125
    property int stackSpacing: frameSize*1.2  // New property for spacing between column stacks
    property int textBarMargin: frameSize*0.25  // Tiny margin above text bar

    // Property to control which side the chosen/opponent buttons are on
    property int chosenSide: 1  // 0, 1, 2, or 3

    function set_chosen_side(side) {
        chosenSide = side
    }

    // Invokable function to show/hide all buttons
    function set_buttons_visible(visible) {
        chosenButtons.visible = visible;
        opponentButtons.visible = visible;
    }

    // Invokable function to update text bar
    function update_text_bar(newText) {
        textBar.text = newText;
    }

    // Text bar at the bottom
    Rectangle {
        id: textBar
        objectName: "textBar"

        property string text: ""

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: frameSize

        color: "white"
        border.color: "gray"
        border.width: 1
        radius: 4

        Text {
            anchors.fill: parent
            anchors.margins: 8
            text: textBar.text
            font.pixelSize: 14
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    // Chosen side buttons (Switch/Run)
    Column {
        id: chosenButtons
        spacing: buttonSpacing

        // Horizontal positioning - fully to the sides
        x: {
            switch(chosenSide) {
                case 0: return parent.width - width  // Right side - no margin
                case 1: return 0  // Left side - no margin
                case 2: return parent.width - width  // Right side - no margin
                case 3: return parent.width - width  // Right side - no margin
                default: return 0
            }
        }

        // Vertical positioning
        y: {
            switch(chosenSide) {
                case 0: return textBar.y - textBarMargin - height - stackSpacing - opponentButtons.height  // Above opponent buttons
                case 1: return textBar.y - frameSize * 2  // Above text bar for horizontal
                case 2: return textBar.y - textBarMargin - height  // Just above text bar margin
                case 3: return textBar.y - frameSize * 2  // Above text bar for horizontal
                default: return textBar.y - frameSize * 2
            }
        }

        RoundButton {
            text: "Switch"
            font.pixelSize: buttonFontSize
            width: buttonWidth
            height: buttonHeight
            radius: buttonHeight / 2
            onClicked: console.log("Switch clicked")
        }

        RoundButton {
            text: "Run"
            font.pixelSize: buttonFontSize
            width: buttonWidth
            height: buttonHeight
            radius: buttonHeight / 2
            onClicked: console.log("Run clicked")
        }
    }

    // Opponent side buttons (Attack/Catch)
    Column {
        id: opponentButtons
        spacing: buttonSpacing

        // Horizontal positioning - fully to the sides
        x: {
            switch(chosenSide) {
                case 0: return parent.width - width  // Right side - no margin
                case 1: return parent.width - width  // Right side - no margin
                case 2: return parent.width - width  // Right side - no margin
                case 3: return 0  // Left side - no margin
                default: return parent.width - width
            }
        }

        // Vertical positioning
        y: {
            switch(chosenSide) {
                case 0: return textBar.y - textBarMargin - height  // Just above text bar margin
                case 1: return textBar.y - frameSize * 2  // Above text bar for horizontal
                case 2: return frameSize  // Top padding for vertical down
                case 3: return textBar.y - frameSize * 2  // Above text bar for horizontal
                default: return textBar.y - frameSize * 2
            }
        }

        RoundButton {
            text: "Attack"
            font.pixelSize: buttonFontSize
            width: buttonWidth
            height: buttonHeight
            radius: buttonHeight / 2
            onClicked: console.log("Attack clicked")
        }

        RoundButton {
            text: "Catch"
            font.pixelSize: buttonFontSize
            width: buttonWidth
            height: buttonHeight
            radius: buttonHeight / 2
            onClicked: console.log("Catch clicked")
        }
    }
}
