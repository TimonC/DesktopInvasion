#include "SystemTrayIcon.h"
#include <QApplication>

SystemTrayIcon::SystemTrayIcon(QObject *parent)
    : QSystemTrayIcon(parent)
    , m_gameActive(true)
    , m_clickEnabled(true)
{
    // Set initial icon
    setIcon(QIcon(":/assets/HGSS/PokeballIcon.png"));
    setVisible(true);

    // Connect signal
    connect(this, &QSystemTrayIcon::activated,
            this, &SystemTrayIcon::onActivated);
}

void SystemTrayIcon::enabled(bool enabled){
    m_clickEnabled = enabled;
}

void SystemTrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason){
    if(m_clickEnabled){
        if(reason == QSystemTrayIcon::Trigger){//left click
            toggleGameActive();
        }else if (reason == QSystemTrayIcon::Context) { //right click
            emit menuButtonPressed();
        }
    };
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
    emit gameActive(m_gameActive);
}

