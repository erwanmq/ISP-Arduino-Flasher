# Arduino ISP flash for Intel 8051 MCUs like

## Tree
The project is divided in three main parts. Two of them are just duplicated code to work with the Arduino IDE ./arduino and the platformio project. These two parts are used to be compiled and upload to an Arduino. The last part is the python script that will automatized the upload and verification of the IHX codes to run the Intel 8051 MCUs.

## How to use it
Use Arduino IDE with the folder ./arduino or platformio in the root of the project. Compile and upload the application like a standard Arduino project.

This step is crucial and depends of your MCU. The example provided below are for the AT89C51RB2 and is not tested for other MCUs. The goal will be to enter the bootloader of your MCU to start the init process. Check your MCU's datasheet to do this step. (A scheme for the AT89C51RB2 will soon be upload to this README.)
With the AT89C51RB2, the initial step is to send the character 'U' to the MCU and it should answer back with the same character (everything is handle by the arduino program). At this point, the MCU is ready to process commands!

When done, you can connect using UART to the Arduino (Serial Monitor with a baudrate of 9600) or using `pio device monitor` with platformio (the default options are the correct ones and are defined in the platformio.ini).
Here, you will have a little help with usable commands. You just have to enter the ID of the wanted command and the program will tell you what to do.

If you have play a little bit with it to see if everything is working correctly, you would want to automatize the upload/verification steps with a python script. The file "send_program.py" is here for you. Just enter `python3 send_program.py --help". 
The two mains commands are  `--program-data <ihx-file>` and `--verify-data <ihx-file>`.

When uploading your code to the Arduino directly or via the python script, I recommend you to use the sdcc-packihx command if your Arduino has not a lot RAM. 

## Example
A folder named `./examples` are here to help you test your Arduino flasher. There is currently one example for the AT89C51RB2. Run these commands:
- sdcc main.c -o build/
- packihx build/main.ihx > build/main_packed.ihx
Then in the root folder:
- python3 send_program.py --program-data examples/at89c51rb2/build/main_packed.ihx
- python3 send_program.py --verify-data examples/at89c51rb2/build/main_packed.ihx

