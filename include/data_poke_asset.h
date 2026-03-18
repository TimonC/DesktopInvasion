#ifndef DATA_POKE_ASSET_H
#define DATA_POKE_ASSET_H

enum class SpriteSheet{
    Standard, Big
};

struct AssetInfo{
    int width;
    int height;
    int hWidth;
    int hHeight;
    int vWidth;
    int vHeight;
    SpriteSheet spriteSheet;
    int rowId;
};

extern const AssetInfo* const kAssetInfo[493];

#endif

