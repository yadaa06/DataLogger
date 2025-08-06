idf_component_register(SRCS "irdecoder.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES dht11 speaker driver esp_timer lcd)