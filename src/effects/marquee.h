/**
 * @file marquee.h
 * @author Kevin Murphy (https://www.SomerledDesign.com)
 * @brief Draws an old Movie Theater  style marquee, but in color
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
#include <FastLED.h>

void DrawMarquee()
{
    static byte j = HUE_BLUE;
    j += 4;
    byte k = j;

    // The following is roughly equivalent to fill_rainbow(g_LEDs, NUM_LEDS, j, 8);

    CRGB c;
    for (int i = 0; i < NUM_LEDS; i++)
        g_LEDs[i] = c.setHue(k += 8);

    static float scroll = 0.0f;
    scroll += 0.1f;
    if (scroll > 5.0f)
        scroll -= 5.0f;

    for (float i = scroll; i < NUM_LEDS / 2 - 1; i += 5)
    {
        DrawPixels(i, 3, CRGB::Green);
    }

    delay(50);
}
