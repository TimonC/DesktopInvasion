#ifndef SYSTEMTRAYICON_H
#define SYSTEMTRAYICON_H
#include <QSystemTrayIcon>
#include <QMenu>
#include <string>
#include <vector>

class SystemTrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    explicit SystemTrayIcon(QObject *parent = nullptr);
    ~SystemTrayIcon();
    void setIconActivityColor(bool active);
    void enabled(bool enabled);
    void createContextMenu(std::vector<std::pair<int, std::string>> trainers, int activeSaveId);
    void setActiveSaveId(int id) { m_activeSaveId = id; }

signals:
    void gameActive(bool active);
    void menuButtonPressed();
    void saveSelected(int saveId);
    void deleteSaveRequested();
    void newGameRequested();

public slots:
    void toggleGameActive();

private:
    bool   m_gameActive;
    bool   m_clickEnabled;
    int    m_activeSaveId = -1;
    QIcon  m_activeIcon;
    QIcon  m_inactiveIcon;
    QMenu*   m_menu;
    QAction* m_quitAction;
    QAction* m_activeAction;
    QAction* m_menuAction;
    QAction* m_newGameAction;
    QAction* m_deleteAction;
};
#endif
