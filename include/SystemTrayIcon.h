#ifndef SYSTEMTRAYICON_H
#define SYSTEMTRAYICON_H

#include <QSystemTrayIcon>
#include <QMenu>

class SystemTrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

public:
    explicit SystemTrayIcon(QObject *parent = nullptr);

    // Public methods
    void swapIcon();

signals:
    void gameActive(bool active);

private slots:
    void onActivated(QSystemTrayIcon::ActivationReason reason);
    void onHideShowTriggered();

private:
    void setupMenu();
    void updateIconAndMenu();

    QMenu *m_menu;
    QAction *m_hideShowAction;
    bool m_gameActive;
};

#endif // SYSTEMTRAYICON_H
