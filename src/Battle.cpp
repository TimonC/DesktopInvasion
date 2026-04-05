#include "BattleMoveHandler.h"
#include "data_poke_asset.h"
#include <Battle.h>
#include <globals.h>
#include <PokeMath.h>
#include <QTimer>
#include <qglobal.h>
#include <qobjectdefs.h>
#include <lookup.h>

Battle::Battle(QPoint initialOppPos, int initialOppDirection, PokemonState wildState, const Party& party, std::array<int, 3> balls, std::string trainerName, std::unique_ptr<BattleMoveHandler> battleMoveHandler, QWindow *parent)
    : DesktopScene(parent)
    , m_oppPos(initialOppPos)
    , m_battleMoveHandler(std::move(battleMoveHandler))
    , m_trainerName(QString::fromStdString(trainerName))
{
    qDebug() << "Battle constructor called!";

    direction(initialOppDirection);

#ifdef DEV_MODE
        setSource(QUrl("../qml/BattleScene.qml"));
#else
        setSource(QUrl("qrc:/qml/BattleScene.qml"));
#endif

    m_battleScene = rootObject();
    assert(m_battleScene);

    connect(m_battleMoveHandler.get(), &BattleMoveHandler::actionSequenceReady,
            this, &Battle::executeActionSequence);

    connect(m_battleScene, SIGNAL(signalToStartActionRound(int,QString)),
            m_battleMoveHandler.get(), SLOT(startActionRound(int, QString)));

    connect(m_battleScene, SIGNAL(_battleEnded(QString, bool)),
            this, SLOT(handleBattleEnded(QString, bool)));

    connect(m_battleScene, SIGNAL(switchedPokemon(int, int)),
        this, SLOT(handleSwitchedPokemon(int, int)));

    connect(m_battleScene, SIGNAL(requestExperienceSpread()),
        this, SLOT(handleGettingExperience()));

    connect(m_battleScene, SIGNAL(requestBallCountUpdate(int, int)),
        this, SLOT(handleBallCountUpdate(int, int)));


    QMetaObject::invokeMethod(m_battleScene, "setBalls",
        Q_ARG(QVariant, balls[0]),
        Q_ARG(QVariant, balls[1]),
        Q_ARG(QVariant, balls[2]));
    m_battleScene->setProperty("direction", m_currentDirection);
    m_battleScene->setProperty("pokeMargin", m_pokeMargin);
    m_battleScene->setProperty("debugLines", Globals::debugLines());
    m_battleScene->setProperty("animationSpeed", Globals::animationSpeed());
    m_battleScene->setProperty("scale", Globals::scale());
    m_battleScene->setProperty("trainerName", m_trainerName);
    m_opp = setupPokemon(wildState.pokedex_id, wildState.name,wildState.lvl, "opponent");
    m_chosen = setupPokemon(party.pokedexIds[0], party.names[0], party.lvls[0], "player");
    initPosition();

    setupParty(party);

    int opponentHealth = PokeMath::calculateHealth(wildState.lvl,
        Lookup::getPoke(wildState.pokedex_id)->base_stats[0]);
    int playerHealth = party.healthTotals[0];
    QMetaObject::invokeMethod(m_battleScene, "setInitialTotalHealth",
        Q_ARG(QVariant, opponentHealth),
        Q_ARG(QVariant, playerHealth));

    QTimer::singleShot(20, this, [this]() {
        show();
        m_width  = width();
        m_height = height();
    });
}

Battle::~Battle(){
    qDebug() << "Battle destructor called!";

    setSource(QUrl());

    m_battleScene = nullptr;
    m_opp = nullptr;
    m_chosen = nullptr;
    m_ui = nullptr;

    m_chosen_info = nullptr;
    m_opp_info = nullptr;

};

void Battle::handleBallCountUpdate(int delta, int row){
    emit updateBallCount(delta,row);
}
void Battle::handleGettingExperience(){
    std::array<int,6> spread = m_battleMoveHandler->getExperienceSpread();
    emit _updatePartyXP(spread);
}

void Battle::showUpdateAndEndBattle(std::array<int,6> spread, std::array<int,6> lvlUps, std::array<QString,6> evolves, int tmGet, int ballGet, int whichBall){
    QVector<int> qmlSpread;
    QVector<int> qmlLvlUps;
    QVector<QString> qmlEvolves;

    for (int i = 0; i < 6; i++) {
        qmlSpread.append(spread[i]);
        qmlLvlUps.append(lvlUps[i]);
        qmlEvolves.append(evolves[i]);
    }



    std::string tmName = (tmGet > 0) ? std::string(Lookup::getMove(tmGet)->name) : "NONE";
    QMetaObject::invokeMethod(m_battleScene, "showExperienceSpreadSequence",
                              Q_ARG(QVariant, QVariant::fromValue(qmlSpread)),
                              Q_ARG(QVariant, QVariant::fromValue(qmlLvlUps)),
                              Q_ARG(QVariant, QVariant::fromValue(qmlEvolves)),
                              Q_ARG(QVariant, QString::fromStdString(tmName)),
                              Q_ARG(QVariant, ballGet),
                              Q_ARG(QVariant, whichBall)
                              );
}

void Battle::handleSwitchedPokemon(int pokedexId, int partyIndex){
    updateSprite(pokedexId, "player");
    QString label = m_battleMoveHandler.get()->switchPartyMember(partyIndex);
    QMetaObject::invokeMethod(m_battleScene, "updatePlayerStatusAilment", Q_ARG(QVariant, label));

}

void Battle::executeActionSequence(QVariantList sequence) {
    QMetaObject::invokeMethod(m_battleScene, "showTextBar");
    QMetaObject::invokeMethod(m_battleScene, "executeActionSequence", Q_ARG(QVariant, QVariant(sequence)));
}

void Battle::handleBattleEnded(QString endState, bool removeWild){
    emit battleEnded(endState.toStdString().data(), removeWild);
}

void Battle::setupParty(const Party& party) {
    for(size_t i = 0; i < party.spriteIds.size(); i++) {

        QVariantList moves;
        for(int moveSlot = 0; moveSlot < 4; moveSlot++) {
            BattleMove _move = party.moves[i][moveSlot];
            QVariantMap moveData;
            moveData["name"] = QString::fromStdString(_move.name);
            moveData["type"] = QString::fromStdString(_move.type);
            moves.append(moveData);
        }

        QMetaObject::invokeMethod(m_battleScene, "setPartyMember",
            Q_ARG(QVariant, QVariant(static_cast<int>(i))),
            Q_ARG(QVariant, QVariant(party.pokedexIds[i])),
            Q_ARG(QVariant, QVariant(party.spriteIds[i])),
            Q_ARG(QVariant, QVariant(party.ballIds[i])),
            Q_ARG(QVariant, QString::fromStdString(party.names[i])),
            Q_ARG(QVariant, QVariant(party.lvls[i])),
            Q_ARG(QVariant, QVariant(party.healthTotals[i])),
            Q_ARG(QVariant, moves));
    }
}

QQuickItem* Battle::setupPokemon(int pokedexId, const std::string& name, int level, const char* role) {
    m_battleScene->setProperty((QString(role) + "Name").toUtf8(), QString::fromStdString(name));
    m_battleScene->setProperty((QString(role) + "LevelText").toUtf8(), QString("Lv" + QString::number(level)));
    QQuickItem* pokemonSprite = updateSprite(pokedexId, role);
    QMetaObject::invokeMethod(m_battleScene, "positionSpriteAndStatusBar", Q_ARG(QVariant, QVariant::fromValue(pokemonSprite)));
    return pokemonSprite;
}

QQuickItem* Battle::updateSprite(int pokedexId, const char* role){
    const AssetInfo* info = Lookup::getSpriteInfo(pokedexId);
    bool isBig = info->spriteSheet == SpriteSheet::Big;
    float scaleDivisor = isBig ? 1.4/3.0f : 1.0f/3.0f;

    QQuickItem* pokemonSprite = m_battleScene->property(role).value<QQuickItem*>();

    // Update sprite properties
    QMetaObject::invokeMethod(pokemonSprite, "updatePokemon",
        Q_ARG(QVariant, info->rowId),
        Q_ARG(QVariant, isBig));

    // Apply correct scaling
    pokemonSprite->setProperty("scaleFactor", Globals::scale() / scaleDivisor);
    pokemonSprite->setProperty("debugLines", Globals::debugLines());
    pokemonSprite->setProperty("horizontalHeight", Globals::scale()/scaleDivisor * info->hHeight);
    pokemonSprite->setProperty("verticalWidth", Globals::scale()/scaleDivisor * info->vWidth);
    pokemonSprite->setProperty("hCenterUp", Globals::scale()/scaleDivisor * info->hCenterUp);
    pokemonSprite->setProperty("hCenterDown", Globals::scale()/scaleDivisor * info->hCenterDown);

    // Calculate dimensions with isBig scaling
    int width = (Globals::scale() / scaleDivisor) * info->width ;
    int height = (Globals::scale() / scaleDivisor) * info->height;


    pokemonSprite->setProperty("itemWidth", width);
    pokemonSprite->setProperty("itemHeight", height);
    pokemonSprite->setProperty("frameWidth", isBig ? 64 : 32);
    pokemonSprite->setProperty("frameHeight", isBig ? 64 : 32);

    return pokemonSprite;
}

void Battle::initPosition() {
    QQuickItem* rootItem = qobject_cast<QQuickItem*>(m_battleScene);
    QQuickItem* opponentItem = m_battleScene->findChild<QQuickItem*>("opponent");
    QPointF floatOffset = opponentItem->mapToItem(rootItem, QPointF(0, 0));
    m_spriteOffset = QPoint(qRound(floatOffset.x()), qRound(floatOffset.y()));

    QPoint origin = m_oppPos - m_spriteOffset;
    QRect screenGeom = Globals::screenGeometry();

    QTimer::singleShot(1, this, [this, screenGeom, origin]() {
        QPoint adjustedOrigin = origin;

        adjustedOrigin.setX(qBound(screenGeom.left(), adjustedOrigin.x(),
                                 screenGeom.right() - width()));
        adjustedOrigin.setY(qBound(screenGeom.top(), adjustedOrigin.y(),
                                 screenGeom.bottom() - height()));

        setPosition(adjustedOrigin);

        if(adjustedOrigin != origin) {
            m_oppPos = m_oppPos - (origin - adjustedOrigin);
        }
    });
}

void Battle::direction(int direction) { m_currentDirection = direction; }
void Battle::handleDrag(bool isDragged) {m_isDragged = isDragged;}
