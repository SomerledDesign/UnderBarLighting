/**
 * @file main.cpp
 * @author Kevin Murphy (https://www.SomerledDesign.com)
 * @brief skeleton for UnderBarLighting
 * @version 0.1
 * @date 09/02/24
 *
 * @copyright Copyright (c) 2024 Somerled Design, LLC in Kevin Murphy
 *
 *                      GNU GENERAL PUBLIC LICENSE
 *   This program is free software : you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *   This code is currently maintained for and compiled with Arduino 1.8.x.
 *   Your mileage may vary with other versions.
 *
 *   ATTENTION: LIBRARY FILES MUST BE PUT IN LIBRARIES DIRECTORIES AND NOT THE INO SKETCH DIRECTORY !!!!
 *
 *   FOR EXAMPLE:
 *   tiny4kOLED.h, tiny4kOLED.cpp ------------------------>   \Arduino\Sketchbook\libraries\tiny4kOLED\
 *
 *   Version History -
 *
 *   0.1 - 09/02/24 - A work in progress
 *
 *
 * DISCLAIMER:
 *   With this design, including both the hardware & software I offer no guarantee that it is bug
 *   free or reliable. So, if you decide to build one and you have problems or issues and/or causes
 *   damage/harm to you, others or property then you are on your own. This work is experimental.
 *
 */

#include <Arduino.h>
#include <stdio.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#define FASTLED_INTERNAL
#include <FastLED.h>
#include <Lib8tion.h>
#include <color.h>
#include "secrets.h"

// OLED definitions
// #define OLED_SCL 22      // Not required as it is the default
// #define OLED_SDA 21      // for I2C on the ESP32
#define OLED_RULER_TEST 0

#define NUM_LEDS 442 // FastLED definitions
#define LED_PIN 5   // Data pin for FastLED

CRGB g_LEDs[NUM_LEDS] = {0}; // Frame buffer for FastLED

void DrawPixels(float fPos, float count, CRGB color);

#include "effects/marquee.h"
#include "effects/rainbow.h"
#include "effects/twinkle.h"
#include "effects/comet.h"
#include "effects/bounce.h"
#include "effects/fire.h"
#include "effects/meteor.h"
#include "effects/palette.h"
#include "effects/doublepalette.h"
#include "effects/stareffect.h"

// U8G2_SSD1305_128X32_NONAME_F_HW_I2C g_OLED(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
// U8G2_SSD1306_128X32_WINSTAR_1_HW_I2C g_OLED(U8G2_R0);
// U8G2_SH1106_128X32_VISIONOX_F_HW_I2C g_OLED(U8G2_R0);
U8G2_SH1106_128X32_VISIONOX_F_HW_I2C g_OLED(U8G2_R0);
int g_lineHeight = 0;
int g_oledTopOffset = 0;
const int kOledTextXOffset = 6; // Nudge text away from left-edge artifacts on some panels.
const int kOledHeight = 32;
int g_Brightness = 12;              // 0 - 255 brightness scale
int g_MaxPowerInMilliwatts = 19750; // max power in milliwatts for a 4 amp power supply
uint8_t g_EffectSpeed = 96;         // 1 - 255
uint8_t g_EffectCount = 4;          // 1 - 16 (effect-specific)

enum EffectId : uint8_t
{
  EFFECT_MARQUEE = 0,
  EFFECT_SOLID = 1,
  EFFECT_RAINBOW = 2,
  EFFECT_TWINKLE = 3,
  EFFECT_COMET = 4,
  EFFECT_BOUNCE = 5,
  EFFECT_FIRE = 6,
  EFFECT_METEOR = 7,
  EFFECT_PALETTE = 8,
  EFFECT_DOUBLEPALETTE = 9,
  EFFECT_STAREFFECT = 10,
  EFFECT_COUNT
};

struct LightingState
{
  bool power;
  uint8_t brightness;
  EffectId effect;
  CRGB color;
  uint8_t speed;
  uint8_t count;
};

struct HAConfig
{
  const char *device_name;
  const char *unique_id;
  const char *base_topic;
  const char *command_topic;
  const char *state_topic;
  const char *availability_topic;
};

static LightingState g_State = {true, 12, EFFECT_MARQUEE, CRGB::White, 96, 4};
static const HAConfig kHAConfig = {
    "underbar_lighting",
    "underbar_lighting_01",
    "underbar/lighting",
    "underbar/lighting/set",
    "underbar/lighting/state",
    "underbar/lighting/availability"};

static char g_commandBuffer[96] = {0};
static size_t g_commandLength = 0;

static const char *g_otaStatus = "OFF";
static uint8_t g_i2cAddress = 0;
static bool g_wifiConnected = false;
static WebServer g_httpServer(80);
static BouncingBallEffect g_bounceEffect(NUM_LEDS, 3, 20, false);
static uint8_t g_lastBounceCount = 0;
static uint8_t g_effectSpeedPreset[EFFECT_COUNT] = {96, 96, 96, 96, 96, 96, 120, 140, 110, 110, 80};
static uint8_t g_effectCountPreset[EFFECT_COUNT] = {4, 4, 4, 4, 5, 3, 3, 4, 6, 6, 4};
static BLEServer *g_bleServer = nullptr;
static BLECharacteristic *g_bleTx = nullptr;
static bool g_bleConnected = false;

void ApplyCommand(const char *command);

EffectId ClampEffect(int effect)
{
  if (effect < 0 || effect >= EFFECT_COUNT)
  {
    return EFFECT_MARQUEE;
  }
  return static_cast<EffectId>(effect);
}

void ApplyEffectPreset(EffectId effect)
{
  g_State.speed = g_effectSpeedPreset[effect];
  g_State.count = g_effectCountPreset[effect];
}

void SaveEffectPreset(EffectId effect)
{
  g_effectSpeedPreset[effect] = g_State.speed;
  g_effectCountPreset[effect] = g_State.count;
}

void SendBleLine(const char *line)
{
  if (!g_bleConnected || g_bleTx == nullptr || line == nullptr)
  {
    return;
  }
  g_bleTx->setValue(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(line)), strlen(line));
  g_bleTx->notify();
}

class BleServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *server) override
  {
    g_bleConnected = true;
  }

  void onDisconnect(BLEServer *server) override
  {
    g_bleConnected = false;
    if (server)
    {
      server->getAdvertising()->start();
    }
  }
};

class BleRxCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *characteristic) override
  {
    std::string value = characteristic->getValue();
    if (value.empty())
    {
      return;
    }
    if (value.back() == '\n' || value.back() == '\r')
    {
      value.pop_back();
    }
    ApplyCommand(value.c_str());
  }
};

void SetupBleSerial()
{
  BLEDevice::init("UnderbarLighting");
  g_bleServer = BLEDevice::createServer();
  g_bleServer->setCallbacks(new BleServerCallbacks());

  BLEService *service = g_bleServer->createService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
  BLECharacteristic *rx = service->createCharacteristic(
      "6E400002-B5A3-F393-E0A9-E50E24DCCA9E",
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  g_bleTx = service->createCharacteristic(
      "6E400003-B5A3-F393-E0A9-E50E24DCCA9E",
      BLECharacteristic::PROPERTY_NOTIFY);

  g_bleTx->addDescriptor(new BLE2902());
  rx->setCallbacks(new BleRxCallbacks());

  service->start();
  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
  advertising->start();
}

const char *EffectName(EffectId effect)
{
  switch (effect)
  {
  case EFFECT_SOLID:
    return "Solid";
  case EFFECT_RAINBOW:
    return "Rainbow";
  case EFFECT_TWINKLE:
    return "Twinkle";
  case EFFECT_COMET:
    return "Comet";
  case EFFECT_BOUNCE:
    return "Bounce";
  case EFFECT_FIRE:
    return "Fire";
  case EFFECT_METEOR:
    return "Meteor";
  case EFFECT_PALETTE:
    return "Palette";
  case EFFECT_DOUBLEPALETTE:
    return "DualPal";
  case EFFECT_STAREFFECT:
    return "Stars";
  case EFFECT_MARQUEE:
  default:
    return "Marq";
  }
}

#ifndef ENABLE_OTA
#define ENABLE_OTA 0
#endif

#ifndef WIFI_SSID
#define WIFI_SSID "CHANGE_ME"
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "CHANGE_ME"
#endif

#ifndef OTA_HOSTNAME
#define OTA_HOSTNAME "underbar-lighting"
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define TIMES_PER_SECOND(x) EVERY_N_MILLISECONDS(1000 / x)

// FractionalColor
/**
 * @brief Returns a fractional color from 0.0 to 1.0; abstracts the fadToBlackBy out to this function in case we
 * want to improve the color math or do color correction all in one location at a later date.
 *
 * @param colorIn The color to return a fraction of
 * @param fraction The fraction of @p colorIn to return, from 0.0 to 1.0
 * @return a color that is a fraction of the input color
 */
CRGB ColorFraction(CRGB colorIn, float fraction)
{
  fraction = min(1.0f, fraction);
  return CRGB(colorIn).fadeToBlackBy(255 * (1.0f - fraction));
}

/**
 * @brief Draw a partial pixel (or more) on a strip of LEDs at a given position, with a given color.
 *
 * @param fPos The position of the first pixel to draw. This is a float, so you can draw a pixel at
 *             a fractional position on the strip.
 * @param count The number of pixels to draw. This can be a fraction of a pixel, so you can draw a
 *              partial pixel at the start and/or end of the run.
 * @param color The color to draw the pixels with.
 *
 * This function draws pixels by blending the color into the current color of the LEDs. This allows
 * you to draw a partial pixel at the start and/or end of the run, and still get a nice-looking result.
 *
 * If the count is more than the number of pixels left on the strip, this function will wrap around to
 * the start of the strip and keep drawing.
 */
void DrawPixels(float fPos, float count, CRGB color)
{
  // Calculate how much the first pixel will hold
  float availFirstPixel = 1.0f - (fPos - (long)(fPos));
  float amtFirstPixel = min(availFirstPixel, count);
  float remaining = min(count, FastLED.size() - fPos);
  int iPos = fPos;

  // Blend (add) in the color of the first partial pixel

  if (remaining > 0.0f)
  {
    FastLED.leds()[iPos++] += ColorFraction(color, amtFirstPixel);
    remaining -= amtFirstPixel;
  }

  // Now draw any full pixels in the middle

  while (remaining > 1.0f)
  {
    FastLED.leds()[iPos++] += color;
    remaining--;
  }

  // Draw tail pixel, up to a single full pixel

  if (remaining > 0.0f)
  {
    FastLED.leds()[iPos++] += ColorFraction(color, remaining);
  }
}

void DrawMarqueeComparison()
{
  static float scroll = 0.0f;
  scroll += 0.1f;
  if (scroll > 5.0f)
    scroll -= 5.0f;

  for (float i = scroll; i < (NUM_LEDS / 2) - 1; i += 5)
  {
    DrawPixels(i, 3, CRGB::BlueViolet);
    DrawPixels(NUM_LEDS - 1 - (int)i, 3, CRGB::DarkOrange);
  }
}

void ApplyState()
{
  g_Brightness = g_State.brightness;
  g_EffectSpeed = g_State.speed;
  g_EffectCount = g_State.count;
  FastLED.setBrightness(g_Brightness);

  if (g_State.effect == EFFECT_BOUNCE && g_EffectCount != g_lastBounceCount)
  {
    uint8_t count = constrain(g_EffectCount, 1, 8);
    g_bounceEffect.Resize(count);
    g_lastBounceCount = count;
  }
}

void RenderEffect()
{
  if (!g_State.power)
  {
    FastLED.clear();
    return;
  }

  switch (g_State.effect)
  {
  case EFFECT_SOLID:
    fill_solid(g_LEDs, NUM_LEDS, g_State.color);
    break;
  case EFFECT_RAINBOW:
    DrawRainbow();
    break;
  case EFFECT_TWINKLE:
    DrawTwinkle();
    break;
  case EFFECT_COMET:
    DrawComet();
    break;
  case EFFECT_BOUNCE:
    g_bounceEffect.Draw();
    break;
  case EFFECT_FIRE:
    DrawFire();
    break;
  case EFFECT_METEOR:
    DrawMeteor();
    break;
  case EFFECT_PALETTE:
    DrawPalette();
    break;
  case EFFECT_DOUBLEPALETTE:
    DrawDoublePalette();
    break;
  case EFFECT_STAREFFECT:
    DrawStarEffect();
    break;
  case EFFECT_MARQUEE:
  default:
    DrawMarquee();
    break;
  }
}

void PrintHAStubHelp()
{
  Serial.println("Home Assistant stub (future MQTT topics):");
  Serial.printf("  device: %s\n", kHAConfig.device_name);
  Serial.printf("  command: %s\n", kHAConfig.command_topic);
  Serial.printf("  state: %s\n", kHAConfig.state_topic);
  Serial.printf("  availability: %s\n", kHAConfig.availability_topic);
  Serial.printf("Serial commands: power on|off, brightness 0-255, effect 0-%u, color r,g,b, speed 1-255, count 1-16\n", EFFECT_COUNT - 1);
  SendBleLine("Serial commands: power on|off, brightness 0-255, effect 0-10, color r,g,b, speed 1-255, count 1-16");
}

void ApplyCommand(const char *command)
{
  if (strncmp(command, "power ", 6) == 0)
  {
    const char *value = command + 6;
    g_State.power = (strcmp(value, "on") == 0 || strcmp(value, "1") == 0);
    return;
  }

  if (strncmp(command, "brightness ", 11) == 0)
  {
    g_State.brightness = (uint8_t)constrain(atoi(command + 11), 0, 255);
    return;
  }

  if (strncmp(command, "effect ", 7) == 0)
  {
    int effect = atoi(command + 7);
    g_State.effect = ClampEffect(effect);
    ApplyEffectPreset(g_State.effect);
    return;
  }

  if (strncmp(command, "color ", 6) == 0)
  {
    int r = 0;
    int g = 0;
    int b = 0;
    sscanf(command + 6, "%d,%d,%d", &r, &g, &b);
    g_State.color = CRGB(constrain(r, 0, 255), constrain(g, 0, 255), constrain(b, 0, 255));
    return;
  }

  if (strncmp(command, "speed ", 6) == 0)
  {
    g_State.speed = (uint8_t)constrain(atoi(command + 6), 1, 255);
    SaveEffectPreset(g_State.effect);
    return;
  }

  if (strncmp(command, "count ", 6) == 0)
  {
    g_State.count = (uint8_t)constrain(atoi(command + 6), 1, 16);
    SaveEffectPreset(g_State.effect);
    return;
  }
}

void HandleSerialControl()
{
  while (Serial.available() > 0)
  {
    char c = (char)Serial.read();
    if (c == '\r')
    {
      continue;
    }
    if (c == '\n')
    {
      if (g_commandLength > 0)
      {
        g_commandBuffer[g_commandLength] = '\0';
        ApplyCommand(g_commandBuffer);
        g_commandLength = 0;
      }
      continue;
    }
    if (g_commandLength < (sizeof(g_commandBuffer) - 1))
    {
      g_commandBuffer[g_commandLength++] = c;
    }
  }
}

void SetupWiFiAndOTA()
{
#if ENABLE_OTA
  g_otaStatus = "WIFI";
  if (strcmp(WIFI_SSID, "CHANGE_ME") == 0 || strlen(WIFI_SSID) == 0)
  {
    Serial.println("OTA disabled: set WIFI_SSID/WIFI_PASS build flags.");
    g_otaStatus = "OFF";
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.printf("Connecting to WiFi SSID: %s\n", WIFI_SSID);

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < 15000)
  {
    delay(250);
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi connect failed; OTA not started.");
    g_otaStatus = "FAIL";
    g_wifiConnected = false;
    return;
  }

  Serial.printf("WiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());
  g_otaStatus = "RDY";
  g_wifiConnected = true;

  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.onStart([]()
                     {
                       Serial.println("OTA update start");
                       g_otaStatus = "UPD";
                     });
  ArduinoOTA.onEnd([]()
                   {
                     Serial.println("OTA update end");
                     g_otaStatus = "RDY";
                   });
  ArduinoOTA.onError([](ota_error_t error)
                     {
                       Serial.printf("OTA error: %u\n", error);
                       g_otaStatus = "ERR";
                     });
  ArduinoOTA.begin();
  Serial.println("OTA ready.");
#endif
}

void HandleHttpSet()
{
  if (g_httpServer.hasArg("power"))
  {
    String value = g_httpServer.arg("power");
    g_State.power = (value == "on" || value == "1" || value == "true");
  }
  if (g_httpServer.hasArg("brightness"))
  {
    g_State.brightness = (uint8_t)constrain(g_httpServer.arg("brightness").toInt(), 0, 255);
  }
  if (g_httpServer.hasArg("effect"))
  {
    int effect = g_httpServer.arg("effect").toInt();
    g_State.effect = ClampEffect(effect);
    ApplyEffectPreset(g_State.effect);
  }
  if (g_httpServer.hasArg("r") || g_httpServer.hasArg("g") || g_httpServer.hasArg("b"))
  {
    int r = g_httpServer.hasArg("r") ? g_httpServer.arg("r").toInt() : g_State.color.r;
    int g = g_httpServer.hasArg("g") ? g_httpServer.arg("g").toInt() : g_State.color.g;
    int b = g_httpServer.hasArg("b") ? g_httpServer.arg("b").toInt() : g_State.color.b;
    g_State.color = CRGB(constrain(r, 0, 255), constrain(g, 0, 255), constrain(b, 0, 255));
  }
  if (g_httpServer.hasArg("speed"))
  {
    g_State.speed = (uint8_t)constrain(g_httpServer.arg("speed").toInt(), 1, 255);
    SaveEffectPreset(g_State.effect);
  }
  if (g_httpServer.hasArg("count"))
  {
    g_State.count = (uint8_t)constrain(g_httpServer.arg("count").toInt(), 1, 16);
    SaveEffectPreset(g_State.effect);
  }

  String json = "{";
  json += "\"power\":" + String(g_State.power ? "true" : "false");
  json += ",\"brightness\":" + String(g_State.brightness);
  json += ",\"effect\":" + String((int)g_State.effect);
  json += ",\"r\":" + String(g_State.color.r);
  json += ",\"g\":" + String(g_State.color.g);
  json += ",\"b\":" + String(g_State.color.b);
  json += ",\"speed\":" + String(g_State.speed);
  json += ",\"count\":" + String(g_State.count);
  json += ",\"ota\":\"" + String(g_otaStatus) + "\"";
  json += "}";
  g_httpServer.send(200, "application/json", json);
}

void SetupHttpServer()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS mount failed.");
  }

  g_httpServer.on("/", []()
                  {
                    File file = SPIFFS.open("/index.html", FILE_READ);
                    if (!file)
                    {
                      g_httpServer.send(500, "text/plain", "index.html missing.");
                      return;
                    }
                    g_httpServer.streamFile(file, "text/html");
                    file.close();
                  });
  g_httpServer.on("/status", []()
                  { HandleHttpSet(); });
  g_httpServer.on("/set", []()
                  { HandleHttpSet(); });
  g_httpServer.on("/updatefs", HTTP_POST, []()
                  {
                    if (Update.hasError())
                    {
                      g_httpServer.send(500, "text/plain", "SPIFFS update failed.");
                    }
                    else
                    {
                      g_httpServer.send(200, "text/plain", "SPIFFS update complete. Rebooting...");
                      delay(250);
                      ESP.restart();
                    }
                  },
                  []()
                  {
                    HTTPUpload &upload = g_httpServer.upload();
                    if (upload.status == UPLOAD_FILE_START)
                    {
                      Serial.printf("SPIFFS update: %s\n", upload.filename.c_str());
                      if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_SPIFFS))
                      {
                        Update.printError(Serial);
                      }
                    }
                    else if (upload.status == UPLOAD_FILE_WRITE)
                    {
                      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                      {
                        Update.printError(Serial);
                      }
                    }
                    else if (upload.status == UPLOAD_FILE_END)
                    {
                      if (!Update.end(true))
                      {
                        Update.printError(Serial);
                      }
                      else
                      {
                        Serial.printf("SPIFFS update success: %u bytes\n", upload.totalSize);
                      }
                    }
                    else if (upload.status == UPLOAD_FILE_ABORTED)
                    {
                      Update.end();
                      Serial.println("SPIFFS update aborted.");
                    }
                  });
  g_httpServer.on("/debug", []()
                  {
                    String json = "{";
                    json += "\"wifi\":" + String(g_wifiConnected ? "true" : "false");
                    json += ",\"ip\":\"" + WiFi.localIP().toString() + "\"";
                    json += ",\"rssi\":" + String(WiFi.RSSI());
                    json += ",\"i2c\":\"";
                    if (g_i2cAddress == 0)
                    {
                      json += "none";
                    }
                    else
                    {
                      json += "0x" + String(g_i2cAddress, HEX);
                    }
                    json += "\"";
                    json += "}";
                    g_httpServer.send(200, "application/json", json);
                  });
  g_httpServer.begin();
  Serial.println("HTTP server started on port 80.");
}

uint8_t ScanI2C()
{
  uint8_t found = 0;
  for (uint8_t addr = 0x03; addr <= 0x77; ++addr)
  {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0)
    {
      found = addr;
      break;
    }
  }
  return found;
}

void StartupLedTest()
{
  FastLED.clear();
  fill_solid(g_LEDs, NUM_LEDS, CRGB::Red);
  FastLED.show();
  delay(350);
  fill_solid(g_LEDs, NUM_LEDS, CRGB::Green);
  FastLED.show();
  delay(350);
  fill_solid(g_LEDs, NUM_LEDS, CRGB::Blue);
  FastLED.show();
  delay(350);
  FastLED.clear();
  FastLED.show();
}

void setup()
{

  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);



  Serial.begin(115200);
  const uint32_t serialWaitStart = millis();
  while (!Serial && (millis() - serialWaitStart) < 1500)
  {
    delay(10);
  }
  Serial.println("ESP32 Startup...");
  PrintHAStubHelp();
  SetupBleSerial();
  SetupWiFiAndOTA();
  if (g_wifiConnected)
  {
    SetupHttpServer();
  }

  Wire.begin(21, 22);
  Wire.setClock(100000);
  Wire.setTimeOut(50);
  g_i2cAddress = ScanI2C();
  if (g_i2cAddress != 0)
  {
    g_OLED.setI2CAddress(g_i2cAddress << 1);
  }
  Serial.printf("I2C address: 0x%02X\n", g_i2cAddress);

  g_OLED.begin();
  g_OLED.clear();
  g_OLED.setFont(u8g2_font_6x10_tf);
  g_OLED.setFontPosTop();
  const int kLineGap = 1; // Minimal vertical spacing to fit three lines on 128x32 OLEDs
  g_lineHeight = g_OLED.getMaxCharHeight() + kLineGap;
  const int totalTextHeight = g_lineHeight * 3;
  g_oledTopOffset = max(0, (kOledHeight - totalTextHeight) / 2);

  if (OLED_RULER_TEST)
  {
    // Pixel ruler: draw alternating horizontal rows so we can see the true visible area.
    g_OLED.clearBuffer();
    for (uint8_t y = 0; y < 32; y++)
    {
      if ((y & 1) == 0)
      {
        g_OLED.drawHLine(0, y, 128);
      }
    }
    g_OLED.sendBuffer();
    for (;;)
    {
      delay(1000);
    }
  }
  g_OLED.clearBuffer();
  g_OLED.sendBuffer();

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(g_LEDs, NUM_LEDS); // Add our LED strip to the FastLED Library
  FastLED.setBrightness(g_Brightness);

  FastLED.setMaxPowerInMilliWatts(g_MaxPowerInMilliwatts);

  StartupLedTest();

  g_OLED.clearBuffer();
  g_OLED.setCursor(kOledTextXOffset, g_oledTopOffset);
  if (g_i2cAddress == 0)
  {
    g_OLED.printf("I2C: none");
  }
  else
  {
    g_OLED.printf("I2C:0x%02X OLED OK", g_i2cAddress);
  }
  g_OLED.setCursor(kOledTextXOffset, g_oledTopOffset + g_lineHeight);
  g_OLED.printf("LED test done");
  g_OLED.sendBuffer();
  delay(8000);

}

void loop()
{
  // put your main code here, to run repeatedly:
  bool bLED = 0;

  while (true)
  {

    EVERY_N_MILLISECONDS(20)
    {
      /*
      fadeToBlackBy(g_LEDs, NUM_LEDS, 64);
      int cometsize = 15;
      int iPos = beatsin16(16, 0, NUM_LEDS - cometsize);
      byte hue = beatsin8(48);

      for (int i = iPos; i < iPos + cometsize; i++)
        g_LEDs[i] = CHSV(hue, 255, 255);
      */

      RenderEffect();
    }

    EVERY_N_MILLISECONDS(250)
    {
      const char *effectName = EffectName(g_State.effect);

      g_OLED.clearBuffer();
      g_OLED.setDrawColor(0);
      g_OLED.drawBox(0, 0, kOledTextXOffset, kOledHeight);
      g_OLED.setDrawColor(1);
      g_OLED.setCursor(kOledTextXOffset, g_oledTopOffset);
      g_OLED.printf("FPS:%3u Fx:%s", FastLED.getFPS(), effectName);
      g_OLED.setCursor(kOledTextXOffset, g_oledTopOffset + g_lineHeight);
      g_OLED.printf("Pwr:%4umW Bright:%3u", calculate_unscaled_power_mW(g_LEDs, NUM_LEDS), g_Brightness);
      g_OLED.setCursor(kOledTextXOffset, g_oledTopOffset + (g_lineHeight * 2));
      g_OLED.printf("OTA: %s IP: %s", g_otaStatus, WiFi.localIP().toString().c_str());
      g_OLED.sendBuffer();
    }

    HandleSerialControl();
    ApplyState();
#if ENABLE_OTA
    ArduinoOTA.handle();
#endif
    if (g_wifiConnected)
    {
      g_httpServer.handleClient();
    }
    FastLED.delay(10);                   // Show and delay
  }
}
