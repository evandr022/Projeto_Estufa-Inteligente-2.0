#ifndef HTTP_POST_H
#define HTTP_POST_H

#define WIFI_SSID       "Evandro 2G"      
#define WIFI_PASSWORD   "966A438d@2"     
#define HTTP_SERVER_IP  "192.168.1.8"      

#include <stdbool.h>

void http_init(void);

bool http_post_json(float temperatura, float umidade, const char* status);

#endif // HTTP_POST_H