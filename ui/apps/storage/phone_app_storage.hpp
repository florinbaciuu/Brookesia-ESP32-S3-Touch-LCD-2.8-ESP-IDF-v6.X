#pragma once

#include "systems/phone/esp_brookesia_phone_app.hpp"

class PhoneAppStorage : public ESP_Brookesia_PhoneApp {
public:
    PhoneAppStorage();
    ~PhoneAppStorage() override = default;

protected:
    bool run(void) override;
    bool back(void) override;
};
