#ifndef WILDPOKEMON_H
#define WILDPOKEMON_H

#include <DesktopScene.h>
#include <data_poke_asset.h>
#include <qtmetamacros.h>

class WildPokemon : public DesktopScene{
    Q_OBJECT
    Q_DISABLE_COPY(WildPokemon)

public:
    explicit WildPokemon(int pokedexId, QPoint spawnPoint = QPoint(-1,-1), int spawnDirection = -1, QWindow *parent = nullptr);

    ~WildPokemon();

    void roaming(bool active);
    void direction(int direction) override;
    const AssetInfo* info;
    QQuickItem* m_sprite;

public slots:
    void handleDrag(bool isDragged) override;

signals:
    void startABattle();


private slots:
    void makeRandomDecision();
    void moveStep();
    void startBattle();
    /* void handleDoubleClick(); */
protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
private:
    QTimer* m_decisionTimer;
    QTimer* m_moveTimer;
    QPoint spawnPoint;
    int m_moveSpeed;
};

#endif
