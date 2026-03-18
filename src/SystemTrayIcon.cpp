#include "SystemTrayIcon.h"
#include <QApplication>
#include <qdir.h>

SystemTrayIcon::SystemTrayIcon(QObject *parent)
    : QSystemTrayIcon(parent)
    , m_gameActive(true)
    , m_clickEnabled(true)
    , m_activeIcon(":/assets/HGSS/PokeballIcon.png")
    , m_inactiveIcon(":/assets/HGSS/PokeballIcon_grayscale.png")
{
    setIcon(m_activeIcon);
    setVisible(true);
    connect(this, &QSystemTrayIcon::activated,
            this, &SystemTrayIcon::onActivated);
}

SystemTrayIcon::~SystemTrayIcon(){
    qDebug() << "SystemTrayIcon destructor called";
}

void SystemTrayIcon::setIconActivityColor(bool active){
    setIcon(active ? m_activeIcon : m_inactiveIcon);
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

void SystemTrayIcon::toggleGameActive(){
    m_gameActive = !m_gameActive;
    setIconActivityColor(m_gameActive);
    emit gameActive(m_gameActive);
}

