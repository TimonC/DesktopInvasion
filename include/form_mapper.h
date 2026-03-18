#ifndef FORM_MAPPER_H
#define FORM_MAPPER_H

class FormMapper {
private:
    // [pokedex_id, total_forms]
    static constexpr int FORMS[][2] = {
        {201, 28},  // Unown (A-Z + ! + ?)
        {386, 4},   // Deoxys (Normal/Attack/Defense/Speed)
        {412, 3},   // Burmy (Plant/Sandy/Trash cloak)
        {413, 3},   // Wormadam (Plant/Sandy/Trash cloak)
        {422, 2},   // Shellos (East/West sea)
        {423, 2},   // Gastrodon (East/West sea)
        {478, 6},   // Rotom (Normal + 5 appliances)
        {487, 2},   // Giratina (Altered/Origin form)
        {492, 2},   // Shaymin (Land/Sky form)
    };

    static constexpr int COUNT = 9;

    static int calculateShift(int pokedexId) {
        int shift = 0;
        for (int i = 0; i < COUNT; i++) {
            if (FORMS[i][0] < pokedexId) {
                shift += FORMS[i][1] - 1;
            } else {
                break;
            }
        }
        return shift;
    }

public:
    static int toIconId(int pokedexId, int form = 0) {
        if (pokedexId < 1 || pokedexId > 1000 || form < 0) return -1;

        int shift = calculateShift(pokedexId);
        int baseIcon = (pokedexId - 1) + shift;

        for (int i = 0; i < COUNT; i++) {
            if (FORMS[i][0] == pokedexId) {
                if (form < FORMS[i][1]) {
                    return baseIcon + form;
                }
                break;
            }
        }

        return baseIcon;
    }

    static bool hasForms(int pokedexId) {
        for (int i = 0; i < COUNT; i++) {
            if (FORMS[i][0] == pokedexId) return true;
        }
        return false;
    }

    static int getFormCount(int pokedexId) {
        for (int i = 0; i < COUNT; i++) {
            if (FORMS[i][0] == pokedexId) return FORMS[i][1];
        }
        return 1;
    }
};

#endif
