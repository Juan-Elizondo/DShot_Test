#ifndef DSHOT_ESC_H
#define DSHOT_ESC_H
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
#include <dshot_esc_encoder.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
class DShot_ESC
{
    public:
        DShot_ESC(const bool _bidirectional, gpio_num_t ESC_Out_Pin, gpio_num_t ESC_In_Pin, const char* _name);
        void DshotWrite(uint16_t throttle);
    private:
        const char* name;
        rmt_channel_handle_t esc_out_chan = NULL;
        rmt_channel_handle_t esc_in_chan = NULL;
        rmt_encoder_handle_t dshot_encoder = NULL;
        const bool bidirectional;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
#endif