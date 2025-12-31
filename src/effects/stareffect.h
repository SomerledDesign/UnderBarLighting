/**
 * @file stareffect.h
 * @author Kevin Murphy (https://www.SomerledDesign.com)
 * @brief Starry twinkle sky effect
 * @version 0.1
 * @date 12/29/24
 */

#include <Arduino.h>
#define FASTLED_INTERNAL
#include <FastLED.h>

extern CRGB g_LEDs[];
extern uint8_t g_EffectSpeed;
extern uint8_t g_EffectCount;

static inline uint8_t MapU8Star(uint8_t x, uint8_t inMin, uint8_t inMax, uint8_t outMin, uint8_t outMax)
{
    return (uint8_t)(((uint32_t)(x - inMin) * (outMax - outMin)) / (inMax - inMin) + outMin);
}

void DrawStarEffect()
{
    uint8_t fadeAmount = MapU8Star(g_EffectSpeed, 1, 255, 30, 6);
    fadeToBlackBy(g_LEDs, NUM_LEDS, fadeAmount);

    uint8_t count = constrain(g_EffectCount, 1, 16);
    uint16_t starMax = max(2, NUM_LEDS / 12);
    uint16_t stars = MapU8Star(count, 1, 16, 1, (uint8_t)min<uint16_t>(255, starMax));
    for (uint16_t i = 0; i < stars; i++)
    {
        int idx = random(NUM_LEDS);
        uint8_t hue = random8();
        g_LEDs[idx] += CHSV(hue, 180, 255);
    }
}
