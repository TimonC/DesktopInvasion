#include "SystemTrayIcon.h"
#include "StartupManager.h"

#include <QApplication>
#include <QMenu>
#include <QActionGroup>
#include <QDir>

SystemTrayIcon::SystemTrayIcon(QObject *parent)
    : QSystemTrayIcon(parent)
    , m_clickEnabled(true)
    , m_autoStartEnabled(false)
    , m_activeIcon(":/assets/icon/icon.png")
    , m_inactiveIcon(":/assets/icon/icon_transparent.png")
    , m_menu(nullptr)
    , m_quitAction(nullptr)
    , m_menuAction(nullptr) {
    setIcon(m_activeIcon);
    setVisible(true);
    m_autoStartEnabled = StartupManager::instance()->isAutoStartEnabled();
}

SystemTrayIcon::~SystemTrayIcon() {
    qDebug() << "SystemTrayIcon destructor called!";
}

void SystemTrayIcon::enabled(bool enabled) {
    m_clickEnabled = enabled;
    m_menu->setEnabled(enabled);
}

void SystemTrayIcon::toggleGameActive() {
    m_gameActive = !m_gameActive;
    m_petAction->setEnabled(m_gameActive);
    m_activeAction->blockSignals(true);
    m_activeAction->setChecked(m_gameActive);
    m_activeAction->blockSignals(false);
    setIconActivityColor(m_gameActive);
    emit gameActive(m_gameActive);
}

void SystemTrayIcon::togglePetMode() {
    m_petActive = !m_petActive;
    m_petAction->blockSignals(true);
    m_petAction->setChecked(m_petActive);
    m_petAction->blockSignals(false);
    emit petActive(m_petActive);
}

void SystemTrayIcon::toggleAutoStart() {
    m_autoStartEnabled = !m_autoStartEnabled;
    m_autoStartAction->blockSignals(true);
    m_autoStartAction->setChecked(m_autoStartEnabled);
    m_autoStartAction->blockSignals(false);
    StartupManager::instance()->setAutoStartEnabled(m_autoStartEnabled);
    emit autoStartToggled(m_autoStartEnabled);
}

void SystemTrayIcon::createContextMenu(std::vector<std::pair<int, std::string>> trainers, int activeSaveId) {
    m_menu = new QMenu();

    m_autoStartAction = new QAction(tr("Start on system boot"), m_menu);
    m_autoStartAction->setCheckable(true);
    m_autoStartAction->setChecked(m_autoStartEnabled);
    m_autoStartAction->setToolTip(tr("Launch Desktop Invasion when your computer starts"));
    connect(m_autoStartAction, &QAction::toggled, this, [this]() {
        toggleAutoStart();
    });
    m_menu->addAction(m_autoStartAction);


    QMenu* gameMenu = new QMenu(tr("Saves"), m_menu);

    if (!trainers.empty()) {
        QActionGroup* trainerGroup = new QActionGroup(gameMenu);
        trainerGroup->setExclusive(true);
        for (const auto& [id, name] : trainers) {
            QAction* a = new QAction(QString::fromStdString(name), gameMenu);
            a->setCheckable(true);
            a->setChecked(id == activeSaveId);
            trainerGroup->addAction(a);
            gameMenu->addAction(a);
            connect(a, &QAction::triggered, this, [this, id]() {
                m_activeSaveId = id;
                emit saveSelected(id);
            });
        }
        gameMenu->addSeparator();
    }

    m_newGameAction = new QAction(tr("New Save"), gameMenu);
    m_newGameAction->setToolTip(tr("Start a new game"));
    connect(m_newGameAction, &QAction::triggered, this, [this]() {
        emit newGameRequested();
    });
    gameMenu->addAction(m_newGameAction);

    m_deleteAction = new QAction(tr("Delete currently active save"), gameMenu);
    m_deleteAction->setToolTip(tr("Delete the currently active game"));
    connect(m_deleteAction, &QAction::triggered, this, [this]() {
        emit deleteSaveRequested();
    });
    gameMenu->addAction(m_deleteAction);

    m_menu->addMenu(gameMenu);
    m_menu->addSeparator();

    m_petAction = new QAction(tr("Pet mode"), m_menu);
    m_petAction->setCheckable(true);
    m_petAction->blockSignals(true);
    m_petAction->setChecked(m_petActive);
    m_petAction->blockSignals(false);
    m_petAction->setToolTip(tr("Toggle between viewing caucht pokemon or encountering new ones."));
    connect(m_petAction, &QAction::toggled, this, [this]() {
        togglePetMode();
    });
    m_menu->addAction(m_petAction);

    m_activeAction = new QAction(tr("Active"), m_menu);
    m_activeAction->setCheckable(true);
    m_activeAction->setChecked(m_gameActive);
    m_activeAction->setToolTip(tr("Enable or disable the game"));
    connect(m_activeAction, &QAction::toggled, this, [this]() {
        toggleGameActive();
    });
    m_menu->addAction(m_activeAction);

    m_menuAction = new QAction(tr("Menu"), m_menu);
    m_menuAction->setToolTip(tr("Open the game menu"));
    connect(m_menuAction, &QAction::triggered, this, [this]() {
        emit menuButtonPressed();
    });
    m_menu->addAction(m_menuAction);

    m_menu->addSeparator();

    m_quitAction = new QAction(tr("Quit"), m_menu);
    m_quitAction->setToolTip(tr("Exit the application"));
    connect(m_quitAction, &QAction::triggered, qApp, [this](){
                QMessageBox msgBox;
                msgBox.setWindowTitle("DesktopInvasion - Confirm Quit");
                msgBox.setText("Are you sure that you want to quit DesktopInvasion? You can also hide the invading sprites by toggling 'active'.");
                msgBox.setStandardButtons(QMessageBox::Discard | QMessageBox::Cancel);
                msgBox.setDefaultButton(QMessageBox::Cancel);
                msgBox.setIcon(QMessageBox::Warning);

                if (msgBox.exec() != QMessageBox::Discard) {
                    return;
                }
                qApp->quit();
            });
    m_menu->addAction(m_quitAction);

    setContextMenu(m_menu);
}

void SystemTrayIcon::setIconActivityColor(bool active) {
    setIcon(active ? m_activeIcon : m_inactiveIcon);
}
