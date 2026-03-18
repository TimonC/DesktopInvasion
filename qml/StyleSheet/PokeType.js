.pragma library

function typeColor(typeName) {
    switch(typeName) {
        case "Normal": return "#A8A878"
        case "Fighting": return "#C03028"
        case "Flying": return "#A890F0"
        case "Poison": return "#A040A0"
        case "Ground": return "#E0C068"
        case "Rock": return "#B8A038"
        case "Bug": return "#A8B820"
        case "Ghost": return "#705898"
        case "Steel": return "#B8B8D0"
        case "Fire": return "#F08030"
        case "Water": return "#6890F0"
        case "Grass": return "#78C850"
        case "Electric": return "#F8D030"
        case "Psychic": return "#F85888"
        case "Ice": return "#98D8D8"
        case "Dragon": return "#7038F8"
        case "Dark": return "#705848"
        default: return "transparent"
    }
}
