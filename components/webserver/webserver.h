// webserver.h

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "esp_https_server.h"

httpd_handle_t start_webserver(void);

#endif