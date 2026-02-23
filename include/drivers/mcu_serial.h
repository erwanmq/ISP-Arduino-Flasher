#ifndef MCU_SERIAL_H__
#define MCU_SERIAL_H__

#include "Arduino.h"


typedef enum 
{
    MCU_SERIAL_OK,
    MCU_SERIAL_ERROR,
    MCU_SERIAL_TIMEOUT,
    MCU_SERIAL_NOT_INIT,
} en_mcu_serial_error_msg;

en_mcu_serial_error_msg mcu_serial_init(void);

/* Used to write to the Serial */
en_mcu_serial_error_msg mcu_serial_write(const uint8_t *bytes_to_write, uint8_t size_to_write);

/* Used to read from the Serial */
int mcu_serial_read(uint8_t *i_buffer, uint8_t size_to_read);

en_mcu_serial_error_msg mcu_serial_peek(uint8_t* i_buffer);

en_mcu_serial_error_msg mcu_serial_empty_buffer(void);

#endif /* MCU_SERIAL_H__ */