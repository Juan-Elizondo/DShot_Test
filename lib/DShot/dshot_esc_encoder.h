/*
 * Relying heavily on https://github.com/espressif/esp-idf/tree/master/examples/peripherals/rmt/dshot_esc
 *
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
#pragma once
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
#include <stdint.h>
#include <stdbool.h>
#include "driver/rmt_encoder.h"
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
#ifdef __cplusplus
extern "C" {
#endif
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * @brief Throttle representation in DShot protocol
 */
typedef struct {
    uint16_t throttle;  /*!< Throttle value */
    bool telemetry_req; /*!< Telemetry request */
    bool bidirectional; /*!< Bidirectional request */
} dshot_esc_throttle_t;

typedef enum {
    DSHOT150 = 150000,
    DSHOT300 = 300000,
    DSHOT600 = 600000,
    DSHOT1200 = 1200000
} dshot_frequency_t;

/**
 * @brief Type of Dshot ESC encoder configuration
 */
typedef struct {
    uint32_t resolution;            /*!< Encoder resolution, in Hz */
    dshot_frequency_t baud_rate;    /*!< Dshot protocol runs at several different baud rates, e.g. DSHOT300 = 300k baud rate */
    uint32_t post_delay_us;         /*!< Delay time after one Dshot frame, in microseconds */
} dshot_esc_encoder_config_t;

typedef struct {
    rmt_encoder_t base;
    rmt_encoder_t *bytes_encoder;
    rmt_encoder_t *copy_encoder;
    rmt_symbol_word_t dshot_delay_symbol;
    int state;
} rmt_dshot_esc_encoder_t;
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
/**
 * @brief Create RMT encoder for encoding Dshot ESC frame into RMT symbols
 *
 * @param[in] config Encoder configuration
 * @param[out] ret_encoder Returned encoder handle
 * @return
 *      - ESP_ERR_INVALID_ARG for any invalid arguments
 *      - ESP_ERR_NO_MEM out of memory when creating Dshot ESC encoder
 *      - ESP_OK if creating encoder successfully
 */
esp_err_t rmt_new_dshot_esc_encoder(const dshot_esc_encoder_config_t *config, rmt_encoder_handle_t *ret_encoder, bool TXS_Buffer);
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
#ifdef __cplusplus
}
#endif
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//