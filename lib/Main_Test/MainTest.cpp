#include <MainTest.h>

void Main_Test::setup(void)
{
    const gpio_config_t dshot_io_config = {
        .pin_bit_mask = (1ULL<<ESC_L_out)|(1ULL<<ESC_R_out),
        .mode = GPIO_MODE_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&dshot_io_config));
}

void Main_Test::loop(void)
{
    bool bidirectional = true;
    //DShot_ESC L_ESC(bidirectional, ESC_L_out, ESC_L_in,"L_ESC");
    DShot_ESC R_ESC(bidirectional, ESC_R_out, ESC_R_in, "R_ESC");
    vTaskDelay(2000/portTICK_PERIOD_MS);
    uint16_t throttle = 100;
    while(true)
    {
        //L_ESC.DshotWrite(throttle);
        R_ESC.DshotWrite(throttle);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}