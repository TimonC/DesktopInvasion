#ifndef WILDPOKEMON_H
#define WILDPOKEMON_H

#include <DesktopScene.h>
#include <pokemon_data.h>

class WildPokemon : public DesktopScene{

    Q_OBJECT

public:
    explicit WildPokemon(const PokemonInfo* info,  QWindow *parent = nullptr);
    ~WildPokemon(){qDebug() << "WildPokemon destructor called, with info: " << info->name;};
    void startRoaming();
    void direction(int direction) override;
    const PokemonInfo* info;
    QQuickItem* m_sprite;
private slots:
    void handleDrag(bool isDragged) override;

    void makeRandomDecision();
    void moveStep();
    void startBattle();
    /* void handleDoubleClick(); */
protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
private:
    QTimer* m_decisionTimer;
    QTimer* m_moveTimer;

    int m_moveSpeed;
};

#endif
