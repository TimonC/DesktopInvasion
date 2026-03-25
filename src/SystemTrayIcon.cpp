#include "SystemTrayIcon.h"

#include <QApplication>
#include <QMenu>
#include <QActionGroup>
#include <QDir>

SystemTrayIcon::SystemTrayIcon(QObject *parent)
    : QSystemTrayIcon(parent)
    , m_gameActive(true)
    , m_clickEnabled(true)
    , m_activeIcon(":/assets/icon/icon.png")
    , m_inactiveIcon(":/assets/icon/icon_transparent.png")
    , m_menu(nullptr)
    , m_quitAction(nullptr)
    , m_menuAction(nullptr) {
    setIcon(m_activeIcon);
    setVisible(true);
}

SystemTrayIcon::~SystemTrayIcon() {
    qDebug() << "SystemTrayIcon destructor called!";
}

void SystemTrayIcon::enabled(bool enabled) {
    m_clickEnabled = enabled;
    m_activeAction->setEnabled(enabled);
    m_menuAction->setEnabled(enabled);
}

void SystemTrayIcon::toggleGameActive() {
    m_gameActive = !m_gameActive;
    m_activeAction->blockSignals(true);
    m_activeAction->setChecked(m_gameActive);
    m_activeAction->blockSignals(false);
    setIconActivityColor(m_gameActive);
    emit gameActive(m_gameActive);
}

void SystemTrayIcon::createContextMenu(std::vector<std::pair<int, std::string>> trainers, int activeSaveId) {
    m_menu = new QMenu();

    // Game submenu
    QMenu* gameMenu = new QMenu(tr("Game"), m_menu);

    // Trainer list (if any)
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

    // New Game action
    m_newGameAction = new QAction(tr("New Game"), gameMenu);
    m_newGameAction->setToolTip(tr("Start a new game"));
    connect(m_newGameAction, &QAction::triggered, this, [this]() {
        emit newGameRequested();
    });
    gameMenu->addAction(m_newGameAction);

    // Delete Current Game action
    m_deleteAction = new QAction(tr("Delete Current Game"), gameMenu);
    m_deleteAction->setToolTip(tr("Delete the currently active game"));
    connect(m_deleteAction, &QAction::triggered, this, [this]() {
        emit deleteSaveRequested();
    });
    gameMenu->addAction(m_deleteAction);

    m_menu->addMenu(gameMenu);
    m_menu->addSeparator();

    // Active toggle
    m_activeAction = new QAction(tr("Active"), m_menu);
    m_activeAction->setCheckable(true);
    m_activeAction->setChecked(m_gameActive);
    m_activeAction->setToolTip(tr("Enable or disable the game"));
    connect(m_activeAction, &QAction::toggled, this, [this]() {
        toggleGameActive();
    });

    m_menu->addAction(m_activeAction);
    // Menu action
    m_menuAction = new QAction(tr("Menu"), m_menu);
    m_menuAction->setToolTip(tr("Open the game menu"));
    connect(m_menuAction, &QAction::triggered, this, [this]() {
        emit menuButtonPressed();
    });
    m_menu->addAction(m_menuAction);


    m_menu->addSeparator();

    // Quit action
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
