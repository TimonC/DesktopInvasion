import Qt.labs.platform 1.1

SystemTrayIcon {
    id: root
    visible: true
    icon.mask: true
    icon.source: "qrc:/assets/HGSS/PokeballIcon.png"
    property alias iconRoot: root.icon
    property alias hideText: hideItem.text

    signal gameActive(bool active);

    onActivated: function(reason) {
        if (reason === SystemTrayIcon.DoubleClick) {
            root.swapIcon();
        }
    }

    menu: Menu {
        id: menu
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
                root.swapIcon();
            }
        }
        MenuItem {
            text: qsTr("Quit")
            onTriggered:{
            }
        }
    }
    function swapIcon(){
        var currentlyVisible = iconRoot.source == "qrc:/assets/HGSS/PokeballIcon.png";

        if(currentlyVisible){
            iconRoot.source = "qrc:/assets/HGSS/PokeballIcon_grayscale.png"
            root.hideText = "Show"
        }else{
            iconRoot.source = "qrc:/assets/HGSS/PokeballIcon.png"
            root.hideText = "Hide"
        }

        root.gameActive(!currentlyVisible);
    }
}

