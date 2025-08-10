# _Custom Datalogger_

## Overview
Custom Datalogger is an ESP32-based embedded system that uses a set of modular, custom drivers to read and display temperature and humidity data from a DHT11 sensor. Readings are displayed on both a web interface and an LCD screen with multiple display modes. The system includes IR and button controls, audio feedback, Wi-Fi connectivity, and status LEDs to give users real-time system feedback.

## Features
- **Sensor Data Collection:** Temperature and humidity readings using a DHT11 sensor  
- **LCD Display Modes:** Switch between temperature, humidity, and time since last read  
- **Control Options:** IR remote and physical button to switch display modes or trigger a reading  
- **Web Interface:** Hosts a simple web server displaying live data and allowing manual readings  
- **Speaker Feedback:** Plays a sound when a new reading is taken  
- **Status LED:**  
  - Green: Ready  
  - Yellow: Setup in progress  
  - Blinking Red: Critical error  
- **Time Management:** Internal timekeeping to track time since last read, adjustable via the `timeset` driver  
- **Auto & Manual Reads:** Automatically takes a reading every minute, or instantly on-demand via web or IR  

## Project Structure

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
### Special Files

- `speaker/audio_data_generator.py`: Converts `untitled.wav` to a header file for embedding audio  
- `webserver/index.html`, `style.css`, `script.js`: Embedded in the firmware for hosting the web UI  
- `audio_data.h`: Auto-generated from `.wav` to feed data to the speaker