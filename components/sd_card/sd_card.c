#include <stdio.h>
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/spi_common.h"
#include "sd_card.h"

static const char *TAG = "SD_CARD";

// ESP32-S3 GPIO assignments

#define PIN_NUM_MISO 13
#define PIN_NUM_MOSI 11
#define PIN_NUM_CLK 12
#define PIN_NUM_CS 10

esp_err_t sd_card_init(void){
    esp_err_t ret;
    // 1. Configure the SPI bus
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_config = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    // 2. Initialise SPI bus
    ret = spi_bus_initialize(host.slot, &bus_config, SDSPI_DEFAULT_DMA);

    if(ret!=ESP_OK){
        ESP_LOGE(TAG, "SPI bus initialisation failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // 3. Configure the SD card spi interface
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();

    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    // 4. Configure FAT filesystem mounting
    esp_vfs_fat_mount_config_t mount_config = {.format_if_mount_failed = false, .max_files = 5, .allocation_unit_size = 16*1024,};

    // 5. Mount SD card

    sdmmc_card_t *card;

    ret = esp_vfs_fat_sdspi_mount(SD_CARD_MOUNT_POINT, &host,&slot_config,&mount_config,&card);

    if(ret!=ESP_OK){
        ESP_LOGE(TAG,"SD card mounting failed: %s", esp_err_to_name(ret));
        spi_bus_free(host.slot);
        return ret;
    }

    ESP_LOGI(TAG,"SD card mounted successfully!");

    // 6, Print info about the card
    sdmmc_card_print_info(stdout, card);

    return ESP_OK;

}