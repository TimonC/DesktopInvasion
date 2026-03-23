import QtQuick 2.15

Item {
    id: root

    readonly property int pad:      20
    readonly property int dividerW: 1
    readonly property color dividerColor: "#3a3a3a"

    readonly property int labelHeight: 24
    readonly property int contentSpacing: 8

    property int iconScale:           8
    property int iconScaleForBig:     6
    property int iconScaleForTrainer: 5

    property color  backgroundColor:     "#2b2b2b"
    property color  textColor:           "#ffffff"
    property color  subheaderColor:      "#aaaaaa"
    property int    fontSizeLg: 22
    property int    fontSizeMd: 18
    property int    fontSizeSm: 16
    property string p2pFont:       "Press Start 2P"
    property string dotGothicFont: "DotGothic16"
}
