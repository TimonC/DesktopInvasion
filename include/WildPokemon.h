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
    void direction(int direction) override;
    const PokemonInfo* info;
private slots:
    void handleDrag(bool isDragged) override;

    void makeRandomDecision();
    void moveStep();
    void startBattle();
    /* void handleDoubleClick(); */
protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
private:
    QQuickItem* m_sprite;
    QTimer* m_decisionTimer;
    QTimer* m_moveTimer;

    int m_moveSpeed;
};

#endif
