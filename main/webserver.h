// webserver.h

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "esp_https_server.h"

esp_err_t root_get_handler(httpd_req_t *req);
httpd_handle_t start_webserver(void);


#endif