#include <GameMenu.h>
#include <QQuickView>

GameMenu::GameMenu(QWindow *parent)
    : QQuickView(parent)
{
    qDebug() << "GameMenu constructor called!";
    setSource(QUrl("qrc:/qml/Menu.qml"));
    setResizeMode(QQuickView::SizeRootObjectToView);

    hide();
}
