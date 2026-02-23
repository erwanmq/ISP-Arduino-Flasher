#include "drivers/mcu_serial.h"

#include "SoftwareSerial.h"

#define RX_PIN 11
#define TX_PIN 10
#define MCU_BAUDRATE 19200

#define TIMER_MCU_ANSWER 10000000 // 10 seconds

static bool mcu_serial_initialized = false;

static SoftwareSerial mcuSerial(RX_PIN, TX_PIN);

en_mcu_serial_error_msg mcu_serial_init(void)
{
    if (false == mcu_serial_initialized)
    {
        mcuSerial.begin(MCU_BAUDRATE);
        mcu_serial_initialized = true;
    }
    return MCU_SERIAL_OK;
}

static en_mcu_serial_error_msg mcu_serial_wait_for_answer(void)
{
    en_mcu_serial_error_msg err = MCU_SERIAL_OK;
    if (false == mcu_serial_initialized)
    {
        return MCU_SERIAL_NOT_INIT;
    }

    const unsigned long start = micros();
    while ((!mcuSerial.available()) && (micros() - start < TIMER_MCU_ANSWER));

    if (!mcuSerial.available())
    {
        err = MCU_SERIAL_TIMEOUT;
    }
    return err;
}

en_mcu_serial_error_msg mcu_serial_write(const uint8_t *bytes_to_write, uint8_t size_to_write)
{
    en_mcu_serial_error_msg err = MCU_SERIAL_OK;
    if (false == mcu_serial_initialized)
    {
        return MCU_SERIAL_NOT_INIT;
    }

    if (NULL != bytes_to_write)
    {
        mcuSerial.write(bytes_to_write, size_to_write);
    }
    else 
    {
        err = MCU_SERIAL_ERROR;
    }

    return err;
}

int mcu_serial_read(uint8_t *i_buffer, uint8_t size_to_read)
{
    /* Wait for a response */
    if (false == mcu_serial_initialized)
    {
        return MCU_SERIAL_NOT_INIT;
    }

    en_mcu_serial_error_msg wait = mcu_serial_wait_for_answer();
    int data_read = 0;
    if (MCU_SERIAL_OK == wait)
    {
        while (mcuSerial.available() && data_read < size_to_read)
        {
            uint8_t b = mcuSerial.read();
            if (NULL != i_buffer)
            {
                i_buffer[data_read++] = b;
            }
        }
    }
    return data_read;
}

en_mcu_serial_error_msg mcu_serial_peek(uint8_t* i_buffer)
{
    en_mcu_serial_error_msg err = MCU_SERIAL_OK;
    if (false == mcu_serial_initialized)
    {
        return MCU_SERIAL_NOT_INIT;
    }

    if (NULL != i_buffer)
    {
        /* Wait for a response */
        err = mcu_serial_wait_for_answer();
        if (MCU_SERIAL_OK == err)
        {
            *i_buffer = (uint8_t)mcuSerial.peek();
        }
    }
    return err;
}

en_mcu_serial_error_msg mcu_serial_empty_buffer(void)
{
    en_mcu_serial_error_msg err = MCU_SERIAL_OK;
    if (false == mcu_serial_initialized)
    {
        return MCU_SERIAL_NOT_INIT;
    }

    mcuSerial.readStringUntil('\n');
    return err;
}