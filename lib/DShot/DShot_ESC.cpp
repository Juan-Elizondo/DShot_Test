#include "DShot_ESC.h"
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
DShot_ESC::DShot_ESC(const bool _bidirectional, gpio_num_t ESC_Out_Pin, gpio_num_t ESC_In_Pin, const char* _name): name(_name),bidirectional(_bidirectional)
{
    rmt_tx_channel_config_t tx_chan_config = {
        .gpio_num = ESC_Out_Pin,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 80000000,
        .mem_block_symbols = 64,
        .trans_queue_depth = 10, // set the number of transactions that can be pending in the background
        .flags = {(uint32_t)bidirectional,0,(uint32_t)bidirectional,1} //invert_out, with_dma, io_loop_back, io_od_mode
    };
    dshot_esc_encoder_config_t encoder_config = {
        .resolution = 80000000,
        .baud_rate = 600000, // DSHOT600 protocol
        .post_delay_us = 100 + (31+28)*((uint32_t)bidirectional), // extra delay between each frame is 30us. about 15us frame length for DSHOT600
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &this->esc_out_chan));
    //ESP_LOGI(this->name, "Created RMT TX Channel!");

    ESP_ERROR_CHECK(rmt_new_dshot_esc_encoder(&encoder_config, &this->dshot_encoder));
    //ESP_LOGI(this->name, "Installed Dshot ESC Encoder!");

    ESP_ERROR_CHECK(rmt_enable(this->esc_out_chan));
    //ESP_LOGI(this->name, "%s Channel Enabled!", this->name);

    rmt_transmit_config_t tx_config = {
        .loop_count = -1, // infinite loop
        .flags = {0}
    };
    dshot_esc_throttle_t throttle = {
        .throttle = 0,
        .telemetry_req = false, // telemetry is not supported in this example
        .bidirectional = bidirectional
    };
    ESP_ERROR_CHECK(rmt_transmit(this->esc_out_chan, dshot_encoder, &throttle, sizeof(throttle), &tx_config));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
void DShot_ESC::DshotWrite(uint16_t throttle)
{
    rmt_transmit_config_t tx_config = {
        .loop_count = -1, // infinite loop
        .flags = {0} // eot_level
    };
    dshot_esc_throttle_t esc_throttle = {
        .throttle = throttle,
        .telemetry_req = false,
        .bidirectional = bidirectional
    };

    ESP_ERROR_CHECK(rmt_transmit(this->esc_out_chan, this->dshot_encoder, &esc_throttle, sizeof(esc_throttle), &tx_config));
    ESP_ERROR_CHECK(rmt_disable(this->esc_out_chan));
    ESP_ERROR_CHECK(rmt_enable(this->esc_out_chan));
    //ESP_LOGI(this->name, "%s Throttle Written!", this->name);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//