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
    ~GameMenu();

    void activate();

    void loadParty(const QVariantList& data, bool displayFirst);
    void loadBox(int boxIndex, const QVariantList& data);
    void showBox(int boxIndex);
    void updateEvolveMenu(QVariantMap evolvesData);

    void setDefaults(const Defaults &d);
    void setTrainer(const QString& name, int trainerId);

    QQuickItem* m_menuRoot;

public slots:
    void evolvePokemon(int boxIndex, int slot, int targetPokedexId, const QString& nickName);

signals:
    void menuClosed();
    void preloadBoxRequested(int boxIndex);
    void swapRequested(int xplace, int xpos, int yplace, int ypos);
    void nameChangeRequested(int xplace, int xpos, const QString& name);
    void moveChangeRequested(int xplace, int xpos, int moveSlot, int moveId);
    void evolvesRequested(int boxIndex, int slot, QVariantMap pokeData);
    void evolvePokemonRequested(int boxIndex, int slot, int targetPokedexId, const std::string& nickName);

    void partyDataReady(const QVariantList& data, bool displayFirst);
    void boxDataReady(int boxIndex, const QVariantList& data);
    void showBoxRequested(int boxIndex);

protected:
    bool event(QEvent* event) override;
};

#endif
