#include "drivers/computer_serial.h"

#include <stdio.h>
#include <stdarg.h>

#define COMPUTER_BAUDRATE 19200

static bool computer_serial_initialized = false;

#define CHECK_INIT() \
    if (false == computer_serial_initialized) return COMPUTER_SERIAL_NOT_INIT;

en_computer_serial_error_msg computer_serial_init()
{
    if (false == computer_serial_initialized)
    {
        Serial.begin(COMPUTER_BAUDRATE);
        computer_serial_initialized = true;
    }
    return COMPUTER_SERIAL_OK;
}

static en_computer_serial_error_msg computer_serial_wait_for_answer()
{
    en_computer_serial_error_msg err = COMPUTER_SERIAL_OK;
    CHECK_INIT();

    while ((!Serial.available()));

    return err;
}

en_computer_serial_error_msg computer_serial_write(const uint8_t *bytes_to_write, uint8_t size_to_write)
{
    en_computer_serial_error_msg err = COMPUTER_SERIAL_OK;
    CHECK_INIT();

    if (NULL != bytes_to_write)
    {
        for (size_t i = 0; i < size_to_write; i++)
        {
            Serial.write(bytes_to_write[i]);
        }
    }
    else 
    {
        err = COMPUTER_SERIAL_ERROR;
    }
    return err;
}

static en_computer_serial_error_msg computer_serial_vprint(const char* buffer, ...)
{
    CHECK_INIT();

    va_list args;
    va_start(args, buffer);

    while(*buffer) {
        if ('%' == *buffer)
        {
            buffer++;
            if ('d' == *buffer)
            {
                int i = va_arg(args, int);
                Serial.print(i);
            }
            else if ('s' == *buffer)
            {
                char *s = va_arg(args, char*);
                Serial.print(s);
            }
        }
        else 
        {
            Serial.print(*buffer);
        }

        buffer++;
    }

    va_end(args);
    return COMPUTER_SERIAL_OK;
}

en_computer_serial_error_msg computer_serial_print(const char* buffer, ...)
{
    va_list args;
    va_start(args, buffer);

    en_computer_serial_error_msg ret = computer_serial_vprint(buffer, args);

    va_end(args);
    return ret;
}



int computer_serial_read_line(uint8_t *i_buffer, uint8_t size_to_read)
{
    CHECK_INIT();

    /* Wait for a response */
    en_computer_serial_error_msg wait = computer_serial_wait_for_answer();
    int data_read = 0;
    if (COMPUTER_SERIAL_OK == wait)
    {
        data_read = Serial.readBytesUntil('\n', i_buffer, size_to_read);
    }

    return data_read;
}
 
en_computer_serial_error_msg computer_serial_peek(uint8_t *i_buffer)
{
    en_computer_serial_error_msg err = COMPUTER_SERIAL_OK;
    CHECK_INIT();

    if (NULL != i_buffer)
    {
        /* Wait for a response */
        err = computer_serial_wait_for_answer();
        if (COMPUTER_SERIAL_OK == err)
        {
            *i_buffer = (uint8_t)Serial.peek();
        }
    }
    return err;
}

en_computer_serial_error_msg computer_serial_empty_buffer(void)
{
    en_computer_serial_error_msg err = COMPUTER_SERIAL_OK;
    CHECK_INIT();

    while (Serial.available())
    {
        Serial.read();
    }
    return err;
}