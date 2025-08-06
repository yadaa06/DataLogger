idf_component_register(SRCS "webserver.c"
                       INCLUDE_DIRS "." 
                       PRIV_REQUIRES "esp_https_server" "dht11" ""
                       EMBED_FILES "index.html" "style.css" "script.js")