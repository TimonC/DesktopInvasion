#ifndef WILDPOKEMON_H
#define WILDPOKEMON_H

#include <DesktopScene.h>
#include <qevent.h>
#include "pokemon_data.h"
class WildPokemon : public DesktopScene{

    Q_OBJECT

public:
    explicit WildPokemon(const PokemonInfo* info,  QWindow *parent = nullptr);
    void startRoaming();
    int direction();
    void direction(int direction);

private slots:
    void handleDrag(bool isDragged) override;

    void makeRandomDecision();
    void moveStep();
    void startBattle();
    /* void handleDoubleClick(); */

private:
    const PokemonInfo* info;
    QQuickItem* m_sprite;
    QTimer* m_decisionTimer;
    QTimer* m_moveTimer;

    int m_moveSpeed;
};

#endif
