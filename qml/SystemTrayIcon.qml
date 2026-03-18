import Qt.labs.platform

SystemTrayIcon {
    visible: true
    icon.source: "qrc:/assets/HGSS/PokeballIcon.png"

    onActivated: {
        window.show()
        window.raise()
        window.requestActivate()
    }
}
