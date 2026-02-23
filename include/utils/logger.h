#ifndef LOGGER_H__
#define LOGGER_H__

typedef enum 
{
    LOG_ERROR,
    LOG_WARNING,
    LOG_DEBUG,
} en_loglevel;

void logger_init(en_loglevel loglevel);
void log_error(const char *buffer, ...);
void log_warning(const char* buffer, ...);
void log_debug(const char *buffer, ...);

#endif 