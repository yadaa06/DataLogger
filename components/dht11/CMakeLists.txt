idf_component_register(SRCS "dht11.c" "dht11_task.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES driver esp_timer speaker)