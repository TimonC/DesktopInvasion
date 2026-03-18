import QtQuick 2.15
SequentialAnimation {
    running: false
    loops: 1
    property Item pokemon
    property int attackDistance: 20


    PropertyAnimation {
        target: pokemon
        property: "x"
        to: pokemon.startingX + (pokemon.direction==1 ? -root.attackDistance : pokemon.direction==3 ? root.attackDistance : 0)
        duration: 50
        easing.type: Easing.InQuad
    }
    PropertyAnimation {
        target: pokemon
        property: "y"
        to: pokemon.startingY + (pokemon.direction==0 ? -root.attackDistance : pokemon.direction==2 ? root.attackDistance : 0)
        duration: 50
        easing.type: Easing.InQuad
    }
    PropertyAnimation {
        target: pokemon
        property: "x"
        to: pokemon.startingX
        duration: 100
        easing.type: Easing.OutQuad
    }
    PropertyAnimation {
        target: pokemon
        property: "y"
        to: pokemon.startingY
        duration: 100
        easing.type: Easing.OutQuad
    }
}

