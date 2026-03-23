import QtQuick 2.15
import "../Style/PokeColor.js" as PokeColor

Item {
    id: pcButtonRoot
    width:  64
    height: 48

    property string label:         ""
    property bool   selectable:    true
    property color  btnColor:      "#5294e2"
    property color  highlightColor: btnColor
    property int    buttonRadius:  10
    property string fontFamily:    "Press Start 2P"

    signal clicked()

    property bool _hovered: mouseArea.containsMouse && selectable
    property bool _pressed: mouseArea.pressed        && selectable

    property color _currentBaseColor: highlightColor != btnColor ? highlightColor : btnColor

    property color _borderColor: selectable
        ? (_hovered ? _currentBaseColor : PokeColor.lighter(_currentBaseColor))
        : "#555555"
    Behavior on _borderColor { ColorAnimation { duration: 120; easing.type: Easing.OutQuad } }

    Item {
        id: buttonContent
        anchors.fill: parent
        scale: pcButtonRoot._pressed ? 0.92 : 1.0
        Behavior on scale {
            NumberAnimation {
                duration:          pcButtonRoot._pressed ? 80 : 200
                easing.type:       pcButtonRoot._pressed ? Easing.OutQuad : Easing.OutBack
                easing.overshoot:  1.2
            }
        }

        Rectangle {
            id: backgroundRect
            anchors.fill: parent
            radius:       pcButtonRoot.buttonRadius
            gradient: Gradient {
                GradientStop {
                    position: 0.0
                    color: pcButtonRoot.selectable
                        ? (pcButtonRoot._hovered
                           ? pcButtonRoot._currentBaseColor
                           : PokeColor.lighter(pcButtonRoot._currentBaseColor))
                        : "#444444"
                    Behavior on color { ColorAnimation { duration: 120; easing.type: Easing.OutCubic } }
                }
                GradientStop {
                    position: 1.0
                    color: pcButtonRoot.selectable
                        ? (pcButtonRoot._hovered
                           ? PokeColor.darker(PokeColor.darker(pcButtonRoot._currentBaseColor))
                           : PokeColor.darker(pcButtonRoot._currentBaseColor))
                        : "#333333"
                    Behavior on color { ColorAnimation { duration: 120; easing.type: Easing.OutCubic } }
                }
            }
        }

        Rectangle {
            id: borderRect
            anchors.fill: parent
            radius:       pcButtonRoot.buttonRadius
            color:        "transparent"
            border {
                width: 2
                color: pcButtonRoot._borderColor
            }
        }

        Text {
            anchors.centerIn: parent
            text:             pcButtonRoot.label
            color:            pcButtonRoot.selectable ? "#ffffff" : "#888888"
            font.family:      pcButtonRoot.fontFamily
            font.pixelSize : 18
            font.bold:        true
            style:            Text.Raised
            styleColor:       "#00000060"
            z: 1
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled:      pcButtonRoot.selectable
        hoverEnabled: true
        onClicked:    pcButtonRoot.clicked()
        cursorShape:  undefined
    }
}
