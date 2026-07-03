# Waveshare ESP32-S3 Touch LCD 2.8 V1 - ESP-IDF 6.x + ESP-Brookesia

Project pentru placa **Waveshare ESP32-S3-Touch-LCD-2.8 V1**, adaptat pentru **ESP-IDF 6.x**, LVGL si **ESP-Brookesia Phone**.

Repository:

```text
https://github.com/florinbaciuu/ESP32-S3-Touch-LCD-2.8-ESP-IDF-v6.X.git
```

Documentatie placa:

```text
https://docs.waveshare.com/ESP32-S3-Touch-LCD-2.8
https://www.waveshare.com/esp32-s3-touch-lcd-2.8.htm
```

![ESP32-S3-Touch-LCD-2.8 details](https://github.com/florinbaciuu/ESP32-S3-Touch-LCD-2.8-ESP-IDF-v6.X/blob/main/images/ESP32-S3-Touch-LCD-2.8-details-1.jpg)
![ESP32-S3-Touch-LCD-2.8 details](https://github.com/florinbaciuu/ESP32-S3-Touch-LCD-2.8-ESP-IDF-v6.X/blob/main/images/ESP32-S3-Touch-LCD-2.8-details-7.jpg)
![ESP32-S3-Touch-LCD-2.8 intro](https://github.com/florinbaciuu/ESP32-S3-Touch-LCD-2.8-ESP-IDF-v6.X/blob/main/images/ESP32-S3-Touch-LCD-2.8-details-intro.jpg)

## Hardware Target

- Board: Waveshare ESP32-S3-Touch-LCD-2.8 V1
- MCU: ESP32-S3
- Flash: 16 MB
- PSRAM: 8 MB Octal PSRAM
- Display: 240 x 320, ST7789, RGB565
- Touch: CST328
- Storage: SD/MMC, FATFS intern, SPIFFS, LittleFS
- RTC: PCF85063
- IMU: QMI8658
- Audio: PCM5101 pe I2S
- Power key/latch: GPIO6/GPIO7

## Ce este implementat

- ESP-IDF 6.x build functional pentru placa Waveshare V1.
- LVGL integrat prin `frameworks/bsp_lvgl_framework`.
- ESP-Brookesia Phone launcher activ.
- Demo-urile predefinite Brookesia au fost scoase din launcher si dezactivate din configuratie.
- UI-ul proiectului este separat in componenta `ui`.
- Aplicatiile Phone sunt in `ui/apps`.
- Prima aplicatie locala este `Board`, in `ui/apps/board_info`.
- HAL Brookesia expune capabilitati pentru board info, display, touch, backlight, battery si storage.
- Montare storage:
  - `/sdcard`
  - `/fatfs`
  - `/spiffs`
  - `/littlefs`
- RTC PCF85063 initializat pe I2C auxiliar.
- QMI8658 initializat pe I2C auxiliar.
- Power key initializat.
- PCM5101 initializat, cu volum setat la 50%.
- `LV_USE_SNAPSHOT=1` pentru preview in Brookesia Recents.

## Fix important pentru touch/scroll

Pe placa aceasta, tap-ul functiona, dar scroll-ul LVGL nu functiona in aplicatia `Board`. Problema era ca callback-ul de touch trimitea `RELEASED` imediat cand o citire CST328 nu returna coordonate, ceea ce rupea secventa de drag.

Fixul este in:

```text
frameworks/bsp_lvgl_framework/bsp_lvgl_indev.c
```

S-a adaugat un `release hold` de 70 ms dupa ultima citire valida. Astfel LVGL vede o secventa continua `PRESSED` in timpul scroll-ului, chiar daca touch controller-ul rateaza 1-2 citiri.

Raport detaliat:

```text
TOUCH_REP.txt
```

## Note ESP-Brookesia

- Launcher-ul foloseste profil adaptat pentru rezolutia 240 x 320.
- Aplicatia `Board` foloseste navigation bar fixa pentru Back/Home/Recents.
- Gesturile Brookesia au fost evitate in app-ul `Board`, pentru ca pe acest touch pot captura evenimentele prea agresiv.
- Recents are nevoie de `LV_USE_SNAPSHOT=1`; altfel Brookesia logheaza eroare la salvarea preview-ului aplicatiei.
- Stack-ul task-ului LVGL a fost marit prin configuratia BSP LVGL pentru stabilitate cu Brookesia Phone.

## Structura relevanta

```text
configs/
  board_config/              Config hardware Waveshare V1
  board_pins/                Pini board
  lv_conf.h                  Config LVGL
  lvgl_framework_config/     Config framework LVGL local

drivers/
  cst328_driver/             Driver touch CST328
  lcd_backlight_driver_interface/
  vernon_st7789t_driver/

frameworks/
  bsp_lvgl_framework/        Init LVGL, display, input, mutex, task LVGL

interfaces/
  bsp_aux_i2c_interface/     I2C auxiliar pentru RTC/IMU
  bsp_i2c_interface/         I2C touch
  bsp_lcd_interface/
  bsp_spi_interface/
  bsp_touch_interface/

modules/
  audio_pcm5101/
  brookesia_adapter/         Adapter ESP-Brookesia + HAL board
  filesystem/
  power_key/
  qmi8658/
  rtc_pcf85063/

ui/
  apps/board_info/           Aplicatia Phone "Board"
  include/
  src/brookesia_phone_launcher.cpp
```

## Build

Incarca mediul ESP-IDF 6.x:

```bash
source ~/.espressif/v6.0.2/esp-idf/export.sh
```

Configureaza proiectul:

```bash
idf.py reconfigure
```

Compileaza:

```bash
idf.py build
```

Flash:

```bash
idf.py -p PORT flash monitor
```

Inlocuieste `PORT` cu portul placii, de exemplu `/dev/cu.usbmodemXXXX` pe macOS.

## Configurari importante

In `sdkconfig.defaults`:

```text
CONFIG_BSP_LVGL_MAIN_TASK_STACK_SIZE=24576
CONFIG_ESP_BROOKESIA_CONF_PHONE_ENABLE_APP_EXAMPLES=n
```

In `configs/lv_conf.h`:

```c
#define LV_DEF_REFR_PERIOD  1
#define LV_USE_SNAPSHOT 1
```

## Status testat pe placa

Confirmat functional pe hardware:

- Display LVGL porneste.
- Touch CST328 functioneaza pentru tap si scroll.
- Brookesia Phone launcher porneste.
- Aplicatia `Board` porneste si se poate scrolla.
- Bara Brookesia Back/Home/Recents functioneaza.
- SD/MMC se monteaza la `/sdcard`.
- FATFS intern, SPIFFS si LittleFS se monteaza.
- RTC PCF85063 raspunde pe I2C.
- QMI8658 raspunde pe I2C.
- Power key raspunde la apasare.
- PCM5101 reda ton de test.
- Snapshot LVGL este activ pentru Recents.

## Note pentru dezvoltare

- Aplicatiile noi pentru Brookesia Phone se adauga modular in `ui/apps/<nume_app>`.
- In `ui/CMakeLists.txt` se adauga sursele si include path-ul aplicatiei noi.
- Instalarea aplicatiilor se face in `ui/src/brookesia_phone_launcher.cpp`.
- Evitati sa modificati direct demo-urile Brookesia; proiectul foloseste aplicatii proprii.
- Pentru probleme de scroll/touch similare in alte proiecte, porniti de la `TOUCH_REP.txt`.

