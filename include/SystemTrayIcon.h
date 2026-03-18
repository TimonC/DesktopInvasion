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

signals:
    void gameActive(bool active);
    void menuButtonPressed();

private slots:
    void onActivated(QSystemTrayIcon::ActivationReason reason);
    void toggleGameActive();

private:
    void setupMenu();

    QMenu *m_menu;
    QAction *m_gameActiveToggle;
    QAction *m_menuButton;
    bool m_gameActive;
};

#endif // SYSTEMTRAYICON_H
