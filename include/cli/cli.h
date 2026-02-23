#ifndef CLI_H__
#define CLI_H__

typedef enum 
{
    CLI_OK,
    CLI_ERROR,
} en_cli_error_msg;

void cli_init(void);

void cli_process(char input_id);

void cli_task(void);

#endif /* CLI_H__ */