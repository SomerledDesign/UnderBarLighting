/**
 * @file doublepalette.h
 * @author Kevin Murphy (https://www.SomerledDesign.com)
 * @brief Two palettes blended together
 * @version 0.1
 * @date 12/29/24
 */

#include <Arduino.h>
#define FASTLED_INTERNAL
#include <FastLED.h>

extern CRGB g_LEDs[];
extern uint8_t g_EffectSpeed;
extern uint8_t g_EffectCount;

static inline uint8_t MapU8Double(uint8_t x, uint8_t inMin, uint8_t inMax, uint8_t outMin, uint8_t outMax)
{
    return (uint8_t)(((uint32_t)(x - inMin) * (outMax - outMin)) / (inMax - inMin) + outMin);
}

void DrawDoublePalette()
{
    static uint8_t indexA = 0;
    static uint8_t indexB = 0;
    CRGBPalette256 paletteA = RainbowColors_p;
    CRGBPalette256 paletteB = PartyColors_p;
    uint8_t step = max<uint8_t>(1, g_EffectSpeed / 10);
    uint8_t scale = constrain(g_EffectCount, 1, 16);
    uint8_t blendAmount = MapU8Double(scale, 1, 16, 32, 224);

    indexA += step;
    indexB -= step;

    for (int i = 0; i < NUM_LEDS; i++)
    {
        uint8_t idx = i * scale;
        CRGB colorA = ColorFromPalette(paletteA, indexA + idx, 255, LINEARBLEND);
        CRGB colorB = ColorFromPalette(paletteB, indexB + idx, 255, LINEARBLEND);
        g_LEDs[i] = blend(colorA, colorB, blendAmount);
    }
}
