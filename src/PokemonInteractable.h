#ifndef POKEMONINTERACTABLE_H
#define POKEMONINTERACTABLE_H

#include <QQuickView>

class PokemonInteractable : public QQuickView
{
    Q_OBJECT

public:
    explicit PokemonInteractable(QWindow *parent = nullptr);

private slots:
    void makeRandomDecision();
    void moveStep();

private:
    QTimer* m_decisionTimer;
    QTimer* m_moveTimer;

    QQuickItem* m_wildPokemon;

    int m_moveSpeed;
    int m_currentDirection;
    QRect m_screenGeometry;

    static constexpr int SPRITE_SIZE = 32 * 4;
};

#endif
