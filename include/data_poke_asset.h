#ifndef DATA_POKE_ASSET_H
#define DATA_POKE_ASSET_H

enum class SpriteSheet{
    Standard, Big
};

struct AssetInfo{
    int width;
    int height;
    int minWidth;
    int minHeight;
    SpriteSheet spriteSheet;
    int rowId;
};

extern const AssetInfo* const kAssetInfo[493];

#endif

