#ifndef GAMEMENU_H
#define GAMEMENU_H

#include <QQuickView>
#include <QCursor>
#include <QVariantList>
#include <PokemonDatabase.h>
#include <data_move.h>

class GameMenu : public QQuickView {
    Q_OBJECT
public:
    GameMenu();
    ~GameMenu(){
        qDebug() << "GameMenu destructor called!";
    };
    void activate();

    void loadParty(const QVariantList& data, bool displayFirst);
    void loadBox(int boxIndex, const QVariantList& data);
    void showBox(int boxIndex);
    void _evolvesRequested(int boxIndex, QVariantMap pokeData);
    void updateEvolveMenu(QVariantMap evolvesData);

    void setDefaults(Defaults &d);
    void setTrainer(QString name, int trainerId);

    QQuickItem* m_menuRoot;

signals:
    void menuClosed();
    void preloadBoxRequested(int boxIndex);  // QML -> Game
    void swapRequested(int xplace, int xpos, int yplace, int ypos);
    void nameChangeRequested(int xplace, int xpos, QString name);
    void moveChangeRequested(int xplace, int xpos, int moveSlot, int moveId);
    void evolvesRequested(int boxIndex, QVariantMap pokeData);

    void partyDataReady(QVariantList data, bool displayFirst);
    void boxDataReady(int boxIndex, QVariantList data);
    void showBoxRequested(int boxIndex);

protected:
    bool event(QEvent* event) override;
};

#endif
