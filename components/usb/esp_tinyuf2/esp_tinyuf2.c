/* SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tusb.h"
#include "uf2.h"
#include "board_flash.h"
#include "esp_log.h"
#include "esp_rom_gpio.h"
#include "esp_tinyuf2.h"
#include "hal/usb_phy_hal.h"
#include "hal/usb_phy_ll.h"
#include "soc/gpio_pins.h"
#include "soc/gpio_sig_map.h"
#include "soc/soc_caps.h"

const static char* TAG = "TUF2";
static bool _if_init = false;

esp_err_t esp_tinyuf2_install(tinyuf2_ota_config_t* ota_config, tinyuf2_nvs_config_t* nvs_config)
{
    if (_if_init) {
        ESP_LOGE(TAG, "Tinyuf2 already installed");
        return ESP_ERR_INVALID_STATE;
    }
    _if_init = true;

    if (!(ota_config || nvs_config)) {
        ESP_LOGE(TAG, "Invalid Parameter, config can’t be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (ota_config) {
        if (ota_config->subtype < ESP_PARTITION_SUBTYPE_APP_OTA_MIN || (ota_config->subtype > ESP_PARTITION_SUBTYPE_APP_OTA_MAX && ota_config->subtype != ESP_PARTITION_SUBTYPE_ANY)) {
            ESP_LOGE(TAG, "Invalid partition type");
            return ESP_ERR_INVALID_ARG;
        }
        if (ota_config->if_restart) {
            ESP_LOGW(TAG, "Enable restart, SoC will restart after update complete");
        }
        board_flash_init(ota_config->subtype, ota_config->label, ota_config->complete_cb, ota_config->if_restart);
    }

    if (nvs_config) {
        board_flash_nvs_init(nvs_config->part_name, nvs_config->namespace_name, nvs_config->modified_cb);
    }

    uf2_init();

    return ESP_OK;
}

esp_err_t esp_tinyuf2_uninstall(void)
{
    if (!_if_init) {
        ESP_LOGE(TAG, "Tinyuf2 not installed");
        return ESP_ERR_INVALID_STATE;
    }
    board_flash_deinit();
    board_flash_nvs_deinit();
    ESP_LOGI(TAG, "UF2 Updater uninstall succeed");
    _if_init = false;
    return ESP_OK;
}
