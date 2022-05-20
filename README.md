# poor-mans-smu
Part of my efforts to create a "poor man's SMU", ref https://poormanssmu.wordpress.com/2020/06/05/first-prototype/ in my blog https://poormanssmu.wordpress.com/

Code started out from https://github.com/hellange/easysmu (a working prototype for EasySMU and 5" capacitive touch display), but has changed significantly after that.

The code in this repo is currently only for a prototype. Code is messy and work-in-slow-progress...

UI is based on display and libraries I have worked with before. Refer to by blog entries in the 'display' category: https://weatherhelge.wordpress.com/category/display/, especially https://weatherhelge.wordpress.com/2018/02/06/5-800x480-capacitive-touch-lcd-with-built-in-ft811-graphics-controller/. Graphics library for the FT81x is based on an early version of https://github.com/jamesbowman/gd2-lib

DAC/ADC low level code is mainly based on official documentation from Analog Device.

SCPI "interpreter" is a copy of https://github.com/Vrekrer/Vrekrer_scpi_parser. 

Ethernet for Teensy 4.1 is a small layer on top of a copy of https://github.com/ssilverman/QNEthernet

From May 2020, using PlatformIO instead of ArduinoIDE/Teensyduino. Mainly because the project has grown. Should be possible to use ArduinoIDE by renaming the main.cpp to *.ino. All files are in the src/ directory. Building and testing is only performed using PlatformIO.


Note:
The refs/ directory contains stuff that is not relevant for the source code. Just to keep it somewhere.
