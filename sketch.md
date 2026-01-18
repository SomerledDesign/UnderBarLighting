### Under bar lighting SPECS ###

__Hardware__
1. ESP32 Mini
2. WS2814 LED Strip (LEDS?)
3. 128x64 OLED Display (I2C)
4. 5v 3A supply
5. 3D printed case

***Software***
- Over the Air updates
- Web UI
- many and varied LED effects which make the heart tingle.

### Milestones ###
1. OTA + Web UI baseline (SPIFFS hosting, /status + /set endpoints, OLED status line).
  - OTA size compliance (trim build to fit OTA partitions; BLE off by default).
  - Effects + presets pass (marquee/rainbow/twinkle/comet/bounce/fire/meteor/palette/dual/star).

2. Integrations (MQTT/Home Assistant) + release polish.
