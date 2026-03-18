.pragma library

// https://bulbapedia.bulbagarden.net/wiki/Help:Color_templates
function typeColor(typeName) {
    switch(typeName) {
        case "Bug":      return "#91A119"
        case "Dark":     return "#624D4E"
        case "Dragon":   return "#5060E1"
        case "Electric": return "#FAC000"
        case "Fighting": return "#FF8000"
        case "Fire":     return "#E62829"
        case "Flying":   return "#81B9EF"
        case "Ghost":    return "#704170"
        case "Grass":    return "#3FA129"
        case "Ground":   return "#915121";
        case "Ice":      return "#3DCEF3"
        case "Normal":   return "#9FA19F"
        case "Poison":   return "#9141CB"
        case "Psychic":  return "#EF4179"
        case "Rock":     return "#AFA981"
        case "Steel":    return "#60A1B8"
        case "Water":    return "#2980EF"
        default:         return "transparent"
    }
}

    function lighter(baseColor) {
        var colorRatio = 1.05
        var c = Qt.color(baseColor)
        return Qt.rgba(
            Math.min(1, c.r * colorRatio),
            Math.min(1, c.g * colorRatio),
            Math.min(1, c.b * colorRatio),
            1
        )
    }

    function darker(baseColor) {
        var colorRatio = 0.7
        var c = Qt.color(baseColor)
        return Qt.rgba(
            c.r * colorRatio,
            c.g * colorRatio,
            c.b * colorRatio,
            1
        )
    }

    function healthColor(healthRatio) {
        if (healthRatio >= 0.5) return "#388E3C"
        if (healthRatio >= 0.25) return "#FF9800"
        if (healthRatio > 0) return "#FF0000"
        return "#8B0000"
    }


    function statusConditionColor(label) {
        switch(label) {
            case "BRN": return typeColor("Fire")
            case "FRZ": return typeColor("Ice")
            case "PAR": return typeColor("Electric")
            case "SLP": return typeColor("Normal")
            case "PSN": return typeColor("Poison")
            default:    return "transparent"
        }
    }

