import QtQuick 2.15
import "../SpriteAnimation"  // Import base animations

SpriteAnimation {
    // Get hit movement (backward)
    PropertyAnimation {
        target: root.target
        property: "x"
        to: (root.direction==1 ? 2.5*root.scaleFactor :
             root.direction==3 ? -2.5*root.scaleFactor :
             0)
        duration: 100
    }

    PropertyAnimation {
        target: root.target
        property: "y"
        to: (root.direction==0 ? 2.5*root.scaleFactor :
             root.direction==2 ? -2.5*root.scaleFactor :
             0)
        duration: 100
    }

    // Return to original position
    PropertyAnimation {
        target: root.target
        property: "x"
        to: 0
        duration: 100
        easing.type: Easing.InQuad
    }

    PropertyAnimation {
        target: root.target
        property: "y"
        to: 0
        duration: 100
        easing.type: Easing.InQuad
    }
}
