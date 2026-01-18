1. User asked for current status of UnderBarLighting project (inventory and code flow).
2. Switched partitions to default.csv for OTA; build now exceeds OTA slot size (firmware 1.69 MB > 1.31 MB).
3. Disabled BLE via ENABLE_BLE flag to reduce firmware size for OTA partitions.
4. LTO discussed; user will consider enabling for release builds.
5. OLED power display now shows Watts with one decimal when >= 1000 mW.
6. OLED IP/OTA line now uses 5x7 font to fit full IP + status.
7. SSH key generation attempted; empty passphrase non-interactive attempts failed.
8. Added milestones to sketch.md (OTA baseline, OTA size trim, effects, controls, integrations).
9. Removed BLE code paths entirely to avoid BLE library compilation.
10. Switched FastLED color order to RGB to fix red/green swap.
11. Set FastLED color order back to GRB and apply state in HTTP handler for immediate brightness updates.
12. Switched FastLED color order back to GRB to match WS2812B data order (fix green/red swap).
13. Web UI color picker now swaps R/G before sending to match GRB data order.
14. Added MQTT + Home Assistant discovery support (PubSubClient + ArduinoJson), with secrets.h.example MQTT settings.
15. Fixed MQTT build errors (macro ordering, forward declaration, publish payload cast).
16. User asked how to verify MQTT integration is enabled in Home Assistant.
17. MQTT discovery publish succeeded; HA config message observed on homeassistant/light/underbar_lighting_01/config.
