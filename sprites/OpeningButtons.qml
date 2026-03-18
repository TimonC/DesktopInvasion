import QtQuick.Controls 2.15

RoundButton {
    text: "\u2713" // Unicode Character 'CHECK MARK'
    onClicked: textArea.readOnly = true
    visible: false
    enabled: false
}
