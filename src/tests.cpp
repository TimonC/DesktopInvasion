#include <lookup.h>
#include <data_poke.h>
#include <tests.h>

void testWeightedSamplePokemon() {
    std::mt19937 rng(std::random_device{}());

    const std::vector<std::pair<int, QString>> testPokemon = {
        // early route commons
        {10,  "Caterpie"},
        {13,  "Weedle"},
        {16,  "Pidgey"},
        {263, "Zigzagoon"},
        {401, "Kricketot"},
        // mid commons with evolution chains
        {4,   "Charmander"},
        {152, "Chikorita"},
        {255, "Torchic"},
        {387, "Turtwig"},
        {129, "Magikarp"},
        {443, "Gible"},
        // mid-chain
        {5,   "Charmeleon"},
        {148, "Dragonair"},
        {444, "Gabite"},
        // fully evolved
        {6,   "Charizard"},
        {130, "Gyarados"},
        {149, "Dragonite"},
        {445, "Garchomp"},
        {373, "Salamence"},
        // no-evolution standalones
        {132, "Ditto"},
        {131, "Lapras"},
        {143, "Snorlax"},
        {474, "Porygon-Z"},
        // pseudo-legendary pre-evos
        {147, "Dratini"},
        {371, "Bagon"},
        {436, "Bronzor"},
        // legendaries
        {144, "Articuno"},
        {150, "Mewtwo"},
        {245, "Suicune"},
        {382, "Kyogre"},
        {483, "Dialga"},
        {151, "Mew"},
        {385, "Jirachi"},
    };

    const std::vector<int> testLevels = {1, 10, 30, 40, 50, 75, 100};
    const std::vector<int> noTms = {};
    const int N = 100000;

    QString header = QString("%1").arg("Pokemon", -14)
                   + QString("%1").arg("catch", 6)
                   + QString("%1").arg("parent", 8)
                   + QString("%1").arg("evo@", 6);
    for (int level : testLevels)
        header += QString("  %1").arg("Lv" + QString::number(level), 7);
    qDebug().noquote() << header;
    qDebug().noquote() << QString("-").repeated(34 + 9 * testLevels.size());

    for (const auto& [id, name] : testPokemon) {
        const Poke* poke = Lookup::getPoke(id);
        int parentLvl = kEvolutionParentLevel[id];
        int evoLvl = poke->eligible_evolve_count > 0 ? poke->eligible_evolves[0].level : -1;

        QString info = QString("%1").arg(name, -14)
                     + QString("%1").arg(poke->catch_rate, 6)
                     + QString("%1").arg(parentLvl == -1 ? "-" : QString::number(parentLvl), 8)
                     + QString("%1").arg(evoLvl == -1 ? "-" : QString::number(evoLvl), 6);

        QMap<int, int> counts;
        for (int level : testLevels) counts[level] = 0;

        for (int level : testLevels) {
            for (int i = 0; i < N; ++i) {
                PokeRoll roll = Lookup::weightedSamplePokemon(level, noTms, rng);
                if (roll.poke_id == id) counts[level]++;
            }
        }

        for (int level : testLevels)
            info += QString("  %1").arg(QString::number(counts[level] * 100.0 / N, 'f', 2), 7);
        qDebug().noquote() << info;
    }
}
