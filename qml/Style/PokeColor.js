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

