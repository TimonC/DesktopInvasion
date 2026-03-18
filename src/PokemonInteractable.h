#ifndef POKEMONINTERACTABLE_H
#define POKEMONINTERACTABLE_H

#include <QQuickView>

class PokemonInteractable : public QQuickView
{
    Q_OBJECT

public:
    explicit PokemonInteractable(QWindow *parent = nullptr, int row = 0);

private slots:
    void makeRandomDecision();
    void moveStep();
    void onClick();
    void stopOpening();

private:
    QTimer* m_decisionTimer;
    QTimer* m_moveTimer;
    QTimer* m_openingTimer;

    QQuickItem* m_wildPokemon;

    int m_row;

    int m_scaleFactor;
    int m_moveSpeed;
    int m_currentDirection;
    QRect m_screenGeometry;

    int m_minX = 1e5;
    int m_maxX = 0;
    int m_minY = 1e5;
    int m_maxY = 0;
    static constexpr int SPRITE_SIZE = 32 * 4;

    void startOpening(int durationMs = 5000);
};

#endif
