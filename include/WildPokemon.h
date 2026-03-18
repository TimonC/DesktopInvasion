#ifndef WILDPOKEMON_H
#define WILDPOKEMON_H

#include <DesktopScene.h>
#include <pokemon_data.h>
#include <qtmetamacros.h>
#include <optional>

class WildPokemon : public DesktopScene{
    Q_OBJECT

public:
    explicit WildPokemon(const PokemonInfo* info, std::optional<QPoint> spawnPoint = std::nullopt, int spawnDirection = -1, QWindow *parent = nullptr);
    ~WildPokemon(){qDebug() << "WildPokemon destructor called, with info: " << info->name;};
    void roaming(bool active);
    void direction(int direction) override;
    const PokemonInfo* info;
    QQuickItem* m_sprite;

public slots:
    void handleDrag(bool isDragged) override;

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
