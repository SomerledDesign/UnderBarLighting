1. User asked for current status of UnderBarLighting project (inventory and code flow).
2. Switched partitions to default.csv for OTA; build now exceeds OTA slot size (firmware 1.69 MB > 1.31 MB).
3. Disabled BLE via ENABLE_BLE flag to reduce firmware size for OTA partitions.
4. LTO discussed; user will consider enabling for release builds.
5. OLED power display now shows Watts with one decimal when >= 1000 mW.
6. OLED IP/OTA line now uses 5x7 font to fit full IP + status.
