#include "SystemTrayIcon.h"
#include <QApplication>

SystemTrayIcon::SystemTrayIcon(QObject *parent)
    : QSystemTrayIcon(parent)
    , m_menu(new QMenu())
    , m_gameActive(true)
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

void SystemTrayIcon::setupMenu()
{
    // Player action
    QAction *playerAction = new QAction("Player", this);
    m_menu->addAction(playerAction);

    // Settings action
    QAction *settingsAction = new QAction("Settings", this);
    m_menu->addAction(settingsAction);

    // Hide/Show action
    m_hideShowAction = new QAction("Hide", this);
    connect(m_hideShowAction, &QAction::triggered,
            this, &SystemTrayIcon::onHideShowTriggered);
    m_menu->addAction(m_hideShowAction);

    // Separator
    m_menu->addSeparator();

    // Quit action
    QAction *quitAction = new QAction("Quit", this);
    connect(quitAction, &QAction::triggered,
            qApp, &QApplication::quit);
    m_menu->addAction(quitAction);

    // Set the menu
    setContextMenu(m_menu);
}

void SystemTrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason)
{
    // Double click to toggle icon
    if (reason == QSystemTrayIcon::DoubleClick) {
        swapIcon();
    }
}

void SystemTrayIcon::onHideShowTriggered()
{
    swapIcon();
}

void SystemTrayIcon::swapIcon()
{
    // Toggle game active state
    m_gameActive = !m_gameActive;

    // Update icon based on state
    if (m_gameActive) {
        setIcon(QIcon(":/assets/HGSS/PokeballIcon.png"));
        m_hideShowAction->setText("Hide");
    } else {
        setIcon(QIcon(":/assets/HGSS/PokeballIcon_grayscale.png"));
        m_hideShowAction->setText("Show");
    }

    // Emit signal
    emit gameActive(m_gameActive);
}

void SystemTrayIcon::updateIconAndMenu()
{
    // Helper method if needed elsewhere
    swapIcon();
}
