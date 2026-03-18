#include <Battle.h>
#include <globals.h>
#include <QTimer>

Battle::Battle(WildPokemon* opp, const PokemonInfo* chosen_info, QWindow *parent)
    : DesktopScene(parent)
    , m_oppReference(opp)
    , m_initialOppPos(opp->position())
    , m_opp_info(opp->info)
    , m_chosen_info(chosen_info)
{
    qDebug() << "Battle constructor called!";
    m_currentDirection = opp->m_currentDirection;

    // Load the PokemonSprite as root
    setSource(QUrl("qrc:/qml/BattleScene.qml"));
    m_battleScene = rootObject();
    assert(m_battleScene);

    // Short delay before hiding for smooth transition
    QTimer::singleShot(50, this, [opp]() {
        opp->hide();
    });

    // Use connectWithQML for clean signal handling
    connectWithQML(SIGNAL(runChosen()), [this]() {
        handleRunChosen();
    });

    connectWithQML(SIGNAL(opponentWon()), [this]() {
        handleOpponentWon();
    });

    connectWithQML(SIGNAL(playerWon()), [this]() {
        handlePlayerWon();
    });

    connectWithQML(SIGNAL(pokemonCaught()), [this]() {
        handlePokemonCaught();
    });

    m_battleScene->setProperty("direction", m_currentDirection);
    m_battleScene->setProperty("pokeMargin", m_pokeMargin);
    m_battleScene->setProperty("debugLines", Globals::DEBUG);
    m_opp = setupPokemon(m_opp_info, "opponent");
    m_chosen = setupPokemon(m_chosen_info, "player");
    initPosition();
    show();
}

void Battle::handleRunChosen() {
    QPoint currentBattlePos = position();
    QPoint delta = currentBattlePos - m_origin;
    QPoint newOppPos = m_oppReference->position() + delta;

    // Delay slightly to ensure reposition
    QTimer::singleShot(2, this, [this, newOppPos]() {
        if (m_oppReference) {
            m_oppReference->setPosition(newOppPos);
            m_oppReference->startRoaming();
            m_oppReference->show();
        }
        QTimer::singleShot(50, this, [this]() { //Short delay to ensure smooth visual transition
            emit battleEnded(this, m_oppReference, false);
        });
    });
}

void Battle::handleOpponentWon() {
    QPoint currentBattlePos = position();
    QPoint delta = currentBattlePos - m_origin;
    QPoint newOppPos = m_oppReference->position() + delta;

    QTimer::singleShot(2, this, [this, newOppPos]() {
        if (m_oppReference) {  // Safety check
            m_oppReference->setPosition(newOppPos);
            m_oppReference->startRoaming();
            m_oppReference->show();
        }
        emit battleEnded(this, m_oppReference, false);
    });
}

void Battle::handlePlayerWon() {
    emit battleEnded(this, m_oppReference, true);
}

void Battle::handlePokemonCaught() {
    emit battleEnded(this, m_oppReference, true);
}

QQuickItem* Battle::setupPokemon(const PokemonInfo* info, const char* role) {
    QQuickItem* pokemonSprite = m_battleScene->property(role).value<QQuickItem*>();
    m_battleScene->setProperty((QString(role) + "Name").toUtf8(), info->name);
    Q_ASSERT_X(pokemonSprite, "Battle::setupPokemon", "pokemonSprite: '%1' is null".arg(role));

    // Set the basic sprite properties
    pokemonSprite->setProperty("spriteSheet", QString("qrc:/assets/HGSS/PokGen%1_transparent_reordered.png").arg(info->generation));
    pokemonSprite->setProperty("row", info->spriteId);
    pokemonSprite->setProperty("scaleFactor", Globals::SCALE);
    pokemonSprite->setProperty("debugLines", Globals::DEBUG);

    const SpriteInfo* spriteInfo = Globals::getSpriteInfo(info->spriteId, info->generation);

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
    QMetaObject::invokeMethod(m_battleScene, "positionSpriteAndStatusBar", Q_ARG(QVariant, QVariant::fromValue(pokemonSprite)));

    return pokemonSprite;
}

void Battle::initPosition() {
    QQuickItem* rootItem = qobject_cast<QQuickItem*>(m_battleScene);
    QQuickItem* opponentItem = m_battleScene->findChild<QQuickItem*>("opponent");
    // has to be QPointF for the method, but since we set offsets as integer in qml it gives smooth transition
    QPointF spriteOffset = opponentItem->mapToItem(rootItem, QPointF(0, 0));

    m_origin = m_initialOppPos + QPoint(-spriteOffset.x(), -spriteOffset.y());
    setPosition(m_origin);
}

void Battle::direction(int direction) { m_currentDirection = direction; }
void Battle::handleDrag(bool isDragged) { m_isDragged = isDragged; }
