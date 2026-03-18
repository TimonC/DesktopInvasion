// Battle.cpp
#include "BattleMoveHandler.h"
#include "PokeMath/calculatePokeStats.h"
#include <Battle.h>
#include <globals.h>
#include <QTimer>
#include <qobjectdefs.h>

Battle::Battle(WildPokemon* opp, PokemonState wildState, Party party, std::unique_ptr<BattleMoveHandler> battleMoveHandler, QWindow *parent)
    : DesktopScene(parent)
    , m_oppReference(opp)
    , m_initialOppPos(opp->position())
    , m_battleMoveHandler(std::move(battleMoveHandler))
{
    qDebug() << "Battle constructor called!";


    m_currentDirection = opp->m_currentDirection;

    // Load the PokemonSprite as root
    setSource(QUrl("qrc:/qml/BattleScene.qml"));
    m_battleScene = rootObject();
    assert(m_battleScene);

    // Short delay before hiding for smooth transition
    QTimer::singleShot(100, this, [opp]() {
        opp->hide();
    });

    // Connect BattleMoveHandler's action sequence signal to Battle's slot
    connect(m_battleMoveHandler.get(), &BattleMoveHandler::actionSequenceReady,
            this, &Battle::executeActionSequence);

    // Connect BattleScene's action start signal to BattleMoveHandler
    connect(m_battleScene, SIGNAL(_startActionRound(int,QString)),
            m_battleMoveHandler.get(), SLOT(startActionRound(int, QString)));

    connect(m_battleScene, SIGNAL(_battleEnded(QString)),
            this, SLOT(handleBattleEnded(QString)));

    connect(m_battleScene, SIGNAL(switchedPokemon(int, int)),
        this, SLOT(handleSwitchedPokemon(int, int)));


    m_battleScene->setProperty("direction", m_currentDirection);
    m_battleScene->setProperty("pokeMargin", m_pokeMargin);
    m_battleScene->setProperty("debugLines", Globals::DEBUG);
    m_opp = setupPokemon(opp->info, "opponent");
    m_chosen = setupPokemon(Globals::getPokemonInfo(party.pokedexIds[0]), "player");
    initPosition();

    setupParty(party);

    int opponentHealth = calculateHealth(wildState.lvl,
        Globals::getPoke(opp->info->pokedexId)->base_stats[0],
        wildState.ivs[0],
        wildState.evs[0]);

    int playerHealth = party.healthTotals[0];
    qDebug() << opponentHealth;
    for(int i = 0; i<6; i++){
        qDebug() << party.healthTotals[i];
    }
    QMetaObject::invokeMethod(m_battleScene, "setInitialTotalHealth",
        Q_ARG(QVariant, opponentHealth),
        Q_ARG(QVariant, playerHealth));

    QTimer::singleShot(20, this, [this]() {
        show();
        m_width  = width();
        m_height = height();
    });
}
void Battle::handleSwitchedPokemon(int generation, int spriteId){
    updateSprite(spriteId, generation, "player");
}
void Battle::executeActionSequence(QVariantList sequence) {
    QMetaObject::invokeMethod(m_battleScene, "showTextBar");
    QMetaObject::invokeMethod(m_battleScene, "executeActionSequence", Q_ARG(QVariant, QVariant(sequence)));
}

void Battle::handleBattleEnded(QString endState){
    emit battleEnded(endState.toStdString().data());
}

void Battle::setupParty(Party party) {
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
            Q_ARG(QVariant, QVariant(party.spriteIds[i])),
            Q_ARG(QVariant, QVariant(party.iconIds[i])),
            Q_ARG(QVariant, QVariant(party.ballIds[i])),
            Q_ARG(QVariant, QVariant(party.gens[i])),
            Q_ARG(QVariant, QString::fromStdString(party.names[i])),
            Q_ARG(QVariant, QVariant(party.healthTotals[i])),
            Q_ARG(QVariant, moves));
    }
}

QQuickItem* Battle::setupPokemon(const PokemonInfo* info, const char* role) {
    m_battleScene->setProperty((QString(role) + "Name").toUtf8(), info->name);
    QQuickItem* pokemonSprite = updateSprite(info->spriteId, info->generation, role);
    QMetaObject::invokeMethod(m_battleScene, "positionSpriteAndStatusBar", Q_ARG(QVariant, QVariant::fromValue(pokemonSprite)));
    return pokemonSprite;
}

QQuickItem* Battle::updateSprite(int spriteId, int generation, const char* role){
    QQuickItem* pokemonSprite = m_battleScene->property(role).value<QQuickItem*>();
    Q_ASSERT_X(pokemonSprite, "Battle::setupPokemon", "pokemonSprite: '%1' is null".arg(role));
    /* // Set the basic sprite properties */
    QMetaObject::invokeMethod(pokemonSprite, "updatePokemon", Q_ARG(QVariant, generation), Q_ARG(QVariant, spriteId));
    pokemonSprite->setProperty("scaleFactor", Globals::SCALE);
    pokemonSprite->setProperty("debugLines", Globals::DEBUG);

    const SpriteInfo* spriteInfo = Globals::getSpriteInfo(spriteId, generation);

    // Calculate pokemonSprite size
    int width = Globals::SCALE * (spriteInfo->max_width + Globals::POKE_PADDING);
    int height = Globals::SCALE * (spriteInfo->max_height + Globals::POKE_PADDING);

    // Calculate positioning offsets
    int offsetX = Globals::SCALE * (32 - spriteInfo->max_width) / 2;
    int offsetY = Globals::SCALE * (32 - spriteInfo->max_height) / 2;

    // Set pokemonSprite properties
    pokemonSprite->setProperty("itemWidth", width);
    pokemonSprite->setProperty("itemHeight", height);
    pokemonSprite->setProperty("containerOffsetX", offsetX);
    pokemonSprite->setProperty("containerOffsetY", offsetY);

    // Position the sprite

    return pokemonSprite;
}


void Battle::initPosition() {
    QQuickItem* rootItem = qobject_cast<QQuickItem*>(m_battleScene);
    QQuickItem* opponentItem = m_battleScene->findChild<QQuickItem*>("opponent");
    // has to be QPointF for the method, but since we set offsets as integer in qml it gives smooth transition
    QPointF spriteOffset = opponentItem->mapToItem(rootItem, QPointF(0, 0));

    m_origin = m_initialOppPos + QPoint(-spriteOffset.x(), -spriteOffset.y());
    QRect screenGeom = Globals::screenGeometry();

    QPoint originalOrigin = QPoint(m_origin);
    QTimer::singleShot(20, this, [this, screenGeom, originalOrigin]() {

        //snap battle to bounds
        m_origin.setX(qBound(screenGeom.left(), m_origin.x(), screenGeom.right()-width()));
        m_origin.setY(qBound(screenGeom.top(), m_origin.y(), screenGeom.bottom()-height()));
        setPosition(m_origin);

        //reposition wild pokemon if there was any snap
        if(m_origin!=originalOrigin){
            m_initialOppPos = m_initialOppPos - (originalOrigin - m_origin);
            m_oppReference->setPosition(m_initialOppPos);
        }
    });
}

void Battle::direction(int direction) { m_currentDirection = direction; }
void Battle::handleDrag(bool isDragged) { m_isDragged = isDragged; }
