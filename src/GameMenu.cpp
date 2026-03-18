#include <GameMenu.h>
#include <QTimer>
#include <QScreen>
#include <qnamespace.h>

GameMenu::GameMenu()
    : QQuickView()
{
    qDebug() << "GameMenu constructor called!";

    setFlags( Qt::Dialog
            | Qt::WindowTitleHint
            | Qt::WindowCloseButtonHint
            | Qt::FramelessWindowHint
            /* | Qt::WindowDoesNotAcceptFocus */
    );

    if(strcmp(std::getenv("DOCKER_ENV"), "dev") == 0){
        setSource(QUrl("../qml/qmlGameMenu/Menu.qml"));
    }else{
        setSource(QUrl("qrc:/qml/qmlGameMenu/Menu.qml"));
    }

    m_grabCursor = QCursor(QPixmap(":/assets/XY/grab.png"));
    m_pointerCursor = QCursor(QPixmap(":/assets/XY/pointer.png"), 6, 6);
    setCursor(m_pointerCursor);

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
