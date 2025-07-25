# _Custom Datalogger_

## How to use datalogger 
Build using CMAKE and ninja

## Example folder contents

The project **datalogger** contains one source file in C language [main.c](main/main.c). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   └── main.c
│   └── button
│       └── button.c
│       └── button.h
│   └── dht11
│       └── dht11.c
│       └── dht11.h
│       └── dht11_task.c
│       └── dht11_task.h
│   └── irdecoder
│       └── irdecoder.c
│       └── irdecoder.h
│   └── lcd
│       └── lcd_i2c.c
│       └── lcd_i2c.h
│       └── lcd_task.c
│       └── lcd_task.h
│   └── statusled
│       └── statusled.c
│       └── statusled.h
│   └── timeset
│       └── timeset.c
│       └── timeset.h
│   └── webserver
│       └── webserver.c
│       └── webserver.h
│       └── index.html
│       └── style.css
│       └── script.js
│   └── wifi
│       └── wifi.c
│       └── wifi.h
└── README.md                  This is the file you are currently reading
```
Additionally, the sample project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.
