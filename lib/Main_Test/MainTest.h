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

class Main_Test
{
    public:
        void setup();
        void loop();

        // DShot pins
        const gpio_num_t ESC_L_out = GPIO_NUM_22;
        const gpio_num_t ESC_L_in = GPIO_NUM_4;
        const gpio_num_t ESC_R_out = GPIO_NUM_23;
        const gpio_num_t ESC_R_in = GPIO_NUM_5;


};

#endif