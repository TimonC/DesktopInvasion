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
    QAction *playerAction = new QAction("Player", this);
    m_menu->addAction(playerAction);

    QAction *settingsAction = new QAction("Settings", this);
    m_menu->addAction(settingsAction);

    m_menu->addSeparator();

    m_gameActiveToggle = m_menu->addAction("Active");
    m_gameActiveToggle->setCheckable(true);
    m_gameActiveToggle->setChecked(m_gameActive);
    connect(m_gameActiveToggle, &QAction::triggered,
            this, &SystemTrayIcon::onGameActiveToggled);

    setContextMenu(m_menu);
}

void SystemTrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason){
    if (reason == QSystemTrayIcon::DoubleClick) {
        toggleGameActive();
    }
}

void SystemTrayIcon::onGameActiveToggled(){
    toggleGameActive();
}

void SystemTrayIcon::toggleGameActive(){
    m_gameActive = !m_gameActive;

    if (m_gameActive) {
        setIcon(QIcon(":/assets/HGSS/PokeballIcon.png"));
    } else {
        setIcon(QIcon(":/assets/HGSS/PokeballIcon_grayscale.png"));
    }

    m_gameActiveToggle->setChecked(m_gameActive);

    emit gameActive(m_gameActive);
}

void SystemTrayIcon::updateIconAndMenu(){
    toggleGameActive();
}
