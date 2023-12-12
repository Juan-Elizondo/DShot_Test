#ifndef MainTest
#define MainTest
#include <DShot_ESC.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_intr_alloc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_timer.h"

class Main_Test
{
    public:
        void setup();
        void loop();

        // DShot pins
        const gpio_num_t ESC_L = GPIO_NUM_22;
        const gpio_num_t ESC_R = GPIO_NUM_23;

    private:


};

#endif