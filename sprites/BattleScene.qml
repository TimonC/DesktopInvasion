import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: 32 * 9
    height: 32 * 5

    // Property to control which side the chosen/opponent buttons are on
    property int chosenSide: 1  // 0, 1, 2, or 3

    function set_chosen_side(side) {
        chosenSide = side
    }

    // Invokable function to show/hide all buttons
    function set_buttons_visible(visible) {
        chosenButtons.visible = visible;
        opponentButton.visible = visible;
    }

    // Invokable function to update text bar
    function update_text_bar(newText) {
        textBar.text = newText;
    }

    // Empty space above
    Item {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 32 * 4
    }

    // Text bar at the bottom
    Rectangle {
        id: textBar
        objectName: "textBar"

        property string text: ""

        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: 32 * 5
        height: 32

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

    // Chosen side buttons (Attack/Run)
    Column {
        id: chosenButtons
        spacing: 4

        // Horizontal positioning - right side for vertical cases
        x: {
            switch(chosenSide) {
                case 0: return parent.width - width - 32  // Right side
                case 1: return 32
                case 2: return parent.width - width - 32  // Right side
                case 3: return parent.width - width - 32
                default: return 32
            }
        }

        // Vertical positioning
        y: {
            switch(chosenSide) {
                case 0: return parent.height * 0.25 - height / 2  // Top (~1/4)
                case 1: return textBar.y - 64  // Original position
                case 2: return parent.height * 0.75 - height / 2  // Bottom (~3/4)
                case 3: return textBar.y - 64  // Original position
                default: return textBar.y - 64
            }
        }

        RoundButton {
            text: "Attack"
            width: 80
            height: 24
            radius: 12
            onClicked: console.log("Attack clicked")
        }

        RoundButton {
            text: "Run"
            width: 80
            height: 24
            radius: 12
            onClicked: console.log("Run clicked")
        }
    }

    // Opponent side button (Catch)
    RoundButton {
        id: opponentButton
        text: "Catch"
        width: 80
        height: 24
        radius: 12

        // Horizontal positioning - right side for vertical cases
        x: {
            switch(chosenSide) {
                case 0: return parent.width - width - 32  // Right side
                case 1: return parent.width - width - 32
                case 2: return parent.width - width - 32  // Right side
                case 3: return 32
                default: return parent.width - width - 32
            }
        }

        // Vertical positioning
        y: {
            switch(chosenSide) {
                case 0: return parent.height * 0.75 - height / 2  // Bottom (~3/4)
                case 1: return textBar.y - 32  // Original position
                case 2: return parent.height * 0.25 - height / 2  // Top (~1/4)
                case 3: return textBar.y - 32  // Original position
                default: return textBar.y - 32
            }
        }

        onClicked: console.log("Catch clicked")
    }
}
