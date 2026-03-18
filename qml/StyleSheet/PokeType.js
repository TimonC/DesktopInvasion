.pragma library

function typeColor(typeName) {
    switch(typeName) {
        case "Normal":   return "#9FA19F"
        case "Fighting": return "#FF8000"
        case "Flying":   return "#81B9EF"
        case "Poison":   return "#9141CB"
        case "Ground":   return "#915121"
        case "Rock":     return "#AFA981"
        case "Bug":      return "#91A119"
        case "Ghost":    return "#704170"
        case "Steel":    return "#60A1B8"
        case "Fire":     return "#E62829"
        case "Water":    return "#2980EF"
        case "Grass":    return "#3FA129"
        case "Electric": return "#FAC000"
        case "Psychic":  return "#EF4179"
        case "Ice":      return "#3DCEF3"
        case "Dragon":   return "#5060E1"
        case "Dark":     return "#624D4E"
        default:         return "transparent"
    }
}

var DARK_FACTOR  = 0.8   // multiply RGB to get darker
var LIGHT_FACTOR = 0.15   // add to RGB to get lighter

function darkerTypeColor(typeName) {
    var c = Qt.color(typeColor(typeName))
    return Qt.rgba(c.r * DARK_FACTOR, c.g * DARK_FACTOR, c.b * DARK_FACTOR, 1)
}

function lighterTypeColor(typeName) {
    var c = Qt.color(typeColor(typeName))
    return Qt.rgba(
        Math.min(1, c.r + LIGHT_FACTOR),
        Math.min(1, c.g + LIGHT_FACTOR),
        Math.min(1, c.b + LIGHT_FACTOR),
        1
    )
}

