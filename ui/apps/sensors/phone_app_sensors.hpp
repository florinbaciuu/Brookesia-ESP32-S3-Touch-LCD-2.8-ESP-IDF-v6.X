#pragma once

#include "systems/phone/esp_brookesia_phone_app.hpp"

class PhoneAppSensors : public ESP_Brookesia_PhoneApp {
public:
    PhoneAppSensors();
    ~PhoneAppSensors() override = default;

protected:
    bool run(void) override;
    bool back(void) override;
};
