#include <globals.h>
#include <GameMenu.h>
#include <QTimer>
#include <QScreen>

GameMenu::GameMenu(QWindow *parent)
    : QQuickView(parent)
{
    qDebug() << "GameMenu constructor called!";

    setFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setSource(QUrl("qrc:/qml/Menu.qml"));
    setTitle("DesktopInvasion");
    setModality(Qt::ApplicationModal);

    QRect screenGeometry = Globals::screenGeometry();
    setPosition(QPoint(
        (screenGeometry.width() - width()) / 2,
        (screenGeometry.height() - height()) / 2
    ));

    hide();
}

bool GameMenu::event(QEvent *event) {
    if (event->type() == QEvent::WindowDeactivate) {
        emit menuClosed();
        hide();
        return true;
    }
    return QQuickView::event(event);
}


void GameMenu::activate(){
    show();
    raise();
    requestActivate();
    setVisible(true);
};
