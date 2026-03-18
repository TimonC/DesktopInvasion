import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: (chosenSide === 0 || chosenSide === 2) ? frameSize * 5 : frameSize * 7
    height: (chosenSide === 0 || chosenSide === 2) ? frameSize * 7 : frameSize * 5
    layer.enabled: true

    // Scaling properties
    property int frameSize: 32
    property int buttonWidth: frameSize * 2
    property int buttonHeight: frameSize * 0.7
    property int buttonFontSize: frameSize * 0.4
    property int gridSpacing: frameSize * 0.1  // Small spacing between buttons

    // Property to control which side the chosen/opponent buttons are on
    property int chosenSide: 1  // 0, 1, 2, or 3

    property alias textBar: textBar
    property alias buttonGrid: buttonGrid;
    property alias attackButton: attackButton
    property alias switchButton: switchButton
    property alias catchButton: catchButton
    property alias runButton: runButton
    property alias mouseArea: mouseArea
    MouseArea {
        id: mouseArea
        anchors.fill: parent
    }

    function set_chosen_side(side) {
        chosenSide = side
    }

    // Invokable function to update text bar
    function update_text_bar(newText) {
        textBar.text = newText;
    }

    // New method to swap visibility between buttons and text
    function swap_visibility() {
        var showButtons = !buttonGrid.visible;  // What we want to show after swap
        buttonGrid.visible = showButtons;
        textBarText.visible = !showButtons;

        if (showButtons) {
            textBar.color = "transparent";
        } else {
            textBar.color = "lightgrey";
        }
    }

    // Text bar at the bottom
    Rectangle {
        id: textBar
        objectName: "textBar"
        z: 8000
        property string text: ""

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: frameSize * 1.6

        color: "transparent"
        border.color: "transparent"
        border.width: 1
        radius: 4

        Text {
            id: textBarText
            anchors.fill: parent
            anchors.margins: 6
            text: textBar.text
            font.pixelSize: 14
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            visible: false  // Text hidden by default
            z: 8000
        }

        // Button grid - centered in the text bar
        Grid {
            id: buttonGrid
            columns: 2
            spacing: gridSpacing
            visible: true  // Buttons visible by default

            // Simple centering - no complex anchors
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            z: 8000
            // Attack button - top left
            RoundButton {
                id: attackButton
                text: "Attack"
                font.pixelSize: buttonFontSize
                palette.button:"red"
                width: buttonWidth
                height: buttonHeight
                radius: buttonHeight / 2
                onClicked: {console.log("Attack clicked")}
            }

            // Switch button - top right
            RoundButton {
                id: switchButton
                text: "Switch"
                palette.button: "green"
                font.pixelSize: buttonFontSize
                width: buttonWidth
                height: buttonHeight
                radius: buttonHeight / 2
                onClicked: console.log("Switch clicked")
            }

            // Catch button - bottom left
            RoundButton {
                id: catchButton
                text: "Catch"
                palette.button: "yellow"
                font.pixelSize: buttonFontSize
                width: buttonWidth
                height: buttonHeight
                radius: buttonHeight / 2
                onClicked: console.log("Catch clicked")
            }

            // Run button - bottom right
            RoundButton {
                id: runButton
                text: "Run"
                palette.button: "blue"
                font.pixelSize: buttonFontSize
                width: buttonWidth
                height: buttonHeight
                radius: buttonHeight / 2
                onClicked: console.log("Run clicked")
            }
        }
    }
    Rectangle {
        // optional visual debugging
        anchors.fill: parent
        color: "transparent"
        border.color: "blue"
        border.width: 1
        // visible: false
    }
}
