#ifndef ZPG_PERLIN_DRAW_H
#define ZPG_PERLIN_DRAW_H

#include "zpg_internal.h"
#include "zpg_perlin.h"

struct zpg_perlin_cfg
{
    f32 scaleFactor; // eg 5 - 50
    f32 noiseScalar; // eg 5 - 50
    f32 freq;
    i32 depth;
    // icky way to force noise to discreet values
    i32 bApplyThreshold;
};

// Convert a byte (0...255) to float (0...1)
static f32 ZPG_ByteToFloat(u8 byte)
{
    return (f32)byte / (f32)255;
}

// Convert a float (0...1) to byte (0...255)
static u8 ZPG_FloatToByte(f32 f)
{
    return (u8)(255 * f);
}

static void ZPG_Perlin_SetCfg(zpg_perlin_cfg* cfg,
    f32 scale, f32 noise, f32 freq, i32 depth)
{
    cfg->scaleFactor = scale;
    cfg->noiseScalar = noise;
    cfg->freq = freq;
    cfg->depth = depth;
}

static void ZPG_Perlin_SetCfgPreset(zpg_perlin_cfg* cfg,
    i32 preset)
{
    switch (preset)
    {
        case 1: ZPG_Perlin_SetCfg(cfg, 15, 25, 2, 20); break;
        case 6: ZPG_Perlin_SetCfg(cfg, 10, 25, 2, 2); break;
        // diagonal streaks
        case 7: ZPG_Perlin_SetCfg(cfg, 50, 25, 2, 2); break;
        case 8: ZPG_Perlin_SetCfg(cfg, 50, 5, 10, 2); break;
        case 9: ZPG_Perlin_SetCfg(cfg, 50, 5, 2, 2); break;
        case 10: ZPG_Perlin_SetCfg(cfg, 50, 15, 10, 2); break;
        default: ZPG_Perlin_SetCfg(cfg, 5, 50, 2, 2); break;
    }
}

/**
 * types should be an array of three u8s.
 * 1 for each threshold
 */
static void ZPG_ApplyPerlinThreshold(
    ZPGGrid* grid, ZPGGrid* stencil, u8* types, i32 numTypes)
{
    f32 step = 1.f / numTypes;
    for (i32 pixelY = 0; pixelY < grid->height; ++pixelY)
    {
        for (i32 pixelX = 0; pixelX < grid->width; ++pixelX)
        {
            if (ZPG_Grid_CheckStencilOccupied(stencil, pixelX, pixelY))
            { continue; }
            ZPGCell* cell = ZPG_Grid_GetCellAt(grid, pixelX, pixelY);
            f32 value = ZPG_ByteToFloat(cell->tile.type);
            #if 0
            if (value > 0.66f) { cell->tile.type = types[0]; }
            else if (value > 0.33f) { cell->tile.type = types[1]; }
            else { cell->tile.type = types[2]; }
            #endif
            #if 1
            u8 newType = types[(i32)(numTypes * value)];
            cell->tile.type = newType;
            #endif
        }
    }
}

static void ZPG_DrawPerlinGrid(
    ZPGGrid* grid, ZPGGrid* stencil, i32* seed, zpg_perlin_cfg* cfg)
{
    f32 seedX = 0, seedY = 0;
    #if 1
    seedX = ZPG_Randf32(*seed) * 99999.f;
    *seed += 1;
    seedY = ZPG_Randf32(*seed) * 99999.f;
    *seed += 1;
    #endif
    for (i32 pixelY = 0; pixelY < grid->height; ++pixelY)
    {
        for (i32 pixelX = 0; pixelX < grid->width; ++pixelX)
        {
            if (ZPG_Grid_CheckStencilOccupied(stencil, pixelX, pixelY)) { continue; }
            f32 x = (f32)pixelX / (f32)grid->width; \
            f32 y = (f32)pixelY / (f32)grid->height; \
            //u32 pixelIndex = pixelX + (pixelY * grid->width); \
            //ColourU32* pixel = (ColourU32*)&tex->ptrMemory[pixelIndex];
            f32 result;
            #if 0 // vertical streaks

            f32 sampleY = (y * cfg->scaleFactor) + seedY;
            f32 sampleX = (x * cfg->scaleFactor) + seedX;
            f32 noise = ZPG_Perlin_Get2d(sampleX, sampleY, cfg->freq, depth);
            //f32 noise = Perlin_Get2d((f32)(x * cfg->scaleFactor), (f32)(y * cfg->scaleFactor), 2, 2);
            f32 a = sinf((x + noise / 2) * cfg->noiseScalar);
            //f32 a = cosf((x + noise / 2) * cfg->noiseScalar);
            result = (1 + a)  / 2;
            #endif
            #if 1 // double streaks - top left to bottom right

            f32 scaleFactorX = cfg->scaleFactor;
            f32 scaleFactorY = cfg->scaleFactor;
            f32 noiseScalerX = cfg->noiseScalar;
            f32 noiseScalerY = cfg->noiseScalar;
            f32 noise = ZPG_Perlin_Get2d(
                (f32)(x * scaleFactorX),
                (f32)(y * scaleFactorY),
                cfg->freq,
                cfg->depth);
            noise = -noise;
            f32 a = sinf((x + noise / 2) * noiseScalerX);
            a = -a;
            a = (1 + a)  / 2;
            f32 b = sinf((y + noise / 2) * noiseScalerY);
            b = (1 + b) / 2;
            result = (a * 0.5f) + (b * 0.5f);
            #endif
            #if 0 // double sine

            f32 a = (1 + sinf(x * 50 )) / 2;
            f32 b = (1 + sinf(y * 50 )) / 2;
            result = (a * 0.5f) + (b * 0.5f);
            #endif
            #if 0 // scatter
            // Scale factor of 1 means little variation. 10 for a lot
            //cfg->scaleFactor = 4;//10;//5;
            f32 noise = ZPG_Perlin_Get2d(
                (f32)(x * cfg->scaleFactor), (f32)(y * cfg->scaleFactor), cfg->freq, cfg->depth);
            //result = noise > 0.5f ? 1.0f : 0.0f; // make result binary
            result = noise;
            #endif
            // type is scaled to 0-255
            ZPG_Grid_GetCellAt(grid, pixelX, pixelY)->tile.type = (u8)(255.f * result);
        }
    }
    if (cfg->bApplyThreshold == YES)
    {
        #if 0
        //u8 types[] = { ZPG_CELL_TYPE_WALL, ZPG_CELL_TYPE_FLOOR, ZPG_CELL_TYPE_WATER };
        //u8 types[] = { ZPG_CELL_TYPE_WATER, ZPG_CELL_TYPE_FLOOR, ZPG_CELL_TYPE_WALL };
        u8 types[] = { ZPG_CELL_TYPE_WALL, ZPG_CELL_TYPE_WATER, ZPG_CELL_TYPE_FLOOR };
        ZPG_ApplyPerlinThreshold(grid, stencil, types, 3);
        #endif
        #if 1
        u8 types[] = { ZPG_CELL_TYPE_WALL, ZPG_CELL_TYPE_PATH };
        ZPG_ApplyPerlinThreshold(grid, stencil, types, 2);
        #endif
    }
    
}

#endif // ZPG_PERLIN_DRAW_H