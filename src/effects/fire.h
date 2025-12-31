/**
 * @file fire.h
 * @author Kevin Murphy (https://www.SomerledDesign.com)
 * @brief Fire-like effect adapted for a simple LED strip
 * @version 0.1
 * @date 12/29/24
 */

#include <Arduino.h>
#define FASTLED_INTERNAL
#include <FastLED.h>

extern CRGB g_LEDs[];
extern uint8_t g_EffectSpeed;
extern uint8_t g_EffectCount;

static inline uint8_t MapU8Fire(uint8_t x, uint8_t inMin, uint8_t inMax, uint8_t outMin, uint8_t outMax)
{
    return (uint8_t)(((uint32_t)(x - inMin) * (outMax - outMin)) / (inMax - inMin) + outMin);
}

void DrawFire()
{
    static byte heat[NUM_LEDS];
    const uint8_t cooling = MapU8Fire(g_EffectSpeed, 1, 255, 80, 20);
    const uint8_t sparking = MapU8Fire(g_EffectSpeed, 1, 255, 60, 180);
    const uint8_t sparks = constrain(g_EffectCount, 1, 8);

    // Cool down every cell a little
    for (int i = 0; i < NUM_LEDS; i++)
    {
        heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / NUM_LEDS) + 2));
    }

    // Heat drifts up and diffuses
    for (int k = NUM_LEDS - 1; k >= 2; k--)
    {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Randomly ignite new sparks near the bottom
    for (uint8_t i = 0; i < sparks; i++)
    {
        if (random8() < sparking)
        {
            int y = random8(7);
            heat[y] = qadd8(heat[y], random8(160, 255));
        }
    }

    // Map heat to LED colors
    for (int j = 0; j < NUM_LEDS; j++)
    {
        g_LEDs[j] = HeatColor(heat[j]);
    }
}
