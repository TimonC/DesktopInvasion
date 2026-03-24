#include <GameMenu.h>
#include <QTimer>
#include <QScreen>
#include <QQuickItem>
#include <QQmlContext>
#include <QVariant>
#include <qnamespace.h>
#include <globals.h>


GameMenu::GameMenu()
    : QQuickView()
{
    qDebug() << "GameMenu constructor called!";
    setFlags(Qt::Dialog
           | Qt::WindowTitleHint
           | Qt::WindowMinimizeButtonHint
           | Qt::WindowMaximizeButtonHint
           /* | Qt::FramelessWindowHint */
           );

    // Must be set BEFORE setSource so menuBridge exists when QML loads
    rootContext()->setContextProperty("menuBridge", this);

    const char* env = getenv("DOCKER_ENV");
    if (env && strcmp(env, "dev") == 0)
        setSource(QUrl("../qml/qmlGameMenu/Menu.qml"));
    else
        setSource(QUrl("qrc:/qml/qmlGameMenu/Menu.qml"));
    m_menuRoot = rootObject();

    setCursor(QCursor(QPixmap(":/assets/XY/pointer.png"), 6, 6));
    setTitle("DesktopInvasion");


    /* //Hardcoded values in the root qml (derived from simple log) */
    const double menuWidth = 1361.0;
    const double menuHeight = 945.0;
    QRect availableGeometry = QGuiApplication::primaryScreen()->availableGeometry();

    const double scaleW = std::min(1.0, static_cast<double>(availableGeometry.width())/menuWidth);
    const double scaleH = std::min(1.0, static_cast<double>(availableGeometry.height())/menuHeight);
    const double uiScale = std::min(scaleW, scaleH);

    if(m_menuRoot){
        m_menuRoot->setProperty("uiScale", uiScale);
        //The menu doesn't handle resizing well so I just lock it like this
        setMinimumSize( QSize(menuWidth*uiScale, menuHeight*uiScale));
        setMaximumSize( QSize(menuWidth*uiScale, menuHeight*uiScale));
    }

    hide();
}


bool GameMenu::event(QEvent* event) {
    if (event->type() == QEvent::WindowDeactivate || event->type() == QEvent::Close) {
        QMetaObject::invokeMethod(m_menuRoot, "goToDefaultMenu");
        hide();
        Globals::scale(m_menuRoot->property("spriteSize").toFloat());
        Globals::animationSpeed(m_menuRoot->property("battleSpeed").toFloat());
        Globals::encounterLvlHigh(m_menuRoot->property("encounterLevelHigh").toInt());
        Globals::encounterLvlLow(m_menuRoot->property("encounterLevelLow").toInt());
        Globals::expShare(m_menuRoot->property("expShare").toBool());
        emit menuClosed();

        return true;
    }
    return QQuickView::event(event);
}

void GameMenu::activate() {
    show();
    raise();
    requestActivate();
    setVisible(true);
}

void GameMenu::updateEvolveMenu(QVariantMap evolvesData){
    QMetaObject::invokeMethod(m_menuRoot, "_updateEvolvesMenu",
        Q_ARG(QVariant, QVariant::fromValue(evolvesData)));}

void GameMenu::setDefaults(Defaults &d){
    QMetaObject::invokeMethod(m_menuRoot, "updateDefaults",
                              Q_ARG(QVariant, d.scale),
                              Q_ARG(QVariant, d.speed),
                              Q_ARG(QVariant, d.lvlRangeDown),
                              Q_ARG(QVariant, d.lvlRangeUp),
                              Q_ARG(QVariant, d.expShareOn));
}
void GameMenu::setTrainer(QString name, int trainerId){
    m_menuRoot->setProperty("trainerName", name);
    m_menuRoot->setProperty("trainerId",trainerId);
}

// These are Q_INVOKABLE so QML can also call them directly if needed,
// but primarily Game calls them and they emit signals that QML listens to.

void GameMenu::loadParty(const QVariantList& data, bool displayFirst = true) {
    emit partyDataReady(data, displayFirst);
}

void GameMenu::loadBox(int boxIndex, const QVariantList& data) {
    emit boxDataReady(boxIndex, data);
}

void GameMenu::showBox(int boxIndex) {
    emit showBoxRequested(boxIndex);
}

void GameMenu::_evolvesRequested(int boxIndex, QVariantMap pokeData){
    emit evolvesRequested(boxIndex, pokeData);
};

