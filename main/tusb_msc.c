#include <stdint.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include "tusb_console.h"
#include "sdkconfig.h"

static const char *TAG = "example";
static uint8_t buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];


//------------- prototypes -------------//
// static void cdc_task(void);
void tinyusb_cdc_rx_callback0(int itf, cdcacm_event_t *event);
void tinyusb_cdc_rx_callback1(int itf, cdcacm_event_t *event);


void app_main(void)
{
    ESP_LOGI(TAG, "USB initialization");
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false,
        .configuration_descriptor = NULL,
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    tinyusb_config_cdcacm_t acm_cfg = {
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = 64,
        .callback_rx = &tinyusb_cdc_rx_callback0,
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = NULL,
        .callback_line_coding_changed = NULL
    };

    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));

#if (CONFIG_TINYUSB_CDC_COUNT > 1)
    acm_cfg.cdc_port = TINYUSB_CDC_ACM_1;
    acm_cfg.callback_rx = &tinyusb_cdc_rx_callback1;
    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));
#endif

    ESP_ERROR_CHECK(esp_tusb_init_console(TINYUSB_CDC_ACM_0)); // 使用CDC作为标准输出
    ESP_LOGI(TAG, "USB initialization DONE");
    while(1){
        vTaskDelay(1000);
        printf("hello\n");
    }
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+

void tinyusb_cdc_rx_callback0(int itf, cdcacm_event_t *event)
{
    /* initialization */
    size_t rx_size = 0;

    /* read */
    esp_err_t ret = tinyusb_cdcacm_read(itf, buf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);
    if (ret == ESP_OK) {
        // ESP_LOGI(TAG, "Data from channel %d:", itf);
        // ESP_LOG_BUFFER_HEXDUMP(TAG, buf, rx_size, ESP_LOG_INFO);
    } else {
        // ESP_LOGE(TAG, "Read error");
    }

    /* write back */
    tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM_0, buf, rx_size);
    tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM_1, buf, rx_size);
    tinyusb_cdcacm_write_flush(TINYUSB_CDC_ACM_0, 0);
    tinyusb_cdcacm_write_flush(TINYUSB_CDC_ACM_1, 0);
}

void tinyusb_cdc_rx_callback1(int itf, cdcacm_event_t *event)
{
    /* initialization */
    size_t rx_size = 0;

    /* read */
    esp_err_t ret = tinyusb_cdcacm_read(itf, buf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Data from channel %d:", itf);
        ESP_LOG_BUFFER_HEXDUMP(TAG, buf, rx_size, ESP_LOG_INFO);
    } else {
        ESP_LOGE(TAG, "Read error");
    }

    /* write back */
    tinyusb_cdcacm_write_queue(itf, buf, rx_size);
    tinyusb_cdcacm_write_flush(itf, 0);
}