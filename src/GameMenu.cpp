#include <GameMenu.h>
#include <QTimer>
#include <QScreen>
#include <QQuickItem>
#include <QQmlContext>
#include <QVariant>
#include <qnamespace.h>

GameMenu::GameMenu()
    : QQuickView()
{
    qDebug() << "GameMenu constructor called!";
    setFlags(Qt::Dialog
           | Qt::WindowTitleHint
           | Qt::WindowCloseButtonHint
           | Qt::FramelessWindowHint);

    // Must be set BEFORE setSource so menuBridge exists when QML loads
    rootContext()->setContextProperty("menuBridge", this);

    const char* env = getenv("DOCKER_ENV");
    if (env && strcmp(env, "dev") == 0)
        setSource(QUrl("../qml/qmlGameMenu/Menu.qml"));
    else
        setSource(QUrl("qrc:/qml/qmlGameMenu/Menu.qml"));

    m_grabCursor    = QCursor(QPixmap(":/assets/XY/grab.png"));
    m_pointerCursor = QCursor(QPixmap(":/assets/XY/pointer.png"), 6, 6);
    setCursor(m_pointerCursor);
    setTitle("DesktopInvasion");
    hide();
}

bool GameMenu::event(QEvent* event) {
    /* if (event->type() == QEvent::WindowDeactivate) { */
    /*     emit menuClosed(); */
    /*     hide(); */
    /*     return true; */
    /* } */
    return QQuickView::event(event);
}

QObject* GameMenu::qmlRoot() {
    return rootObject();
}

void GameMenu::activate() {
    show();
    raise();
    requestActivate();
    setVisible(true);
}

// These are Q_INVOKABLE so QML can also call them directly if needed,
// but primarily Game calls them and they emit signals that QML listens to.

void GameMenu::loadParty(const QVariantList& data) {
    emit partyDataReady(data);
}

void GameMenu::loadBox(int boxIndex, const QVariantList& data) {
    emit boxDataReady(boxIndex, data);
}

void GameMenu::showBox(int boxIndex) {
    emit showBoxRequested(boxIndex);
}

