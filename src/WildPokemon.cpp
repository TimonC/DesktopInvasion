#include <QWindow>
#include <QQuickItem>
#include <QQuickView>
#include <QTimer>
#include <WildPokemon.h>
#include <globals.h>
#include <data_poke_asset.h>

WildPokemon::WildPokemon(const PokemonInfo* info, QPoint spawnPoint, int spawnDirection,  QWindow *parent)
    : DesktopScene(parent)
    , info(info)
    , m_decisionTimer(new QTimer(this))
    , m_moveTimer(new QTimer(this))
    , m_moveSpeed(1 + std::rand()%2000/1000)
{
    qDebug() << "WildPokemon constructor called!";

    if(spawnPoint.x()>=0  && spawnPoint.y()>=0){
        setPosition(spawnPoint);
    }else{
        const QRect& screen = Globals::screenGeometry();
        setPosition(QPoint(screen.width()/2, screen.height()/2));
    }

    setSource(QUrl("qrc:/qml/PokemonSprite.qml"));
    m_sprite = rootObject();
    m_sprite->setProperty("spriteSheet", QString("qrc:/assets/HGSS/PokGen%1_transparent_reordered.png").arg(info->generation));
    m_sprite->setProperty("scaleFactor", Globals::SCALE);
    m_sprite->setProperty("row", info->spriteId);
    m_sprite->setProperty("debugLines", Globals::DEBUG);

    const SpriteInfo* spriteInfo = Globals::getSpriteInfo(info->spriteId, info->generation);

    int width = Globals::SCALE* (spriteInfo->max_width + Globals::POKE_PADDING);
    int height = Globals::SCALE * (spriteInfo->max_height + Globals::POKE_PADDING);

    m_sprite->setProperty("itemWidth", width);
    m_sprite->setProperty("itemHeight", height);

    setWidth(width);
    setHeight(height);

    m_sprite->setProperty("spriteOffsetX" , Globals::POKE_PADDING/2);
    m_sprite->setProperty("spriteOffsetY", Globals::POKE_PADDING/2);

    if(spawnDirection>-1 && spawnDirection<5){
        direction(spawnDirection);
    }
    else{
        direction(std::rand()%4);
    }

    m_decisionTimer->setInterval(2000 + std::rand()%2000);
    m_moveTimer->setInterval(50); // 20fps


    qDebug() << "A wild" << info->name << "(#" <<info->pokedexId << ") appeared!"
             << "width:" << spriteInfo->max_width << "height:" << spriteInfo->max_height ;

    roaming(true);
    show();

    QTimer::singleShot(20, this, [this]() {
        m_width  = this->width();
        m_height = this->height();
    });
}

WildPokemon::~WildPokemon(){
    qDebug() << "WildPokemon destructor called, with info: " << info->name;
    m_moveTimer->stop();
    m_decisionTimer->stop();
    delete m_decisionTimer;
    delete m_moveTimer;

};

void WildPokemon::roaming(bool active){
    m_isDragged = false;
    m_sprite->setProperty("frameRate", 4);

    if(active){
        connect(m_decisionTimer, &QTimer::timeout, this, &WildPokemon::makeRandomDecision);
        connect(m_moveTimer, &QTimer::timeout, this, &WildPokemon::moveStep);
        m_decisionTimer->start();
    }else{
        disconnect(m_decisionTimer, &QTimer::timeout, this, &WildPokemon::makeRandomDecision);
        disconnect(m_moveTimer, &QTimer::timeout, this, &WildPokemon::moveStep);
    }
}



void WildPokemon::mouseDoubleClickEvent(QMouseEvent* event){
    startBattle();
};
void WildPokemon::handleDrag(bool isDragged){
    m_isDragged = isDragged;
    if(isDragged){
        m_sprite->setProperty("frameRate", 8);
        m_moveTimer->stop();
        m_decisionTimer->stop();
    }else{
        m_sprite->setProperty("frameRate", 4);
        m_decisionTimer->start();
    }
}

void WildPokemon::direction(int direction){
    m_currentDirection = direction%4;
    m_sprite->setProperty("direction",m_currentDirection);
};

void WildPokemon::startBattle(){
    m_moveTimer->disconnect();
    m_decisionTimer->disconnect();

    const int BOUNDARY_MARGIN = 16;
    const QRect& screen = Globals::screenGeometry();

    bool intop = y() < screen.y() + BOUNDARY_MARGIN;
    bool inbottom = y() + height() > screen.bottom() - BOUNDARY_MARGIN;
    bool inleft = x() < screen.x() + BOUNDARY_MARGIN;
    bool inright = x() + width() > screen.right() - BOUNDARY_MARGIN;

    if(inright){
        direction(1);
    }else if(inleft){
        direction(3);
    }else if(inbottom){
        direction(0);
    }else if(intop){
        direction(2);
    }
    emit startABattle();
}

void WildPokemon::makeRandomDecision(){
    int decision = std::rand()%8;
    direction(decision/2);

    bool moving = (decision % 2) == 1;
    if (moving) {
        m_moveTimer->start();
    } else {
        m_moveTimer->stop();
    }
}


void WildPokemon::moveStep(){
    QPoint delta;
    switch (m_currentDirection) {
        case 0: delta = QPoint(0, -m_moveSpeed); break;
        case 1: delta = QPoint(-m_moveSpeed, 0); break;
        case 2: delta = QPoint(0, m_moveSpeed); break;
        case 3: delta = QPoint(m_moveSpeed, 0); break;
    }

    QPoint newPos = position()+delta;

    QRect screenGeom = Globals::screenGeometry();
    newPos.setX(qBound(screenGeom.left(), newPos.x(), screenGeom.right()- m_width));
    newPos.setY(qBound(screenGeom.top(), newPos.y(), screenGeom.bottom() - m_height));

    setPosition(newPos);
}
