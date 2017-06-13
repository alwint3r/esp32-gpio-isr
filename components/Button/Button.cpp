#include "Button.h"

Button::Button(gpio_num_t gpioNum)
:gpioNum(gpioNum) {
    gpioConfig = new gpio_config_t;
    queue = xQueueCreate(1, sizeof(int));
}

Button::~Button() {
    delete gpioConfig;
    vTaskDelete(taskHandle);
    gpio_uninstall_isr_service();
}

void Button::begin() {
    gpioConfig->pin_bit_mask = BIT(gpioNum);
    gpioConfig->mode = GPIO_MODE_INPUT;
    gpioConfig->pull_up_en = GPIO_PULLUP_DISABLE;
    gpioConfig->pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpioConfig->intr_type = GPIO_INTR_ANYEDGE;

    ESP_ERROR_CHECK(gpio_config(gpioConfig));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(gpioNum, isrHandler, this));

    xTaskCreate(runTask, "gpioISRTask", 2048, this, 5, &taskHandle);
}

void Button::onPressed(OnButtonActionPerformed callback) {
    pressedCallback = callback;
}

void Button::onReleased(OnButtonActionPerformed callback) {
    releasedCallback = callback;
}

void Button::onLongPressed(OnButtonActionPerformed callback) {
    longPressedCallback = callback;
}

uint32_t Button::millis() {
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

void Button::isrHandler(void* arg) {
    Button* mui = (Button*)arg;
    int level = gpio_get_level(mui->gpioNum);
    xQueueSendToBackFromISR(mui->queue, &level, nullptr);
}

void Button::runTask(void* arg) {
    Button* mui = (Button*)arg;

    int readLevel;
    uint32_t lastPressed = 0;

    while (true) {
        xQueueReceive(mui->queue, &readLevel, portMAX_DELAY);
        if (readLevel == mui->RELEASED_STATE) {
            if (lastPressed > 0 && (mui->millis() - lastPressed > 3000)) {
                if (mui->longPressedCallback != nullptr) {
                    mui->longPressedCallback();
                }
            } else {
                if (mui->releasedCallback != nullptr) {
                    mui->releasedCallback();
                }
            }
        }

        if (readLevel == mui->PRESSED_STATE) {
            lastPressed = mui->millis();
            
            if (mui->pressedCallback != nullptr) {
                mui->pressedCallback();
            }
        }
    }
}
