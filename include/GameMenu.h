#pragma once

#include <QQuickView>
#include <QCursor>
#include <QVariantList>

class GameMenu : public QQuickView {
    Q_OBJECT
public:
    GameMenu();
    ~GameMenu(){
        qDebug() << "GameMenu destructor called!";
    };
    void activate();

    // Called by Game — emit signals that QML listens to via Connections
    void loadParty(const QVariantList& data);
    void loadBox(int boxIndex, const QVariantList& data);
    void showBox(int boxIndex);

signals:
    void menuClosed();
    void preloadBoxRequested(int boxIndex);  // QML -> Game
    void swapRequested(int xplace, int xpos, int yplace, int ypos);
    void nameChangeRequested(int xplace, int xpos, QString name);
    void moveChangeRequested(int xplace, int xpos, int moveSlot, int moveId);

    // Game -> QML (QML listens via Connections { target: menuBridge })
    void partyDataReady(QVariantList data);
    void boxDataReady(int boxIndex, QVariantList data);
    void showBoxRequested(int boxIndex);

protected:
    bool event(QEvent* event) override;

private:
    QCursor  m_grabCursor;
    QCursor  m_pointerCursor;
    QObject* qmlRoot();
};
