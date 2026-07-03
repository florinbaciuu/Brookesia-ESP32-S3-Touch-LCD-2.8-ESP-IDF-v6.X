#pragma once

#include "systems/phone/esp_brookesia_phone_app.hpp"

class PhoneAppSystem : public ESP_Brookesia_PhoneApp {
public:
    PhoneAppSystem();
    ~PhoneAppSystem() override = default;

protected:
    bool run(void) override;
    bool back(void) override;
};
