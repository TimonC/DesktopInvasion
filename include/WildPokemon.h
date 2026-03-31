#ifndef WILDPOKEMON_H
#define WILDPOKEMON_H

#include <DesktopScene.h>
#include <data_poke_asset.h>
#include <qtmetamacros.h>

class WildPokemon : public DesktopScene{
    Q_OBJECT
    Q_DISABLE_COPY(WildPokemon)

public:
    explicit WildPokemon(int pokedexId, QPoint spawnPoint = QPoint(-1,-1), int spawnDirection = -1, bool pet = false, QWindow *parent = nullptr);
    ~WildPokemon();

    void roaming(bool active);
    void direction(int direction) override;

    const AssetInfo* info;
    QQuickItem* m_sprite;

public slots:
    void handleDrag(bool isDragged) override;

signals:
    void startABattle();

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private slots:
    void makeRandomDecision();
    void moveStep();
    void startBattle();
    void jumpStep();

private:
    void windowJump();

    QTimer* m_decisionTimer;
    QTimer* m_moveTimer;
    QTimer* m_jumpTimer;
    QPoint spawnPoint;
    int m_moveSpeed;
    bool m_pet;

    QList<int> m_jumpPositions;
    int m_jumpStep = 0;

    int m_jumpHeight = 8;
    int m_jumpUpDuration = 200;
    int m_jumpDownDuration = 150;
    int m_jumpInterval = 20;
};

#endif
