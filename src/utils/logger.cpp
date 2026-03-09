#include "utils/logger.h"
#include "drivers/computer_serial.h"

#include <stdio.h>
#include <stdarg.h>

en_loglevel current_loglevel = LOG_ERROR;

void logger_init(en_loglevel loglevel)
{
    computer_serial_init();

    current_loglevel = loglevel;
}


void log_error(const char* buffer, ...)
{
    if (LOG_ERROR <= current_loglevel)
    {
        va_list args;
        va_start(args, buffer);
        computer_serial_print("ERROR: ");
        computer_serial_print(buffer, args);
        va_end(args);
    }
}

void log_warning(const char* buffer, ...)
{
    if (LOG_WARNING <= current_loglevel)
    {
        va_list args;
        va_start(args, buffer);
        computer_serial_print("WARNING: ");
        computer_serial_print(buffer, args);
        va_end(args);
    }
}

void log_debug(const char* buffer, ...)
{
    if (LOG_DEBUG <= current_loglevel)
    {
        va_list args;
        va_start(args, buffer);
        computer_serial_print("DEBUG: ");
        computer_serial_print(buffer, args);
        va_end(args);
    }
}