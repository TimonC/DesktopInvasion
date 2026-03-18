#ifndef SYSTEMTRAYICON_H
#define SYSTEMTRAYICON_H

#include <QSystemTrayIcon>
#include <QMenu>

class SystemTrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

public:
    explicit SystemTrayIcon(QObject *parent = nullptr);
    void updateIconAndMenu();
    void enabled(bool enabled);

signals:
    void gameActive(bool active);
    void menuButtonPressed();

private slots:
    void onActivated(QSystemTrayIcon::ActivationReason reason);
    void toggleGameActive();

private:
    void setIconActivityColor(bool active);

    /* QAction *m_gameActiveToggle; */
    /* QAction *m_menuButton; */
    bool m_gameActive;
    bool m_clickEnabled;
};

#endif // SYSTEMTRAYICON_H
