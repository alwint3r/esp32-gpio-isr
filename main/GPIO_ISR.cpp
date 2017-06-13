#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_err.h>
#include <GPIOInterruptService.h>

static const char* TAG = "gpio_isr";
GPIOInterruptService* gpioIntrService;

extern "C" void app_main() {
    nvs_flash_init();

    gpioIntrService = new GPIOInterruptService(GPIO_NUM_0);
    gpioIntrService->onPressed([=]() {
        ESP_LOGI(TAG, "GPIO0 is pressed");
    });

    gpioIntrService->onReleased([=]() {
        ESP_LOGI(TAG, "GPIO0 is released");
    });

    gpioIntrService->onLongPressed([=]() {
        ESP_LOGI(TAG, "GPIO0 is long-pressed");
    });

    gpioIntrService->begin();
}
