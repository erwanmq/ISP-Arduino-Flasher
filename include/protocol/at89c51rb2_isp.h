#ifndef PROTOCOL_AT89C51RB2_H__
#define PROTOCOL_AT89C51RB2_H__

#include "Arduino.h"


typedef enum 
{
    AT89C51RB2_ISP_OK,
    AT89C51RB2_ISP_ERROR,
} en_at89c51rb2_isp_error_msg;

/* Bootloader control */
en_at89c51rb2_isp_error_msg at89c51rb2_enter_bootloader(void);
en_at89c51rb2_isp_error_msg at89c51rb2_reset_and_run(void);
en_at89c51rb2_isp_error_msg at89c51rb2_finish_flash(void);

/* Erase */
en_at89c51rb2_isp_error_msg at89c51rb2_erase_block(uint8_t block);
en_at89c51rb2_isp_error_msg at89c51rb2_full_chip_erase(void);

/* Programming */
en_at89c51rb2_isp_error_msg at89c51rb2_write_program_data_chunk(const uint8_t *buffer,
                           uint8_t size,
                           uint16_t address);
en_at89c51rb2_isp_error_msg at89c51rb2_write_program_data(const uint8_t *buffer, uint8_t size);

/* Reading */
en_at89c51rb2_isp_error_msg at89c51rb2_read_serial(uint8_t *buffer, uint8_t size);
en_at89c51rb2_isp_error_msg at89c51rb2_read_id(uint8_t buffer[2]);
en_at89c51rb2_isp_error_msg at89c51rb2_read_ssb(uint8_t buffer[2]);
en_at89c51rb2_isp_error_msg at89c51rb2_read_hardware_bytes(uint8_t *buffer);

en_at89c51rb2_isp_error_msg at89c51rb2_display_memory(const char start_address[2], const char end_address[2], uint8_t *i_buffer, uint8_t size_buffer);

#endif /* PROTOCOL_AT89C51RB2_H__ */
