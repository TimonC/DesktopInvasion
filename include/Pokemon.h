#ifndef Pokemon_H
#define Pokemon_H

#include <QQuickView>

class Pokemon : public QQuickView
{
    Q_OBJECT

public:
    explicit Pokemon(QWindow *parent = nullptr, int row = 0);
    void direction(int direction);
    int direction();
    int m_row;
    int m_scaleFactor;
protected:
    QQuickItem* m_sprite;
    int m_currentDirection;

    int m_minX = 1e5;
    int m_maxX = 0;
    int m_minY = 1e5;
    int m_maxY = 0;
    static constexpr int SPRITE_SIZE = 32 * 4;

};

#endif
