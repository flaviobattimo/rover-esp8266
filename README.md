# R/C Mars Rover

![rendering](https://github.com/flaviobattimo/rover-esp8266/blob/master/docs/Rover_2021-Mar-12_07-08-51PM-000_CustomizedView8327134599aa.jpg?raw=true)


A simple replica of a Martian rover, focusing the design on the rocker-bogie suspension system and the shape of the wheels. 
It has no cosmetic add-ons, and is simple to print and mount. In addition to the molded components, it requires the electronics, screws, some square tubes and a wood plate. 
The rover is controlled by a web application. The source code is available on GitHub. Total cost should be below 300$

[R/C Mars Rover on Thingiverse](https://www.thingiverse.com/thing:4792133)

## Assembly guide

TBD

## Repositories

ESP8266 contains the web application:
[Rover ESP8266 code - this repository](https://github.com/flaviobattimo/rover-esp8266)

ATmega328p controls motors and servos:
[Rover atmega328p code](https://github.com/flaviobattimo/rover-328p)

Optional camera turret: [Rover esp32cam code](https://github.com/flaviobattimo/rover-turret)

## Firmware upload

All firmwares can be uploaded with [VS Code](https://code.visualstudio.com/) and [PlatformIO](https://platformio.org/).
To upload the firmware on the ESP32CAM board an additional UART programmer is required.

## Setup and first upload

Setup your working environment with VSCode and PlatformIO extension

Create a new folder

Clone all the repositories:

```bash
git clone https://github.com/flaviobattimo/rover-esp8266.git
git clone https://github.com/flaviobattimo/rover-328p.git
git clone https://github.com/flaviobattimo/rover-turret.git
```

We will first upload the rover-328p firmware. To upload the firmware to the ATMega328p configure the onboard switch appropriately:

DIP switch configuration

|    | 1  | 2  | 3  | 4  | 5  | 6  | 7  |
|  -  | - | - | - | - | - | - | - |
| **Mode 1**: ATmega328<->ESP8266  | ON | ON | OFF | OFF | OFF | OFF | OFF |
| **Mode 2**: USB <->ATmega328  | OFF | OFF | ON | ON | OFF | OFF | OFF |
| **Mode 3**: USB<->ESP8266 (Update firmware or sketch)  | OFF | OFF | OFF | OFF | ON | ON | ON |
| **Mode 4**: USB<->ESP8266 (communication)  | OFF | OFF | OFF | OFF | ON | ON | OFF |
| **Mode 5**: All independent  | OFF | OFF | OFF | OFF | OFF | OFF | OFF |

Switch the board configuration to **Mode 2** and connect the board to your PC with a usb cable. If you are programming the board when with all cables mounted remember to power up the rover with the 12V when connecting the board to the USB or errors may occur.

Open the rover-328p folder with VSCode, enter the command palette with <kbd>Ctrl</kbd>+<kbd>Alt</kbd>+<kbd>T</kbd> and run the **PlatformIO: upload** task



```bash
Processing uno (platform: atmelavr; board: uno; framework: arduino)
...
RAM:   [=======   ]  67.5% (used 1383 bytes from 2048 bytes)
Flash: [======    ]  56.1% (used 18110 bytes from 32256 bytes)
Configuring upload protocol...
AVAILABLE: arduino
CURRENT: upload_protocol = arduino
Looking for upload port...
Auto-detected: COM5
Uploading .pio\build\uno\firmware.hex

avrdude: AVR device initialized and ready to accept instructions

Reading | ################################################## | 100% 0.00s

avrdude: Device signature = 0x1e950f (probably m328p)
avrdude: reading input file ".pio\build\uno\firmware.hex"
avrdude: writing flash (18110 bytes):

Writing | ################################################## | 100% 3.16s

avrdude: 18110 bytes of flash written
avrdude: verifying flash memory against .pio\build\uno\firmware.hex:
avrdude: load data flash data from input file .pio\build\uno\firmware.hex:
avrdude: input file .pio\build\uno\firmware.hex contains 18110 bytes
avrdude: reading on-chip flash data:

Reading | ################################################## | 100% 2.55s

avrdude: verifying ...
avrdude: 18110 bytes of flash verified

avrdude: safemode: Fuses OK (E:00, H:00, L:00)

avrdude done.  Thank you.
```

Now, switch the board configuration to **Mode 3**

Press the reboot ESP button on the board

Open the rover-esp8266 folder with VSCode, enter the command palette with <kbd>Ctrl</kbd>+<kbd>Alt</kbd>+<kbd>T</kbd> and run the **PlatformIO: upload (esp07)** task

Once done run the **PlatformIO: upload Filesystem image (esp07)** task

Disconnect the USB cable, switch the board to **Mode 1** and connect the board again to the USB.

The ESP8266 on the board will try to connect to the default network without success. After 30 seconds a new network will be available:
    Name: Rover
    Password: R0v3rcmd

Connect to this network with your smartphone the open a web page to the default gateway address http://192.168.4.1

You now gained access to the web interface.
Click on **Other** -> **WiFi Settings** and set your local network settings. This allow you to control the Rover from your local WiFi network when under coverage



To upload the ESP32CAM firmware you need an adapter because ESP32-CAM doesn't have an USB port

You can follow this guide, but remember to upload the code in the rover-turret folder.
[ESP32CAM firmware upload](https://www.survivingwithandroid.com/esp32-cam-platformio-video-streaming-face-recognition/)


## Wirings

![wirings](https://github.com/flaviobattimo/rover-esp8266/blob/master/docs/Sketch_aa.png?raw=true)

**Traction motor colors are wrong, sorry**

## Traction Motor wiring:

|      |    Black | Red | White | Yellow | Blue |
| :-: | :-: | :-: | :-: | :-: | :-: |
|      |    GND | 12V | CW/CCW | Hall sensor | PWM |
| Front Left | PCA9685 GND | 12V | PIN D8 | PIN D2 | PCA9685 PWM CH8 |
| Middle Left | PCA9685 GND | 12V | PIN D9 | PIN D3 | PCA9685 PWM CH9 |
| Rear Left | PCA9685 GND | 12V | PIN D10 | PIN D4 | PCA9685 PWM CH10 |
| Front Right | PCA9685 GND | 12V | PIN D11 | PIN D5 | PCA9685 PWM CH11 |
| Middle Right | PCA9685 GND | 12V | PIN D12 | PIN D6 | PCA9685 PWM CH12 |
| Rear Right | PCA9685 GND | 12V | PIN A3 | PIN D7 | PCA9685 PWM CH13 |

## Servo wiring:

|      |    Yellow | Red | Black |
| :-: | :-: | :-: | :-: |
|      |    PWM | 5V | GND |
| Front Left | PCA9685 CH0 | PCA9685 V+ | PCA9685 GND |
| Front Right | PCA9685 CH1 | PCA9685 V+ | PCA9685 GND |
| Rear Left | PCA9685 CH2 | PCA9685 V+ | PCA9685 GND |
| Rear Right | PCA9685 CH3 | PCA9685 V+ | PCA9685 GND |
| Turret Tilt | PCA9685 CH4 | PCA9685 V+ | PCA9685 GND |
| Turret Pan | PCA9685 CH5 | PCA9685 V+ | PCA9685 GND |


## Optional components

- The rover uses two PCA9685 cards, one to control the servomotors, the other to control the traction motors. The motivation is from the limitation of each single card to have the same frequency in common for each channel. Using a high frequency for the motors improves the behavior of the robot. The frequency of 50Hz, on the other hand, is mandatory for servomotors. This second board is not strictly necessary and could even be eliminated, although I do not recommend it.

- There are two DC-DC converters in the robot, one for the servomotors and one for powering the boards. Again only one converter could be used, but I do not recommend it

- The turret is not essential, as is the buzzer to play some tunes and the LED strip that I installed on the bottom of the rover.

- The current sensor module is also not very useful. The rover does not have excessive power consumption.


## Photos and videos

The rover at the beach

![rendering](https://github.com/flaviobattimo/rover-esp8266/blob/master/docs/20210206_142426.jpg?raw=true)

At night with bottom led strip on, still without camera turret

![rendering](https://github.com/flaviobattimo/rover-esp8266/blob/master/docs/20201030_174512a.jpg?raw=true)

UI interface

![ui](https://github.com/flaviobattimo/rover-esp8266/blob/master/docs/Screenshot_20210312-210327_Chrome.jpg?raw=true)


[![Video](https://img.youtube.com/vi/kT9wiiX7X_E/0.jpg)](http://www.youtube.com/watch?v=kT9wiiX7X_E "Rover al poetto")

## Bill of materials

[2x PCA9685 PWM drivers](https://www.aliexpress.com/item/33006092134.html)

[2x LM2596 DC-DC converter](https://www.aliexpress.com/item/32957064724.html)

[1x UNO + WiFi-R3-AT328-ESP8266-32MB-CH340G board](https://it.aliexpress.com/item/4001355258686.html)

[3x Wire Cable Connector Terminals - 1 lot of 10 pieces](https://www.aliexpress.com/item/32921676725.html)

[6x JGB37-3525 37mm - 200RPM - 12VOLT](https://www.aliexpress.com/item/4001265315402.html)

[4x MG996R Servo](https://www.aliexpress.com/item/4000536728030.html)

[4x 10x 25T Servo Arm Round Type Disc](https://www.aliexpress.com/item/1306265255.html)

[1x ESP32-CAM WiFi Module](https://www.aliexpress.com/item/32999991632.html)

[ACS712 5a DC current sensor](https://www.aliexpress.com/item/32830307620.html)

8x MR105-2RS Rubber Sealed Ball Bearing 5 x 10 x 4mm

2x SG90 Micro servo

12 volt battery

cables

20x - 20mm x 4mm bolts (and nuts)

2x - 50mm x 5mm bolts (6 security nuts)

Wood plate 400x280x10mm

10mm square tube (Length TBD)

## Related projects

* [JPL Open Source Rover Project](https://github.com/nasa-jpl/open-source-rover)
* [Sawppy Rover](https://hackaday.io/project/158208-sawppy-the-rover)
* [ESA ExoMy](https://github.com/esa-prl/ExoMy)
