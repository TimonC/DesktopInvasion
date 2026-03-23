#ifndef SYSTEMTRAYICON_H
#define SYSTEMTRAYICON_H

#include <QSystemTrayIcon>
#include <QMenu>

class SystemTrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

public:
    explicit SystemTrayIcon(QObject *parent = nullptr);
    ~SystemTrayIcon();

    void setIconActivityColor(bool active);
    void enabled(bool enabled);

signals:
    void gameActive(bool active);
    void menuButtonPressed();

private slots:
    void toggleGameActive();

private:
    void createContextMenu();
    bool m_gameActive;
    bool m_clickEnabled;
    QIcon m_activeIcon;
    QIcon m_inactiveIcon;

    QMenu* m_menu;
    QAction* m_quitAction;
    QAction* m_activeAction;
    QAction* m_menuAction;
};

#endif
