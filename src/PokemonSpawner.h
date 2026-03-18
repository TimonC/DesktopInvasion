#ifndef POKEMONSPAWNER_H
#define POKEMONSPAWNER_H
#include <QQuickItem>
#include <QObject>
#include <QQmlEngine>
#include <QTimer>
#include <QGuiApplication>
class PokemonSpawner : public QObject {
    Q_OBJECT
    Q_PROPERTY(int maxSprites READ maxSprites WRITE setMaxSprites NOTIFY maxSpritesChanged)

public:
    // Constructor with just the essentials
    explicit PokemonSpawner(const QString& spriteSheet = "qrc:/assets/HGSS/PokGen1_transparent_reordered.png",
                          int minRow = 0,
                          int maxRow = 4,
                          int maxSprites = 50,
                          QObject *parent = nullptr);

    ~PokemonSpawner();

    Q_INVOKABLE void startSpawning();
    Q_INVOKABLE void stopSpawning();

    // Getters and setters
    int maxSprites() const { return m_maxSprites; }
    void setMaxSprites(int max);

signals:
    void maxSpritesChanged();

private slots:
    void spawnFromEdge();

private:
    QQuickItem* findSpriteContainer();
    void spawnPokemonAtEdge(int row);
    int currentSpriteCount();
    int getRandomRow();

    QTimer* m_spawnTimer;
    QString m_spriteSheet;
    int m_minRow;
    int m_maxRow;
    int m_maxSprites;
};
#endif
