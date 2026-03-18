#ifndef DATA_POKE_ASSET_H
#define DATA_POKE_ASSET_H

enum class SpriteSheet{
    Standard, Big
};

struct asset_info{
    int width;
    int height;
    SpriteSheet spriteSheet;
    int rowId;
};

extern const asset_info* const kAssetInfo[493];

#endif

