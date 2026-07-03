#pragma once

#include "systems/phone/esp_brookesia_phone_app.hpp"

class PhoneAppSettings : public ESP_Brookesia_PhoneApp {
public:
    PhoneAppSettings();
    ~PhoneAppSettings() override = default;

protected:
    bool run(void) override;
    bool back(void) override;
};
