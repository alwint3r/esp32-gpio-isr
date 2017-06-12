#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_err.h>

QueueHandle_t qHandle;

static const char* TAG = "gpio_isr";

void pollTask(void* arg);
void gpioISRHandler(void* arg);
unsigned long IRAM_ATTR millis();

extern "C" void app_main() {
    nvs_flash_init();
    qHandle = xQueueCreate(5, sizeof(int));

    xTaskCreate(pollTask, "pollTask", 2048, NULL, 5, NULL);
}

unsigned long IRAM_ATTR millis() {
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void gpioISRHandler(void* arg) {
    int level = gpio_get_level(GPIO_NUM_0);
    xQueueSendToBackFromISR(qHandle, &level, NULL);
}

void pollTask(void* arg) {
    gpio_config_t gpioCfg;
    gpioCfg.pin_bit_mask = GPIO_SEL_0;
    gpioCfg.mode = GPIO_MODE_INPUT;
    gpioCfg.pull_up_en = GPIO_PULLUP_DISABLE;
    gpioCfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpioCfg.intr_type = GPIO_INTR_ANYEDGE;

    const int PRESSED_STATE = 0;
    const int RELEASED_STATE = 1;

    uint32_t lastPressed = 0;
    
    ESP_ERROR_CHECK(gpio_config(&gpioCfg));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(GPIO_NUM_0, gpioISRHandler, NULL));

    int readLevel;
    while (true) {
        ESP_LOGI(TAG, "Waiting for Queue items from ISR");
        xQueueReceive(qHandle, &readLevel, portMAX_DELAY);
        ESP_LOGI(TAG, "Woken up by Queue wait.");

        if (readLevel == RELEASED_STATE) {
            if (lastPressed > 0 && (millis() - lastPressed) > 3000) {
                ESP_LOGI(TAG, "Long pressed");
            } else {
                ESP_LOGI(TAG, "GPIO0 released");
            }
        }

        if (readLevel == PRESSED_STATE) {
            ESP_LOGI(TAG, "GPIO0 pressed");
            lastPressed = millis();
        }
    }
}