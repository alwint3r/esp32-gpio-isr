#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_err.h>
#include <Button.h>

static const char* TAG = "gpio_isr";
Button* button;

extern "C" void app_main() {
    nvs_flash_init();

    button = new Button(GPIO_NUM_0);
    button->onPressed([=]() {
        ESP_LOGI(TAG, "GPIO0 is pressed");
    });

    button->onReleased([=]() {
        ESP_LOGI(TAG, "GPIO0 is released");
    });

    button->onLongPressed([=]() {
        ESP_LOGI(TAG, "GPIO0 is long-pressed");
    });

    button->begin();
}
