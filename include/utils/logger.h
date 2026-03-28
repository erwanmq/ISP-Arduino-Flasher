#ifndef LOGGER_H__
#define LOGGER_H__
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

static int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

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