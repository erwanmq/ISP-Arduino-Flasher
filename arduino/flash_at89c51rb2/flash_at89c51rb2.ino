#include "computer_serial.h"
#include "mcu_serial.h"
#include "at89c51rb2_isp.h"
#include "cli.h"


void setup ()
{
  computer_serial_init();
  computer_serial_print("Computer communication initialized\n");
  mcu_serial_init();
  computer_serial_print("MCU communication initialized\n");
  if (0 != at89c51rb2_enter_bootloader())
  {
    computer_serial_print("Failed to enter bootloader\n");
    delay(1000);
    exit(0);
  }
  computer_serial_print("Bootloader enabled\n");
  cli_init();
}

void loop ()
{
  cli_task();
}  
