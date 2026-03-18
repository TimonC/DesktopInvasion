#include "SystemTrayIcon.h"
#include <QApplication>

SystemTrayIcon::SystemTrayIcon(QObject *parent) : QSystemTrayIcon(parent),
    m_menu(new QMenu()),
    m_gameActive(true)
{
    // Set initial icon
    setIcon(QIcon(":/assets/HGSS/PokeballIcon.png"));
    setVisible(true);

    // Setup context menu
    setupMenu();

    // Connect signal
    connect(this, &QSystemTrayIcon::activated,
            this, &SystemTrayIcon::onActivated);
}

void SystemTrayIcon::setupMenu(){
    m_menuButton = m_menu->addAction("Menu");
    connect(m_menuButton, &QAction::triggered,
            this, &SystemTrayIcon::menuButtonPressed);

    m_gameActiveToggle = m_menu->addAction("Active");
    m_gameActiveToggle->setCheckable(true);
    m_gameActiveToggle->setChecked(m_gameActive);
    connect(m_gameActiveToggle, &QAction::triggered,
            this, &SystemTrayIcon::toggleGameActive);

    setContextMenu(m_menu);
}

void SystemTrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason){
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::Context) {
        QPoint pos = QCursor::pos();
        m_menu->popup(pos);
    }
}

void SystemTrayIcon::setIconActivityColor(bool active){
    if (active) {
        setIcon(QIcon(":/assets/HGSS/PokeballIcon.png"));
    } else {
        setIcon(QIcon(":/assets/HGSS/PokeballIcon_grayscale.png"));
    }
}

void SystemTrayIcon::toggleGameActive(){
    m_gameActive = !m_gameActive;

    setIconActivityColor(m_gameActive);
    m_gameActiveToggle->setChecked(m_gameActive);

    emit gameActive(m_gameActive);
}

void SystemTrayIcon::updateIconAndMenu(){
    toggleGameActive();
}
