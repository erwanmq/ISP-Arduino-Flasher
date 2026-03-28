#include "at89c51rb2_isp.h"
#include "mcu_serial.h"

#define DELAY_FOR_STABILITY 5000
#define DELAY_BOOTTIME 10000
#define DELAY_RESET 20000

#define MCU_RST_PIN     4
#define MCU_PSEN_PIN    6

/* ISP functions */
/** Read functions **/
#define READ_FCT              0x05
/*** IDs ***/
#define READ_IDs              0x00
#define READ_MANUFACTURER_ID  0x00
/*** Security Bytes ***/
#define READ_SECURITY_BYTE    0x07
#define READ_SSB 0x00
#define READ_BSB 0x01

#define READ_HARDWARE_BYTE1   0x0B
#define READ_HARDWARE_BYTE2   0x00

/** Write functions **/
#define WRITE_FCT 0x03
#define ERASE     0x01
#define ERASE_BLK_0 0x00
#define ERASE_BLK_1 0x20
#define ERASE_BLK_2 0x40
#define ERASE_BLK_3 0x80
#define ERASE_BLK_4 0xC0
#define FULL_CHIP_ERASE 0x07
#define WRITE_FUSE  0x0A
#define WRITE_FUSE_BLJB 0x04
#define WRITE_BSB_SBV 0x06
#define WRITE_BSB 0x00

/** Display functions **/
#define DISPLAY_FCT 0x04
#define DISPLAY_DATA  0x00
#define BLANK_CHECK   0x01

/** Program data functions */
#define PROGRAM_DATA_FCT  0x00

static void byte_to_ascii(uint8_t byte, char *out) {
    const char hex_chars[] = "0123456789ABCDEF";
    out[0] = hex_chars[(byte >> 4) & 0x0F];
    out[1] = hex_chars[byte & 0x0F];
}

static uint8_t hex_char_to_val(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;

    return 0; 
}

uint8_t ascii_to_byte(const char *in) {
    uint8_t high = hex_char_to_val(in[0]);
    uint8_t low  = hex_char_to_val(in[1]);
    return (high << 4) | low;
}

static en_at89c51rb2_isp_error_msg at89c51rb2_write_and_check(const uint8_t *buffer, uint8_t size)
{
    en_at89c51rb2_isp_error_msg ret = AT89C51RB2_ISP_OK;
    en_mcu_serial_error_msg err = MCU_SERIAL_OK;
    for (int i = 0; i < size; i++)
    {
        /* Write to MCU */
        err = mcu_serial_write(&buffer[i], 1);
        if (MCU_SERIAL_OK != err)
        {
            Serial.print("FAiled to mcu serial write");
            ret = AT89C51RB2_ISP_ERROR;
            break;
        }

        uint8_t echo;
        uint8_t byte_read = 0;
        err = mcu_serial_read(&echo, 1, &byte_read);
        if (MCU_SERIAL_OK != err || 1 != byte_read)
        {
            ret = AT89C51RB2_ISP_ERROR;
            break;
        }
        if (echo != buffer[i])
        {
            ret = AT89C51RB2_ISP_ERROR;
            break;
        }
    }

    if (MCU_SERIAL_OK == err)
    {
        /* Check the status */
        uint8_t status;
        mcu_serial_peek(&status);

        /* 
        *   X: checksum error
        *   P: Security error
        *   L: Security error
        *   .: OK
        */
        bool status_error   = ('X' == status || 'L' == status || 'P' == status); 
        bool status_ok      = ('.' == status);

        if (status_error || status_ok)
        {
            /* We empty the buffer ('X' + 'CR+LF')*/
            uint8_t discard_read = 0;
            err = mcu_serial_read(NULL, 3, &discard_read); // discard 3 char
        }
        
        /* err will be != 0 if status_error is true */
        if (status_error)
        {
            ret = AT89C51RB2_ISP_ERROR;
        }
    }

    if (MCU_SERIAL_OK != err)
    {
        ret = AT89C51RB2_ISP_ERROR;
    }

    return ret;
}

static en_at89c51rb2_isp_error_msg at89c51rb2_create_frame_header_and_write(const uint8_t *buffer,
                                         uint8_t size,
                                         uint16_t address)
{
    const uint8_t record_mark = ':';
    uint8_t address_8bits[2] = { ((uint8_t)(address >> 8)), (uint8_t)(address) };
    uint8_t reclen = size - 1; // We don't count the command
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < size; i++)
    {
        checksum += buffer[i];
    }
    checksum += reclen;
    checksum += address_8bits[0];
    checksum += address_8bits[1];
    checksum = 256 - checksum;

    const uint16_t MAX_FRAME_SIZE = 256;
    char bytes_with_frame[MAX_FRAME_SIZE];

    uint8_t offset = 0;
    bytes_with_frame[offset++] = record_mark;
    byte_to_ascii(reclen, &bytes_with_frame[offset]); offset += 2;
    byte_to_ascii(address_8bits[0], &bytes_with_frame[offset]); offset += 2;
    byte_to_ascii(address_8bits[1], &bytes_with_frame[offset]); offset += 2;

    for (uint8_t i = 0; i < size; i++)
    {
        byte_to_ascii(buffer[i], &bytes_with_frame[offset]);
        offset += 2;
    }

    byte_to_ascii(checksum, &bytes_with_frame[offset]); offset += 2;

    Serial.println("PRINT:");
    for (int i = 0; i < offset; i++)
    {
        Serial.print(bytes_with_frame[i]);
        Serial.print(" ");
    }
    Serial.println("");


    return at89c51rb2_write_and_check((uint8_t*)bytes_with_frame, offset);
}

static void delay_without_cpu_stop(unsigned long ms)
{
    const unsigned long start = micros();
    while (micros() - start < ms);
}

/* Bootloader control */
en_at89c51rb2_isp_error_msg at89c51rb2_enter_bootloader(void)
{
    delay_without_cpu_stop(DELAY_BOOTTIME);

    /* Entering bootloader */
    pinMode(MCU_RST_PIN, OUTPUT);
    pinMode(MCU_PSEN_PIN, OUTPUT);
    
    digitalWrite(MCU_RST_PIN, HIGH);
    delay_without_cpu_stop(DELAY_FOR_STABILITY);
    digitalWrite(MCU_PSEN_PIN, LOW);
    delay_without_cpu_stop(DELAY_RESET);
    digitalWrite(MCU_RST_PIN, LOW);
    delay_without_cpu_stop(DELAY_FOR_STABILITY);
    digitalWrite(MCU_PSEN_PIN, HIGH);


    /* We set to input to increase impedance and let the MCU drive the pins */
    pinMode(MCU_PSEN_PIN, INPUT);

    delay_without_cpu_stop(DELAY_FOR_STABILITY);


    /* Initialize baudrate */
    const uint8_t data = 'U';

    return at89c51rb2_write_and_check(&data, 1);
}

/* 1 = User's application

   0 = Bootloader
*/
en_at89c51rb2_isp_error_msg at89c51rb2_finish_flash(void)
{
    en_at89c51rb2_isp_error_msg ret = AT89C51RB2_ISP_OK;
    const uint8_t data[] = {
        WRITE_FCT,
        WRITE_FUSE,
        WRITE_FUSE_BLJB,
        1
    };

    if (AT89C51RB2_ISP_OK != at89c51rb2_create_frame_header_and_write(data, sizeof(data), 0))
    {
        ret = AT89C51RB2_ISP_ERROR;
    }

    const uint8_t data2[] = {
        WRITE_FCT,
        WRITE_BSB_SBV,
        WRITE_BSB,
        0
    };

    if (AT89C51RB2_ISP_OK != at89c51rb2_create_frame_header_and_write(data2, sizeof(data2), 0))
    {
        ret = AT89C51RB2_ISP_ERROR;
    }

    mcu_serial_empty_buffer();
    return ret;
}

/* Erase */
en_at89c51rb2_isp_error_msg at89c51rb2_erase_block(uint8_t block)
{
    const uint8_t data[] = { WRITE_FCT, ERASE, block };
    en_at89c51rb2_isp_error_msg ret = at89c51rb2_create_frame_header_and_write(data, sizeof(data), 0);
    mcu_serial_empty_buffer();
    return ret;
}
en_at89c51rb2_isp_error_msg at89c51rb2_full_chip_erase(void)
{
    const uint8_t data[] = { WRITE_FCT, FULL_CHIP_ERASE };
    en_at89c51rb2_isp_error_msg ret = at89c51rb2_create_frame_header_and_write(data, sizeof(data), 0);
    mcu_serial_empty_buffer();
    return ret;
}

/* Programming */
en_at89c51rb2_isp_error_msg at89c51rb2_write_program_data_chunk(const uint8_t *buffer,
                           uint8_t size,
                           uint16_t address)
{
    en_at89c51rb2_isp_error_msg ret = AT89C51RB2_ISP_OK;
    const uint8_t MAX_PAGE_SIZE = 128;
    const uint8_t *data = buffer;

    /* Check if the data will cross a page boundary */
    const uint16_t end_position = address + size;
    const uint16_t page_pos     = address / MAX_PAGE_SIZE + 1;
    if (address < (MAX_PAGE_SIZE * page_pos) && end_position > (MAX_PAGE_SIZE * page_pos)) // Cross a page
    {
        /* Split the data in 2 chunks */
        const uint16_t size_until_page = MAX_PAGE_SIZE * page_pos - address;

        /* Left part */
        /* size * 2 because 1 data is 2 ascii */
        if (AT89C51RB2_ISP_OK != at89c51rb2_write_program_data_chunk(data, size_until_page * 2, address))
        {
            ret = AT89C51RB2_ISP_ERROR;
        } 
        /* New address */
        address = MAX_PAGE_SIZE * page_pos;
        size    = size - size_until_page;
        data    = &buffer[size_until_page]; // Increase the pointer pos 
        
        /* Right part */
        if (AT89C51RB2_ISP_OK != at89c51rb2_write_program_data_chunk(data, size * 2, address))
        {
            ret = AT89C51RB2_ISP_ERROR;
        }
    }
    else 
    {
        uint8_t data_processed[MAX_PAGE_SIZE];
        data_processed[0] = PROGRAM_DATA_FCT;

        if (MAX_PAGE_SIZE > size)
        {
            uint16_t j = 0;
            uint8_t new_size = 0;
            for (uint16_t i = 0; i < size; i += 2)
            {
                data_processed[j + 1] = ascii_to_byte((const char*)&data[i]);
                new_size = j + 2;
                j++;
            }

            if (AT89C51RB2_ISP_OK != at89c51rb2_create_frame_header_and_write(data_processed, new_size, address)) // + 1 because there is PROGRAM_DATA_FCT now
            {
                ret = AT89C51RB2_ISP_ERROR;
            }
        }
        else 
        {
            ret = AT89C51RB2_ISP_ERROR;
        }
    }

    return ret; 
}

en_at89c51rb2_isp_error_msg at89c51rb2_write_program_data(const uint8_t *buffer, uint8_t size)
{
    en_at89c51rb2_isp_error_msg ret = AT89C51RB2_ISP_OK;
    if (0 == size || NULL == buffer)
    {
        return AT89C51RB2_ISP_ERROR;
    }

    uint16_t offset = 0;
    while (size > offset)
    {
        if (':' != buffer[offset + 0])
        {
            Serial.print("Break\n");
            break;
        }

        uint8_t byte_count = ascii_to_byte((const char*)&buffer[offset + 1]);
        uint8_t address_msb = ascii_to_byte((const char*)&buffer[offset + 3]);
        uint8_t address_lsb = ascii_to_byte((const char*)&buffer[offset + 5]);
        uint16_t address = (((uint16_t)address_msb << 8) | (uint16_t)address_lsb);

        const uint8_t *data = &buffer[offset + 9];
        /* byte_count * 2 because one data is 2 ascii */
        if (AT89C51RB2_ISP_OK != at89c51rb2_write_program_data_chunk(data, byte_count * 2, address))
        {
            Serial.print("Write data chunk fail\n");
            ret = AT89C51RB2_ISP_ERROR;
            break;
        }

        offset += 9 + byte_count * 2 + 2; // header + data + checksum
    }

    return ret;
}

/* Reading */
en_at89c51rb2_isp_error_msg at89c51rb2_read_data(uint8_t *buffer, uint8_t size)
{
    en_at89c51rb2_isp_error_msg ret = AT89C51RB2_ISP_OK;

    uint8_t read_bytes = 0;
    en_mcu_serial_error_msg err = mcu_serial_read(buffer, size, &read_bytes);
    if (MCU_SERIAL_OK != err)
    {
        ret = AT89C51RB2_ISP_ERROR;
    }
    
    buffer[read_bytes] = '\0';

    mcu_serial_empty_buffer();
    return ret;
}
en_at89c51rb2_isp_error_msg at89c51rb2_read_id(uint8_t buffer[2])
{
    en_at89c51rb2_isp_error_msg ret = AT89C51RB2_ISP_OK;
    const uint8_t data[] = { READ_FCT, READ_IDs, READ_MANUFACTURER_ID };
    if (AT89C51RB2_ISP_OK != at89c51rb2_create_frame_header_and_write(data, sizeof(data), 0))
    {
        ret = AT89C51RB2_ISP_ERROR;
    }

    if (AT89C51RB2_ISP_OK == ret)
    {
        if (AT89C51RB2_ISP_OK != at89c51rb2_read_data(buffer, 2))
        {
            ret = AT89C51RB2_ISP_ERROR;
        }
    }

    mcu_serial_empty_buffer();
    return ret;
}
en_at89c51rb2_isp_error_msg at89c51rb2_read_ssb(uint8_t buffer[2])
{
    en_at89c51rb2_isp_error_msg ret = AT89C51RB2_ISP_OK;
    const byte data[] = { READ_FCT, READ_SECURITY_BYTE, READ_SSB };
    if (AT89C51RB2_ISP_OK != at89c51rb2_create_frame_header_and_write(data, sizeof(data), 0))
    {
        ret = AT89C51RB2_ISP_ERROR;
    }

    if (AT89C51RB2_ISP_OK == ret)
    {
        if (AT89C51RB2_ISP_OK != at89c51rb2_read_data(buffer, 2))
        {
            ret = AT89C51RB2_ISP_ERROR;
        }
    }
    mcu_serial_empty_buffer();
    return ret;
}
en_at89c51rb2_isp_error_msg at89c51rb2_read_hardware_bytes(uint8_t buffer[4])
{
    en_at89c51rb2_isp_error_msg ret = AT89C51RB2_ISP_OK;
    const uint8_t data_hardware_byte[] = {
        READ_FCT,
        READ_HARDWARE_BYTE1,
        READ_HARDWARE_BYTE2
    };
    if (AT89C51RB2_ISP_OK != at89c51rb2_create_frame_header_and_write(data_hardware_byte, sizeof(data_hardware_byte), 0))
    {
        ret = AT89C51RB2_ISP_ERROR;
    }

    const uint8_t data_bsb_byte[] = {
        READ_FCT,
        READ_SECURITY_BYTE,
        READ_BSB
    };
    if (AT89C51RB2_ISP_OK != at89c51rb2_create_frame_header_and_write(data_bsb_byte, sizeof(data_bsb_byte), 0))
    {
        ret = AT89C51RB2_ISP_ERROR;
    }

    if (AT89C51RB2_ISP_OK == ret)
    {
        if (AT89C51RB2_ISP_OK != at89c51rb2_read_data(buffer, 4))
        {
            ret = AT89C51RB2_ISP_ERROR;
        }
    }

    mcu_serial_empty_buffer();
    return ret;
}

en_at89c51rb2_isp_error_msg at89c51rb2_display_memory(const char start_address[4], const char end_address[4], uint8_t *i_buffer, uint8_t size_buffer)
{
    en_at89c51rb2_isp_error_msg ret = AT89C51RB2_ISP_OK;
    uint8_t start_address_byte[2] = { ascii_to_byte(&start_address[0]), ascii_to_byte(&start_address[2]) };
    uint8_t end_address_byte[2] = { ascii_to_byte(&end_address[0]), ascii_to_byte(&end_address[2]) };

    const uint8_t data[] = { 
      DISPLAY_FCT, 
      start_address_byte[0], start_address_byte[1], 
      end_address_byte[0], end_address_byte[1], 
      DISPLAY_DATA
    };

    if (AT89C51RB2_ISP_OK != at89c51rb2_create_frame_header_and_write(data, sizeof(data), 0))
    {
        Serial.println("Failed to write and check in display memory");
        ret = AT89C51RB2_ISP_ERROR;
    }

    if (AT89C51RB2_ISP_OK == ret)
    {
        if (AT89C51RB2_ISP_OK != at89c51rb2_read_data(i_buffer, size_buffer))
        {
            ret = AT89C51RB2_ISP_ERROR;
        }
    }

    mcu_serial_empty_buffer();

    return ret;
}
