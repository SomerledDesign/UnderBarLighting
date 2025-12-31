/**
 * @file bounce.h
 * @author Kevin Murphy (https://www.SomerledDesign.com)
 * @brief Bouncing Ball effect on an LED strip
 * @version 0.1
 * @date 09/02/24
 *
 *   Version History -
 *
 *   0.1 - 09/02/24 - A work in progress
 *
 *
 */
#include <sys/time.h>

#include <Arduino.h>
#define FASTLED_INTERNAL
#include <FastLED.h> // https://github.com/FastLED/FastLED

using namespace std;
#include <vector>

extern CRGB g_LEDs[];
extern uint8_t g_EffectSpeed;

#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0])) // Calculate the number of elements in an array

static const CRGB ballColors[] =
    {
        CRGB::Orange,
        CRGB::Yellow,
        CRGB::Cyan,
        CRGB::Indigo,
        CRGB::Red,
        CRGB::Blue,
        CRGB::Green,
    };
class BouncingBallEffect
{

  private:
    double InitialBallSpeed(double height) const
    {
        return sqrt(-2 * Gravity * height); // Because MATH!
    }

    size_t _cLength;
    size_t _cBalls;
    byte _fadeRate;
    bool _bMirrored;

    const double Gravity = -9.81; // Because PHYSICS!
    const double StartHeight = 1; // Drop balls from max height initially
    const double ImpactVelocity = InitialBallSpeed(StartHeight);
    const double SpeedKnob = 4.0; // Higher values will slow the effect

    vector<double> ClockTimeAtLastBounce, Height, BallSpeed, Dampening;
    vector<CRGB> Colors;

    static double Time()
    {
        timeval tv = {0};
        gettimeofday(&tv, nullptr);
        return (double)(tv.tv_usec / 1000000.0 + (double)tv.tv_sec);
    }

  public:
    // BouncingBallEffect
    //
    // Caller specs strip length, number of balls, persistance level (255 is least), and whether the
    // balls should be drawn mirrored from each side.

    BouncingBallEffect(size_t cLength, size_t ballCount = 3, byte fade = 0, bool bMirrored = false)
        : _cLength(cLength - 1),
          _cBalls(ballCount),
          _fadeRate(fade),
          _bMirrored(bMirrored),
          ClockTimeAtLastBounce(ballCount),
          Height(ballCount),
          BallSpeed(ballCount),
          Dampening(ballCount),
          Colors(ballCount)
    {
        Reset();
    }

    void Reset()
    {
        for (size_t i = 0; i < _cBalls; i++)
        {
            Height[i] = StartHeight;                      // Starting height
            ClockTimeAtLastBounce[i] = Time();            // When ball last hit ground state
            Dampening[i] = 0.90 - i / pow(_cBalls, 2);    // Bounciness of this ball
            BallSpeed[i] = InitialBallSpeed(StartHeight); // Don't dampen initial launch
            Colors[i] = ballColors[i % ARRAYSIZE(ballColors)];
        }
    }

    void Resize(size_t ballCount)
    {
        _cBalls = ballCount;
        ClockTimeAtLastBounce.assign(ballCount, 0.0);
        Height.assign(ballCount, 0.0);
        BallSpeed.assign(ballCount, 0.0);
        Dampening.assign(ballCount, 0.0);
        Colors.assign(ballCount, CRGB::Black);
        Reset();
    }

    // Draw
    //
    // Draw each of the balls.  When any ball settles with too little energy it is 'kicked' to the other direction

    virtual void Draw()
    {   
        if (_fadeRate != 0)
        {
            for (size_t i = 0; i < _cLength; i++)
               g_LEDs[i].fadeToBlackBy(_fadeRate);
        }
        else
            FastLED.clear();

        // Draw each of the balls


        for (size_t i = 0; i < _cBalls; i++)
        {
            double speedKnob = 8.0 - (g_EffectSpeed / 255.0) * 6.0;
            speedKnob = max(1.5, speedKnob);
            double TimeSinceLastBounce = (Time() - ClockTimeAtLastBounce[i]) / speedKnob;

            // Use standard constant acceleration function - see https://en.wikipedia.org/wiki/Acceleration
            Height[i] = 0.5 * Gravity * pow(TimeSinceLastBounce, 2.0) + BallSpeed[i] * TimeSinceLastBounce;

            // Ball hits ground - bounce!
            if (Height[i] < 0)
            {
                Height[i] = 0;
                BallSpeed[i] = Dampening[i] * BallSpeed[i];
                ClockTimeAtLastBounce[i] = Time();

                if (BallSpeed[i] < 0.01)
                    BallSpeed[i] = InitialBallSpeed(StartHeight) * Dampening[i];
            }

            size_t position = (size_t)(Height[i] * (_cLength - 1 ) / StartHeight);

            g_LEDs[position]     += Colors[i];
            g_LEDs[position + 1] += Colors[i];

            if (_bMirrored)
            {
                g_LEDs[_cLength - 1 - position] += Colors[i];
                g_LEDs[_cLength - position]     += Colors[i];
            }
        }
            delay(20);
    }
};
