#include <QWindow>
#include <QQuickItem>
#include <QQuickView>
#include <QTimer>
#include <WildPokemon.h>
#include <globals.h>
#include <data_poke_asset.h>
#include <lookup.h>

WildPokemon::WildPokemon(int pokedexId, QPoint spawnPoint, int spawnDirection, bool pet, QWindow *parent)
    : DesktopScene(parent)
    , info(Lookup::getSpriteInfo(pokedexId))
    , m_decisionTimer(new QTimer(this))
    , m_moveTimer(new QTimer(this))
    , m_jumpTimer(new QTimer(this))
    , m_moveSpeed(1 + std::rand()%2000/1000)
    , m_pet(pet)
{
    qDebug() << "WildPokemon constructor called!";

    if(spawnPoint.x()>=0  && spawnPoint.y()>=0){
        setPosition(spawnPoint);
    }else{
        const QRect& screen = Globals::screenGeometry();
        setPosition(QPoint(screen.width()/2, screen.height()/2));
    }

    const char* env =  getenv("DOCKER_ENV");
    if(env && strcmp(env, "dev") == 0){
        setSource(QUrl("../qml/PokemonSprite.qml"));
    }else{
        setSource(QUrl("qrc:/qml/PokemonSprite.qml"));
    }

    bool isBig = info->spriteSheet==SpriteSheet::Big;
    m_sprite = rootObject();
    m_sprite->setProperty("spriteSheet",
        isBig
        ? QString("qrc:/assets/HGSS/reordered_sprites_big.png")
        : QString("qrc:/assets/HGSS/reordered_sprites.png"));

    float scaleDivisor = isBig ? 1.4/3.0f : 1.0f/3.0f;
    m_sprite->setProperty("scaleFactor", Globals::scale()/scaleDivisor) ;
    m_sprite->setProperty("row", info->rowId);
    m_sprite->setProperty("debugLines", Globals::debugLines());
    m_sprite->setProperty("frameWidth", isBig ? 64  : 32);
    m_sprite->setProperty("frameHeight", isBig ? 64  : 32);

    int width = Globals::scale()/scaleDivisor * info->width;
    int height = Globals::scale()/scaleDivisor * info->height;
    m_sprite->setProperty("itemWidth", width);
    m_sprite->setProperty("itemHeight", height);

    setWidth(width);
    setHeight(height);

    if(spawnDirection>-1 && spawnDirection<5){
        direction(spawnDirection);
    }
    else{
        direction(std::rand()%4);
    }

    m_decisionTimer->setInterval(2000 + std::rand()%2000);
    m_moveTimer->setInterval(50);
    m_jumpTimer->setInterval(m_jumpInterval);

    roaming(true);
    show();

    m_width  = this->width();
    m_height = this->height();

    connect(m_jumpTimer, &QTimer::timeout, this, &WildPokemon::jumpStep);
}

WildPokemon::~WildPokemon(){
    qDebug() << "WildPokemon destructor called!";
    disconnect(m_decisionTimer, nullptr, this, nullptr);
    disconnect(m_moveTimer, nullptr, this, nullptr);
    disconnect(m_jumpTimer, nullptr, this, nullptr);
    setSource(QUrl());
    m_sprite = nullptr;
};

void WildPokemon::roaming(bool active){
    m_isDragged = false;
    m_sprite->setProperty("frameRate", 4);

    if(active){
        connect(m_decisionTimer, &QTimer::timeout, this, &WildPokemon::makeRandomDecision, Qt::UniqueConnection);
        connect(m_moveTimer, &QTimer::timeout, this, &WildPokemon::moveStep, Qt::UniqueConnection);
        m_decisionTimer->start();
    }else{
        disconnect(m_decisionTimer, &QTimer::timeout, this, &WildPokemon::makeRandomDecision);
        disconnect(m_moveTimer, &QTimer::timeout, this, &WildPokemon::moveStep);
    }
}

void WildPokemon::mouseDoubleClickEvent(QMouseEvent* event){
    if(m_pet){
        if(!m_isJumping) windowJump();
    }else{
        startBattle();
    }
};

void WildPokemon::windowJump(){
    if (m_isJumping) return;
    qDebug() << "Yippeee!";
    m_isJumping = true;

    m_moveTimer->stop();
    m_decisionTimer->stop();

    int startY = y();
    int upTargetY = startY - m_jumpHeight*Globals::scale();
    const QRect& screen = Globals::screenGeometry();
    if (upTargetY < screen.top()) upTargetY = screen.top();

    int upSteps = m_jumpUpDuration / m_jumpInterval;
    int downSteps = m_jumpDownDuration / m_jumpInterval;
    if (upSteps < 1) upSteps = 1;
    if (downSteps < 1) downSteps = 1;

    QList<int> yPositions;
    yPositions.append(startY);

    for (int i = 1; i <= upSteps; ++i) {
        double t = (double)i / upSteps;
        double eased = 1 - (1 - t) * (1 - t);
        int currentY = startY + (upTargetY - startY) * eased;
        yPositions.append(currentY);
    }

    for (int i = 1; i <= downSteps; ++i) {
        double t = (double)i / downSteps;
        double eased = t * t;
        int currentY = upTargetY + (startY - upTargetY) * eased;
        yPositions.append(currentY);
    }

    yPositions.append(startY);

    m_jumpPositions = yPositions;
    m_jumpStep = 0;
    m_jumpTimer->start();
}

void WildPokemon::jumpStep(){
    if (m_jumpStep >= m_jumpPositions.size()) {
        m_jumpTimer->stop();
        m_isJumping = false;
        roaming(true);
        return;
    }

    int newY = m_jumpPositions[m_jumpStep];
    setY(newY);
    m_jumpStep++;
}

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
