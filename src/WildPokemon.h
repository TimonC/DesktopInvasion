#ifndef WILDPOKEMON_H
#define WILDPOKEMON_H

#include "Pokemon.h"

class WildPokemon : public Pokemon{

    Q_OBJECT

public:
    explicit WildPokemon(QWindow *parent = nullptr, int row = 0);

private slots:
    void makeRandomDecision();
    void moveStep();
    void onClick();
    void stopOpening();
    void startBattle();

private:
    QTimer* m_decisionTimer;
    QTimer* m_moveTimer;
    QTimer* m_openingTimer;

    int m_moveSpeed;
    void startOpening(int durationMs = 5000);
};

#endif
