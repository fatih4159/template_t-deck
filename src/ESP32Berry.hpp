
#pragma once

#include "config/ESP32Berry_Config.hpp"


#include <vector>
#include "ui/ESP32Berry_Display.hpp"
#include "network/ESP32Berry_Network.hpp"
#include "system/ESP32Berry_System.hpp"
#include "apps/ChatGPT/ESP32Berry_AppChatGPT.hpp"

class ESP32Berry {
private:

public:
  Display *display;
  Network *network;
  System *system;
  AppChatGPT *appChatGPT;

  ESP32Berry();
  ~ESP32Berry();
  void begin();
};