idf_component_register(SRCS "lcd/lcd_task.c" "dht11/dht11_task.c" "webserver/webserver.c" "wifi/wifi.c" "lcd/lcd_i2c.c" "dht11/dht11.c" "timeset/timeset.c" "main.c"
                    INCLUDE_DIRS "." "dht11" "lcd" "webserver" "wifi" "timeset"
                    EMBED_FILES "webserver/index.html")