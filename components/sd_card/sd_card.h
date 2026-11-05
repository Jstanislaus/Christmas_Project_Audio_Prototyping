#ifndef SD_CARD_H
#define SD_CARD_H

#include "esp_err.h"

#define SD_CARD_MOUNT_POINT "/sdcard"

esp_err_t sd_card_init(void);

#endif