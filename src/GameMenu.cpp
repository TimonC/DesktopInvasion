#include <GameMenu.h>
#include <QTimer>
#include <QScreen>

GameMenu::GameMenu()
    : QQuickView()
{
    qDebug() << "GameMenu constructor called!";

    setFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    if(strcmp(std::getenv("DOCKER_ENV"), "dev") == 0){
        setSource(QUrl("../qml/qmlGameMenu/Menu.qml"));
    }else{
        setSource(QUrl("qrc:/qml/qmlGameMenu/Menu.qml"));
    }
    setTitle("DesktopInvasion");
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
