

#include <Arduino.h>
#include "utilities.h"

// Set User Name For Display
#define USER_NAME "Eric" 

#define WIFI_SCAN_ITER 5
#define WIFI_SSID_PW_DELIMITER "^_"
#define WIFI_CONNECT_TIMEOUT 10 * 1000

// Set your Timezone
#define TIME_ZONE "PST8PDT,M3.2.0,M11.1.0" // https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h

#define SYSTEM_UPDATE_INTERVAL 60 * 1000

#define BAT_MAX_VOLT 4.2

// Set your OpenAPI Key for ChatGPT 
// https://youtu.be/IuvEtq73gyE 
#define OPENAI_API_KEY "<YOUR_OPENAPI_KEY>"

#define AUDIO_BOOT "/startup-sound.mp3" // Download the mp3 file from https://www.myinstants.com/en/instant/blackberry-startup-sound-67394/
