#ifndef WILDPOKEMON_H
#define WILDPOKEMON_H

#include "Pokemon.h"
class WildPokemon : public Pokemon{

    Q_OBJECT

public:
    explicit WildPokemon(QWindow *parent = nullptr, int row = 0);
    virtual void startRoaming();
    QQuickView* m_hitbox;

private slots:
    void makeRandomDecision();
    void moveStep();
    void onSelect();
    void stopOpening();
    void systemMove();
public slots:
    void startBattle();

private:
    void setupHitbox();
    QTimer* m_decisionTimer;
    QTimer* m_moveTimer;

    int m_moveSpeed;
    void startOpening(int durationMs = 5000);
};

#endif
