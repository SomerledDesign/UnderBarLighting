/**
 * @file meteor.h
 * @author Kevin Murphy (https://www.SomerledDesign.com)
 * @brief Color meteor effect with fading trail
 * @version 0.1
 * @date 12/29/24
 */

#include <Arduino.h>
#define FASTLED_INTERNAL
#include <FastLED.h>

extern CRGB g_LEDs[];
extern uint8_t g_EffectSpeed;
extern uint8_t g_EffectCount;

void DrawMeteor()
{
    const uint8_t meteorSize = 6;
    const uint8_t trailDecay = 64;
    const bool randomDecay = true;
    static const uint8_t kMaxMeteors = 8;

    static bool initialized = false;
    static float pos[kMaxMeteors];
    static float speed[kMaxMeteors];
    static float hue[kMaxMeteors];
    static bool left[kMaxMeteors];

    if (!initialized)
    {
        for (int i = 0; i < kMaxMeteors; i++)
        {
            pos[i] = (NUM_LEDS / kMaxMeteors) * i;
            speed[i] = 0.6f + (random(0, 40) / 100.0f);
            hue[i] = (i * 48) % 255;
            left[i] = (i & 1) != 0;
        }
        initialized = true;
    }

    // Fade all LEDs down slightly
    for (int j = 0; j < NUM_LEDS; j++)
    {
        if (!randomDecay || (random8() > 64))
        {
            g_LEDs[j].fadeToBlackBy(trailDecay);
        }
    }

    // Move and draw each meteor
    uint8_t meteorCount = constrain(g_EffectCount, 1, kMaxMeteors);
    float speedScale = 0.4f + (g_EffectSpeed / 255.0f) * 1.6f;
    for (int i = 0; i < meteorCount; i++)
    {
        pos[i] = left[i] ? pos[i] - (speed[i] * speedScale) : pos[i] + (speed[i] * speedScale);

        if (pos[i] < meteorSize)
        {
            left[i] = false;
            pos[i] = meteorSize;
        }
        if (pos[i] >= NUM_LEDS)
        {
            left[i] = true;
            pos[i] = NUM_LEDS - 1;
        }

        hue[i] += 0.6f;
        if (hue[i] > 255.0f)
        {
            hue[i] -= 255.0f;
        }

        CHSV hsv((uint8_t)hue[i], 240, 255);
        CRGB rgb;
        hsv2rgb_rainbow(hsv, rgb);

        for (int j = 0; j < meteorSize; j++)
        {
            int idx = (int)(pos[i] - j);
            if (idx >= 0 && idx < NUM_LEDS)
            {
                nblend(g_LEDs[idx], rgb, 128);
            }
        }
    }
}
