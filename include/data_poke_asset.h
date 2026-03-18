#ifndef DATA_POKE_ASSET_H
#define DATA_POKE_ASSET_H

enum class SpriteSheet{
    Standard, Big
};

struct asset_info{
    int width;
    int height;
    SpriteSheet SpriteSheet;
    int rowId;
};

extern const asset_info* const kAssetInfo[493];


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

