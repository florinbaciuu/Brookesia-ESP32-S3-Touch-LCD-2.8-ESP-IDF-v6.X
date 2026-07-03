#pragma once


#define BASE_PATH "/spiflash"

/******************************
 *    INTERNAL MMC DEFINES
 *****************************/
#define FAT_PARTITION_LABEL "fatfs"
#define FAT_MOUNT_PATH "/fatfs"

/******************************
 *    INTERNAL SPIFFS DEFINES
 *****************************/
#define SPIFFS_PARTITION_LABEL "spiffs"
#define SPIFFS_MOUNT_PATH "/spiffs"

/******************************
 *    INTERNAL littlefs DEFINES
 *****************************/
#define LITTLEFS_PARTITION_LABEL "littlefs"
#define LITTLEFS_MOUNT_PATH "/littlefs"

//-------------------------

/******************************
 *      SD MMC DEFINES
 *****************************/
#define PIN_NUM_MISO  16
#define PIN_NUM_MOSI  17
#define PIN_NUM_CLK   14
#define PIN_NUM_CS    21

#define SDMMC_CLK_PIN  14 // SD_CLK
#define SDMMC_CMD_PIN  17 // SD_MOSI // 45 
#define SDMMC_D0_PIN   16 // SD_MISO // 46
#define SDMMC_D1_PIN   18
#define SDMMC_D2_PIN   15
#define SDMMC_D3_PIN   21 // SD_CS


//---
#define SD_FREQ_DEFAULT 20000   /*!< SD/MMC Default speed (limited by clock divider) */
#define SD_FREQ_HIGHSPEED 40000 /*!< SD High speed (limited by clock divider) */

#define SD_MOUNT_LABEL "sdcard"
#define SD_MOUNT_PATH "/sdcard"

#define SDMMC_SYSTEM
//#define SPISD_SYSTEM


//-------------------------

#define GPIO_NO (-1)
#define SDMMC_NO_CD GPIO_NO  ///< indicates that card detect line is not used
#define SDMMC_NO_WP GPIO_NO  ///< indicates that write protect line is not used

#define SDMMC_SLOT_CONFIG_DEF()  \
    {                            \
        .clk   = SDMMC_CLK_PIN,  \
        .cmd   = SDMMC_CMD_PIN,   \
        .d0    = SDMMC_D0_PIN, \
        .d1    = SDMMC_D1_PIN,        \
        .d2    = SDMMC_D2_PIN,        \
        .d3    = SDMMC_D3_PIN,        \
        .d4    = GPIO_NO,        \
        .d5    = GPIO_NO,        \
        .d6    = GPIO_NO,        \
        .d7    = GPIO_NO,        \
        .cd    = SDMMC_NO_CD,    \
        .wp    = SDMMC_NO_WP,    \
        .width = 4,              \
        .flags = 0,              \
    }
//-------------------------
