# _Custom Datalogger_

## What does Custom DataLogger do?
This project uses a variety of custom drivers to take a temperature and humidity and display it to a website and an lcd.
The lcd has multiple modes and can be controlled via ir remote or button and the modes are temperature, humidity, and time since last read.
A new reading is taken every minute but can be done instantly through either the website or the ir remote. The speaker will play a sound when a reading is taken. 
An led also shows when the system is ready (yellow means in progress, green means ready, and blinking red means critical error).
The time of on the dht11 is also controlled via a timesetting driver

## Custom DataLogger contents

The project **datalogger** contains one source file in C language [main.c](main/main.c). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── components
    └── button
│       ├── CMakeLists.txt
│       ├── button.c
│       └── button.h
│   └── dht11
│       ├── CMakeLists.txt
│       ├── dht11.c
│       ├── dht11.h
│       ├── dht11_task.c
│       └── dht11_task.h
│   └── irdecoder
│       ├── CMakeLists.txt
│       ├── irdecoder.c
│       └── irdecoder.h
│   └── lcd
│       ├── CMakeLists.txt
│       ├── lcd_i2c.c
│       ├── lcd_i2c.h
│       ├── lcd_task.c
│       └── lcd_task.h
│   └── speaker
│       ├── CMakeLists.txt
│       ├── audio_data_generator.py
│       ├── audio_data.h
│       ├── speaker_driver.c
│       ├── speaker_driver.h
│       └── untitled.wav
│   └── statusled
│       ├── CMakeLists.txt
│       ├── statusled.c
│       └── statusled.h
│   └── timeset
│       ├── CMakeLists.txt
│       └── timeset.c
│       └── timeset.h
│   └── webserver
│       ├── CMakeLists.txt
│       ├── webserver.c
│       ├── webserver.h
│       ├── index.html
│       ├── style.css
│       └── script.js
│   └── wifi
│       ├── CMakeLists.txt
│       ├── wifi.c
│       └── wifi.h
├── main
│   ├── CMakeLists.txt
│   └── main.c
└── README.md                  This is the file you are currently reading
```
Additionally, the sample project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.
