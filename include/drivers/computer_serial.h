#ifndef COMPUTER_SERIAL_H__
#define COMPUTER_SERIAL_H__

#include "Arduino.h"

typedef enum 
{
    COMPUTER_SERIAL_OK,
    COMPUTER_SERIAL_ERROR,
    COMPUTER_SERIAL_NOT_INIT,
} en_computer_serial_error_msg;

en_computer_serial_error_msg computer_serial_init(void);

/* Used to write to the Serial */
en_computer_serial_error_msg computer_serial_write(const uint8_t *bytes_to_write, uint8_t size_to_write);
en_computer_serial_error_msg computer_serial_print(const char* buffer, ...);
en_computer_serial_error_msg computer_serial_print_args(const char* buffer, va_list args);

/* Used to read from the Serial */
int computer_serial_read_line(uint8_t *i_buffer, uint8_t size_to_read);

en_computer_serial_error_msg computer_serial_empty_buffer(void);

#endif /* COMPUTER_SERIAL_H__ */