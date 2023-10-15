
#pragma once

#include <lvgl.h>
#include <vector>
#include "../config/ESP32Berry_Config.hpp"
#include "../ui/ESP32Berry_Display.hpp"
#include "../network/ESP32Berry_Network.hpp"
#include "../system/ESP32Berry_System.hpp"

class AppBase {
private:
  void ui_app(const char *title);
  void ui_loading();
protected:
  lv_obj_t *_bodyScreen;
  lv_obj_t *ui_AppPanel;
  lv_obj_t *ui_AppTitle;
  lv_obj_t *ui_AppCloseBtn;
  lv_obj_t *ui_Loading;
  
public:
  AppBase(Display *display, System *system, Network *network, const char *title);
  ~AppBase();
  Display *_display;
  System *_system;
  Network *_network;

  void base_event_handler(lv_event_t *e);
  void show_loading_popup(bool isOn);
  virtual void close_app(){};
};