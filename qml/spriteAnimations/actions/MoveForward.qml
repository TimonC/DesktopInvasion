import QtQuick 2.15
import "../SpriteAnimation"  // Import base animations

SpriteAnimation {
    PropertyAnimation {
        target: root.target
        property: "x"
        to: (root.direction==1 ? -6*root.scaleFactor :
             root.direction==3 ? 6*root.scaleFactor :
             0)
        duration: 50
        easing.type: Easing.InQuad
    }

    PropertyAnimation {
        target: root.target
        property: "y"
        to: (root.direction==0 ? -6*root.scaleFactor :
             root.direction==2 ? 6*root.scaleFactor :
             0)
        duration: 50
        easing.type: Easing.InQuad
    }

    PropertyAnimation {
        target: root.target
        property: "x"
        to: 0
        duration: 100
        easing.type: Easing.OutQuad
    }

    PropertyAnimation {
        target: root.target
        property: "y"
        to: 0
        duration: 100
        easing.type: Easing.OutQuad
    }
}
