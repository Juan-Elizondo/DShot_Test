#ifndef DSHOT_ESC_H
#define DSHOT_ESC_H
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
#include <dshot_esc_encoder.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "esp_attr.h"
#include "esp_timer.h"
#include <cmath>

#define ESCRX_BITMAX 21 // 12 bit data, 4 bit crc. encoded into 20+1 bit GCR
#define ESCRX_WORDMAX 64 // worst case is flip in every bit. but must be even -> 22. min is 64
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
struct ESCData {
    uint32_t latest_eRPM;
    bool eRPM_data_available;
    bool data_valid;
    struct {
        uint8_t error_message;
    } debug;
};
namespace DShot_callback
{
    struct tx_user_data {
        rmt_channel_handle_t receive_channel;
        rmt_receive_config_t receive_config;
        rmt_symbol_word_t word_arr[ESCRX_WORDMAX];
    };
    struct rx_user_data {
        uint32_t rx_clock_div;
        ESCData& ESC_data;
        rx_user_data(ESCData& _ESC_data):ESC_data(_ESC_data){}
    };
    const uint8_t nibble_inv_map[32] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x9,0xA,0xB,0x0,0xD,0xE,0xF,
                                        0x0,0x0,0x2,0x3,0x0,0x5,0x6,0x7,0x0,0x0,0x8,0x1,0x0,0x4,0xC,0x0};
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
class DShot_ESC
{
    public:
        DShot_ESC(const dshot_frequency_t _frequency, const bool _bidirectional, gpio_num_t ESC_Pin, bool TXS_Buffer, ESCData& _ESC_data);
        void Arm_ESC(void);
        void Throttle_Write(uint16_t throttle); // care to limit time between calls to at least (2.0*31.0 + 21.0*1.0e6*1.0/(((double)frequency)*5.0/4.0)) in bidirectional mode

    private:
        const dshot_frequency_t& frequency;
        const bool bidirectional;

        rmt_channel_handle_t esc_out_chan = NULL;
        rmt_tx_channel_config_t tx_chan_config;
        rmt_encoder_handle_t dshot_loop_encoder = NULL;
        dshot_esc_encoder_config_t loop_encoder_config;
        dshot_esc_encoder_config_t burst_encoder_config;
        rmt_encoder_handle_t dshot_burst_encoder = NULL;
        rmt_tx_event_callbacks_t tx_cbs;

        rmt_channel_handle_t esc_in_chan = NULL;
        rmt_rx_channel_config_t rx_chan_config;
        rmt_rx_event_callbacks_t rx_cbs;
        rmt_receive_config_t receive_config;
        bool rx_enabled = false;

        DShot_callback::tx_user_data tx_callback_data;
        DShot_callback::rx_user_data rx_callback_data;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
#endif