idf_component_register(SRCS "main.c" "webserver/webserver.c"
                    INCLUDE_DIRS "." "webserver"
                    EMBED_FILES "webserver/index.html" "webserver/style.css" "webserver/script.js")