#ifndef DATA_ASSET_H
#define DATA_ASSET_H

enum class SpriteSheet{
    Standard, Big, Form
};

struct asset_info{
    int width;
    int height;
    SpriteSheet SpriteSheet;
    int rowId;
};

static asset_info getAssetInfo(int pokedexId, int formId);
extern const asset_info* const kStandardAssetInfo[512];
extern const asset_info* const kBigAssetInfo[16];
extern const asset_info* const kFormAssetInfo[64];

// Form IDs with their variant counts
int FORM_IDS_AND_NRS[][2] = {
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

// Large Pokemon IDs
int BIG_IDS[] = {
    208,  // Steelix
    249,  // Lugia
    250,  // Ho-Oh
    483,  // Dialga
    484,  // Palkia
    486,  // Regigigas
    487,  // Giratina
    493,  // Arceus
    321,  // Wailord
    382,  // Kyogre
    383,  // Groudon
    384,  // Rayquaza
};

// Generation limits (number of unique Pokemon per generation)
int GEN_LIMS[] = {151, 100, 135, 107};
#endif
