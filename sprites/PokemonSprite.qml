import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    property string spriteSheet: "qrc:/assets/HGSS/PokGen1_transparent_reordered.png"
    property int row: 0
    property int direction: 0
    property real scaleFactor: 4
    property int frameWidth: 32
    property int frameHeight: 32
    property int frameCount: 2
    property int frameRate: 3
    property int itemWidth: 0
    property int itemHeight: 0
    property int offsetX: 0
    property int offsetY: 0

    property bool clickable: true
    property bool tackle: false
    property bool attacked: false
    property bool debugLines: false

    property alias mouseArea: mouseArea
    property alias battlebutton: battleButton

    width: itemWidth > 0 ? itemWidth : frameWidth * scaleFactor
    height: itemHeight > 0 ? itemHeight : frameHeight * scaleFactor
    clip: true
    layer.enabled: true
    z: 1

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: true
        hoverEnabled: true
    }

    RoundButton {
        id: battleButton
        text: "BATTLE"
        anchors.top: parent.top
        visible: false
        enabled: false
        z: 10000
        onClicked: console.log("Battle clicked!")
    }

    Item {
        id: spriteContainer
        anchors.centerIn: parent
        anchors.horizontalCenterOffset: offsetX
        anchors.verticalCenterOffset: offsetY
        width: sprite.width
        height: sprite.height

        AnimatedSprite {
            id: sprite
            z: 1000
            scale: scaleFactor

            source: spriteSheet
            frameWidth: root.frameWidth
            frameHeight: root.frameHeight
            frameCount: root.frameCount
            frameRate: root.frameRate
            interpolate: false
            smooth: false
            antialiasing: false

            frameX: {
                switch (direction) {
                    case 0: return 0;
                    case 1: return frameWidth * frameCount;
                    case 2: return frameWidth * frameCount * 2;
                    case 3: return frameWidth * frameCount * 3;
                }
                return 0;
            }
            frameY: row * frameHeight
        }
    }

    SequentialAnimation {
        id: tackleAnim
        running: tackle
        loops: 1
        onStopped: tackle = false

        PropertyAnimation {
            target: sprite
            property: "x"
            to: (direction==1 ? -6*scaleFactor :
                 direction==3 ? 6*scaleFactor :
                 0)
            duration: 50
            easing.type: Easing.InQuad
        }

        PropertyAnimation {
            target: sprite
            property: "y"
            to: (direction==0 ? -6*scaleFactor :
                 direction==2 ? 6*scaleFactor :
                 0)
            duration: 50
            easing.type: Easing.InQuad
        }

        PropertyAnimation {
            target: sprite
            property: "x"
            to: 0
            duration: 100
            easing.type: Easing.OutQuad
        }

        PropertyAnimation {
            target: sprite
            property: "y"
            to: 0
            duration: 100
            easing.type: Easing.OutQuad
        }
    }

    SequentialAnimation {
        id: attackId
        running: attacked
        loops: 1
        onStopped: attacked = false

        PropertyAnimation {
            target: sprite
            property: "x"
            to: (direction==1 ? 2.5*scaleFactor :
                 direction==3 ? -2.5*scaleFactor :
                 0)
            duration: 100
        }

        PropertyAnimation {
            target: sprite
            property: "y"
            to: (direction==0 ? 2.5*scaleFactor :
                 direction==2 ? -2.5*scaleFactor :
                 0)
            duration: 100
        }

        PropertyAnimation {
            target: sprite
            property: "x"
            to: 0
            duration: 100
            easing.type: Easing.InQuad
        }

        PropertyAnimation {
            target: sprite
            property: "y"
            to: 0
            duration: 100
            easing.type: Easing.InQuad
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: "yellow"
        border.width: 1
        visible: debugLines
    }
}
