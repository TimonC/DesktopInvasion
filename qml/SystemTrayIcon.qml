import Qt.labs.platform

SystemTrayIcon {
    visible: true
    icon.source: "qrc:/assets/HGSS/PokeballIcon.png"
    menu: Menu {
        MenuItem {
            text: qsTr("Quit")
            onTriggered: Qt.quit()
        }
    }
    onActivated: {
    }
}
