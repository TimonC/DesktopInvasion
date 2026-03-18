#ifndef WILDPOKEMON_H
#define WILDPOKEMON_H

#include "Pokemon.h"
#include "Hitbox.h"
class WildPokemon : public Pokemon{

    Q_OBJECT

public:
    explicit WildPokemon(QWindow *parent = nullptr, int row = 0);
    virtual void startRoaming();

private slots:
    void makeRandomDecision();
    void moveStep();
    void startBattle();
    void handleDoubleClick();
public slots:
    QPoint move(QPoint delta);
private:
    Hitbox* m_hitbox;
    QTimer* m_decisionTimer;
    QTimer* m_moveTimer;

    int m_moveSpeed;
};

#endif
