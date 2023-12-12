#include "DShot_ESC.h"
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
bool IRAM_ATTR esc_rmt_tx_done_callback(rmt_channel_handle_t channel, const rmt_tx_done_event_data_t *data_tx_num, void *user_data)
{

    DShot_callback::tx_user_data* callback_data = (DShot_callback::tx_user_data*)user_data;
    ESP_ERROR_CHECK(rmt_receive(callback_data->receive_channel, callback_data->word_arr, sizeof(callback_data->word_arr), &callback_data->receive_config));

    return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
bool IRAM_ATTR esc_rmt_rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *received_data, void *user_data)
{
    DShot_callback::rx_user_data* callback_data = (DShot_callback::rx_user_data*)user_data;

    if(received_data->num_symbols == 0)
    {
        callback_data->ESC_data.debug.error_message = 0x01;
        return 0;
    }
    if(received_data->received_symbols[0].level0 != 0)
    {
        callback_data->ESC_data.debug.error_message = 0x02;
        return 0;
    }

    uint32_t encoded_value = 0;
    uint32_t decoded_message = 0;
    uint8_t crc = 0;
    int value_length = 0;  
    for(int i=0; i<(int)received_data->num_symbols; i++)
    {
        for(int j=0; j<(int)round((float)received_data->received_symbols[i].duration0/(float)callback_data->rx_clock_div); j++)
        {
            encoded_value = (encoded_value<<1)|0;
            value_length++;
        }
        for(int j=0; j<(int)round((float)received_data->received_symbols[i].duration1/(float)callback_data->rx_clock_div); j++)
        {
            encoded_value = (encoded_value<<1)|1;
            value_length++;
        }
    }
    if((21-4) < value_length && value_length < 21)
    {
        if(received_data->received_symbols[(int)received_data->num_symbols-1].level1 == 1)
        {
            while(value_length<21)
            {
                encoded_value = (encoded_value<<1)|1;
                value_length++;
            }
        }
    }
    if(value_length != 21)
    {
        callback_data->ESC_data.debug.error_message = 0x03;
        return 0;
    }

    encoded_value = encoded_value^(encoded_value>>1); //gcr

    crc = DShot_callback::nibble_inv_map[(int)(encoded_value&0x1F)];
    decoded_message |= DShot_callback::nibble_inv_map[(int)((encoded_value>>5)&0x1F)];
    decoded_message |= DShot_callback::nibble_inv_map[(int)((encoded_value>>10)&0x1F)]<<4;
    decoded_message |= DShot_callback::nibble_inv_map[(int)((encoded_value>>15)&0x1F)]<<8;

    if((~(decoded_message ^ (decoded_message >> 4) ^ (decoded_message >> 8)) & 0x0F) == crc)
    {
        if(((decoded_message >> 8)&0x1) != 0)
        {
            //eRPM period us
            callback_data->ESC_data.latest_eRPM = (decoded_message&0b000111111111)<<(decoded_message>>9);
            callback_data->ESC_data.debug.error_message = 0xFF;
            callback_data->ESC_data.eRPM_data_available = true;
            callback_data->ESC_data.data_valid = true;
            return 1;
        }
    }
    callback_data->ESC_data.debug.error_message = 0x04;
    return 0;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
DShot_ESC::DShot_ESC(const dshot_frequency_t _frequency, bool _bidirectional, gpio_num_t ESC_Pin, bool TXS_Buffer, const char* _name, ESCData& _ESC_data):
    name(_name), frequency(_frequency), bidirectional(_bidirectional), rx_callback_data(_ESC_data)
{
    if(bidirectional) //Rx channel must be initialized first if bidirectional
    {
        this->rx_chan_config = {
            .gpio_num = ESC_Pin,
            .clk_src = RMT_CLK_SRC_DEFAULT,
            .resolution_hz = 80000000, // 80MHz is APB max. max out resolution to keep tight timing
            .mem_block_symbols = 64, // worst case is flip in every bit. but must be even -> 22. minimum is 64
            .flags = {0,0,1} // invert_in, with_dma, io_loop_back
        };
        this->rx_cbs = {
            .on_recv_done = esc_rmt_rx_done_callback
        };
        this->receive_config = {
            // returned signal is of frequency*5/4
            .signal_range_min_ns = (uint32_t)(0.5*1.0e9*1.0/(((double)frequency)*5.0/4.0)), // A pulse whose width is smaller than this threshold will be treated as glitch and ignored 
            .signal_range_max_ns = (uint32_t)(4.0*1.0e9*1.0/(((double)frequency)*5.0/4.0)) // RMT will stop receiving if one symbol level has kept more than `signal_range_max_ns`
                                    // due to encoding, max consecutive sequence is of length 3 bits (max 2 0s in a row in gcr). so 4 bit lengths can be treated as a stop. 
        };
        this->rx_callback_data.rx_clock_div = (uint32_t)(80000000.0/(((double)frequency)*5.0/4.0));

        ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_chan_config, &this->esc_in_chan));
        ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(this->esc_in_chan, &rx_cbs, &this->rx_callback_data));
        ESP_ERROR_CHECK(rmt_enable(this->esc_in_chan));
        this->rx_enabled = true;
    }

    this->tx_chan_config = {
        .gpio_num = ESC_Pin,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 80000000, // 80MHz is APB max. max out resolution to keep tight timing
        .mem_block_symbols = 64, // worst case is flip in every sent bit. 16 bit. 8 words. minimum is 64 when no DMA used.
        .trans_queue_depth = 1, // set the number of transactions that can be pending in the background. NOTE: if exceeded current behavior is program hangs
        .flags = {(uint32_t)bidirectional,0,(uint32_t)bidirectional,1} //invert_out, with_dma, io_loop_back, io_od_mode
    };
    this->loop_encoder_config = {
        .resolution = 80000000, // 80MHz resolution
        .baud_rate = frequency, // DShot frequency
        .post_delay_us = (uint32_t)(2.0*31.0 + 21.0*1.0e6*1.0/(((double)frequency)*5.0/4.0)) // min 30us between frames. doing 31us to account for frame length calc truncation
                                            // returned frame length in us
    };
    this->burst_encoder_config = {
        .resolution = 80000000, // 80MHz resolution
        .baud_rate = frequency, // DShot frequency
        .post_delay_us = 0 // care to limit time between calls to at least (2.0*31.0 + 21.0*1.0e6*1.0/(((double)frequency)*5.0/4.0)) in bidirectional mode
    };

    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &this->esc_out_chan));
    ESP_ERROR_CHECK(rmt_new_dshot_esc_encoder(&this->loop_encoder_config, &this->dshot_loop_encoder, TXS_Buffer));
    ESP_ERROR_CHECK(rmt_new_dshot_esc_encoder(&this->burst_encoder_config, &this->dshot_burst_encoder, TXS_Buffer));

    if(bidirectional)
    {
        this->tx_cbs = {
            .on_trans_done = esc_rmt_tx_done_callback
        };
        this->tx_callback_data = {
            .receive_channel = this->esc_in_chan,
            .receive_config = this->receive_config,
            .word_arr = {}
        };
        
        ESP_ERROR_CHECK(rmt_tx_register_event_callbacks(this->esc_out_chan, &tx_cbs, &this->tx_callback_data));
    }

    ESP_ERROR_CHECK(rmt_enable(this->esc_out_chan));

    Arm_ESC();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
void DShot_ESC::Throttle_Write(uint16_t throttle)
{
    rmt_transmit_config_t tx_config = {
        .loop_count = 0, // no loop
        .flags = {0} // eot_level
    };
    dshot_esc_throttle_t esc_throttle = {
        .throttle = throttle,
        .telemetry_req = false,
        .bidirectional = bidirectional
    };

    if(bidirectional)
    {
        this->rx_callback_data.ESC_data.latest_eRPM = 0;
        this->rx_callback_data.ESC_data.eRPM_data_available = 0;
        this->rx_callback_data.ESC_data.data_valid = 0;
        this->rx_callback_data.ESC_data.debug.error_message = 0x00;

        if(rx_enabled)
        {
            ESP_ERROR_CHECK(rmt_disable(this->esc_in_chan));
        }
        ESP_ERROR_CHECK(rmt_enable(this->esc_in_chan));
        rx_enabled = true;
    }

    ESP_ERROR_CHECK(rmt_disable(this->esc_out_chan));
    ESP_ERROR_CHECK(rmt_enable(this->esc_out_chan));
    ESP_ERROR_CHECK(rmt_transmit(this->esc_out_chan, this->dshot_burst_encoder, &esc_throttle, sizeof(esc_throttle), &tx_config));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
void IRAM_ATTR ESC_Arm_Timer_Callback(void* params)
{
    bool* timer_trigger = (bool*)params;
    *timer_trigger = true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
void DShot_ESC::Arm_ESC(void)
{
    rmt_transmit_config_t tx_config = {
        .loop_count = -1, // infinite loop
        .flags = {0} // eot_level
    };
    dshot_esc_throttle_t esc_throttle = {
        .throttle = 0,
        .telemetry_req = false,
        .bidirectional = bidirectional
    };

    if(bidirectional)
    {
        ESP_ERROR_CHECK(rmt_disable(this->esc_in_chan));
        rx_enabled = false;
    }
    ESP_ERROR_CHECK(rmt_disable(this->esc_out_chan));
    ESP_ERROR_CHECK(rmt_enable(this->esc_out_chan));
    ESP_ERROR_CHECK(rmt_transmit(this->esc_out_chan, this->dshot_loop_encoder, &esc_throttle, sizeof(esc_throttle), &tx_config));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//