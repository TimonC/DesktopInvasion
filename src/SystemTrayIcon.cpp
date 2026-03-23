#include "SystemTrayIcon.h"
#include <QApplication>
#include <qdir.h>

SystemTrayIcon::SystemTrayIcon(QObject *parent)
    : QSystemTrayIcon(parent)
    , m_gameActive(true)
    , m_clickEnabled(true)
    , m_activeIcon(":/assets/icon/icon.png")
    , m_inactiveIcon(":/assets/icon/icon_transparent.png")
    , m_menu(nullptr)
    , m_quitAction(nullptr)
    , m_menuAction(nullptr)
{
    setIcon(m_activeIcon);

    setVisible(true);

}

SystemTrayIcon::~SystemTrayIcon(){
    qDebug() << "SystemTrayIcon destructor called!";
    m_menu->deleteLater();
    m_menu = nullptr;
}

void SystemTrayIcon::enabled(bool enabled){
    m_clickEnabled = enabled;
    m_activeAction->setEnabled(enabled);
    m_menuAction->setEnabled(enabled);
};

void SystemTrayIcon::toggleGameActive(){
    m_gameActive = !m_gameActive;

    m_activeAction->blockSignals(true);
    m_activeAction->setChecked(m_gameActive);
    m_activeAction->blockSignals(false);

    setIconActivityColor(m_gameActive);
    emit gameActive(m_gameActive);
}


void SystemTrayIcon::createContextMenu(std::vector<std::pair<int, std::string>> trainers, int activeSaveId) {
    m_menu = new QMenu();

    m_quitAction = new QAction(tr("Quit"), m_menu);
    m_quitAction->setToolTip(tr("Exit the application"));
    connect(m_quitAction, &QAction::triggered, qApp, &QApplication::quit);

    m_deleteAction = new QAction(tr("Delete current save"));
    m_deleteAction->setToolTip(tr("Delete the currently selected save"));
    connect(m_deleteAction, &QAction::triggered, qApp, [this](){
            emit deleteSaveRequested();
    });

    m_menu->addAction(m_quitAction);
    m_menu->addAction(m_deleteAction);
    m_menu->addSeparator();

    m_activeAction = new QAction(tr("Active"), m_menu);
    m_activeAction->setCheckable(true);
    m_activeAction->setChecked(m_gameActive);
    m_activeAction->setToolTip(tr("Toggle game active state"));
    connect(m_activeAction, &QAction::toggled, qApp, [this]() {
        toggleGameActive();
    });

    m_menuAction = new QAction(tr("Menu"), m_menu);
    m_menuAction->setToolTip(tr("Open game menu"));
    connect(m_menuAction, &QAction::triggered, qApp, [this]() {
        emit menuButtonPressed();
    });

    m_newGameAction = new QAction(tr("New Game"), m_menu);
    m_newGameAction->setToolTip(tr("Start a new game"));
    connect(m_newGameAction, &QAction::triggered, this, [this]() {
        emit newGameRequested();
    });


    QActionGroup* trainerGroup = new QActionGroup(m_menu);
    trainerGroup->setExclusive(true);
    if(!trainers.empty()){
        for (auto& [id, name] : trainers) {
            QAction* a = new QAction(QString::fromStdString(name), m_menu);
            a->setCheckable(true);
            a->setChecked(id == activeSaveId);
            trainerGroup->addAction(a);
            m_menu->addAction(a);
            connect(a, &QAction::triggered, this, [this, id]() {
                m_activeSaveId = id;
                emit saveSelected(id);
            });
        }
    }

    m_menu->addAction(m_newGameAction);
    m_menu->addSeparator();
    m_menu->addAction(m_menuAction);
    m_menu->addAction(m_activeAction);
    setContextMenu(m_menu);
}

void SystemTrayIcon::setIconActivityColor(bool active){
    setIcon(active ? m_activeIcon : m_inactiveIcon);
}



