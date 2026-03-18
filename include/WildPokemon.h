#ifndef WILDPOKEMON_H
#define WILDPOKEMON_H

#include <QQuickView>
#include <qevent.h>
#include "pokemon_data.h"
class WildPokemon : public QQuickView{

    Q_OBJECT

public:
    explicit WildPokemon(const PokemonInfo* info,  QWindow *parent = nullptr);
    void startRoaming();
    int direction();
    void direction(int direction);

private slots:
    void makeRandomDecision();
    void moveStep();
    void startBattle();
    /* void handleDoubleClick(); */
    void handleDrag(bool isDragged);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
private:
    const PokemonInfo* info;
    int m_currentDirection;
    QPointF m_oldPos;
    QQuickItem* m_sprite;
    QTimer* m_decisionTimer;
    QTimer* m_moveTimer;

    int m_moveSpeed;
};

#endif
