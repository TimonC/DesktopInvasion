#ifndef WILDPOKEMON_H
#define WILDPOKEMON_H

#include "Pokemon.h"
#include "Hitbox.h"
class WildPokemon : public Pokemon{

    Q_OBJECT

public:
    explicit WildPokemon(QWindow *parent = nullptr, int row = 0);
    void startRoaming() override;

private slots:
    void makeRandomDecision();
    void moveStep();
    void startBattle();
    void handleDoubleClick();
    void handleDrag(bool isDragged);
public slots:
    QPoint movePos(QPoint delta) override;
private:
    Hitbox* m_hitbox;
    QTimer* m_decisionTimer;
    QTimer* m_moveTimer;

    int m_moveSpeed;
};

#endif
