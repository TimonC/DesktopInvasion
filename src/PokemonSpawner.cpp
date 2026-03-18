#include "PokemonSpawner.h"
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QRandomGenerator>

PokemonSpawner::PokemonSpawner(const QString& spriteSheet, int minRow, int maxRow, int maxSprites, QObject *parent)
    : QObject(parent)
    , m_spriteSheet(spriteSheet)
    , m_minRow(minRow)
    , m_maxRow(maxRow)
    , m_maxSprites(maxSprites)
{
    m_spawnTimer = new QTimer(this);
    m_spawnTimer->setSingleShot(false);
    connect(m_spawnTimer, &QTimer::timeout, this, &PokemonSpawner::spawnFromEdge);

    qDebug() << "PokemonSpawner created: sheet=" << m_spriteSheet
             << "rows=" << m_minRow << "-" << m_maxRow
             << "max=" << m_maxSprites;
}

PokemonSpawner::~PokemonSpawner() {
    stopSpawning();
}

void PokemonSpawner::setMaxSprites(int max) {
    if (m_maxSprites != max) {
        m_maxSprites = max;
        qDebug() << "Max sprites set to:" << m_maxSprites;
        emit maxSpritesChanged();
    }
}

int PokemonSpawner::getRandomRow() {
    return QRandomGenerator::global()->bounded(m_minRow, m_maxRow + 1);
}

void PokemonSpawner::startSpawning() {
    m_spawnTimer->start(2000 + QRandomGenerator::global()->bounded(3000));
    qDebug() << "Spawning started! Rows:" << m_minRow << "-" << m_maxRow << "Max:" << m_maxSprites;
}

void PokemonSpawner::stopSpawning() {
    m_spawnTimer->stop();
    qDebug() << "Spawning stopped";
}

void PokemonSpawner::spawnFromEdge() {
    if (currentSpriteCount() >= m_maxSprites) {
        return;
    }

    if (!findSpriteContainer()) return;

    int row = getRandomRow();
    spawnPokemonAtEdge(row);

    m_spawnTimer->setInterval(1500 + QRandomGenerator::global()->bounded(2500));
}

void PokemonSpawner::spawnPokemonAtEdge(int row) {
    QQuickItem* container = findSpriteContainer();
    if (!container) return;

    QQmlEngine* engine = qmlEngine(container);
    if (!engine) return;

    int edge = QRandomGenerator::global()->bounded(4);
    int x = 0, y = 0;
    QString initialDirection = "down";

    switch(edge) {
        case 0: x = QRandomGenerator::global()->bounded(container->width() - 32); y = -32; initialDirection = "down"; break;
        case 1: x = container->width(); y = QRandomGenerator::global()->bounded(container->height() - 32); initialDirection = "left"; break;
        case 2: x = QRandomGenerator::global()->bounded(container->width() - 32); y = container->height(); initialDirection = "up"; break;
        case 3: x = -32; y = QRandomGenerator::global()->bounded(container->height() - 32); initialDirection = "right"; break;
    }

    QQmlComponent component(engine, QUrl("qrc:/sprites/PokemonSprite.qml"));
    if (component.isReady()) {
        QObject* obj = component.createWithInitialProperties({
            {"sheetRow", row},
            {"spriteSheet", m_spriteSheet},
            {"x", x},
            {"y", y},
            {"parent", QVariant::fromValue(container)},
            {"currentDirection", initialDirection},
            {"isMoving", true}
        });

        if (obj) {
            qDebug() << "Spawned row" << row << "(" << currentSpriteCount() << "/" << m_maxSprites << ")";
        }
    }
}

int PokemonSpawner::currentSpriteCount() {
    QQuickItem* container = findSpriteContainer();
    if (!container) return 0;

    int count = 0;
    auto children = container->childItems();
    for (auto child : children) {
        if (child->inherits("AnimatedSprite")) {
            count++;
        }
    }
    return count;
}

QQuickItem* PokemonSpawner::findSpriteContainer() {
    QQuickWindow* window = nullptr;
    for (auto win : QGuiApplication::allWindows()) {
        if (qobject_cast<QQuickWindow*>(win)) {
            window = qobject_cast<QQuickWindow*>(win);
            break;
        }
    }
    return window ? window->contentItem()->findChild<QQuickItem*>("spriteContainer") : nullptr;
}
