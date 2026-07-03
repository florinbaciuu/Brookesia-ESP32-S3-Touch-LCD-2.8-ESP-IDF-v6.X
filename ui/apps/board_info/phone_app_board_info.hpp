#pragma once

#include "systems/phone/esp_brookesia_phone_app.hpp"

class PhoneAppBoardInfo : public ESP_Brookesia_PhoneApp {
public:
    PhoneAppBoardInfo();
    ~PhoneAppBoardInfo() override = default;

protected:
    bool run(void) override;
    bool back(void) override;
};
