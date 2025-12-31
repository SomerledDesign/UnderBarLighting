/**
 * @file palette.h
 * @author Kevin Murphy (https://www.SomerledDesign.com)
 * @brief Palette-based color wash
 * @version 0.1
 * @date 12/29/24
 */

#include <Arduino.h>
#define FASTLED_INTERNAL
#include <FastLED.h>

extern CRGB g_LEDs[];
extern uint8_t g_EffectSpeed;
extern uint8_t g_EffectCount;

void DrawPalette()
{
    static uint8_t startIndex = 0;
    CRGBPalette256 palette = RainbowColors_p;
    uint8_t step = max<uint8_t>(1, g_EffectSpeed / 12);
    uint8_t scale = constrain(g_EffectCount, 1, 16);

    startIndex += step;

    for (int i = 0; i < NUM_LEDS; i++)
    {
        uint8_t colorIndex = startIndex + (i * scale);
        g_LEDs[i] = ColorFromPalette(palette, colorIndex, 255, LINEARBLEND);
    }
}
