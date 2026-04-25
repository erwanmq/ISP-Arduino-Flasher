#include "cli/cli.h"
#include "drivers/computer_serial.h"
#include "protocol/at89c51rb2_isp.h"

#include <stdio.h>

typedef struct 
{
    char ID;
    en_cli_error_msg (*command)(void);
    const char *description;
} cmd_parser_t;

en_cli_error_msg cli_read_manufacturer_id(void);
en_cli_error_msg cli_read_ssb(void);
en_cli_error_msg cli_full_chip_erase(void);
en_cli_error_msg cli_page_chip_erase(void);
en_cli_error_msg cli_program_data(void);
en_cli_error_msg cli_display_memory(void);
en_cli_error_msg cli_finish_flash(void);
en_cli_error_msg cli_read_hardware_bytes(void);
static const cmd_parser_t command_table[] = 
{
    //{'1', cli_read_manufacturer_id, "Read manufacturer ID"},
    //{'2', cli_read_ssb, "Read SSB"},
    {'3', cli_full_chip_erase, "Full Chip Erase"},
    {'4', cli_page_chip_erase, "Page Chip Erase"},
    {'5', cli_program_data, "Program data"},
    {'6', cli_display_memory, "Display memory data"},
    {'7', cli_finish_flash, "Finish flash"},
    //{'8', cli_read_hardware_bytes, "Read Hardware Bytes"},
};

#define CMD_COUNT sizeof(command_table) / sizeof(command_table[0])

void cli_init(void)
{
    computer_serial_print("Welcome to the AT89C51RB2 flash program\n");
    computer_serial_print("---------------------------------------\n");
    for (uint8_t i = 0; i < (uint8_t)CMD_COUNT; i++)
    {
        char line[64];
        snprintf(line, sizeof(line), "%c: %s\r\n",
                command_table[i].ID,
                command_table[i].description);
        computer_serial_print(line);
    }
    computer_serial_print(">");
}

void cli_task(void)
{
    while (1)
    {
        uint8_t c = 254;
        computer_serial_read_line(&c, 1);
        computer_serial_empty_buffer();
        if (254 != c)
        {
            cli_process(c);
            computer_serial_empty_buffer();
            cli_init();
        }
    }
}

void cli_process(char input_id)
{
    for (uint8_t i = 0; i < (uint8_t)CMD_COUNT; i++)
    {
        if (input_id == command_table[i].ID)
        {
            if (NULL != command_table[i].command)
            {
                char buffer[40];
                snprintf(buffer, sizeof(buffer), "Processing command with ID: %c...\r\n", command_table[i].ID);
                computer_serial_print(buffer);
                if (CLI_OK != command_table[i].command())
                {
                    computer_serial_print("Failed to process the command\n");
                }
                return;
            }
        }
    }

    char buffer[24];
    snprintf(buffer, sizeof(buffer), "Unknown command: %c\r\n", input_id);
    computer_serial_print(buffer);
}

en_cli_error_msg cli_read_manufacturer_id(void)
{
    uint8_t id[2];
    en_cli_error_msg ret = CLI_OK;
    if (AT89C51RB2_ISP_OK != at89c51rb2_read_id(id))
    {
        ret = CLI_ERROR;
    }

    if (CLI_OK == ret)
    {
        char buffer[14];
        snprintf(buffer, sizeof(buffer), "ID is: %c %c\r\n", id[0], id[1]);
        computer_serial_print(buffer);
    }
    else 
    {
        computer_serial_print("Failed to read MCU ID\n");
    }
    return ret;
}

en_cli_error_msg cli_read_ssb(void)
{
    uint8_t ssb[2];
    en_cli_error_msg ret = CLI_OK;
    
    if (AT89C51RB2_ISP_OK != at89c51rb2_read_ssb(ssb))
    {
        ret = CLI_ERROR;
    }

    if (CLI_OK == ret)
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "SSB value is: %c %c\r\n", ssb[0], ssb[1]);
        computer_serial_print(buffer);
    }
    else 
    {
        computer_serial_print("Failed to read MCU SSB\n");
    }
    return ret;
}

en_cli_error_msg cli_full_chip_erase(void)
{
    en_cli_error_msg ret = CLI_OK;
    if (AT89C51RB2_ISP_OK != at89c51rb2_full_chip_erase())
    {
        ret = CLI_ERROR;
    }

    if (CLI_OK == ret)
    {
        computer_serial_print("Chip full erased\n");
    }
    else 
    {
        computer_serial_print("Error during full erase\n");
    }
    return ret;
}

en_cli_error_msg cli_page_chip_erase(void)
{
    en_cli_error_msg ret = CLI_OK;
    computer_serial_print("Enter the page to erase: \n");
    uint8_t page;
    int size = computer_serial_read_line(&page, 1);
    computer_serial_empty_buffer();

    if (1 != size)
    {
        computer_serial_print("Fail to get the page number.\n");
        ret = CLI_ERROR;
    }

    if (AT89C51RB2_ISP_OK != at89c51rb2_erase_block(page))
    {
        computer_serial_print("Error erasing page %d.\n", page);
        ret = CLI_ERROR;
    }

    if (CLI_OK == ret)
    {
        computer_serial_print("Page %d erased.\n", page);
    }

    return ret;
}

en_cli_error_msg cli_program_data(void)
{
    en_cli_error_msg ret = CLI_OK;

    computer_serial_print("Enter your hex intel data in one row with the semicolon >");

    uint8_t program_data[64] = { 0 };
    int size = computer_serial_read_line(program_data, sizeof(program_data));
    computer_serial_empty_buffer();

    if (AT89C51RB2_ISP_OK != at89c51rb2_write_program_data(program_data, size))
    {
        ret = CLI_ERROR;
    }
    return ret;
}

en_cli_error_msg cli_display_memory(void)
{
    en_cli_error_msg ret = CLI_OK;

    char start_address[4];
    char end_address[4];
    int byte_read = 0;
    computer_serial_print("Enter the start address MSB>");
    computer_serial_empty_buffer();
    byte_read = computer_serial_read_line((uint8_t*)start_address, 2);
    computer_serial_empty_buffer();    
    
    computer_serial_print("Enter the start address LSB>");
    computer_serial_empty_buffer();
    byte_read = computer_serial_read_line((uint8_t*)&start_address[2], 2);
    computer_serial_empty_buffer();

    computer_serial_print("Enter the end address MSB>");
    computer_serial_empty_buffer();
    byte_read = computer_serial_read_line((uint8_t*)end_address, 2);
    computer_serial_empty_buffer();

    computer_serial_print("Enter the end address LSB>");
    computer_serial_empty_buffer();
    byte_read = computer_serial_read_line((uint8_t*)&end_address[2], 2);
    computer_serial_empty_buffer();

    delay(20);

    uint8_t buffer[128];
    memset(buffer, '0', 128);
    if (AT89C51RB2_ISP_OK != at89c51rb2_display_memory(start_address, end_address, buffer, sizeof(buffer)))
    {
        ret = CLI_ERROR;
    }

    if (CLI_OK == ret)
    {
        computer_serial_print("Memory is: %s\n", (const char*)buffer);
    }
    else 
    {
        computer_serial_print("Failed to fetch the memory\n");
    }
    return ret;
}
en_cli_error_msg cli_finish_flash(void)
{
    en_cli_error_msg ret = CLI_OK;
    
    if (AT89C51RB2_ISP_OK != at89c51rb2_finish_flash())
    {
        ret = CLI_ERROR;
    }

    if (CLI_OK == ret)
    {
        computer_serial_print("Flash finished, restarting MCU...\n");
        if (AT89C51RB2_ISP_OK == at89c51rb2_reset_and_run())
        {
            computer_serial_print("MCU restarted!\n");
        }
        else 
        {
            computer_serial_print("Failed to restart MCU...\n");
        }
    }
    else 
    {
        computer_serial_print("Finish flash error\n");
    }
    return ret;
}

en_cli_error_msg cli_read_hardware_bytes(void)
{
    en_cli_error_msg err = CLI_OK;
    computer_serial_print("Not implemented\n");
    return err;
}
