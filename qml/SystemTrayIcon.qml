import Qt.labs.platform

SystemTrayIcon {
    id: root
    visible: true
    icon.source: "qrc:/assets/HGSS/PokeballIcon.png"
    property alias iconRoot: root.icon
    property alias hideText: hideItem.text
    menu: Menu {
        MenuItem {
            text: qsTr("Player")
            onTriggered:{
            }
        }
        MenuItem {
            text: qsTr("Settings")
            onTriggered:{
            }
        }
        MenuItem {
            id: hideItem
            text: qsTr("Hide")
            onTriggered: {
                console.log("hello world")
                root.swapSource();
            }
        }
        MenuItem {
            text: qsTr("Quit")
            onTriggered:{
            }
        }
    }
    function swapSource(){
        if(iconRoot.source == "qrc:/assets/HGSS/PokeballIcon.png"){
            iconRoot.source = "qrc:/assets/HGSS/PokeballIcon_grayscale.png"
            root.hideText = "Show"
        }else{
            iconRoot.source = "qrc:/assets/HGSS/PokeballIcon.png"
            root.hideText = "Hide"
        }
    }
}

