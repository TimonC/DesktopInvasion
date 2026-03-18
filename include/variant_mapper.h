#ifndef VARIANT_MAPPER_H
#define VARIANT_MAPPER_H


class VariantMapper {
private:
    static constexpr int MAX_POKEDEX_ID = 1000;

    static constexpr int kVariantList[][2] = {
        {201, 28},  // unown (27 extra variants)
        {386, 4},   // deoxys (3 extra)
        {412, 3},   // burmy (2 extra)
        {413, 3},   // wormadam (2 extra)
        {422, 2},   // shellos (1 extra)
        {423, 2},   // gastrodon (1 extra)
        {478, 6},   // rotom (5 extra)
        {487, 2},   // giratina (1 extra)
        {492, 2},   // shaymin (1 extra)
    };

    static constexpr int kVariantCount = 9; // Changed from 8 to 9
    static constexpr int TOTAL_EXTRA_VARIANTS = 43; // 27 + 3 + 2 + 2 + 1 + 1 + 5 + 1 + 1

public:
    VariantMapper() = delete;

    // Main conversion function
    static int pokedexID2IconID(int pokedexId, int variantId = 0);

    // Direct calculation without tables
    static int pokedexID2IconIDDirect(int pokedexId, int variantId = 0);

    static int getVariantCount(int pokedexId);
    static int getTotalExtraVariants() { return TOTAL_EXTRA_VARIANTS; }
};

#endif // VARIANT_MAPPER_H
