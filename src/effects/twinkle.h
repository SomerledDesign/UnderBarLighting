/**
 * @file twinkle.h
 * @author Kevin Murphy (https://www.SomerledDesign.com)
 * @brief random LED stars on a LED strip for a twinkle effect
 * @version 0.1
 * @date 09/02/24
 *
 *   Version History -
 *
 *   0.1 - 09/02/24 - A work in progress
 *
 *
 */

#include <Arduino.h>
#define FASTLED_INTERNAL
#include <FastLED.h> // https://github.com/FastLED/FastLED

extern int g_Brightness;
extern uint8_t g_EffectSpeed;
extern uint8_t g_EffectCount;

#define NUM_COLORS 5
static const CRGB TwinkleColors[NUM_COLORS] =
    {
        CRGB::Red,
        CRGB::Blue,
        CRGB::Purple,
        CRGB::Green,
        CRGB::Orange};

void DrawTwinkleOld()
{
    FastLED.clear(false); // Clear the strip, but don't push out the bits quite yet

    for (int i = 0; i < NUM_LEDS / 4; i++)
    {
        g_LEDs[random(NUM_LEDS)] = TwinkleColors[random(NUM_COLORS)];
        delay(200);
    }
}

void DrawTwinkle()
{
    static int passCount = 0;
    passCount += max<uint8_t>(1, g_EffectSpeed / 64);
    // Every time passCount reaches a quarter of the LED total, we reset the strip
    if (passCount == NUM_LEDS )
    {
        passCount = 0;
        FastLED.clear(false); // Clear the strip, but don't push out the bits quite yet
    }

    uint16_t count = constrain(g_EffectCount, 1, NUM_LEDS / 2);
    for (uint16_t i = 0; i < count; i++)
    {
        g_LEDs[random(NUM_LEDS)] = TwinkleColors[random(NUM_COLORS)];
    }
    // delay(200);
}
