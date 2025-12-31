/**
 * @file comet.h
 * @author Kevin Murphy (https://www.SomerledDesign.com)
 * @brief draws a moving comet with a fading tail
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

extern CRGB g_LEDs[];
extern uint8_t g_EffectSpeed;
extern uint8_t g_EffectCount;

void DrawComet()
{
    const byte fadeAmt = 64;       // Fraction of 256 to fade a pixel by if it is chosen to be faded
    const int cometSize = constrain(g_EffectCount, 2, 20);        // Size of the comet in pixels
    const int deltaHue = 4;         // How far to step the cycling hue each draw call
    const double cometSpeed = 0.2 + (g_EffectSpeed / 255.0) * 1.6;  // How far to advance the comet every frame

    static byte hue = HUE_RED;      // Current color
    static int iDirection = 1;      // current direction (-1 or +1)
    static double iPos = 0.0;       // current comet position on strip

    hue += deltaHue;                // Update comet color
    iPos += iDirection * cometSpeed;// Updtate comet position

// Flip the comet direction when it hits either side
if (iPos == (NUM_LEDS - cometSize) || iPos == 0)
    iDirection *= -1;
    
// Draw a comet at its current position
for (int i = 0; i < cometSize; i++)
    g_LEDs[(int)iPos + i].setHue(hue);

// Fade the LEDs one step
for (int j = 0; j < NUM_LEDS; j++)
    if (random(2) == 1)
        g_LEDs[j] = g_LEDs[j].fadeToBlackBy(fadeAmt);

    delay(10);
}   
