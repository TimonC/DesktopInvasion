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

private slots:
    void onActivated(QSystemTrayIcon::ActivationReason reason);
    void onGameActiveToggled();

private:
    void setupMenu();
    void toggleGameActive();

    QMenu *m_menu;
    QAction *m_gameActiveToggle;
    bool m_gameActive;
};

#endif // SYSTEMTRAYICON_H
