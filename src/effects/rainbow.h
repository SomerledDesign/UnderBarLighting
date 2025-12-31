/**
 * @file rainbow.h
 * @author Kevin Murphy (https://www.SomerledDesign.com)
 * @brief draws a rainbow on a LED strip
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

uint8_t initalHue = 0;
const uint8_t deltaHue = 4;
const uint8_t hueDensity = 8;

extern uint8_t g_EffectSpeed;

void DrawRainbow()
{
    fill_rainbow(g_LEDs, NUM_LEDS, initalHue, deltaHue);

    uint8_t step = max<uint8_t>(1, g_EffectSpeed / 8);
    initalHue += step;
}
